// tests.hpp -- auto-tests : chaque résultat physique connu de la littérature est revérifié par le calcul.
#pragma once
#include <iostream>
#include <sstream>
#include "calabi.hpp"
#include "cas.hpp"
#include "corde.hpp"
#include "figures.hpp"
#include "jeux.hpp"
#include "physique.hpp"
#include "spectre.hpp"

namespace tests {

struct Bilan {
    int ok = 0, ko = 0;
    std::ostream& out;
    explicit Bilan(std::ostream& o) : out(o) {}
    void verifier(bool cond, const std::string& nom) {
        if (cond) { ++ok; out << "  [ OK ] " << nom << "\n"; }
        else { ++ko; out << "  [FAIL] " << nom << "\n"; }
    }
    void proche(double a, double b, double tol_rel, const std::string& nom) {
        double e = std::fabs(a - b) / (std::fabs(b) > 1e-300 ? std::fabs(b) : 1.0);
        std::ostringstream s;
        s << nom << "  (" << a << " vs " << b << ")";
        verifier(e <= tol_rel, s.str());
    }
};

inline int lancer(std::ostream& out) {
    Bilan B(out);
    out << "AUTO-TESTS de la théorie des cordes\n";
    // --- dénombrement
    auto d = spectre::degenerescences<spectre::i128>(24, 6);
    B.verifier(d[0] == 1 && d[1] == 24 && d[2] == 324 && d[3] == 3200 && d[4] == 25650 && d[5] == 176256, "dégénérescences bosoniques 1,24,324,3200,25650,176256");
    B.verifier(d[1] * d[1] == 576 && 299 + 276 + 1 == 576, "niveau 1 fermé : 576 = 299 + 276 + 1 (graviton + B + dilaton)");
    auto s = spectre::supercorde(8);
    bool susy = true;
    for (int n = 0; n <= 8; ++n) susy = susy && s.ns[n] == s.r[n];
    B.verifier(susy, "supersymétrie : bosons NS = fermions R à chaque niveau (identité de Jacobi)");
    B.verifier(s.ns[0] == 8 && s.ns[1] == 128 && s.ns[2] == 1152 && s.ns[3] == 7680, "supercorde : 8, 128, 1152, 7680 états");
    // --- dimension critique
    B.verifier(spectre::dimension_critique_bosonique() == 26, "D critique bosonique = 26");
    B.verifier(spectre::dimension_critique_super() == 10, "D critique supercorde = 10");
    B.proche(spectre::somme_regularisee(0.01), -1.0 / 12, 1e-3, "régularisation : 1+2+3+... -> -1/12");
    B.proche(spectre::somme_brute(0.01, 5000), spectre::somme_regularisee(0.01), 1e-6, "somme brute (5000 termes) = forme close");
    // --- Calabi-Yau
    bool cy = true;
    for (auto& X : calabi::galerie()) {
        auto r = calabi::analyser(X);
        cy = cy && r.dimension == 3 && r.c1_nul && r.euler == 2 * (X.h11 - X.h21);
    }
    B.verifier(cy, "8 Calabi-Yau : c1 = 0 et χ = 2(h11-h21) (classes de Chern)");
    B.verifier(calabi::analyser(calabi::galerie()[0]).euler == -200 && calabi::analyser(calabi::galerie()[0]).kappa[0] == 5, "quintique : χ = -200, H³ = 5");
    B.verifier(calabi::generations(6, 9) == 3, "Tian-Yau/Z3 : 3 générations");
    // --- Hagedorn
    auto h = spectre::hagedorn_fermee(3000);
    double pente = spectre::pente_hagedorn(h, 2700 - 2, 3000 - 2);
    B.proche(pente, spectre::BETA_H, 0.03, "pente de Hagedorn -> 4π (à 3 %)");
    // --- Regge
    std::vector<double> m2, J;
    for (auto& x : spectre::hadrons_rho()) { m2.push_back(x.m_GeV * x.m_GeV); J.push_back(x.J); }
    auto r = spectre::regression(m2, J);
    B.verifier(r.r2 > 0.99 && r.pente > 0.8 && r.pente < 0.95, "mésons ρ : droite de Regge α' ≈ 0,84-0,9 GeV⁻²");
    B.proche(spectre::regge_corde_tournante(0.9), 0.9, 1e-6, "corde tournante classique : J = α' E²");
    // --- T-dualité
    bool dual = true;
    for (int n = -2; n <= 2; ++n) for (int w = -2; w <= 2; ++w) for (double R : {0.3, 0.8, 1.0, 2.5})
        dual = dual && std::fabs(spectre::m2_cercle(n, w, 1, 1, R) - spectre::m2_cercle(w, n, 1, 1, spectre::dual(R))) < 1e-12;
    B.verifier(dual, "T-dualité : m²(n,w,R) = m²(w,n,1/R) sur 100 configurations");
    B.proche(spectre::m2_cercle(1, 1, 1, 0, 1.0), 0.0, 1e-12, "rayon auto-dual : état (1,1) sans masse (SU(2))");
    // --- Veneziano
    B.proche(spectre::beta_fn(1, 1), 1.0, 1e-12, "B(1,1) = 1");
    B.proche(spectre::veneziano(-0.7, -1.3), spectre::veneziano(-1.3, -0.7), 1e-12, "Veneziano symétrique s<->t");
    // --- simulation
    corde::Corde1D C(400, 1.0);
    C.pincer(0.27, 1.0);
    double E0 = C.energie(), dmax = 0;
    for (int i = 0; i < 1600; ++i) { C.pas(); dmax = std::max(dmax, std::fabs(C.energie() - E0) / E0); }
    B.verifier(dmax < 1e-9, "simulation : énergie conservée sur 2 périodes (écart < 1e-9)");
    C = corde::Corde1D(400, 1.0);
    C.pincer(0.27, 1.0);
    for (int i = 0; i < 173; ++i) C.pas();
    double err = 0;
    for (int i = 0; i <= 400; ++i) err = std::max(err, std::fabs(C.u[i] - corde::u_analytique(i / 400.0, C.t, 0.27, 1.0, 600)));
    B.verifier(err < 2e-3, "simulation = série de Fourier (écart max < 2e-3)");
    B.verifier(std::fabs(corde::amplitude_mode(3, 1.0 / 3)) < 1e-12, "pincer en x0=1/3 éteint l'harmonique 3");
    double somme = 0;
    for (int n = 1; n <= 2000; ++n) somme += corde::energie_mode(n, 0.27);
    B.proche(somme, corde::energie_analytique_totale(0.27), 1e-3, "somme des énergies des modes = énergie totale");
    // --- constantes & astrophysique
    B.proche(phys::tension_newton(phys::M_planck_GeV) * 2 * phys::PI, std::pow(phys::c, 4) / phys::G, 1e-3, "corde de Planck : 2πT = c⁴/G");
    B.proche(phys::T_hawking(phys::M_sun), 6.17e-8, 0.01, "température de Hawking du Soleil = 6,17e-8 K");
    B.proche(phys::pression_casimir(1e-6), -1.30e-3, 0.01, "Casimir à 1 µm = -1,30 mPa");
    B.proche(phys::l_planck, 1.616255e-35, 1e-4, "longueur de Planck");
    B.proche(phys::M_planck_GeV, 1.2209e19, 1e-3, "masse de Planck en GeV");
    B.proche(phys::deficit_angle_rad(1e-6) * 180 / phys::PI * 3600, 5.184, 1e-3, "déficit angulaire Gμ=1e-6 : 5,18″");
    double R2 = phys::rayon_ADD_m(2, 1000), R1 = phys::rayon_ADD_m(1, 1000);
    B.verifier(R2 > 1e-3 && R2 < 1e-2 && R1 > 1e13 && R1 < 1e14, "ADD, M*=1 TeV : n=2 -> millimètre, n=1 -> 10¹³ m");
    B.proche(phys::gamma_quart(), 172.79, 1e-3, "Γ(1/4)⁴ = 172,79");
    B.proche(4 * phys::PI * phys::PI / phys::gamma_quart(), 0.2285, 1e-3, "coefficient de Maldacena 0,2285");
    B.proche(phys::eta_sur_s_SI(), 6.08e-13, 0.01, "η/s = ħ/(4πk_B) = 6,08e-13 K·s");
    // --- illustrations
    bool svg_ok = true;
    for (auto& f : fig::toutes()) {
        std::string s = f.fabrique();
        svg_ok = svg_ok && s.find("<svg") == 0 && s.find("</svg>") != std::string::npos && s.size() > 1000;
    }
    B.verifier(svg_ok, std::to_string(fig::toutes().size()) + " illustrations SVG générées et bien formées");
    // --- études de cas : aucune ne doit planter et toutes doivent produire du texte
    cas::Params p;
    bool cas_ok = true;
    for (auto& c : cas::tous()) {
        std::ostringstream o;
        c.executer(o, p);
        cas_ok = cas_ok && o.str().size() > 200;
    }
    B.verifier(cas_ok, std::to_string(cas::tous().size()) + " études de cas exécutées");
    // --- jeux avec entrées scriptées
    {
        std::istringstream in("A\nB\nC\nD\nA\n");
        std::ostringstream o;
        jeux::Contexte ctx(in, o, 1);
        int sc = jeux::quiz(ctx, 5);
        B.verifier(sc >= 0 && sc <= 5 && o.str().find("Score") != std::string::npos, "quiz jouable (entrée scriptée)");
    }
    {
        std::istringstream in("0.3333333\n0.5\n0.25\n");
        std::ostringstream o;
        jeux::Contexte ctx(in, o, 2);
        int sc = jeux::pincer(ctx);
        B.verifier(sc >= 0 && o.str().find("Score total") != std::string::npos, "jeu de pincement jouable");
    }
    {
        std::istringstream in("4\n");
        std::ostringstream o;
        jeux::Contexte ctx(in, o, 3);
        int sc = jeux::univers(ctx);
        B.verifier(sc == 100, "univers à la carte : Tian-Yau/Z3 au 1er essai = 100 pts");
    }
    {
        std::istringstream in("2\n2\n2\n2\n");
        std::ostringstream o;
        jeux::Contexte ctx(in, o, 4);
        int sc = jeux::trous_noirs(ctx);
        B.verifier(sc >= 0, "chasseur de trous noirs jouable");
    }
    {
        std::istringstream in("");
        std::ostringstream o;
        jeux::Contexte ctx(in, o, 5);
        B.verifier(jeux::accordeur(ctx) == 0, "accordeur : fin de flux gérée sans blocage");
    }
    out << "\nRésultat : " << B.ok << " réussis, " << B.ko << " échecs\n";
    return B.ko;
}

}  // namespace tests
