// spectre.hpp -- spectre de la corde quantique : dégénérescences, dimension critique, Hagedorn,
// T-dualité, trajectoires de Regge, amplitude de Veneziano, corde tournante.
#pragma once
#include <cmath>
#include <string>
#include <vector>
#include "physique.hpp"

namespace spectre {

using i128 = __int128;
using ld = long double;
inline std::string str(i128 v) {
    if (v == 0) return "0";
    bool neg = v < 0;
    if (neg) v = -v;
    std::string s;
    while (v > 0) { s += char('0' + (int)(v % 10)); v /= 10; }
    if (neg) s += '-';
    return std::string(s.rbegin(), s.rend());
}

// ------------------------------------------------------------------ dénombrement d'états
// d_k(N) : nombre d'états au niveau N pour k oscillateurs bosoniques = coeff de q^N dans prod (1-q^n)^-k
// Récurrence : N d(N) = k sum_{j=1..N} sigma(j) d(N-j)
template <class T>
std::vector<T> degenerescences(int k, int Nmax) {
    std::vector<T> d(Nmax + 1, T(0));
    std::vector<T> sig(Nmax + 1, T(0));
    for (int j = 1; j <= Nmax; ++j)
        for (int m = j; m <= Nmax; m += j) sig[m] += T(j);
    d[0] = T(1);
    for (int N = 1; N <= Nmax; ++N) {
        T s = 0;
        for (int j = 1; j <= N; ++j) s += sig[j] * d[N - j];
        d[N] = s * T(k) / T(N);
    }
    return d;
}

// Séries en t = q^{1/2} pour la supercorde (8 bosons + 8 fermions transverses en jauge du cône de lumière)
struct SuperSpectre {
    std::vector<i128> ns, r;  // ns[n], r[n] : dégénérescences au niveau alpha' m^2 = n
};
inline void mul1p(std::vector<i128>& a, int m, int sgn) {  // a *= (1 + sgn t^m)
    for (int k = (int)a.size() - 1; k >= m; --k) a[k] += sgn * a[k - m];
}
inline void div1m(std::vector<i128>& a, int m) {  // a /= (1 - t^m)
    for (int k = m; k < (int)a.size(); ++k) a[k] += a[k - m];
}
inline SuperSpectre supercorde(int nmax) {
    int M = 2 * nmax + 2;
    std::vector<i128> A(M + 1, 0), B(M + 1, 0), Rr(M + 1, 0);
    A[0] = B[0] = Rr[0] = 1;
    for (int n = 1; 2 * n - 1 <= M; ++n)
        for (int rep = 0; rep < 8; ++rep) { mul1p(A, 2 * n - 1, +1); mul1p(B, 2 * n - 1, -1); }
    for (int n = 1; 2 * n <= M; ++n)
        for (int rep = 0; rep < 8; ++rep) { div1m(A, 2 * n); div1m(B, 2 * n); div1m(Rr, 2 * n); mul1p(Rr, 2 * n, +1); }
    SuperSpectre s;
    for (int n = 0; n <= nmax; ++n) {
        s.ns.push_back((A[2 * n + 1] - B[2 * n + 1]) / 2);  // projection GSO
        s.r.push_back(8 * Rr[2 * n]);
    }
    return s;
}

// ------------------------------------------------------------------ dimension critique
// Ordonnée à l'origine (énergie de point zéro) : a = (D-2)/24 (bosonique), (D-2)/16 (NS de la supercorde)
inline double intercept_bosonique(int D) { return (D - 2) / 24.0; }
inline double intercept_NS(int D) { return (D - 2) / 16.0; }
// Le premier état excité est un vecteur sans masse (jauge de Lorentz) ssi a = 1 (bosonique) / 1/2 (NS).
inline int dimension_critique_bosonique() {
    for (int D = 3; D < 100; ++D)
        if (std::fabs(intercept_bosonique(D) - 1.0) < 1e-12) return D;
    return -1;
}
inline int dimension_critique_super() {
    for (int D = 3; D < 100; ++D)
        if (std::fabs(intercept_NS(D) - 0.5) < 1e-12) return D;
    return -1;
}

// Régularisation « zêta » de 1+2+3+... : sum n e^{-eps n} - 1/eps^2 -> -1/12
inline double somme_regularisee(double eps) {
    double s = 1.0 / (4 * std::sinh(eps / 2) * std::sinh(eps / 2));  // forme close de sum n e^{-eps n}
    return s - 1.0 / (eps * eps);
}
inline double somme_brute(double eps, int nmax) {
    double s = 0;
    for (int n = 1; n <= nmax; ++n) s += n * std::exp(-eps * n);
    return s - 1.0 / (eps * eps);
}

// ------------------------------------------------------------------ masses (unités alpha' = 1/M_s^2)
inline double m2_ouverte(int N, int D = 26) { return (N - intercept_bosonique(D)); }             // alpha' m^2
inline double m2_fermee(int N, int D = 26) { return 4.0 * (N - intercept_bosonique(D)); }        // alpha' m^2 (N = N~)
inline double m2_super_ouverte(int n) { return n; }
inline double m2_super_fermee(int n) { return 4.0 * n; }

// ------------------------------------------------------------------ Hagedorn
struct Hagedorn {
    std::vector<double> m, S;  // m sqrt(alpha'), entropie S = ln(degenerescence) (corde fermee : d(N)^2)
};
inline Hagedorn hagedorn_fermee(int Nmax) {
    auto d = degenerescences<ld>(24, Nmax);
    Hagedorn h;
    for (int N = 2; N <= Nmax; ++N) {
        h.m.push_back(2 * std::sqrt((double)(N - 1)));
        h.S.push_back((double)(2 * std::log(d[N])));
    }
    return h;
}
// pente locale dS/dm = beta_H / sqrt(alpha')  ; T_H = 1/beta_H
inline double pente_hagedorn(const Hagedorn& h, size_t i, size_t j) { return (h.S[j] - h.S[i]) / (h.m[j] - h.m[i]); }
const double BETA_H = 4 * phys::PI;  // en unités sqrt(alpha')

// ------------------------------------------------------------------ compactification sur un cercle
// Corde fermée bosonique : alpha' m^2 = alpha'(n/R)^2 + (wR/alpha')^2 alpha' + 2(N+N~-2), N-N~ = n w.
// On utilise alpha' = 1 : m^2 = n^2/R^2 + w^2 R^2 + 2(N + N~ - 2).
inline double m2_cercle(int n, int w, int N, int Nt, double R) {
    return n * n / (R * R) + w * w * R * R + 2.0 * (N + Nt - 2);
}
inline bool niveaux_appaires(int n, int w, int N, int Nt) { return N - Nt == n * w; }
inline double dual(double R) { return 1.0 / R; }  // T-dualité : R -> alpha'/R (alpha'=1)

// ------------------------------------------------------------------ Regge
struct Hadron { const char* nom; int J; double m_GeV; };
inline const std::vector<Hadron>& hadrons_rho() {
    // trajectoire principale ρ/a2/ρ3/a4/ρ5 (masses PDG arrondies, J = spin)
    static const std::vector<Hadron> h = {{"ρ(770)", 1, 0.775}, {"a2(1320)", 2, 1.318}, {"ρ3(1690)", 3, 1.689},
                                          {"a4(2040)", 4, 1.996}, {"ρ5(2350)", 5, 2.330}};
    return h;
}
struct Regression { double pente, ordonnee, r2; };
inline Regression regression(const std::vector<double>& x, const std::vector<double>& y) {
    double n = x.size(), sx = 0, sy = 0, sxx = 0, sxy = 0, syy = 0;
    for (size_t i = 0; i < x.size(); ++i) { sx += x[i]; sy += y[i]; sxx += x[i] * x[i]; sxy += x[i] * y[i]; syy += y[i] * y[i]; }
    double p = (n * sxy - sx * sy) / (n * sxx - sx * sx), o = (sy - p * sx) / n;
    double ssr = 0, sst = 0, my = sy / n;
    for (size_t i = 0; i < x.size(); ++i) { ssr += (y[i] - (o + p * x[i])) * (y[i] - (o + p * x[i])); sst += (y[i] - my) * (y[i] - my); }
    return {p, o, 1 - ssr / sst};
}
// Corde classique ouverte tournante (extrémités à la vitesse c), tension T = 1/(2 pi alpha') :
// E = 2T int_0^l dr/sqrt(1-(r/l)^2), J = 2T int_0^l r (r/l)/sqrt(1-(r/l)^2) dr. On retrouve J = alpha' E^2.
inline double regge_corde_tournante(double alpha_p, double l = 1.0, int pas = 200000) {
    double T = 1.0 / (2 * phys::PI * alpha_p), E = 0, J = 0, dth = (phys::PI / 2) / pas;
    for (int i = 0; i < pas; ++i) {  // r = l sin(theta)
        double th = (i + 0.5) * dth, r = l * std::sin(th);
        E += 2 * T * l * dth;  // dr/sqrt(1-(r/l)^2) = l dtheta
        J += 2 * T * r * (r / l) * l * dth;
    }
    return J / (E * E);  // doit valoir alpha'
}

// ------------------------------------------------------------------ Veneziano (1968)
inline double beta_fn(double a, double b) { return std::tgamma(a) * std::tgamma(b) / std::tgamma(a + b); }
inline double alpha_traj(double s, double a0 = 0.5, double ap = 0.9) { return a0 + ap * s; }
inline double veneziano(double s, double t, double a0 = 0.5, double ap = 0.9) {
    return beta_fn(-alpha_traj(s, a0, ap), -alpha_traj(t, a0, ap));
}

// ------------------------------------------------------------------ BH <-> corde
// Entropie d'une corde de masse m : S = beta_H m ; celle d'un trou noir de même masse (D=4) : S = 4 pi G m^2 /(hbar c).
// Point de correspondance (Horowitz-Polchinski) : m ~ M_s/g_s^2  (à des facteurs O(1) près).
inline double masse_correspondance_GeV(double Ms_GeV, double gs) { return Ms_GeV / (gs * gs); }

}  // namespace spectre
