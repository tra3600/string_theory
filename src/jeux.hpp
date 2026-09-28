// jeux.hpp -- la partie ludique : quiz, accordeur cosmique, pincement de corde, univers à la carte,
// chasseur de trous noirs et animation ASCII. Tous les jeux fonctionnent aussi en entrée redirigée.
#pragma once
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>
#include "calabi.hpp"
#include "cas.hpp"
#include "corde.hpp"
#include "physique.hpp"
#include "spectre.hpp"

namespace jeux {

using phys::nb;
const double PI = phys::PI;

struct Contexte {
    std::istream& in;
    std::ostream& out;
    std::mt19937 rng;
    Contexte(std::istream& i, std::ostream& o, unsigned graine) : in(i), out(o), rng(graine) {}
    double alea(double a, double b) { return std::uniform_real_distribution<double>(a, b)(rng); }
    int alea_int(int a, int b) { return std::uniform_int_distribution<int>(a, b)(rng); }
};

// Lit une ligne ; retourne false en fin de flux.
inline bool ligne(Contexte& c, const std::string& invite, std::string& s) {
    c.out << invite << std::flush;
    if (!std::getline(c.in, s)) { c.out << "\n"; return false; }
    return true;
}
inline bool lire_nombre(Contexte& c, const std::string& invite, double& v) {
    std::string s;
    while (ligne(c, invite, s)) {
        for (char& ch : s) if (ch == ',') ch = '.';
        try {
            size_t pos = 0;
            v = std::stod(s, &pos);
            if (pos > 0 && std::isfinite(v)) return true;
        } catch (...) {}
        c.out << "   (entrez un nombre)\n";
    }
    return false;
}
inline const char* grade(double pct) {
    if (pct >= 95) return "Maître(sse) des cordes (Witten approuve)";
    if (pct >= 75) return "Physicien(ne) des cordes";
    if (pct >= 50) return "Doctorant(e) en théorie des cordes";
    if (pct >= 25) return "Étudiant(e) curieux(se)";
    return "Boson de Higgs égaré(e) : courage !";
}

// ------------------------------------------------------------------------------------------ quiz
struct Question {
    std::string texte;
    std::vector<std::string> choix;
    int bonne;  // index
    std::string explication;
};

inline std::vector<Question> banque_questions() {
    auto d = spectre::degenerescences<spectre::i128>(24, 3);
    auto s = spectre::supercorde(2);
    std::vector<Question> q;
    q.push_back({"Combien de dimensions d'espace-temps la supercorde exige-t-elle ?", {"4", "10", "11", "26"}, 1,
                 "a = (D-2)/16 = 1/2 impose D = " + std::to_string(spectre::dimension_critique_super()) + "."});
    q.push_back({"Et la corde bosonique ?", {"10", "24", "26", "12"}, 2,
                 "a = (D-2)/24 = 1 impose D = " + std::to_string(spectre::dimension_critique_bosonique()) + "."});
    q.push_back({"Régularisée, la somme 1+2+3+... vaut ?", {"+infini", "-1/12", "0", "1/12"}, 1,
                 "ζ(-1) = -1/12 : le programme le retrouve en cas « dimension »."});
    q.push_back({"Combien d'états au niveau N = 2 de la corde bosonique ouverte ?", {"24", "300", spectre::str(d[2]), "576"}, 2,
                 "coefficient de q² dans ∏(1-qⁿ)⁻²⁴ = " + spectre::str(d[2]) + "."});
    q.push_back({"Combien d'états sans masse au niveau 1 de la corde ouverte bosonique ?", {"8", "24", "26", "16"}, 1, "un vecteur de SO(24) : 24 polarisations transverses."});
    q.push_back({"Combien d'états bosoniques (NS) au premier niveau massif de la supercorde ouverte ?", {"8", "64", spectre::str(s.ns[1]), "1152"}, 2,
                 "on trouve " + spectre::str(s.ns[1]) + " (et autant de fermions)."});
    q.push_back({"Quelle est la caractéristique d'Euler de la quintique ?", {"-200", "-100", "200", "0"}, 0, "χ = -200 : |χ|/2 = 100 générations sur la quintique."});
    q.push_back({"Combien de générations pour un Calabi-Yau de nombres de Hodge (h11,h21) = (6,9) ?", {"1", "3", "6", "9"}, 1, "|h11-h21| = 3."});
    q.push_back({"Que fait la T-dualité ?", {"g → 1/g", "R → α'/R", "D → D+1", "t → -t"}, 1, "elle échange impulsion et enroulement."});
    q.push_back({"Le rayon auto-dual d'un cercle vaut ?", {"l_Planck", "√α'", "α'", "1 mètre"}, 1, "R = √α' : SU(2)×SU(2) apparaît."});
    q.push_back({"La température de Hagedorn est ?", {"1/(4π√α')", "√α'", "4π√α'", "infinie"}, 0, "T_H = M_s/(4π) pour la corde bosonique fermée."});
    q.push_back({"Combien de supercordes cohérentes en dimension 10 ?", {"1", "3", "5", "10"}, 2, "I, IIA, IIB, hétérotique SO(32) et E8×E8, unifiées par la théorie M."});
    q.push_back({"Le rayon de la 11e dimension de la théorie M est proportionnel à ?", {"g_s", "1/g_s", "α'", "T"}, 0, "R₁₁ = g_s l_s."});
    q.push_back({"Les trajectoires de Regge sont : J = ?", {"α₀ + α' m²", "α' m", "m³", "e^m"}, 0, "droite J en fonction de m²."});
    q.push_back({"L'entropie de Bekenstein-Hawking d'un trou noir est proportionnelle à ?", {"son volume", "sa masse", "l'aire de son horizon", "sa charge"}, 2, "S = A/(4 l_P²)."});
    q.push_back({"Plus un trou noir est massif, plus sa température de Hawking est ?", {"élevée", "basse", "constante", "négative"}, 1, "T ∝ 1/M."});
    q.push_back({"Une corde cosmique de tension μ produit un déficit angulaire de ?", {"8πGμ/c²", "Gμ", "μ/c", "0"}, 0, "δ = 8πGμ/c²."});
    q.push_back({"La dualité AdS/CFT relie une théorie de cordes dans AdS₅×S⁵ à ?", {"QED", "N=4 super Yang-Mills", "la gravité de Newton", "le modèle d'Ising"}, 1, "Maldacena, 1997."});
    q.push_back({"Combien de dimensions spatiales doit-on cacher (D=10 → 4) ?", {"3", "6", "7", "10"}, 1, "10 - 4 = 6 : la variété de Calabi-Yau."});
    q.push_back({"Qui a proposé en 1968 la formule qui a donné naissance à la théorie des cordes ?", {"Veneziano", "Einstein", "Feynman", "Dirac"}, 0, "Gabriele Veneziano, à propos des résonances hadroniques."});
    q.push_back({"L'effet Casimir mesure ?", {"l'énergie du vide", "la gravité quantique", "l'antimatière", "le boson de Higgs"}, 0, "P = -π²ħc/(240 d⁴)."});
    return q;
}

inline int quiz(Contexte& c, int nb_q) {
    auto Q = banque_questions();
    std::shuffle(Q.begin(), Q.end(), c.rng);
    nb_q = std::clamp(nb_q, 1, (int)Q.size());
    c.out << "\n*** LE DÉFI DE LA CORDE : " << nb_q << " questions ***\n";
    int score = 0, posees = 0;
    for (int i = 0; i < nb_q; ++i) {
        Question q = Q[i];
        std::vector<int> ordre(q.choix.size());
        for (size_t k = 0; k < ordre.size(); ++k) ordre[k] = (int)k;
        std::shuffle(ordre.begin(), ordre.end(), c.rng);
        c.out << "\nQ" << i + 1 << ". " << q.texte << "\n";
        int bonne_lettre = 0;
        for (size_t k = 0; k < ordre.size(); ++k) {
            c.out << "   " << char('A' + k) << ") " << q.choix[ordre[k]] << "\n";
            if (ordre[k] == q.bonne) bonne_lettre = (int)k;
        }
        std::string r;
        if (!ligne(c, "   Votre réponse (A-" + std::string(1, char('A' + ordre.size() - 1)) + ") : ", r)) break;
        ++posees;
        int lettre = r.empty() ? -1 : std::toupper((unsigned char)r[0]) - 'A';
        if (lettre == bonne_lettre) { ++score; c.out << "   ✔ Exact ! " << q.explication << "\n"; }
        else c.out << "   ✘ Non : c'était " << char('A' + bonne_lettre) << ") " << q.choix[q.bonne] << ". " << q.explication << "\n";
    }
    double pct = posees ? 100.0 * score / posees : 0;
    c.out << "\nScore : " << score << "/" << posees << " (" << (int)pct << "%) -> " << grade(pct) << "\n";
    return score;
}

// ------------------------------------------------------------------------------------------ accordeur cosmique
inline int accordeur(Contexte& c) {
    double m = std::round(c.alea(0.35, 0.85) * 100) / 100;
    c.out << "\n*** L'ACCORDEUR COSMIQUE ***\n"
          << "Une corde fermée s'enroule sur un cercle de rayon R (en unités de √α'). Son état excité le plus léger\n"
          << "a la masse m(R) = min(1/R, R) (en unités de M_s) : impulsion 1/R ou enroulement R.\n"
          << "Cible : m = " << m << ".  Trouvez LES DEUX rayons (T-dualité !) à 2 % près. 14 essais.\n";
    double R1 = m, R2 = 1 / m;
    bool t1 = false, t2 = false;
    int essais = 0;
    for (; essais < 14 && !(t1 && t2); ++essais) {
        double R;
        if (!lire_nombre(c, "   R = ", R)) return 0;
        if (R <= 0) { c.out << "   R doit être positif.\n"; --essais; continue; }
        double mm = std::min(1 / R, R);
        c.out << "   m(R) = " << nb(mm, 4) << (mm > m ? "  → trop lourd" : "  → trop léger");
        bool a = std::fabs(R - R1) / R1 < 0.02, b = std::fabs(R - R2) / R2 < 0.02;
        if (a && !t1) { t1 = true; c.out << "   ★ rayon « enroulement » trouvé !"; }
        else if (b && !t2) { t2 = true; c.out << "   ★ rayon « impulsion » trouvé !"; }
        else if (a || b) c.out << "   (déjà trouvé)";
        c.out << "\n";
        if (mm > m && std::fabs(mm - m) / m > 0.02) c.out << "       (indice : m(R) monte jusqu'à R = 1 puis redescend, la cible est atteinte deux fois)\n";
    }
    int pts = (t1 ? 50 : 0) + (t2 ? 50 : 0);
    c.out << "Solutions : R = " << nb(R1, 4) << " et R = " << nb(R2, 4) << " (dualité R ↔ 1/R). Score : " << pts << "/100\n";
    return pts;
}

// ------------------------------------------------------------------------------------------ pincement
inline int pincer(Contexte& c) {
    c.out << "\n*** PINCEZ LA CORDE ***\n"
          << "Une corde de longueur 1 est pincée au point x0 ∈ ]0,1[. Objectif : ÉTEINDRE une harmonique donnée\n"
          << "(énergie du mode n ∝ sin²(nπx0)/n² : pincez sur un de ses nœuds !). 3 manches.\n";
    int total = 0;
    for (int manche = 1; manche <= 3; ++manche) {
        int n = c.alea_int(2, 6);
        c.out << "\nManche " << manche << " : éteignez l'harmonique n = " << n << ".\n";
        double x0;
        if (!lire_nombre(c, "   x0 = ", x0)) return total;
        if (x0 <= 0.01 || x0 >= 0.99) { c.out << "   x0 borné dans [0.01, 0.99].\n"; x0 = std::clamp(x0, 0.01, 0.99); }
        double Ea = corde::energie_analytique_totale(x0);
        c.out << "   mode | énergie\n";
        for (int k = 1; k <= 8; ++k) {
            double e = corde::energie_mode(k, x0) / Ea * 100;
            c.out << "   " << (k == n ? "→" : " ") << std::setw(3) << k << "  | " << std::setw(7) << std::setprecision(4) << e << "% " << corde::barre(e, 100, 40) << "\n";
        }
        double e = corde::energie_mode(n, x0) / Ea;
        int pts = (int)std::lround(100.0 * std::exp(-e * 60));
        c.out << "   énergie résiduelle du mode " << n << " : " << nb(e * 100, 4) << "%  → " << pts << " points\n";
        if (pts < 90) {
            std::ostringstream sol;
            for (int k = 1; k < n; ++k) sol << k << "/" << n << (k + 1 < n ? ", " : "");
            c.out << "   (solutions exactes : x0 = " << sol.str() << ")\n";
        }
        total += pts;
    }
    c.out << "\nScore total : " << total << "/300 -> " << grade(total / 3.0) << "\n";
    return total;
}

// ------------------------------------------------------------------------------------------ univers à la carte
inline int univers(Contexte& c) {
    auto& M = calabi::modeles();
    c.out << "\n*** UNIVERS À LA CARTE ***\n"
          << "Choisissez la géométrie des 6 dimensions cachées. Il faut |h11 - h21| = 3 générations de fermions (e, μ, τ).\n";
    for (size_t i = 0; i < M.size(); ++i)
        c.out << "  " << i + 1 << ") " << cas::pad(M[i].nom, 24) << " (h11,h21) = (" << M[i].h11 << "," << M[i].h21 << ")  " << M[i].note << "\n";
    c.out << "  (tapez un numéro ; « r » = tirage aléatoire ; « q » = quitter ; 5 essais)\n";
    int score = 0;
    for (int essai = 1; essai <= 5; ++essai) {
        std::string s;
        if (!ligne(c, "  choix " + std::to_string(essai) + "/5 : ", s)) break;
        if (!s.empty() && (s[0] == 'q' || s[0] == 'Q')) break;
        int idx;
        if (!s.empty() && (s[0] == 'r' || s[0] == 'R')) idx = c.alea_int(0, (int)M.size() - 1);
        else {
            try { idx = std::stoi(s) - 1; } catch (...) { idx = -1; }
        }
        if (idx < 0 || idx >= (int)M.size()) { c.out << "   numéro invalide\n"; --essai; continue; }
        auto& m = M[idx];
        int g = calabi::generations(m.h11, m.h21);
        c.out << "   → " << m.nom << " : χ = 2(h11-h21) = " << 2 * (m.h11 - m.h21) << ", " << g << " génération(s), "
              << m.h11 << " modules de Kähler, " << m.h21 << " modules complexes (=> " << m.h11 + m.h21 << " champs scalaires sans masse à expliquer).\n";
        if (g == 3) { c.out << "   ★★★ BRAVO : univers à 3 générations ! (comme le nôtre)\n"; score = 100 - (essai - 1) * 20; break; }
        c.out << "   ✘ " << (g == 0 ? "aucune famille chirale : monde vectoriel" : g < 3 ? "trop peu de familles" : "trop de familles") << " — retentez.\n";
    }
    c.out << "Score : " << score << "/100\n";
    return score;
}

// ------------------------------------------------------------------------------------------ chasseur de trous noirs
inline int trous_noirs(Contexte& c) {
    c.out << "\n*** CHASSEUR DE TROUS NOIRS ***\n"
          << "On vous donne la masse ; estimez log₁₀ de la température de Hawking (en kelvins). 4 manches, tolérance ±1 = 50 pts.\n"
          << "Indice : T = 6,2×10⁻⁸ K × (M☉ / M).\n";
    int total = 0;
    for (int manche = 1; manche <= 4; ++manche) {
        double lm = c.alea(9, 42);
        double M = std::pow(10.0, lm);
        double T = phys::T_hawking(M);
        c.out << "\nManche " << manche << " : M = " << phys::sci(M, 2) << " kg  (= " << phys::sci(M / phys::M_sun, 2) << " M☉), rayon de Schwarzschild = "
              << phys::longueur(phys::rayon_schwarzschild(M)) << "\n";
        double g;
        if (!lire_nombre(c, "   log10(T / K) = ", g)) return total;
        double err = std::fabs(g - std::log10(T));
        int pts = std::max(0, (int)std::lround(100 - 50 * err));
        c.out << "   Réponse : T = " << phys::sci(T, 2) << " K (log = " << nb(std::log10(T), 4) << "), écart " << nb(err, 3) << " décades → " << pts << " pts\n";
        total += pts;
    }
    c.out << "\nScore total : " << total << "/400 -> " << grade(total / 4.0) << "\n";
    return total;
}

// ------------------------------------------------------------------------------------------ animation ASCII
inline void animation(std::ostream& out, int images, bool pause, int mode_max = 3) {
    corde::Corde4D C = corde::Corde4D::exemple(false);
    C.points = 72;
    for (int f = 0; f < images; ++f) {
        double tau = 2 * PI * f / std::max(1, images);
        auto P = C.position(tau);
        std::vector<double> y;
        for (auto& p : P) y.push_back(p[1] + 0.6 * p[2]);
        if (pause) out << "\x1b[2J\x1b[H";
        out << "corde en vibration — image " << f + 1 << "/" << images << "  (τ = " << nb(tau, 3) << ")  modes 1.." << mode_max << "\n";
        out << corde::ascii_courbe(y, 72, 17, 2.0);
        out << std::flush;
        if (pause) std::this_thread::sleep_for(std::chrono::milliseconds(60));
    }
}

// Sortie « brute » de l'ancien programme, améliorée : positions dans R^4 à différents instants.
inline void sortie_brute(std::ostream& out, int points, double duree, double pas) {
    corde::Corde4D C = corde::Corde4D::exemple(false);
    C.points = points;
    for (double t = 0; t < duree; t += pas) {
        out << "Time: " << t << "\n";
        for (auto& p : C.position(t)) {
            for (double v : p) out << std::setw(10) << std::setprecision(5) << v << " ";
            out << "\n";
        }
        out << "\n";
    }
}

// Vue 4D en ASCII : rotation dans le plan (x,w) puis projection 4D->3D->2D
inline void vue4d(std::ostream& out, double angle, double tau) {
    corde::Corde4D C = corde::Corde4D::exemple(true);
    C.points = 900;
    std::vector<std::string> g(23, std::string(72, ' '));
    for (auto& p : C.position(tau)) {
        auto q = corde::rotation4(p, 0, 3, angle);
        auto r = corde::projeter4_3(q, 5.0);
        int x = 36 + (int)std::lround(r[0] * 24), y = 11 - (int)std::lround(r[1] * 7.5);
        char ch = r[2] > 0.3 ? '@' : r[2] > -0.3 ? 'o' : '.';
        if (x >= 0 && x < 72 && y >= 0 && y < 23) g[y][x] = ch;
    }
    out << "Corde fermée dans R⁴, rotation x-w de " << nb(angle * 180 / PI, 3) << "° (la profondeur z est le caractère : @ proche, o milieu, . loin)\n";
    for (auto& l : g) out << l << "\n";
}

}  // namespace jeux
