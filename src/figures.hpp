// figures.hpp -- toutes les illustrations (SVG générés par le programme lui-même, sans bibliothèque externe).
#pragma once
#include <functional>
#include <map>
#include "calabi.hpp"
#include "corde.hpp"
#include "physique.hpp"
#include "spectre.hpp"
#include "svg.hpp"

namespace fig {

using svg::Graphique;
using svg::Serie;
using namespace svg;
const double PI = phys::PI;

struct Figure {
    std::string id, titre, legende;
    std::function<std::string()> fabrique;
};

inline std::vector<double> lin(double a, double b, int n) {
    std::vector<double> v(n);
    for (int i = 0; i < n; ++i) v[i] = a + (b - a) * i / (n - 1);
    return v;
}
inline std::vector<double> logsp(double a, double b, int n) {  // a,b sont des exposants décimaux
    std::vector<double> v(n);
    for (int i = 0; i < n; ++i) v[i] = std::pow(10.0, a + (b - a) * i / (n - 1));
    return v;
}
inline Serie serie(const std::string& nom, int couleur, std::vector<double> x, std::vector<double> y,
                   bool points = false, bool ligne = true, const std::string& tiret = "") {
    Serie s;
    s.nom = nom; s.couleur = palette()[couleur % palette().size()];
    s.x = std::move(x); s.y = std::move(y); s.points = points; s.ligne = ligne; s.tiret = tiret;
    return s;
}

// 1 ------------------------------------------------------------------------------------------
inline std::string f_modes() {
    Graphique g;
    g.titre = "Les modes de vibration d'une corde ouverte";
    g.sous_titre = "extrémités fixes (Dirichlet) : sin(nπx) — chaque mode est une « particule » différente";
    g.xlabel = "position le long de la corde  x / L"; g.ylabel = "déplacement";
    auto x = lin(0, 1, 400);
    for (int n = 1; n <= 4; ++n) {
        std::vector<double> y;
        for (double v : x) y.push_back(std::sin(n * PI * v) / n);
        g.series.push_back(serie("mode n = " + std::to_string(n) + "  (f = " + std::to_string(n) + " f₁)", n - 1, x, y));
    }
    g.ymin = -1.15; g.ymax = 1.15;
    return g.rendre();
}

// 2 ------------------------------------------------------------------------------------------
inline std::string f_corde_animee() {
    corde::Corde1D C(200, 1.0);
    C.pincer(0.27, 0.6);
    std::vector<std::vector<double>> frames;
    for (int k = 0; k < 400; ++k) {
        if (k % 5 == 0) frames.push_back(C.u);
        C.pas();
    }
    Doc d(900, 360);
    const double gl = 50, pw = 800, cy = 200, ech = 210;
    d.rect(20, 20, 860, 320, PANNEAU, GRILLE, 8);
    d.text(450, 46, "Une corde pincée : la simulation numérique en direct (SVG animé)", 18, TEXTE, "middle", true);
    d.text(450, 66, "équation d'onde ∂²u/∂t² = c² ∂²u/∂x², pincée en x = 0,27 L — on voit les ondes se réfléchir aux extrémités",
           12.5, TEXTE2, "middle");
    d.line(gl, cy, gl + pw, cy, GRILLE, 1, "4 4");
    std::ostringstream values;
    for (size_t k = 0; k < frames.size(); ++k) {
        for (int i = 0; i <= 200; i += 2) values << f(gl + pw * i / 200.0) << "," << f(cy - ech * frames[k][i]) << " ";
        values << ";";
    }
    std::string v = values.str();
    v.pop_back();
    d.b << "<polyline fill='none' stroke='#5cc8ff' stroke-width='3.2' stroke-linejoin='round' points='";
    for (int i = 0; i <= 200; i += 2) d.b << f(gl + pw * i / 200.0) << "," << f(cy - ech * frames[0][i]) << " ";
    d.b << "'><animate attributeName='points' dur='8s' repeatCount='indefinite' values='" << v << "'/></polyline>\n";
    d.circle(gl, cy, 5, "#ffd166"); d.circle(gl + pw, cy, 5, "#ffd166");
    d.text(gl, cy + 28, "extrémité fixe", 12, "#ffd166", "middle");
    d.text(gl + pw, cy + 28, "extrémité fixe", 12, "#ffd166", "middle");
    return d.rendre();
}

// 3 ------------------------------------------------------------------------------------------
inline std::string f_pincement() {
    Graphique g;
    g.titre = "Où pincer la corde ? Le spectre d'énergie dépend du point de pincement";
    g.sous_titre = "pincer en un nœud du mode n supprime ce mode : E_n ∝ sin²(nπx₀)/n²";
    g.xlabel = "numéro du mode n"; g.ylabel = "fraction d'énergie dans le mode n"; g.logy = true;
    g.xmin = 0.5; g.xmax = 10.5; g.ymin = 1e-4; g.ymax = 1;
    struct C { double x0; const char* nom; };
    int k = 0;
    for (C c : {C{0.5, "x₀ = 1/2 (milieu)"}, C{1.0 / 3, "x₀ = 1/3"}, C{0.1, "x₀ = 1/10"}}) {
        std::vector<double> x, y;
        double tot = corde::energie_analytique_totale(c.x0);
        for (int n = 1; n <= 10; ++n) {
            double e = corde::energie_mode(n, c.x0) / tot;
            x.push_back(n); y.push_back(e > 1e-9 ? e : NAN);
        }
        g.series.push_back(serie(c.nom, k++, x, y, true, true, "5 3"));
    }
    return g.rendre();
}

// 4 ------------------------------------------------------------------------------------------
inline std::string f_regge() {
    Graphique g;
    auto& H = spectre::hadrons_rho();
    std::vector<double> m2, J;
    for (auto& h : H) { m2.push_back(h.m_GeV * h.m_GeV); J.push_back(h.J); }
    auto r = spectre::regression(m2, J);
    g.titre = "Trajectoire de Regge des mésons ρ : la corde qui a donné naissance à la théorie";
    g.sous_titre = "J = α₀ + α' m²  avec  α' = " + phys::nb(r.pente, 3) + " GeV⁻²  (R² = " + phys::nb(r.r2, 5) +
                   ")  →  tension T = 1/(2πα') ≈ " + phys::nb(spectre::regression(m2, J).pente > 0 ? phys::tension_GeV2(r.pente) : 0, 3) + " GeV²";
    g.xlabel = "masse au carré  m² (GeV²)"; g.ylabel = "spin J";
    g.series.push_back(serie("mesons mesurés (PDG)", 2, m2, J, true, false));
    auto x = lin(0, 5.8, 50);
    std::vector<double> y;
    for (double v : x) y.push_back(r.ordonnee + r.pente * v);
    g.series.push_back(serie("droite de Regge (corde tournante)", 0, x, y, false, true, "8 5"));
    g.ymin = 0; g.ymax = 6.2; g.xmin = 0; g.xmax = 5.8;
    for (auto& h : H) g.notes.push_back({h.m_GeV * h.m_GeV + 0.08, h.J - 0.28, h.nom, ""});
    return g.rendre();
}

// 5 ------------------------------------------------------------------------------------------
inline std::string f_spectre() {
    Graphique g;
    g.titre = "Explosion du nombre d'états : le spectre de la corde bosonique";
    g.sous_titre = "D = 26 : 24 directions transverses. Niveau 1 = 24 états (photon/gluon sans masse), niveau 2 = 324...";
    g.xlabel = "niveau d'excitation N"; g.ylabel = "dégénérescence"; g.logy = true;
    auto d = spectre::degenerescences<long double>(24, 20);
    std::vector<double> x, y, yc;
    for (int N = 0; N <= 20; ++N) { x.push_back(N); y.push_back((double)d[N]); yc.push_back((double)(d[N] * d[N])); }
    g.series.push_back(serie("corde ouverte  d(N)", 0, x, y, true));
    g.series.push_back(serie("corde fermée  d(N)²  (N = Ñ)", 1, x, yc, true));
    return g.rendre();
}

// 6 ------------------------------------------------------------------------------------------
inline std::string f_hagedorn() {
    Graphique g;
    auto h = spectre::hagedorn_fermee(1200);
    g.titre = "Température de Hagedorn : l'entropie d'une corde est linéaire en sa masse";
    g.sous_titre = "S = ln(dégénérescence) → β_H m avec β_H = 4π√α' : la matière ne peut pas être chauffée au-delà de T_H = 1/(4π√α')";
    g.xlabel = "masse  m√α'"; g.ylabel = "entropie  S = ln d";
    std::vector<double> x, y, ya;
    for (size_t i = 0; i < h.m.size(); i += 6) { x.push_back(h.m[i]); y.push_back(h.S[i]); ya.push_back(spectre::BETA_H * h.m[i]); }
    g.series.push_back(serie("énumération exacte (corde fermée)", 0, x, y));
    g.series.push_back(serie("asymptote 4π m√α'", 2, x, ya, false, true, "8 5"));
    return g.rendre();
}

// 7 ------------------------------------------------------------------------------------------
inline std::string f_tdualite() {
    Graphique g;
    g.titre = "T-dualité : une corde sur un cercle de rayon R ≡ un cercle de rayon α'/R";
    g.sous_titre = "m² = n²/R² + w²R² + 2(N+Ñ−2) : impulsion n et enroulement w s'échangent — au rayon auto-dual R = √α' surgit SU(2)×SU(2)";
    g.xlabel = "rayon R / √α'"; g.ylabel = "masse  m √α'"; g.logx = true;
    g.xmin = 0.25; g.xmax = 4; g.ymin = 0; g.ymax = 4.2;
    auto R = logsp(std::log10(0.25), std::log10(4.0), 300);
    struct E { int n, w, N, Nt; std::string nom; };
    int k = 0;
    for (E e : {E{1, 0, 1, 1, "n=1, w=0 (Kaluza-Klein)"}, E{0, 1, 1, 1, "n=0, w=1 (enroulement)"},
                E{1, 1, 1, 0, "n=1, w=1 : masse |1/R − R| → 0 en R=1"}}) {
        std::vector<double> y;
        for (double r : R) y.push_back(std::sqrt(std::max(0.0, spectre::m2_cercle(e.n, e.w, e.N, e.Nt, r))));
        g.series.push_back(serie(e.nom, k++, R, y));
    }
    g.reperes.push_back({true, 1.0, "R = √α' (auto-dual)", "#ff9f68"});
    return g.rendre();
}

// 8 ------------------------------------------------------------------------------------------
inline std::string f_calabi(double ax = -0.55, double ay = 0.6) {
    Doc d(900, 760);
    d.rect(20, 20, 860, 720, PANNEAU, GRILLE, 8);
    d.text(450, 50, "Les dimensions cachées : coupe d'une variété de Calabi-Yau", 20, TEXTE, "middle", true);
    d.text(450, 72, "surface z₁⁵ + z₂⁵ = 1 (25 morceaux, un par couleur) — la quintique, projetée de 4 dimensions réelles vers votre écran", 12.5, TEXTE2, "middle");
    auto qs = calabi::maillage_hanson(5, 12, 0.6);
    dessiner_quads(d, qs, 450, 405, 185, ax, ay, 9.0);
    d.text(450, 720, "χ = −200  →  100 familles de fermions   (calculé par le programme : classes de Chern, cas « calabi »)", 12.5, "#ffd166", "middle");
    return d.rendre();
}

// 9 ------------------------------------------------------------------------------------------
inline std::string f_veneziano() {
    Graphique g;
    g.titre = "Amplitude de Veneziano (1968) : la « dualité » qui a engendré la théorie des cordes";
    g.sous_titre = "A(s,t) = Γ(−α(s)) Γ(−α(t)) / Γ(−α(s)−α(t)),  α(x) = 0,5 + 0,9 x — pôles = résonances hadroniques (ρ, a₂, ρ₃...)";
    g.xlabel = "s = énergie² (GeV²)"; g.ylabel = "|A(s,t)|"; g.logy = true;
    g.xmin = -1.5; g.xmax = 5.2; g.ymin = 1e-2; g.ymax = 1e3;
    auto s = lin(-1.5, 5.2, 3000);
    int k = 0;
    for (double t : {-2.0, -1.0, -0.4}) {
        std::vector<double> y;
        for (double v : s) { double a = std::fabs(spectre::veneziano(v, t)); y.push_back(std::isfinite(a) ? a : NAN); }
        g.series.push_back(serie("t = " + phys::nb(t, 2) + " GeV²", k++, s, y));
    }
    for (int n = 0; n <= 5; ++n)
        g.reperes.push_back({true, (n - 0.5) / 0.9, n == 0 ? "J=0 (pôle α=0)" : "J=" + std::to_string(n), "#5b6aa8"});
    return g.rendre();
}

// 10 -----------------------------------------------------------------------------------------
inline std::string f_worldsheet() {
    Doc d(900, 760);
    d.rect(20, 20, 860, 720, PANNEAU, GRILLE, 8);
    d.text(450, 50, "La surface d'univers d'une corde fermée", 20, TEXTE, "middle", true);
    d.text(450, 72, "une corde ponctuelle balaie une ligne d'univers ; une corde balaie une SURFACE : ici oscillation de mode 2 (+ mode 3)", 12.5, TEXTE2, "middle");
    std::vector<Quad> qs;
    const int NS = 60, NT = 40;
    auto P = [&](double s, double t) {
        double r = 1 + 0.32 * std::cos(2 * s) * std::cos(2 * t) + 0.12 * std::sin(3 * s + 1) * std::cos(3 * t);
        return V3{r * std::cos(s), r * std::sin(s), (t - PI) * 0.62};
    };
    for (int i = 0; i < NS; ++i)
        for (int j = 0; j < NT; ++j) {
            double s0 = 2 * PI * i / NS, s1 = 2 * PI * (i + 1) / NS, t0 = 2 * PI * j / NT, t1 = 2 * PI * (j + 1) / NT;
            Quad q;
            q.p[0] = P(s0, t0); q.p[1] = P(s1, t0); q.p[2] = P(s1, t1); q.p[3] = P(s0, t1);
            double r = std::hypot(q.p[0].x, q.p[0].y);
            q.couleur = hsl(215 - (r - 0.55) * 210, 0.8, 0.38 + 0.25 * (q.p[0].z + 2) / 4);
            qs.push_back(q);
        }
    dessiner_quads(d, qs, 450, 400, 135, 1.05, 0.5, 8.0, "#00000033");
    d.text(450, 722, "axe vertical = temps propre τ ; chaque coupe horizontale = la corde à un instant donné", 12.5, TEXTE2, "middle");
    return d.rendre();
}

// 11 -----------------------------------------------------------------------------------------
inline std::string f_add() {
    Graphique g;
    g.titre = "Grandes dimensions supplémentaires (ADD) : combien mesurent-elles ?";
    g.sous_titre = "M_Pl² ≈ M*^(n+2) R^n : si la gravité ne fait sentir sa vraie force qu'à M*, les dimensions cachées peuvent être macroscopiques";
    g.xlabel = "nombre n de dimensions supplémentaires"; g.ylabel = "rayon R (mètres)"; g.logy = true;
    g.xmin = 0.8; g.xmax = 6.2;
    int k = 0;
    for (double M : {1000.0, 10000.0, 100000.0}) {
        std::vector<double> x, y;
        for (int n = 1; n <= 6; ++n) { x.push_back(n); y.push_back(phys::rayon_ADD_m(n, M)); }
        g.series.push_back(serie("M* = " + phys::nb(M / 1000, 3) + " TeV", k++, x, y, true));
    }
    g.reperes.push_back({false, 3e-5, "limite des tests de la loi de Newton ≈ 30 µm", "#ff9f68"});
    g.reperes.push_back({false, 1.496e11, "Terre-Soleil (exclu !)", "#ff7eb6"});
    g.reperes.push_back({false, 1e-18, "portée du LHC ≈ 10⁻¹⁹ m", "#7ee081"});
    g.ymin = 1e-20; g.ymax = 1e16;
    return g.rendre();
}

// 12 -----------------------------------------------------------------------------------------
inline std::string f_trous_noirs() {
    Graphique g;
    g.titre = "Trous noirs : rayonnement de Hawking  T = ħc³ / (8πGMk_B)";
    g.sous_titre = "plus un trou noir est petit, plus il est chaud — le pont entre gravité quantique, thermodynamique et cordes";
    g.xlabel = "masse M (kg)"; g.ylabel = "température de Hawking (K)"; g.logx = true; g.logy = true;
    auto M = logsp(9, 43, 200);
    std::vector<double> T;
    for (double m : M) T.push_back(phys::T_hawking(m));
    g.series.push_back(serie("T_H(M)", 0, M, T));
    struct O { const char* nom; double m; };
    std::vector<double> xs, ys;
    for (O o : {O{"Sgr A* (4·10⁶ M☉)", 4.3e6 * phys::M_sun}, O{"Soleil", phys::M_sun}, O{"Terre", phys::M_terre},
                O{"trou noir primordial 10¹² kg", 1e12}}) {
        xs.push_back(o.m); ys.push_back(phys::T_hawking(o.m));
        g.notes.push_back({o.m * 2.2, phys::T_hawking(o.m) * 1.6, o.nom, "#ffd166"});
    }
    g.series.push_back(serie("", 2, xs, ys, true, false));
    g.reperes.push_back({false, 2.725, "fond diffus cosmologique 2,7 K", "#ff9f68"});
    g.xmin = 1e9; g.xmax = 1e43;
    g.ymin = 1e-30; g.ymax = 1e15;
    return g.rendre();
}

// 13 -----------------------------------------------------------------------------------------
inline std::string f_generations() {
    std::vector<std::string> noms;
    std::vector<double> val;
    for (auto& m : calabi::modeles()) { noms.push_back(m.nom); val.push_back(std::max(0.3, (double)calabi::generations(m.h11, m.h21))); }
    return barres("Combien de familles de fermions ? |h¹¹ − h²¹| = |χ|/2", "le nombre de générations est une propriété GÉOMÉTRIQUE du Calabi-Yau (les barres nulles sont tracées à 0,3)",
                  "nombre de générations", noms, val, {}, 3, "3 générations observées (e,μ,τ)", true);
}

// 14 -----------------------------------------------------------------------------------------
inline std::string f_cordes_cosmiques() {
    Graphique g;
    g.titre = "Cordes cosmiques : un défaut topologique qui découpe l'espace";
    g.sous_titre = "une corde de tension μ retire un angle δ = 8πGμ/c² : le ciel derrière elle apparaît en double (lentille gravitationnelle)";
    g.xlabel = "Gμ/c²  (tension sans dimension)"; g.ylabel = "angle de déficit δ (secondes d'arc)"; g.logx = true; g.logy = true;
    auto x = logsp(-12, -4, 100);
    std::vector<double> y;
    for (double v : x) y.push_back(phys::deficit_angle_rad(v) * 180 / PI * 3600);
    g.series.push_back(serie("δ = 8π Gμ", 0, x, y));
    g.reperes.push_back({true, 1e-7, "≈ limite CMB (dépend du modèle)", "#ff9f68"});
    g.reperes.push_back({true, 1e-6, "échelle GUT  Gμ ~ 10⁻⁶", "#ff7eb6"});
    g.reperes.push_back({false, 1.0, "1″", "#5b6aa8"});
    return g.rendre();
}

// 15 -----------------------------------------------------------------------------------------
inline std::string f_casimir() {
    Graphique g;
    g.titre = "Effet Casimir : l'énergie de point zéro 1+2+3+… = −1/12 se mesure au laboratoire";
    g.sous_titre = "pression entre deux plaques conductrices :  P = −π²ħc / (240 d⁴)  — même mécanisme que la dimension critique D = 26";
    g.xlabel = "distance d entre les plaques (m)"; g.ylabel = "|pression| (Pa)"; g.logx = true; g.logy = true;
    auto d = logsp(-8, -5, 100);
    std::vector<double> p;
    for (double v : d) p.push_back(std::fabs(phys::pression_casimir(v)));
    g.series.push_back(serie("|P_Casimir|", 0, d, p));
    g.reperes.push_back({false, 101325, "1 atmosphère", "#ff9f68"});
    return g.rendre();
}

// 16 -----------------------------------------------------------------------------------------
inline std::string f_zeta() {
    Graphique g;
    g.titre = "Pourquoi 1 + 2 + 3 + … = −1/12 : la régularisation des modes de la corde";
    g.sous_titre = "Σ n·e^(−εn) − 1/ε² → −1/12 quand ε → 0 : la partie divergente est absorbée, il reste −1/12 par oscillateur → D = 26";
    g.xlabel = "coupure ε"; g.ylabel = "Σ n e^(−εn) − 1/ε²"; g.logx = true;
    auto e = logsp(-3, 0.6, 200);
    std::vector<double> y, y0;
    for (double v : e) { y.push_back(spectre::somme_regularisee(v)); y0.push_back(-1.0 / 12); }
    g.series.push_back(serie("somme régularisée", 0, e, y));
    g.series.push_back(serie("−1/12 = ζ(−1)", 2, e, y0, false, true, "8 5"));
    g.ymin = -0.2; g.ymax = 0.02;
    return g.rendre();
}

// 17 -----------------------------------------------------------------------------------------
inline std::string f_ads() {
    Graphique g;
    g.titre = "AdS/CFT (Maldacena 1997) : la force entre quarks calculée avec une corde";
    g.sous_titre = "coefficient de −L·V(L) dans N=4 Super-Yang-Mills : faible couplage λ/4π  contre  fort couplage (corde dans AdS₅) 4π²√λ/Γ(¼)⁴";
    g.xlabel = "couplage de 't Hooft  λ = g²N"; g.ylabel = "−L·V(L)"; g.logx = true; g.logy = true;
    auto l = logsp(-1, 3, 100);
    std::vector<double> a, b;
    for (double v : l) { a.push_back(v / (4 * PI)); b.push_back(-phys::potentiel_qqbar(v, 1.0)); }
    g.series.push_back(serie("théorie des perturbations  λ/(4π)", 0, l, a));
    g.series.push_back(serie("corde dans AdS₅  ≈ 0,2285 √λ", 1, l, b));
    g.reperes.push_back({true, 1.0, "λ ~ 1 : transition", "#5b6aa8"});
    return g.rendre();
}

// 18 -----------------------------------------------------------------------------------------
inline std::string f_dualites() {
    Doc d(900, 720);
    d.rect(20, 20, 860, 680, PANNEAU, GRILLE, 8);
    d.text(450, 50, "La carte des dualités : cinq théories des cordes, une seule théorie M", 20, TEXTE, "middle", true);
    d.text(450, 72, "chaque trait est une équivalence exacte (T = T-dualité R ↔ α'/R, S = dualité de couplage g ↔ 1/g)", 12.5, TEXTE2, "middle");
    struct N { const char* nom; double x, y; const char* col; };
    N nodes[] = {{"M-théorie (11D)", 450, 370, "#ffd166"}, {"Type I (SO(32))", 200, 190, "#5cc8ff"},
                 {"Type IIA", 450, 130, "#ff7eb6"}, {"Type IIB", 700, 190, "#7ee081"},
                 {"Hétérotique SO(32)", 200, 560, "#c792ea"}, {"Hétérotique E₈×E₈", 700, 560, "#ff9f68"}};
    struct L { int a, b; const char* txt; };
    L links[] = {{1, 4, "S"}, {1, 3, "orientifold"}, {2, 3, "T"}, {4, 5, "T"}, {0, 2, "g→∞"}, {0, 5, "g→∞ (segment)"},
                 {0, 1, ""}, {0, 3, ""}, {0, 4, ""}};
    for (auto& l : links) {
        if (l.a == 0 && (l.b == 1 || l.b == 3 || l.b == 4)) {
            d.line(nodes[l.a].x, nodes[l.a].y, nodes[l.b].x, nodes[l.b].y, "#2a3358", 1.3, "3 5");
            continue;
        }
        d.line(nodes[l.a].x, nodes[l.a].y, nodes[l.b].x, nodes[l.b].y, "#8090c8", 2.2);
        d.text((nodes[l.a].x + nodes[l.b].x) / 2, (nodes[l.a].y + nodes[l.b].y) / 2 - 8, l.txt, 13, "#ffd166", "middle", true);
    }
    for (auto& n : nodes) {
        d.rect(n.x - 92, n.y - 22, 184, 44, FOND, n.col, 22);
        d.text(n.x, n.y + 5, n.nom, 14, n.col, "middle", true);
    }
    d.text(450, 660, "IIB s'auto-duale sous S : g_s → 1/g_s.   Le rayon de la 11e dimension est R₁₁ = g_s ℓ_s (IIA fortement couplée).", 12.5, TEXTE2, "middle");
    return d.rendre();
}

inline const std::vector<Figure>& toutes() {
    static const std::vector<Figure> v = {
        {"01_modes", "Modes de vibration", "Chaque harmonique d'une corde fixée à ses extrémités correspond à un état différent : les fréquences n·f₁ deviennent des masses m² ∝ N.", f_modes},
        {"02_corde_animee", "Corde animée", "Simulation numérique de l'équation d'onde (schéma saute-mouton, CFL=1 : exact sur la grille). Ouvrez le SVG dans un navigateur pour voir l'animation.", f_corde_animee},
        {"03_pincement", "Pincement et spectre", "La corde pincée au milieu ne contient que des harmoniques impairs ; pincée à x₀=1/3, l'harmonique 3 disparaît : la position d'excitation sélectionne les particules produites.", f_pincement},
        {"04_regge", "Trajectoire de Regge", "Les mésons ρ, a₂, ρ₃, a₄, ρ₅ s'alignent sur J = α₀ + α' m² : c'est exactement ce que fait une corde tournante (J = α' E²). C'est de là qu'est née la théorie des cordes (Nambu, 1970).", f_regge},
        {"05_spectre", "Dégénérescences", "Le nombre d'états explose : d(N) = coefficient de qᴺ dans ∏(1−qⁿ)⁻²⁴. Ces valeurs (24, 324, 3200, 25650…) sont calculées, pas recopiées.", f_spectre},
        {"06_hagedorn", "Température de Hagedorn", "La croissance exponentielle des états entraîne une température maximale T_H = 1/(4π√α') : au-delà, l'énergie fabrique des cordes plus longues au lieu de chauffer.", f_hagedorn},
        {"07_tdualite", "T-dualité", "Une corde ne distingue pas un cercle de rayon R d'un cercle de rayon α'/R : la longueur minimale √α' est intrinsèque. Au rayon auto-dual, des états sans masse supplémentaires forment SU(2)×SU(2).", f_tdualite},
        {"08_calabi_yau", "Calabi-Yau", "Les 6 dimensions cachées de la supercorde sont repliées sur une variété de Calabi-Yau. Représentation de Hanson de la quintique de Fermat.", []() { return f_calabi(); }},
        {"09_veneziano", "Amplitude de Veneziano", "La formule qui a lancé la théorie : symétrique en s↔t, avec une infinité de pôles (résonances) alignés sur la trajectoire de Regge.", f_veneziano},
        {"10_surface_univers", "Surface d'univers", "Une corde fermée trace un tube dans l'espace-temps ; ses oscillations le déforment.", f_worldsheet},
        {"11_dimensions_ADD", "Grandes dimensions", "Modèle ADD : le rayon des dimensions supplémentaires pour M* donné. n = 1 est exclu, n = 2 est testé à quelques dizaines de µm.", f_add},
        {"12_trous_noirs", "Trous noirs", "Température de Hawking de 10⁹ à 10⁴³ kg. Les cordes expliquent l'entropie de Bekenstein-Hawking (Strominger-Vafa 1996).", f_trous_noirs},
        {"13_generations", "Générations", "Le nombre de familles de fermions est |χ|/2 : les Calabi-Yau réalistes doivent avoir |h¹¹−h²¹| = 3.", f_generations},
        {"14_cordes_cosmiques", "Cordes cosmiques", "Angle de déficit δ = 8πGμ : mesurer des images doubles de galaxies permettrait de détecter une corde cosmique.", f_cordes_cosmiques},
        {"15_casimir", "Effet Casimir", "L'énergie de point zéro régularisée des modes fait apparaître −1/12 par oscillateur : c'est ce −1/12 qui fixe D = 26 ; l'analogue électromagnétique est mesuré.", f_casimir},
        {"16_regularisation_zeta", "1+2+3+…", "Le calcul explicite de la somme régularisée : le résidu fini est −1/12.", f_zeta},
        {"17_ads_cft", "AdS/CFT", "À fort couplage, le potentiel quark-antiquark se calcule avec une corde suspendue dans AdS₅ : croissance en √λ au lieu de λ.", f_ads},
        {"18_dualites", "Carte des dualités", "Les cinq supercordes et la théorie M sont reliées par des dualités T, S et des limites de couplage fort.", f_dualites},
    };
    return v;
}

inline int ecrire_toutes(const std::string& dossier) {
    int n = 0;
    for (auto& f : toutes()) {
        std::string chemin = dossier + "/" + f.id + ".svg";
        if (!ecrire(chemin, f.fabrique())) return -1;
        ++n;
    }
    return n;
}

}  // namespace fig
