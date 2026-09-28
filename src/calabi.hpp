// calabi.hpp -- espaces de Calabi-Yau : caractéristique d'Euler CALCULÉE par les classes de Chern,
// nombre de générations, et maillage 3D de la coupe de la quintique (représentation de Hanson).
#pragma once
#include <complex>
#include <map>
#include <string>
#include <vector>
#include "svg.hpp"

namespace calabi {

// Anneau Z[H1..Hm] / (H_i^{n_i+1}) : cohomologie d'un produit d'espaces projectifs P^{n_1} x ... x P^{n_m}.
struct Anneau {
    std::vector<int> n;
    using Mono = std::vector<int>;
    using Poly = std::map<Mono, long long>;
    explicit Anneau(std::vector<int> n_) : n(std::move(n_)) {}
    int m() const { return (int)n.size(); }
    int deg_total_max() const { int s = 0; for (int v : n) s += v; return s; }
    static int deg(const Mono& e) { int s = 0; for (int v : e) s += v; return s; }
    Poly un() const { return {{Mono(m(), 0), 1}}; }
    Poly var(int i) const { Mono e(m(), 0); e[i] = 1; return {{e, 1}}; }
    Poly add(const Poly& a, const Poly& b) const {
        Poly r = a;
        for (auto& [e, c] : b) { r[e] += c; if (r[e] == 0) r.erase(e); }
        return r;
    }
    Poly mul(const Poly& a, const Poly& b) const {
        Poly r;
        for (auto& [ea, ca] : a)
            for (auto& [eb, cb] : b) {
                Mono e(m());
                bool ok = true;
                for (int i = 0; i < m(); ++i) { e[i] = ea[i] + eb[i]; if (e[i] > n[i]) { ok = false; break; } }
                if (ok) { r[e] += ca * cb; if (r[e] == 0) r.erase(e); }
            }
        return r;
    }
    Poly scal(const Poly& a, long long k) const {
        Poly r;
        for (auto& [e, c] : a) if (c * k != 0) r[e] = c * k;
        return r;
    }
    Poly puiss(const Poly& a, int k) const { Poly r = un(); for (int i = 0; i < k; ++i) r = mul(r, a); return r; }
    // 1/(1+x) = sum (-x)^k pour x sans terme constant
    Poly inverse_1px(const Poly& x) const {
        Poly r = un(), t = un();
        for (int k = 1; k <= deg_total_max(); ++k) { t = mul(t, scal(x, -1)); r = add(r, t); }
        return r;
    }
    Poly partie_degre(const Poly& a, int k) const {
        Poly r;
        for (auto& [e, c] : a) if (deg(e) == k) r[e] = c;
        return r;
    }
    long long coeff_sommet(const Poly& a) const {  // coefficient de prod H_i^{n_i} = intégrale sur l'ambiant
        auto it = a.find(n);
        return it == a.end() ? 0 : it->second;
    }
};

struct IntersectionComplete {
    std::string nom;
    std::vector<int> n;                 // P^{n_1} x ... x P^{n_m}
    std::vector<std::vector<int>> q;    // q[j][i] : degré de l'équation j par rapport au facteur i
    int h11, h21;                       // nombres de Hodge (littérature) : servent de contrôle croisé
};

struct ResultatCY {
    int dimension = 0;
    bool c1_nul = false;
    long long euler = 0;
    std::vector<long long> kappa;  // intersections H_i^3 (m=1) ou triple (i,j,k) rangées par ordre lexicographique
};

inline ResultatCY analyser(const IntersectionComplete& X) {
    Anneau A(X.n);
    int m = A.m(), K = (int)X.q.size();
    ResultatCY r;
    int somme = 0;
    for (int v : X.n) somme += v;
    r.dimension = somme - K;
    // c(X) = prod (1+H_i)^{n_i+1} / prod_j (1 + sum_i q_ji H_i)
    Anneau::Poly c = A.un();
    for (int i = 0; i < m; ++i) c = A.mul(c, A.puiss(A.add(A.un(), A.var(i)), X.n[i] + 1));
    std::vector<Anneau::Poly> eq;
    for (int j = 0; j < K; ++j) {
        Anneau::Poly e;
        for (int i = 0; i < m; ++i) e = A.add(e, A.scal(A.var(i), X.q[j][i]));
        eq.push_back(e);
        c = A.mul(c, A.inverse_1px(e));
    }
    r.c1_nul = A.partie_degre(c, 1).empty();
    Anneau::Poly euler_top = A.partie_degre(c, r.dimension);
    Anneau::Poly fibre = A.un();
    for (auto& e : eq) fibre = A.mul(fibre, e);
    r.euler = A.coeff_sommet(A.mul(euler_top, fibre));
    // intersections triples kappa_{ijk} = int_X H_i H_j H_k
    for (int i = 0; i < m; ++i)
        for (int j = i; j < m; ++j)
            for (int k = j; k < m; ++k) {
                Anneau::Poly t = A.mul(A.mul(A.var(i), A.var(j)), A.var(k));
                r.kappa.push_back(A.coeff_sommet(A.mul(t, fibre)));
            }
    return r;
}

inline const std::vector<IntersectionComplete>& galerie() {
    static const std::vector<IntersectionComplete> g = {
        {"Quintique  P4[5]", {4}, {{5}}, 1, 101},
        {"P5[3,3]", {5}, {{3}, {3}}, 1, 73},
        {"P5[2,4]", {5}, {{2}, {4}}, 1, 89},
        {"P6[2,2,3]", {6}, {{2}, {2}, {3}}, 1, 73},
        {"P7[2,2,2,2]", {7}, {{2}, {2}, {2}, {2}}, 1, 65},
        {"Bicubique  P2xP2[3,3]", {2, 2}, {{3, 3}}, 2, 83},
        {"(P1)^4 [2,2,2,2]", {1, 1, 1, 1}, {{2, 2, 2, 2}}, 4, 68},
        {"Tian-Yau  P3xP3[(3,0),(0,3),(1,1)]", {3, 3}, {{3, 0}, {0, 3}, {1, 1}}, 14, 23},
    };
    return g;
}

// Modèles de « vraies » compactifications utilisés par le jeu « Univers à la carte ».
struct ModeleUnivers { std::string nom; int h11, h21; std::string note; };
inline const std::vector<ModeleUnivers>& modeles() {
    static const std::vector<ModeleUnivers> m = {
        {"Quintique", 1, 101, "le plus célèbre (Candelas et al. 1991)"},
        {"Bicubique", 2, 83, "P2xP2, deux modules de Kähler"},
        {"Tian-Yau", 14, 23, "9 générations : trop pour notre monde"},
        {"Tian-Yau / Z3", 6, 9, "quotient libre : 3 générations !"},
        {"Schoen", 19, 19, "chi = 0 : autant de fermions que d'antifermions"},
        {"Tore T^6", 3, 3, "trivial (pas de courbure), chi = 0"},
        {"P4[1,1,1,1,2][6]", 1, 103, "hypersurface pondérée, chi = -204"},
        {"Miroir de la quintique", 101, 1, "échange h11 <-> h21 : symétrie miroir"},
    };
    return m;
}
inline int generations(int h11, int h21) { return std::abs(h11 - h21); }  // |chi|/2

// ---------------------------------------------------------------- Hanson : coupe de la quintique de Fermat
// z1^n + z2^n = 1 ; z1 = w^k1 cos(z)^(2/n), z2 = w^k2 sin(z)^(2/n), z = x + i y.
inline std::vector<svg::Quad> maillage_hanson(int n = 5, int N = 14, double alpha = 0.6) {
    using cd = std::complex<double>;
    std::vector<svg::Quad> qs;
    const double PI = 3.14159265358979323846;
    auto point = [&](int k1, int k2, double x, double y) {
        cd z(x, y);
        cd c = std::pow(std::cos(z), 2.0 / n), s = std::pow(std::sin(z), 2.0 / n);
        cd z1 = std::polar(1.0, 2 * PI * k1 / n) * c, z2 = std::polar(1.0, 2 * PI * k2 / n) * s;
        return svg::V3{z1.real(), z2.real(), std::cos(alpha) * z1.imag() + std::sin(alpha) * z2.imag()};
    };
    for (int k1 = 0; k1 < n; ++k1)
        for (int k2 = 0; k2 < n; ++k2) {
            std::string col = svg::hsl(360.0 * ((k1 * 2 + k2 * 3) % n) / n + 20 * k1, 0.75, 0.42 + 0.06 * k2);
            for (int i = 0; i < N; ++i)
                for (int j = 0; j < N; ++j) {
                    double x0 = (PI / 2) * i / N, x1 = (PI / 2) * (i + 1) / N;
                    double y0 = -PI / 2 + PI * j / N, y1 = -PI / 2 + PI * (j + 1) / N;
                    svg::Quad q;
                    q.p[0] = point(k1, k2, x0, y0); q.p[1] = point(k1, k2, x1, y0);
                    q.p[2] = point(k1, k2, x1, y1); q.p[3] = point(k1, k2, x0, y1);
                    q.couleur = col; q.opa = 0.9;
                    qs.push_back(q);
                }
        }
    return qs;
}

}  // namespace calabi
