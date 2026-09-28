// cas.hpp -- études de cas : chaque cas prend des valeurs physiques (échelle de corde M_s, couplage g_s, masse,
// tension...) et CALCULE les prédictions. Rien n'est codé en dur, sauf les données expérimentales citées.
#pragma once
#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "calabi.hpp"
#include "corde.hpp"
#include "physique.hpp"
#include "spectre.hpp"

namespace cas {

using phys::nb;
using phys::sci;
const double PI = phys::PI;

struct Params {
    double Ms = 5000;          // échelle de corde M_s = 1/sqrt(alpha') en GeV
    double gs = 0.1;           // couplage de corde
    double R = 1.0;            // rayon de compactification en unités de sqrt(alpha')
    double x0 = 0.27;          // point de pincement (0..1)
    double Gmu = 1e-7;         // tension d'une corde cosmique
    double masse = phys::M_sun;  // masse d'un trou noir (kg)
    double lambda = 10;        // couplage de 't Hooft
    int n_extra = 2;           // nombre de dimensions supplémentaires (ADD)
};

struct Cas {
    std::string id, titre, resume;
    std::function<void(std::ostream&, const Params&)> executer;
};

inline void titre(std::ostream& o, const std::string& t) {
    o << "\n" << std::string(78, '=') << "\n  " << t << "\n" << std::string(78, '=') << "\n";
}
inline void lecon(std::ostream& o, const std::string& t) { o << "\n  >> " << t << "\n"; }
inline std::string pad(const std::string& s, size_t n) {  // largeur en caractères (UTF-8 compté par code point)
    size_t len = 0;
    for (unsigned char c : s) if ((c & 0xC0) != 0x80) ++len;
    return s + std::string(len < n ? n - len : 0, ' ');
}
inline std::string padg(const std::string& s, size_t n) {
    size_t len = 0;
    for (unsigned char c : s) if ((c & 0xC0) != 0x80) ++len;
    return std::string(len < n ? n - len : 0, ' ') + s;
}

// ------------------------------------------------------------------------------------------ 1. corde
inline void cas_corde(std::ostream& o, const Params& p) {
    titre(o, "CAS 1 - La corde pincée : modes, énergie, simulation numérique");
    corde::Corde1D C(400, 1.0);
    double h = 1.0;
    C.pincer(p.x0, h);
    double E0 = C.energie(), Ea = corde::energie_analytique_totale(p.x0, h);
    o << "  Corde de longueur L=1, extrémités fixes, pincée en x0 = " << p.x0 << " (flèche h = 1).\n";
    o << "  Énergie numérique initiale : " << nb(E0, 6) << "   énergie analytique h²/(2 x0 (1-x0)) : " << nb(Ea, 6) << "  (écart = discrétisation)\n\n";
    o << "   mode n | fréquence f_n | amplitude b_n | énergie (%) | barre\n";
    o << "   -------+---------------+---------------+-------------+---------------------------------\n";
    double somme = 0;
    for (int n = 1; n <= 12; ++n) {
        double e = corde::energie_mode(n, p.x0, h) / Ea * 100;
        somme += e;
        o << "   " << std::setw(6) << n << " | " << std::setw(13) << corde::frequence_mode(n) << " | " << std::setw(13)
          << std::setprecision(4) << corde::amplitude_mode(n, p.x0, h) << " | " << std::setw(10) << std::setprecision(4) << e
          << "% | " << corde::barre(e, 100, 30) << "\n";
    }
    o << "   (somme des 12 premiers modes : " << std::setprecision(5) << somme << "% de l'énergie totale)\n";
    double emax = 0;
    for (int k = 0; k < 800; ++k) {
        C.pas();
        emax = std::max(emax, std::fabs(C.energie() - E0) / E0);
    }
    o << "\n  Simulation : 800 pas de temps (une période = 2), dérive relative maximale de l'énergie = " << sci(emax, 2) << "\n";
    double err = 0;
    for (int i = 0; i <= C.N; i += 10) err = std::max(err, std::fabs(C.u[i] - corde::u_analytique(i * C.dx, C.t, p.x0, h)));
    o << "  Écart max simulation / série de Fourier à t = " << std::setprecision(4) << C.t << " : " << sci(err, 2) << "\n";
    o << "\n  Forme de la corde à cet instant :\n" << corde::ascii_courbe(C.u, 70, 13, 1.0);
    lecon(o, "Pincer en x0 = k/n annule le mode n : on choisit quelles « particules » on excite. Essayez --x0=0.5 !");
}

// ------------------------------------------------------------------------------------------ 2. spectre
inline void cas_spectre(std::ostream& o, const Params& p) {
    titre(o, "CAS 2 - Le spectre de particules d'une corde (M_s = " + nb(p.Ms) + " GeV)");
    double ap = phys::alpha_prime(p.Ms);
    o << "  alpha' = 1/M_s² = " << sci(ap) << " GeV⁻²  ;  longueur de corde l_s = sqrt(alpha') = "
      << phys::longueur(phys::GeVinv_en_m(1.0 / p.Ms)) << "\n";
    o << "  Tension T = M_s²/(2π) = " << sci(phys::tension_newton(p.Ms)) << " N   (cf. corde de guitare ~ 100 N)\n\n";
    o << "  CORDE BOSONIQUE OUVERTE (D=26, 24 directions transverses), alpha' m² = N - 1 :\n";
    auto d = spectre::degenerescences<spectre::i128>(24, 8);
    o << "   N | alpha' m² | masse (GeV)  | états d(N)   | commentaire\n";
    const char* comm[] = {"tachyon (instable)", "24 états = vecteur sans masse (photon)", "324 = tenseur symétrique de SO(25)", "", "", "", "", "", ""};
    for (int N = 0; N <= 6; ++N) {
        double m2 = spectre::m2_ouverte(N);
        double m = m2 >= 0 ? std::sqrt(m2) * p.Ms : -std::sqrt(-m2) * p.Ms;
        o << "   " << N << " | " << std::setw(9) << m2 << " | " << std::setw(12) << (m2 < 0 ? "i" + nb(-m, 4) : nb(m, 4)) << " | "
          << std::setw(12) << spectre::str(d[N]) << " | " << comm[N] << "\n";
    }
    o << "\n  CORDE FERMÉE (N = Ñ, appariement de niveaux) : degénérescence d(N)² :\n";
    o << "   N = 1 : " << spectre::str(d[1] * d[1]) << " états = 299 (graviton) + 276 (champ B) + 1 (dilaton)  [somme = "
      << 299 + 276 + 1 << "]\n";
    o << "   N = 2 : " << spectre::str(d[2] * d[2]) << " états à m = " << nb(std::sqrt(4.0) * p.Ms, 4) << " GeV\n\n";
    auto s = spectre::supercorde(6);
    o << "  SUPERCORDE (D=10), spectre calculé par la fonction de partition + projection GSO :\n";
    o << "   niveau n | alpha' m² | masse (GeV) | secteur NS | secteur R | supersymétrie NS = R ?\n";
    for (int n = 0; n <= 6; ++n)
        o << "   " << std::setw(8) << n << " | " << std::setw(9) << n << " | " << std::setw(11) << nb(std::sqrt((double)n) * p.Ms, 4)
          << " | " << std::setw(10) << spectre::str(s.ns[n]) << " | " << std::setw(9) << spectre::str(s.r[n]) << " | "
          << (s.ns[n] == s.r[n] ? "oui" : "NON") << "\n";
    lecon(o, "Bosons (NS) et fermions (R) sont en nombre égal à chaque niveau : c'est la supersymétrie, et elle sort du calcul.");
    lecon(o, "Le niveau massif n=1 de la supercorde a 128+128 états = un supermultiplet de spin 2 à la masse M_s.");
}

// ------------------------------------------------------------------------------------------ 3. dimension
inline void cas_dimension(std::ostream& o, const Params&) {
    titre(o, "CAS 3 - Pourquoi 26 dimensions ? Et 10 pour la supercorde ?");
    o << "  Chacun des D-2 oscillateurs transverses ajoute ½Σn = ½·ζ(-1) = -1/24 à l'énergie du vide (effet Casimir),\n"
         "  d'où l'ordonnée à l'origine a = (D-2)/24 : c'est la régularisation de 1+2+3+... qui la fixe.\n\n";
    o << "  Régularisation numérique : S(ε) = Σ n e^{-εn} - 1/ε²\n";
    for (double e : {1.0, 0.3, 0.1, 0.03, 0.01, 0.001})
        o << "     ε = " << std::setw(6) << e << "  ->  " << std::setw(12) << std::setprecision(8) << spectre::somme_regularisee(e)
          << "   (limite : -1/12 = " << -1.0 / 12 << ")\n";
    o << "\n  L'état d'énergie 1 doit être un vecteur SANS masse (invariance de Lorentz) : il faut a = 1.\n";
    o << "     D  | a(D)=(D-2)/24 | masse² du niveau 1 (α'm² = 1-a) | verdict\n";
    for (int D : {4, 10, 24, 25, 26, 27, 30}) {
        double a = spectre::intercept_bosonique(D);
        o << "    " << std::setw(3) << D << " | " << std::setw(13) << a << " | " << std::setw(30) << 1 - a << "  | "
          << (std::fabs(a - 1) < 1e-12 ? "COHÉRENT : D = 26" : (a > 1 ? "niveau 1 tachyonique : instable" : "niveau 1 massif : il lui manque une polarisation (SO(D-1) ≠ SO(D-2))")) << "\n";
    }
    o << "\n  Supercorde : fermions NS -> a = (D-2)/16, il faut a = 1/2 :\n";
    for (int D : {4, 9, 10, 11})
        o << "    D = " << std::setw(2) << D << "  a = " << std::setw(6) << spectre::intercept_NS(D) << (std::fabs(spectre::intercept_NS(D) - 0.5) < 1e-12 ? "   <== D = 10 !" : "") << "\n";
    o << "\n  Le programme retrouve seul : D_bosonique = " << spectre::dimension_critique_bosonique()
      << ", D_supercorde = " << spectre::dimension_critique_super() << "\n";
    lecon(o, "D = 10 (ou 11 en théorie M) n'est pas un choix : c'est une condition de cohérence quantique. Restent 6 dimensions à cacher.");
}

// ------------------------------------------------------------------------------------------ 4. Regge
inline void cas_regge(std::ostream& o, const Params&) {
    titre(o, "CAS 4 - Trajectoire de Regge : les hadrons sont des cordes tournantes");
    o << "  Données (masses PDG arrondies) : mésons ρ, a2, ρ3, a4, ρ5 de spin J = 1..5\n\n";
    std::vector<double> m2, J;
    o << "   particule | J | masse (GeV) | m² (GeV²)\n";
    for (auto& h : spectre::hadrons_rho()) {
        o << "   " << pad(h.nom, 9) << " | " << h.J << " | " << std::setw(11) << h.m_GeV << " | " << std::setw(8) << h.m_GeV * h.m_GeV << "\n";
        m2.push_back(h.m_GeV * h.m_GeV); J.push_back(h.J);
    }
    auto r = spectre::regression(m2, J);
    o << "\n  Régression J = α₀ + α' m²  :  α' = " << nb(r.pente, 4) << " GeV⁻²,  α₀ = " << nb(r.ordonnee, 3) << ",  R² = " << nb(r.r2, 5) << "\n";
    double T = phys::tension_GeV2(r.pente);
    o << "  Tension de la corde de QCD : T = 1/(2πα') = " << nb(T, 3) << " GeV² = " << nb(T / (phys::hbarc_GeVm * 1e15), 3)
      << " GeV/fm  (~ 1 GeV par fermi : le confinement des quarks)\n";
    o << "  Tension en newtons : " << sci(T * phys::GeV / phys::hbarc_GeVm, 2) << " N  (≈ le poids de " << nb(T * phys::GeV / phys::hbarc_GeVm / 9.81 / 1000, 3) << " tonnes !)\n\n";
    double val = spectre::regge_corde_tournante(r.pente);
    o << "  Corde tournante classique (ouverte, extrémités à la vitesse c) : J/E² calculé par intégration numérique = " << nb(val, 6)
      << "  (attendu α' = " << nb(r.pente, 6) << ")\n";
    lecon(o, "Une corde relativiste qui tourne obéit exactement à J = α' E² : la droite de Regge. C'est l'origine historique de la théorie.");
}

// ------------------------------------------------------------------------------------------ 5. Hagedorn
inline void cas_hagedorn(std::ostream& o, const Params& p) {
    titre(o, "CAS 5 - Température de Hagedorn : la chaleur maximale de l'univers ? (M_s = " + nb(p.Ms) + " GeV)");
    auto h = spectre::hagedorn_fermee(3000);
    o << "  Entropie S = ln(nombre d'états) d'une corde fermée de masse m : croissance LINÉAIRE en m.\n";
    o << "     N   | m√α'    | S = 2 ln d(N) | pente locale dS/dm | (limite 4π = " << nb(spectre::BETA_H, 5) << ")\n";
    for (int N : {50, 100, 300, 1000, 3000}) {
        size_t i = N - 2, i0 = (size_t)(N * 0.9) - 2;
        o << "   " << std::setw(5) << N << " | " << std::setw(7) << nb(h.m[i], 4) << " | " << std::setw(13) << nb(h.S[i], 5) << " | "
          << std::setw(18) << nb(spectre::pente_hagedorn(h, i0, i), 5) << "\n";
    }
    double TH = p.Ms / spectre::BETA_H;
    o << "\n  T_Hagedorn = M_s / (4π) = " << nb(TH, 4) << " GeV = " << sci(TH * phys::GeV_en_K, 3) << " K\n";
    o << "  (pour comparaison : centre du Soleil 1.6e7 K, transition quarks-hadrons de QCD ~ 1.7e12 K)\n";
    lecon(o, "La pente 4π émerge lentement (corrections en 1/√N) : le comptage exact des états prédit la température limite.");
}

// ------------------------------------------------------------------------------------------ 6. cercle
inline void cas_cercle(std::ostream& o, const Params& p) {
    titre(o, "CAS 6 - T-dualité : corde sur un cercle de rayon R = " + nb(p.R) + " √α'");
    struct E { int n, w, N, Nt; const char* nom; };
    E etats[] = {{1, 0, 1, 1, "impulsion 1 (Kaluza-Klein)"}, {0, 1, 1, 1, "enroulement 1"}, {2, 0, 1, 1, "impulsion 2"},
                 {1, 1, 1, 0, "impulsion 1 + enroulement 1"}, {1, -1, 0, 1, "impulsion 1, enroulement -1"}};
    o << "  m²α' = n²/R² + w²R² + 2(N+Ñ-2), avec appariement N - Ñ = n·w\n\n";
    o << "   état                          | m(R)/M_s  | m(α'/R)/M_s | appariement\n";
    for (auto& e : etats) {
        double m2 = spectre::m2_cercle(e.n, e.w, e.N, e.Nt, p.R);
        double m2d = spectre::m2_cercle(e.w, e.n, e.N, e.Nt, spectre::dual(p.R));  // échange n<->w, R->1/R
        o << "   " << pad(e.nom, 29) << " | " << std::setw(9) << (m2 >= 0 ? nb(std::sqrt(m2), 5) : "tach.") << " | " << std::setw(11)
          << (m2d >= 0 ? nb(std::sqrt(m2d), 5) : "tach.") << "   | " << (spectre::niveaux_appaires(e.n, e.w, e.N, e.Nt) ? "OK" : "violé") << "\n";
    }
    o << "\n  Les deux colonnes coïncident : le spectre à R est IDENTIQUE à celui à α'/R (n ↔ w).\n";
    double Rc = 1.0;
    o << "  Au rayon auto-dual R = √α' l'état (1,1) a m = |1/R - R| = " << nb(std::fabs(1.0 / Rc - Rc), 3) << " : de nouveaux bosons de jauge sans masse -> SU(2)xSU(2).\n";
    lecon(o, "Il n'existe pas de distance plus petite que √α' : la géométrie classique s'effondre en dessous de la longueur de corde.");
}

// ------------------------------------------------------------------------------------------ 7. calabi
inline void cas_calabi(std::ostream& o, const Params&) {
    titre(o, "CAS 7 - Calabi-Yau : la géométrie des dimensions cachées calculée par les classes de Chern");
    o << "  Pour chaque variété (intersection complète dans un produit d'espaces projectifs), le programme calcule\n"
         "  c(X) = ∏(1+H_i)^(n_i+1) / ∏(1+Σ q_ji H_i) puis χ = ∫ c₃.\n\n";
    o << "   variété                                | dim | c₁=0 | χ calculé | 2(h11-h21) | générations |χ|/2 | H³ (triples)\n";
    bool tout_ok = true;
    for (auto& X : calabi::galerie()) {
        auto r = calabi::analyser(X);
        bool ok = r.euler == 2 * (X.h11 - X.h21);
        tout_ok = tout_ok && ok && r.c1_nul;
        o << "   " << pad(X.nom, 38) << " | " << std::setw(3) << r.dimension << " | " << (r.c1_nul ? " oui" : " NON") << "  | " << std::setw(9) << r.euler
          << " | " << std::setw(10) << 2 * (X.h11 - X.h21) << " | " << std::setw(15) << std::abs(r.euler) / 2 << "     | " << r.kappa[0] << (ok ? "" : "  ERREUR") << "\n";
    }
    o << "\n  Contrôle croisé avec les nombres de Hodge de la littérature : " << (tout_ok ? "TOUS COHÉRENTS" : "INCOHÉRENCE") << "\n\n";
    o << "  Diamant de Hodge de la quintique (h11=1, h21=101) :\n"
         "                1\n"
         "              0   0\n"
         "            0   1   0\n"
         "          1  101 101  1\n"
         "            0   1   0\n"
         "              0   0\n"
         "                1\n";
    o << "  Symétrie miroir : la quintique miroir a (h11, h21) = (101, 1) : mêmes physique, géométries échangées.\n";
    lecon(o, "Le monde réel a 3 familles : il faut |h11-h21| = 3. Le quotient Tian-Yau/Z3 (6,9) y parvient. Jouez avec « jeu universe ».");
}

// ------------------------------------------------------------------------------------------ 8. ADD
inline void cas_dimensions(std::ostream& o, const Params& p) {
    titre(o, "CAS 8 - Grandes dimensions supplémentaires : à quelle taille ? (M* = " + nb(p.Ms / 1000) + " TeV)");
    o << "  M_Pl² ≈ M*^(n+2) R^n  (à des facteurs 2π près) : la gravité serait « faible » parce qu'elle se dilue dans n dimensions.\n";
    o << "  M_Planck = " << sci(phys::M_planck_GeV) << " GeV ;  l_Planck = " << sci(phys::l_planck) << " m\n\n";
    o << "   n | rayon R                | 1ère excitation KK 1/R  | statut\n";
    for (int n = 1; n <= 7; ++n) {
        double R = phys::rayon_ADD_m(n, p.Ms);
        double KK_GeV = phys::hbarc_GeVm / R;
        std::string st = R > 3e-5 ? "EXCLU (loi de Newton vérifiée jusqu'à ~30 µm)"
                         : R > 1e-12 ? "contraint par l'astrophysique (supernovae, étoiles à neutrons)"
                         : R > 1e-16 ? "sondable par les collisionneurs (tours de Kaluza-Klein)" : "trop petit pour être détecté directement";
        if (n == 1 && R > 1e9) st = "EXCLU (orbites planétaires)";
        o << "   " << n << " | " << pad(phys::longueur(R), 22) << " | " << std::setw(15) << sci(KK_GeV, 2) << " GeV  | " << st << "\n";
    }
    lecon(o, "Avec n = " + std::to_string(p.n_extra) + " et M* = " + nb(p.Ms / 1000) + " TeV : R = " + phys::longueur(phys::rayon_ADD_m(p.n_extra, p.Ms)) + ".");
    if (p.Ms <= phys::LHC_GeV)
        o << "  Le LHC (13,6 TeV) pourrait produire des gravitons KK à cette échelle.\n";
}

// ------------------------------------------------------------------------------------------ 9. trous noirs
inline void cas_trous_noirs(std::ostream& o, const Params& p) {
    titre(o, "CAS 9 - Trous noirs, Hawking, et la borne des cordes");
    o << "   objet                     | masse (kg)  | R_s          | T Hawking (K)| S/k_B     | évaporation\n";
    struct O { const char* nom; double m; };
    for (O ob : {O{"Terre", phys::M_terre}, O{"Soleil", phys::M_sun}, O{"Sgr A* (4.3e6 Msol)", 4.3e6 * phys::M_sun},
                 O{"M87* (6.5e9 Msol)", 6.5e9 * phys::M_sun}, O{"trou primordial 1e12 kg", 1e12}, O{"votre masse (--masse)", p.masse}})
        o << "   " << pad(ob.nom, 25) << " | " << std::setw(11) << sci(ob.m, 2) << " | " << pad(phys::longueur(phys::rayon_schwarzschild(ob.m)), 12)
          << " | " << std::setw(12) << sci(phys::T_hawking(ob.m), 2) << " | " << std::setw(9) << sci(phys::S_bh_sur_k(ob.m), 2) << " | "
          << phys::duree(phys::t_evaporation(ob.m)) << "\n";
    double Ms = p.Ms, gs = p.gs;
    double mc = spectre::masse_correspondance_GeV(Ms, gs);
    o << "\n  Correspondance corde <-> trou noir (Horowitz-Polchinski), M_s = " << nb(Ms) << " GeV, g_s = " << gs << " :\n";
    o << "  en théorie des cordes la gravité est faible car G ~ g_s²/M_s² (4D naïf, sans volume interne).\n";
    o << "     m/M_s     | S corde = 4π m/M_s | S trou noir = 4π g_s² (m/M_s)² | état dominant\n";
    for (double x : {1.0, 1.0 / gs, 1.0 / (gs * gs), 10.0 / (gs * gs)}) {
        double Sc = 4 * PI * x, Sb = 4 * PI * gs * gs * x * x;
        o << "     " << std::setw(9) << nb(x, 4) << " | " << std::setw(18) << nb(Sc, 4) << " | " << std::setw(30) << nb(Sb, 4) << " | "
          << (Sc > Sb * 1.0000001 ? "corde" : Sc < Sb * 0.9999999 ? "trou noir" : "égalité (transition)") << "\n";
    }
    o << "  Transition à m = M_s/g_s² = " << sci(mc) << " GeV : en dessous, un objet est une corde excitée ; au-dessus, un trou noir.\n";
    lecon(o, "Strominger-Vafa (1996) ont compté les micro-états de certains trous noirs avec des cordes/branes et retrouvé S = A/4l_P².");
}

// ------------------------------------------------------------------------------------------ 10. cordes cosmiques
inline void cas_cosmique(std::ostream& o, const Params& p) {
    titre(o, "CAS 10 - Cordes cosmiques : Gμ = " + sci(p.Gmu, 2));
    double mu = p.Gmu * phys::c * phys::c / phys::G;
    double delta = phys::deficit_angle_rad(p.Gmu);
    double eta = std::sqrt(p.Gmu) * phys::M_planck_GeV;
    o << "  tension μ = Gμ c²/G = " << sci(mu) << " kg/m  (1 km ≈ " << nb(mu * 1e3 / phys::M_terre, 3) << " masse terrestre)\n";
    o << "  échelle d'énergie de la brisure de symétrie ~ √μ = " << sci(eta) << " GeV\n";
    o << "  angle de déficit δ = 8πGμ = " << sci(delta) << " rad = " << nb(delta * 180 / PI * 3600, 4) << " secondes d'arc\n";
    o << "  -> une galaxie derrière la corde apparaît en DEUX images séparées de ≈ " << nb(delta * 180 / PI * 3600, 4) << "″ (lentille gravitationnelle sans masse locale).\n";
    o << "  Les boucles de corde rayonnent des ondes gravitationnelles et disparaissent : τ ≈ ℓ/(Γ Gμ c), Γ ≈ 50.\n";
    double ell = 1e16;  // boucle de 1e16 m
    o << "  Durée de vie d'une boucle de " << phys::longueur(ell) << " : " << phys::duree(ell / (50 * p.Gmu * phys::c)) << "\n";
    lecon(o, "Les supercordes de la cosmologie peuvent être produites à la fin de l'inflation : les détecter prouverait qu'elles existent.");
}

// ------------------------------------------------------------------------------------------ 11. Casimir
inline void cas_casimir(std::ostream& o, const Params&) {
    titre(o, "CAS 11 - Effet Casimir : les fluctuations du vide se mesurent");
    o << "   distance d | pression de Casimir  | force sur 1 cm²   | équivalent\n";
    for (double d : {1e-8, 1e-7, 1e-6, 1e-5}) {
        double P = std::fabs(phys::pression_casimir(d));
        o << "   " << pad(phys::longueur(d), 10) << " | " << std::setw(15) << sci(P, 3) << " Pa | " << std::setw(12) << sci(P * 1e-4, 3)
          << " N | " << (P > 1e5 ? "≈ 1 atmosphère !" : P > 1 ? "≈ " + nb(P / 133.3, 3) + " mmHg" : "faible") << "\n";
    }
    o << "\n  C'est le même calcul que celui de la corde : Σ (n ω/2) régularisé = -π²ħc/(720 d³) par unité de surface (énergie).\n";
    lecon(o, "L'énergie de point zéro régularisée est réelle (mesurée depuis Lamoreaux 1997 à quelques % près).");
}

// ------------------------------------------------------------------------------------------ 12. AdS/CFT
inline void cas_ads(std::ostream& o, const Params& p) {
    titre(o, "CAS 12 - AdS/CFT : quand une corde calcule la QCD (λ = " + nb(p.lambda) + ")");
    o << "  Potentiel quark-antiquark dans N=4 super Yang-Mills (L = 1, unités arbitraires) :\n";
    o << "     Γ(1/4)⁴ = " << nb(phys::gamma_quart(), 6) << "   coefficient fort couplage 4π²/Γ(1/4)⁴ = " << nb(4 * PI * PI / phys::gamma_quart(), 5) << "\n\n";
    o << "     λ       | faible couplage λ/4π | fort couplage 0.2285√λ | plus fiable\n";
    std::vector<double> ls = {0.1, 1.0, 10.0, 100.0, 1000.0};
    if (std::find(ls.begin(), ls.end(), p.lambda) == ls.end()) ls.push_back(p.lambda);
    for (double l : ls) {
        double a = l / (4 * PI), b = -phys::potentiel_qqbar(l, 1.0);
        o << "     " << std::setw(7) << nb(l, 4) << " | " << std::setw(20) << nb(a, 5) << " | " << std::setw(22) << nb(b, 5) << " | " << (l < 1 ? "perturbation" : "corde (AdS)") << "\n";
    }
    o << "\n  Viscosité du plasma quarks-gluons : η/s = ħ/(4πk_B) = " << nb(phys::eta_sur_s, 5) << " ħ/k_B = " << sci(phys::eta_sur_s_SI(), 3) << " K·s\n";
    o << "  Le plasma de RHIC/LHC a η/s ≈ 1-3 × 1/(4π) : « le fluide le plus parfait connu », proche de la borne des cordes (Kovtun-Son-Starinets).\n";
    o << "  Rayon d'AdS : L⁴ = λ α'² (avec λ = g²N) → L/l_s = λ^(1/4) = " << nb(std::pow(p.lambda, 0.25), 4) << " : la corde voit un espace courbé de rayon " << nb(std::pow(p.lambda, 0.25), 3) << " l_s.\n";
    lecon(o, "À λ grand, la théorie de jauge (dure) est duale à une gravité/corde faiblement courbée (facile) : c'est la force de la dualité.");
}

// ------------------------------------------------------------------------------------------ 13. Veneziano
inline void cas_veneziano(std::ostream& o, const Params&) {
    titre(o, "CAS 13 - L'amplitude de Veneziano : dualité, pôles et résonances");
    o << "  A(s,t) = Γ(-α(s))Γ(-α(t))/Γ(-α(s)-α(t)),  α(x) = 0.5 + 0.9 x  (trajectoire du ρ)\n\n";
    o << "  Pôles en s : α(s) = J entier ⇒ s = (J - 0.5)/0.9 = m² des résonances J = 0,1,2,... :\n";
    for (int J = 0; J <= 5; ++J) {
        double s0 = (J - 0.5) / 0.9;
        if (s0 < 0) { o << "     J = " << J << "  :  s = " << nb(s0, 4) << " GeV² < 0 : pas de résonance physique (pôle sous le seuil)\n"; continue; }
        double a = spectre::veneziano(s0 - 1e-4, -1.0), b = spectre::veneziano(s0 + 1e-4, -1.0);
        o << "     J = " << J << "  :  m = " << nb(std::sqrt(s0), 4) << " GeV   A(s-ε) = " << nb(a, 3) << ", A(s+ε) = " << nb(b, 3) << "  (pôle simple : signe opposé)\n";
    }
    o << "\n  Symétrie de croisement A(s,t) = A(t,s) (dualité s-t) :\n";
    double maxd = 0;
    for (double s : {-2.0, -1.3, -0.7, -0.2, 0.3}) for (double t : {-1.9, -0.9, -0.3, 0.2}) {
        double a = spectre::veneziano(s, t), b = spectre::veneziano(t, s);
        if (std::isfinite(a) && std::isfinite(b)) maxd = std::max(maxd, std::fabs(a - b) / (std::fabs(a) + 1e-30));
    }
    o << "     écart relatif maximal sur 20 points : " << sci(maxd, 2) << "\n";
    o << "\n  À grand s (t fixé, hors des pôles) l'amplitude suit A ~ s^α(t) : c'est le « comportement de Regge » (figure 09).\n";
    lecon(o, "Veneziano cherchait la force forte, il a trouvé une corde : la théorie des cordes est née d'une formule d'Euler de 1730 (la fonction bêta).");
}

// ------------------------------------------------------------------------------------------ 14. Planck / échelles
inline void cas_planck(std::ostream& o, const Params& p) {
    titre(o, "CAS 14 - Échelles : de la guitare à la corde de Planck");
    o << "   corde                         | échelle M_s (GeV) | tension (N)   | ℓ_s\n";
    struct C { const char* nom; double ms; };
    for (C c : {C{"QCD (α' = 0,84 GeV⁻²)", 1.09}, C{"LHC-testable", p.Ms}, C{"unification (GUT)", 2e16}, C{"Planck", phys::M_planck_GeV}}) {
        o << "   " << pad(c.nom, 29) << " | " << std::setw(17) << sci(c.ms, 2) << " | " << std::setw(13) << sci(phys::tension_newton(c.ms), 2) << " | " << phys::longueur(phys::GeVinv_en_m(1 / c.ms)) << "\n";
    }
    o << "\n  Comparaison : corde de guitare ≈ 100 N ; câble d'ascenseur ≈ 1e5 N ; force de Planck c⁴/G = " << sci(std::pow(phys::c, 4) / phys::G, 3) << " N\n";
    o << "  Tension de la corde de Planck × 2π = " << sci(phys::tension_newton(phys::M_planck_GeV) * 2 * PI, 3) << " N = c⁴/G  (la force maximale de la relativité générale !)\n";
    lecon(o, "La corde de Planck porte la « force maximale » c⁴/(2πG) de la relativité générale : gravité et corde parlent de la même échelle.");
}

// ------------------------------------------------------------------------------------------ 15. LHC
inline void cas_lhc(std::ostream& o, const Params& p) {
    titre(o, "CAS 15 - Peut-on fabriquer des cordes au LHC ? (M_s = " + nb(p.Ms) + " GeV)");
    o << "  Résonances de Regge : m_n = M_s √n (supercorde ouverte), spin maximal J = n + 1.\n";
    o << "   n | masse (GeV) | spin max | atteignable à 13,6 TeV ?\n";
    int atteint = 0;
    for (int n = 1; n <= 12; ++n) {
        double m = p.Ms * std::sqrt((double)n);
        bool ok = m < phys::LHC_GeV;
        if (ok) ++atteint;
        o << "   " << std::setw(2) << n << " | " << std::setw(11) << nb(m, 5) << " | " << std::setw(8) << n + 1 << " | " << (ok ? "oui (partons énergiques nécessaires)" : "non") << "\n";
    }
    o << "\n  Niveaux atteignables (énergie de collision totale) : " << atteint << " sur 12.\n";
    o << "  Aucune résonance de corde n'a été observée : M_s > ~ 8 TeV est la limite typique (dépend des modèles).\n";
    lecon(o, "Changez --Ms=… : à M_s = 1 TeV le LHC ferait défiler les résonances de Regge, au-delà de 13,6 TeV il n'en verrait aucune.");
}

// ------------------------------------------------------------------------------------------ 16. dualités
inline void cas_dualites(std::ostream& o, const Params& p) {
    titre(o, "CAS 16 - Dualités : g_s = " + nb(p.gs) + ", R = " + nb(p.R) + " √α'");
    double gs = p.gs, R = p.R;
    o << "  Description initiale : Type IIA, rayon R = " << nb(R) << " √α', couplage g_s = " << nb(gs) << "\n";
    o << "  - T-dualité :   ↔ Type IIB, rayon R' = α'/R = " << nb(1 / R) << " √α', couplage g_s' = g_s/R = " << nb(gs / R) << "\n";
    o << "  - Couplage fort (g_s > 1) : IIA ↔ M-théorie, rayon de la 11e dimension R₁₁ = g_s l_s = " << nb(gs) << " l_s.\n";
    o << "  - S-dualité de IIB :  g_s ↔ 1/g_s = " << nb(1 / gs) << "\n";
    o << "  - Hétérotique SO(32) à g_s ↔ Type I à 1/g_s = " << nb(1 / gs) << "\n\n";
    o << "  Régime de validité : g_s = " << nb(gs) << (gs < 1 ? " → perturbation OK (description en cordes)" : " → fort couplage : utilisez la description duale") << "\n";
    double gs_dual = gs / R;
    o << "  Après T-dualité, g_s' = " << nb(gs_dual) << (gs_dual > 1 ? " > 1 : utiliser plutôt la description initiale." : " < 1 : les deux descriptions sont utilisables.") << "\n";
    lecon(o, "Toutes les théories des cordes sont des régions d'un seul « paysage de paramètres » : c'est la théorie M.");
}

// ------------------------------------------------------------------------------------------ 17. atelier
inline void cas_atelier(std::ostream& o, const Params& p) {
    titre(o, "ATELIER - Votre univers de cordes personnel (M_s = " + nb(p.Ms) + " GeV, g_s = " + nb(p.gs) + ")");
    double ls = phys::GeVinv_en_m(1 / p.Ms);
    o << "  Longueur de corde       l_s = " << sci(ls, 3) << " m\n";
    o << "  Pente de Regge          α'  = " << sci(1 / (p.Ms * p.Ms), 3) << " GeV⁻²\n";
    o << "  Tension                 T   = " << sci(phys::tension_newton(p.Ms), 3) << " N\n";
    o << "  Température Hagedorn    T_H = " << nb(p.Ms / spectre::BETA_H, 4) << " GeV = " << sci(p.Ms / spectre::BETA_H * phys::GeV_en_K, 3) << " K\n";
    // M_Pl² ≈ M_s^8 V6 / g_s²  ->  (2π R M_s)^6 = M_Pl² g_s² / M_s²
    double Rms = std::pow(phys::M_planck_GeV * p.gs / p.Ms, 1.0 / 3.0) / (2 * PI);
    o << "  Pour retrouver M_Planck = " << sci(phys::M_planck_GeV) << " GeV avec 6 dimensions de rayon R égal :\n";
    o << "        M_Pl² ≈ M_s^8 (2πR)^6 / g_s²  ⇒  R = " << nb(Rms, 4) << " l_s = " << phys::longueur(Rms * ls) << "\n";
    o << "        1ère excitation KK : 1/R = " << sci(p.Ms / Rms, 3) << " GeV\n";
    if (Rms < 1) o << "        ATTENTION R < l_s : la T-dualité montre que la description est équivalente à R' = l_s²/R = " << nb(1 / Rms, 4) << " l_s.\n";
    o << "  Masse de la première résonance : " << nb(p.Ms, 4) << " GeV (× √1 pour la supercorde) ; masse de la n-ième : M_s√n\n";
    o << "  " << (p.Ms < phys::LHC_GeV ? "Cette échelle est accessible au LHC." : "Cette échelle est hors de portée du LHC (13,6 TeV).") << "\n";
    o << "  Rapport M_s / M_Pl = " << sci(p.Ms / phys::M_planck_GeV, 3) << "\n";
    lecon(o, "Essayez : --Ms=1e16 --gs=0.05 (échelle de grande unification) ou --Ms=1000 --gs=0.5 (échelle du TeV).");
}

inline const std::vector<Cas>& tous() {
    static const std::vector<Cas> v = {
        {"corde", "La corde pincée : modes et simulation", "vibrations, énergie par mode, simulation numérique", cas_corde},
        {"spectre", "Spectre de particules", "états bosoniques, GSO, supersymétrie", cas_spectre},
        {"dimension", "Pourquoi D = 26 / 10", "énergie de Casimir, régularisation zêta", cas_dimension},
        {"regge", "Trajectoire de Regge des hadrons", "données PDG, tension de corde, corde tournante", cas_regge},
        {"hagedorn", "Température de Hagedorn", "entropie, dénombrement exact", cas_hagedorn},
        {"cercle", "T-dualité", "corde sur un cercle, rayon auto-dual", cas_cercle},
        {"calabi", "Calabi-Yau", "classes de Chern, générations", cas_calabi},
        {"dimensions", "Grandes dimensions supplémentaires", "ADD : rayon vs échelle de gravité", cas_dimensions},
        {"trous_noirs", "Trous noirs", "Hawking, entropie, correspondance corde-trou noir", cas_trous_noirs},
        {"cosmique", "Cordes cosmiques", "tension, déficit angulaire, lentille", cas_cosmique},
        {"casimir", "Effet Casimir", "énergie du vide mesurable", cas_casimir},
        {"ads", "AdS/CFT", "corde ↔ plasma quarks-gluons", cas_ads},
        {"veneziano", "Amplitude de Veneziano", "pôles, dualité s-t", cas_veneziano},
        {"planck", "Échelles de corde", "de QCD à Planck, force maximale", cas_planck},
        {"lhc", "Les cordes au LHC", "résonances de Regge atteignables", cas_lhc},
        {"dualites", "Dualités", "T, S, théorie M pour vos paramètres", cas_dualites},
        {"atelier", "Atelier personnalisé", "toutes les grandeurs dérivées de M_s et g_s", cas_atelier},
    };
    return v;
}

}  // namespace cas
