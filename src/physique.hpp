// physique.hpp -- constantes (CODATA 2018), conversions d'unités et formules de physique « haute énergie ».
#pragma once
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

namespace phys {

const double PI = 3.14159265358979323846;
const double hbar = 1.054571817e-34;   // J s
const double c = 299792458.0;          // m/s
const double G = 6.67430e-11;          // m^3 kg^-1 s^-2
const double kB = 1.380649e-23;        // J/K
const double eV = 1.602176634e-19;     // J
const double GeV = 1e9 * eV;
const double hbarc_GeVm = hbar * c / GeV;  // 1.973e-16 GeV.m  (1 GeV^-1 = 1.973e-16 m)
const double M_sun = 1.98847e30, M_terre = 5.9722e24, R_sun = 6.957e8;
const double annee = 3.15576e7;                            // s
const double l_planck = std::sqrt(hbar * G / (c * c * c));  // 1.616e-35 m
const double m_planck = std::sqrt(hbar * c / G);            // 2.176e-8 kg
const double M_planck_GeV = m_planck * c * c / GeV;         // 1.221e19 GeV
const double T_planck = std::sqrt(hbar * c * c * c * c * c / G) / kB;  // 1.417e32 K
const double GeV_en_K = GeV / kB;                           // 1 GeV = 1.16e13 K
const double LHC_GeV = 13600.0;                             // énergie dans le centre de masse (2022-)

// Notation scientifique lisible : 1.23e+05
inline std::string sci(double v, int prec = 3) {
    std::ostringstream o;
    o << std::scientific << std::setprecision(prec) << v;
    return o.str();
}
// Nombre « intelligent » : décimal si raisonnable, scientifique sinon.
inline std::string nb(double v, int prec = 4) {
    std::ostringstream o;
    double a = std::fabs(v);
    if (a == 0) return "0";
    if (a >= 1e6 || a < 1e-3) o << std::scientific << std::setprecision(prec - 1) << v;
    else if (a >= 1e3) o << std::fixed << std::setprecision(0) << v;
    else o << std::setprecision(prec) << v;
    return o.str();
}
// Longueur en mètres avec une échelle parlante.
inline std::string longueur(double m) {
    std::ostringstream o;
    o << std::setprecision(3);
    double a = std::fabs(m);
    if (a >= 9.4607e15) o << m / 9.4607e15 << " années-lumière";
    else if (a >= 1.496e11) o << m / 1.496e11 << " UA";
    else if (a >= 1e3) o << m / 1e3 << " km";
    else if (a >= 1e-2) o << m << " m";
    else if (a >= 1e-3) o << m * 1e3 << " mm";
    else if (a >= 1e-6) o << m * 1e6 << " µm";
    else if (a >= 1e-9) o << m * 1e9 << " nm";
    else if (a >= 1e-15) o << m * 1e15 << " fm";
    else o << sci(m, 2) << " m";
    return o.str();
}
inline std::string duree(double s) {
    std::ostringstream o;
    o << std::setprecision(3);
    if (s >= annee * 1e3) o << sci(s / annee, 2) << " ans";
    else if (s >= annee) o << s / annee << " ans";
    else if (s >= 86400) o << s / 86400 << " jours";
    else if (s >= 1) o << s << " s";
    else o << sci(s, 2) << " s";
    return o.str();
}

// ---- Conversions énergie <-> longueur (unités naturelles, ħ=c=1) ----
inline double GeVinv_en_m(double inv_GeV) { return inv_GeV * hbarc_GeVm; }
inline double m_en_GeVinv(double m) { return m / hbarc_GeVm; }

// ---- Corde ----
// Pente de Regge alpha' [GeV^-2] <-> tension T = 1/(2 pi alpha') [GeV^2] (= GeV/fm apres conversion)
inline double tension_GeV2(double alpha_p) { return 1.0 / (2 * PI * alpha_p); }
inline double alpha_prime(double Ms_GeV) { return 1.0 / (Ms_GeV * Ms_GeV); }  // convention M_s = 1/sqrt(alpha')
inline double tension_newton(double Ms_GeV) {  // T = M_s^2/(2pi) en unités SI : (E/L)
    double T_GeV2 = Ms_GeV * Ms_GeV / (2 * PI);
    return T_GeV2 * GeV / (hbarc_GeVm);  // GeV^2 = GeV / GeV^-1 -> J / m
}

// ---- Trous noirs (Schwarzschild) ----
inline double rayon_schwarzschild(double M) { return 2 * G * M / (c * c); }
inline double T_hawking(double M) { return hbar * c * c * c / (8 * PI * G * M * kB); }
inline double S_bh_sur_k(double M) { return 4 * PI * G * M * M / (hbar * c); }  // A/(4 l_P^2)
inline double t_evaporation(double M) { return 5120 * PI * G * G * M * M * M / (hbar * c * c * c * c); }

// ---- Cordes cosmiques ----
inline double deficit_angle_rad(double Gmu) { return 8 * PI * Gmu; }  // delta = 8 pi G mu / c^2

// ---- Casimir ----
inline double pression_casimir(double d) { return -PI * PI * hbar * c / (240 * std::pow(d, 4)); }  // N/m^2

// ---- Dimensions supplémentaires (ADD) : M_Pl^2 ~ M_*^(n+2) R^n ----
inline double rayon_ADD_m(int n, double Mstar_GeV) {
    double R_inv_GeV = (1.0 / Mstar_GeV) * std::pow(M_planck_GeV / Mstar_GeV, 2.0 / n);
    return GeVinv_en_m(R_inv_GeV);
}

// ---- AdS/CFT ----
const double eta_sur_s = 1.0 / (4 * PI);  // en unités ħ/k_B
inline double eta_sur_s_SI() { return hbar / (4 * PI * kB); }  // K.s
inline double gamma_quart() {  // Gamma(1/4)^4
    double g = std::tgamma(0.25);
    return g * g * g * g;
}
// Potentiel quark-antiquark à fort couplage : V(L) = -4 pi^2 sqrt(lambda) / (Gamma(1/4)^4 L)
inline double potentiel_qqbar(double lambda, double L) { return -4 * PI * PI * std::sqrt(lambda) / (gamma_quart() * L); }

}  // namespace phys
