// corde.hpp -- dynamique de la corde : équation d'onde discrétisée (schéma saute-mouton exact à CFL=1),
// modes propres, corde dans un espace à 4 dimensions spatiales, rendu ASCII.
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>
#include "physique.hpp"

namespace corde {

const double PI = phys::PI;

// ---------------------------------------------------------------- corde 1D aux extrémités fixes (L = 1, c = 1)
struct Corde1D {
    int N;                     // nombre d'intervalles
    double dx, dt;             // dt = r dx, r <= 1 (stabilité de Courant)
    double r;
    std::vector<double> u_prec, u, u_suiv;
    double t = 0;

    explicit Corde1D(int N_ = 400, double r_ = 1.0) : N(N_), dx(1.0 / N_), dt(r_ / N_), r(r_) {
        u_prec.assign(N + 1, 0); u.assign(N + 1, 0); u_suiv.assign(N + 1, 0);
    }
    // Pincement triangulaire en x0 avec la flèche h, corde initialement au repos.
    void pincer(double x0, double h) {
        for (int i = 0; i <= N; ++i) {
            double x = i * dx;
            u[i] = x <= x0 ? h * x / x0 : h * (1 - x) / (1 - x0);
        }
        u[0] = u[N] = 0;
        // premier pas : u^1 = u^0 + (r^2/2) d2u  (vitesse initiale nulle)
        for (int i = 1; i < N; ++i) u_suiv[i] = u[i] + 0.5 * r * r * (u[i + 1] - 2 * u[i] + u[i - 1]);
        u_prec = u;
        u = u_suiv;
        u[0] = u[N] = 0;
        t = dt;
    }
    void pas() {
        for (int i = 1; i < N; ++i) u_suiv[i] = 2 * u[i] - u_prec[i] + r * r * (u[i + 1] - 2 * u[i] + u[i - 1]);
        u_suiv[0] = u_suiv[N] = 0;
        u_prec.swap(u);
        u.swap(u_suiv);
        t += dt;
    }
    // Énergie discrète exactement conservée : cinétique moyenne + potentiel croisé.
    double energie() const {
        double e = 0;
        for (int i = 1; i < N; ++i) {
            double v = (u[i] - u_prec[i]) / dt;
            e += 0.5 * v * v * dx;
        }
        for (int i = 0; i < N; ++i) e += 0.5 * (u[i + 1] - u[i]) * (u_prec[i + 1] - u_prec[i]) / dx;
        return e;
    }
};

// Solution analytique : b_n = 2h sin(n pi x0) / (n^2 pi^2 x0 (1-x0))
inline double amplitude_mode(int n, double x0, double h = 1.0) {
    double sn = std::sin(n * PI * x0);
    if (std::fabs(sn) < 1e-12) sn = 0;  // nœud exact : le mode est éteint
    return 2 * h * sn / (n * n * PI * PI * x0 * (1 - x0));
}
inline double u_analytique(double x, double t, double x0, double h = 1.0, int nmax = 400) {
    double s = 0;
    for (int n = 1; n <= nmax; ++n) s += amplitude_mode(n, x0, h) * std::sin(n * PI * x) * std::cos(n * PI * t);
    return s;
}
// Énergie portée par le mode n : E_n proportionnelle à n^2 b_n^2 (pour L=1, mu=1 : E_n = (n pi)^2 b_n^2 / 4)
inline double energie_mode(int n, double x0, double h = 1.0) {
    double b = amplitude_mode(n, x0, h);
    return (n * PI) * (n * PI) * b * b / 4;
}
inline double energie_analytique_totale(double x0, double h = 1.0) { return h * h / (2 * x0 * (1 - x0)); }
// Fréquence du mode n : f_n = n c / (2L)
inline double frequence_mode(int n, double c = 1.0, double L = 1.0) { return n * c / (2 * L); }

// ---------------------------------------------------------------- corde dans R^4 (l'original, en mieux)
// Coordonnées X^mu(sigma, tau), mu = 0..3 : chacune est une superposition de modes cos(n sigma) cos(n tau + phi).
struct Corde4D {
    struct Mode { int mu, n; double a, phi; };
    std::vector<Mode> modes;
    bool fermee = false;
    int points = 100;

    static Corde4D exemple(bool fermee_ = false) {
        Corde4D c;
        c.fermee = fermee_;
        c.modes = {{0, 1, 1.0, 0.0}, {1, 1, 0.8, PI / 2}, {2, 2, 0.5, 0.0}, {3, 3, 0.35, PI / 3}, {1, 2, 0.25, 1.0}, {3, 1, 0.3, 2.0}};
        return c;
    }
    std::vector<std::array<double, 4>> position(double tau) const {
        std::vector<std::array<double, 4>> P(points);
        for (int i = 0; i < points; ++i) {
            double sigma = fermee ? 2 * PI * i / points : PI * i / (points - 1);
            std::array<double, 4> X{0, 0, 0, 0};
            for (auto& m : modes) X[m.mu] += m.a * std::cos(m.n * sigma + (fermee ? m.phi : 0.0)) * std::cos(m.n * tau + m.phi);
            P[i] = X;
        }
        return P;
    }
};

// Rotation 4D dans le plan (axe a, axe b)
inline std::array<double, 4> rotation4(std::array<double, 4> p, int a, int b, double th) {
    double c = std::cos(th), s = std::sin(th);
    double pa = p[a] * c - p[b] * s, pb = p[a] * s + p[b] * c;
    p[a] = pa; p[b] = pb;
    return p;
}
// Projection perspective 4D -> 3D (caméra sur l'axe w) : x' = x d/(d - w)
inline std::array<double, 3> projeter4_3(const std::array<double, 4>& p, double d = 4.0) {
    double k = d / (d - p[3]);
    return {p[0] * k, p[1] * k, p[2] * k};
}

// ---------------------------------------------------------------- rendu ASCII
inline std::string ascii_courbe(const std::vector<double>& y, int larg = 72, int haut = 15, double ymax = 0) {
    if (ymax <= 0) for (double v : y) ymax = std::max(ymax, std::fabs(v));
    if (ymax == 0) ymax = 1;
    std::vector<std::string> g(haut, std::string(larg, ' '));
    int mid = haut / 2;
    for (int x = 0; x < larg; ++x) g[mid][x] = '-';
    for (int x = 0; x < larg; ++x) {
        double v = y[(size_t)((double)x / (larg - 1) * (y.size() - 1))];
        int row = mid - (int)std::lround(v / ymax * (mid));
        row = std::clamp(row, 0, haut - 1);
        g[row][x] = '*';
    }
    std::string s;
    for (auto& l : g) s += l + "\n";
    return s;
}
inline std::string barre(double v, double vmax, int larg = 40) {
    int k = vmax > 0 ? (int)std::lround(std::fabs(v) / vmax * larg) : 0;
    return std::string(std::min(k, larg), '#');
}

}  // namespace corde
