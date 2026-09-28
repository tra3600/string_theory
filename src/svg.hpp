// svg.hpp -- mini bibliothèque de dessin SVG (aucune dépendance) : courbes, barres, surfaces 3D.
#pragma once
#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace svg {

inline std::string f(double v) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(2) << v;
    return o.str();
}

inline std::string echapper(const std::string& s) {
    std::string r;
    for (char c : s) {
        if (c == '&') r += "&amp;";
        else if (c == '<') r += "&lt;";
        else if (c == '>') r += "&gt;";
        else r += c;
    }
    return r;
}

// Palette « nuit » lisible sur fond sombre.
const std::string FOND = "#0b1020", PANNEAU = "#131a33", GRILLE = "#2a3358", TEXTE = "#e6e9f5",
                  TEXTE2 = "#9aa4c7";
inline const std::vector<std::string>& palette() {
    static const std::vector<std::string> p = {"#5cc8ff", "#ff7eb6", "#ffd166", "#7ee081", "#c792ea", "#ff9f68"};
    return p;
}

struct Doc {
    double w, h;
    std::ostringstream b;
    Doc(double w_, double h_) : w(w_), h(h_) {}
    void rect(double x, double y, double ww, double hh, const std::string& fill, const std::string& stroke = "none",
              double rx = 0) {
        b << "<rect x='" << f(x) << "' y='" << f(y) << "' width='" << f(ww) << "' height='" << f(hh) << "' rx='"
          << f(rx) << "' fill='" << fill << "' stroke='" << stroke << "'/>\n";
    }
    void line(double x1, double y1, double x2, double y2, const std::string& col, double ep = 1,
              const std::string& dash = "") {
        b << "<line x1='" << f(x1) << "' y1='" << f(y1) << "' x2='" << f(x2) << "' y2='" << f(y2) << "' stroke='" << col
          << "' stroke-width='" << f(ep) << "'";
        if (!dash.empty()) b << " stroke-dasharray='" << dash << "'";
        b << "/>\n";
    }
    void circle(double cx, double cy, double r, const std::string& fill, const std::string& stroke = "none") {
        b << "<circle cx='" << f(cx) << "' cy='" << f(cy) << "' r='" << f(r) << "' fill='" << fill << "' stroke='"
          << stroke << "'/>\n";
    }
    void text(double x, double y, const std::string& t, double taille = 13, const std::string& col = TEXTE,
              const std::string& ancre = "start", bool gras = false, double rot = 0) {
        b << "<text x='" << f(x) << "' y='" << f(y) << "' font-size='" << f(taille) << "' fill='" << col
          << "' text-anchor='" << ancre << "' font-family='Helvetica,Arial,sans-serif'";
        if (gras) b << " font-weight='bold'";
        if (rot != 0) b << " transform='rotate(" << f(rot) << " " << f(x) << " " << f(y) << ")'";
        b << ">" << echapper(t) << "</text>\n";
    }
    void polyline(const std::vector<std::pair<double, double>>& pts, const std::string& col, double ep = 2,
                  const std::string& dash = "") {
        if (pts.size() < 2) return;
        b << "<polyline fill='none' stroke='" << col << "' stroke-width='" << f(ep)
          << "' stroke-linejoin='round' stroke-linecap='round'";
        if (!dash.empty()) b << " stroke-dasharray='" << dash << "'";
        b << " points='";
        for (auto& p : pts) b << f(p.first) << "," << f(p.second) << " ";
        b << "'/>\n";
    }
    void polygon(const std::vector<std::pair<double, double>>& pts, const std::string& fill,
                 const std::string& stroke = "none", double opa = 1, double ep = 0.4) {
        b << "<polygon fill='" << fill << "' fill-opacity='" << f(opa) << "' stroke='" << stroke << "' stroke-width='"
          << f(ep) << "' stroke-linejoin='round' points='";
        for (auto& p : pts) b << f(p.first) << "," << f(p.second) << " ";
        b << "'/>\n";
    }
    void brut(const std::string& s) { b << s; }
    std::string rendre() const {
        std::ostringstream o;
        o << "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 " << f(w) << " " << f(h) << "' width='" << f(w)
          << "' height='" << f(h) << "'>\n<rect width='100%' height='100%' fill='" << FOND << "'/>\n"
          << b.str() << "</svg>\n";
        return o.str();
    }
};

inline bool ecrire(const std::string& chemin, const std::string& contenu) {
    std::ofstream o(chemin);
    if (!o) return false;
    o << contenu;
    return bool(o);
}

// Couleur HSL -> "#rrggbb"
inline std::string hsl(double h, double s, double l) {
    h = std::fmod(std::fmod(h, 360.0) + 360.0, 360.0);
    double c = (1 - std::fabs(2 * l - 1)) * s, x = c * (1 - std::fabs(std::fmod(h / 60.0, 2) - 1)), m = l - c / 2;
    double r = 0, g = 0, b = 0;
    if (h < 60) { r = c; g = x; }
    else if (h < 120) { r = x; g = c; }
    else if (h < 180) { g = c; b = x; }
    else if (h < 240) { g = x; b = c; }
    else if (h < 300) { r = x; b = c; }
    else { r = c; b = x; }
    char buf[16];
    std::snprintf(buf, sizeof buf, "#%02x%02x%02x", int((r + m) * 255 + .5), int((g + m) * 255 + .5),
                  int((b + m) * 255 + .5));
    return buf;
}

// ---------------------------------------------------------------- graphiques
struct Serie {
    std::string nom, couleur;
    std::vector<double> x, y;
    bool ligne = true, points = false;
    double epaisseur = 2.2;
    std::string tiret;
};
struct Repere {  // droite verticale/horizontale annotée
    bool vertical;
    double valeur;
    std::string texte, couleur;
};
struct Note {
    double x, y;
    std::string texte, couleur;
};

struct Graphique {
    std::string titre, sous_titre, xlabel, ylabel;
    bool logx = false, logy = false;
    double xmin = NAN, xmax = NAN, ymin = NAN, ymax = NAN;
    std::vector<Serie> series;
    std::vector<Repere> reperes;
    std::vector<Note> notes;

    static std::vector<double> graduations(double a, double b, bool log) {
        std::vector<double> t;
        if (log) {
            int pas = std::max(1, (int)std::ceil((b - a) / 9.0));
            for (int e = (int)std::ceil(a - 1e-9); e <= (int)std::floor(b + 1e-9); e += pas) t.push_back(e);
            return t;
        }
        double pas = std::pow(10.0, std::floor(std::log10((b - a) / 6)));
        double r = (b - a) / 6 / pas;
        pas *= (r < 1.5 ? 1 : r < 3.5 ? 2 : r < 7.5 ? 5 : 10);
        for (double v = std::ceil(a / pas) * pas; v <= b + pas * 1e-9; v += pas) t.push_back(std::fabs(v) < pas * 1e-9 ? 0 : v);
        return t;
    }
    static std::string etiquette(double v, bool log) {
        std::ostringstream o;
        if (log) {
            if (v >= -2 && v <= 3) { o << std::setprecision(6) << std::pow(10.0, v); }
            else o << "1e" << (int)v;
        } else {
            double a = std::fabs(v);
            if (a != 0 && (a >= 1e5 || a < 1e-3)) o << std::scientific << std::setprecision(1) << v;
            else o << std::setprecision(6) << v;
        }
        return o.str();
    }

    std::string rendre(double W = 900, double H = 560) const {
        Doc d(W, H);
        const double gl = 82, gr = 30, gt = 66, gb = 66;
        double pw = W - gl - gr, ph = H - gt - gb;
        auto T = [&](double v, bool lg) { return lg ? std::log10(v) : v; };
        double x0 = xmin, x1 = xmax, y0 = ymin, y1 = ymax;
        double ax = 1e300, bx = -1e300, ay = 1e300, by = -1e300;
        for (auto& s : series)
            for (size_t i = 0; i < s.x.size(); ++i) {
                if (!std::isfinite(s.x[i]) || !std::isfinite(s.y[i])) continue;
                if ((logx && s.x[i] <= 0) || (logy && s.y[i] <= 0)) continue;
                ax = std::min(ax, T(s.x[i], logx)); bx = std::max(bx, T(s.x[i], logx));
                ay = std::min(ay, T(s.y[i], logy)); by = std::max(by, T(s.y[i], logy));
            }
        if (ax > bx) { ax = 0; bx = 1; }
        if (ay > by) { ay = 0; by = 1; }
        if (std::isnan(x0)) x0 = ax; else x0 = T(x0, logx);
        if (std::isnan(x1)) x1 = bx; else x1 = T(x1, logx);
        if (std::isnan(y0)) y0 = ay - (logy ? 0 : 0.05 * (by - ay)); else y0 = T(y0, logy);
        if (std::isnan(y1)) y1 = by + (logy ? 0 : 0.05 * (by - ay)); else y1 = T(y1, logy);
        if (x1 <= x0) x1 = x0 + 1;
        if (y1 <= y0) y1 = y0 + 1;
        auto PX = [&](double v) { return gl + (T(v, logx) - x0) / (x1 - x0) * pw; };
        auto PY = [&](double v) { return gt + ph - (T(v, logy) - y0) / (y1 - y0) * ph; };
        d.rect(gl, gt, pw, ph, PANNEAU, GRILLE, 4);
        d.text(W / 2, 30, titre, 20, TEXTE, "middle", true);
        if (!sous_titre.empty()) d.text(W / 2, 50, sous_titre, 12.5, TEXTE2, "middle");
        for (double v : graduations(x0, x1, logx)) {
            double px = gl + (v - x0) / (x1 - x0) * pw;
            if (px < gl - 1 || px > gl + pw + 1) continue;
            d.line(px, gt, px, gt + ph, GRILLE, 1);
            d.text(px, gt + ph + 18, etiquette(v, logx), 12, TEXTE2, "middle");
        }
        for (double v : graduations(y0, y1, logy)) {
            double py = gt + ph - (v - y0) / (y1 - y0) * ph;
            if (py < gt - 1 || py > gt + ph + 1) continue;
            d.line(gl, py, gl + pw, py, GRILLE, 1);
            d.text(gl - 8, py + 4, etiquette(v, logy), 12, TEXTE2, "end");
        }
        d.text(gl + pw / 2, H - 14, xlabel, 14, TEXTE, "middle");
        d.text(20, gt + ph / 2, ylabel, 14, TEXTE, "middle", false, -90);
        d.b << "<clipPath id='c'><rect x='" << f(gl) << "' y='" << f(gt) << "' width='" << f(pw) << "' height='"
            << f(ph) << "'/></clipPath><g clip-path='url(#c)'>\n";
        for (auto& r : reperes) {
            if (r.vertical) d.line(PX(r.valeur), gt, PX(r.valeur), gt + ph, r.couleur, 1.5, "6 4");
            else d.line(gl, PY(r.valeur), gl + pw, PY(r.valeur), r.couleur, 1.5, "6 4");
        }
        for (auto& s : series) {
            if (s.ligne) {
                std::vector<std::pair<double, double>> seg;
                for (size_t i = 0; i < s.x.size(); ++i) {
                    bool ok = std::isfinite(s.x[i]) && std::isfinite(s.y[i]) && !(logx && s.x[i] <= 0) &&
                              !(logy && s.y[i] <= 0);
                    if (ok) seg.push_back({PX(s.x[i]), PY(s.y[i])});
                    if (!ok || i + 1 == s.x.size()) { d.polyline(seg, s.couleur, s.epaisseur, s.tiret); seg.clear(); }
                }
            }
            if (s.points)
                for (size_t i = 0; i < s.x.size(); ++i)
                    if (std::isfinite(s.x[i]) && std::isfinite(s.y[i]) && !(logx && s.x[i] <= 0) && !(logy && s.y[i] <= 0))
                        d.circle(PX(s.x[i]), PY(s.y[i]), 4.5, s.couleur, FOND);
        }
        d.b << "</g>\n";
        for (auto& r : reperes)
            if (!r.texte.empty()) {
                if (r.vertical) d.text(PX(r.valeur) + 5, gt + 16, r.texte, 12, r.couleur);
                else d.text(gl + pw - 6, PY(r.valeur) - 6, r.texte, 12, r.couleur, "end");
            }
        for (auto& n : notes) d.text(PX(n.x), PY(n.y), n.texte, 12, n.couleur.empty() ? TEXTE : n.couleur);
        // légende
        double ly = gt + 14, lx = gl + 12;
        int nl = 0;
        for (auto& s : series) if (!s.nom.empty()) ++nl;
        if (nl) {
            double wl = 0;
            for (auto& s : series) wl = std::max(wl, (double)s.nom.size());
            d.rect(lx - 6, ly - 12, wl * 6.6 + 40, nl * 19 + 6, FOND, GRILLE, 4);
            for (auto& s : series) {
                if (s.nom.empty()) continue;
                d.line(lx, ly - 3, lx + 20, ly - 3, s.couleur, 3, s.tiret);
                d.text(lx + 26, ly + 1, s.nom, 12, TEXTE);
                ly += 19;
            }
        }
        return d.rendre();
    }
};

// Diagramme en barres (catégories).
inline std::string barres(const std::string& titre, const std::string& sous, const std::string& ylabel,
                          const std::vector<std::string>& noms, const std::vector<double>& val,
                          const std::vector<std::string>& couleurs, double repere = NAN,
                          const std::string& txt_repere = "", bool log = false, double W = 900, double H = 560) {
    Doc d(W, H);
    const double gl = 82, gr = 30, gt = 66, gb = 110;
    double pw = W - gl - gr, ph = H - gt - gb;
    double lo = 0, hi = 1;
    if (log) { lo = 1e300; hi = -1e300; }
    for (double v : val) {
        if (log && v <= 0) continue;
        double t = log ? std::log10(v) : v;
        lo = std::min(lo, t); hi = std::max(hi, t);
    }
    if (log) { lo = std::floor(lo) - 0; hi = std::ceil(hi); } else { lo = std::min(lo, 0.0); hi *= 1.1; if (hi <= lo) hi = lo + 1; }
    auto PY = [&](double t) { return gt + ph - (t - lo) / (hi - lo) * ph; };
    d.rect(gl, gt, pw, ph, PANNEAU, GRILLE, 4);
    d.text(W / 2, 30, titre, 20, TEXTE, "middle", true);
    d.text(W / 2, 50, sous, 12.5, TEXTE2, "middle");
    for (double v : Graphique::graduations(lo, hi, log)) {
        double py = PY(v);
        if (py < gt - 1 || py > gt + ph + 1) continue;
        d.line(gl, py, gl + pw, py, GRILLE, 1);
        d.text(gl - 8, py + 4, Graphique::etiquette(v, log), 12, TEXTE2, "end");
    }
    d.text(20, gt + ph / 2, ylabel, 14, TEXTE, "middle", false, -90);
    double bw = pw / noms.size();
    for (size_t i = 0; i < noms.size(); ++i) {
        double t = log ? (val[i] > 0 ? std::log10(val[i]) : lo) : val[i];
        double yy = PY(t), y0 = PY(log ? lo : 0);
        std::string c = couleurs.empty() ? palette()[i % palette().size()] : couleurs[i % couleurs.size()];
        d.rect(gl + i * bw + bw * 0.16, std::min(yy, y0), bw * 0.68, std::fabs(y0 - yy), c, "none", 3);
        std::ostringstream o;
        double a = std::fabs(val[i]);
        if (a != 0 && (a >= 1e5 || a < 1e-2)) o << std::scientific << std::setprecision(1) << val[i];
        else o << std::setprecision(4) << val[i];
        d.text(gl + (i + .5) * bw, std::min(yy, y0) - 6, o.str(), 11.5, TEXTE, "middle");
        d.text(gl + (i + .5) * bw, gt + ph + 16, noms[i], 11, TEXTE2, "end", false, -30);
    }
    if (!std::isnan(repere)) {
        double py = PY(log ? std::log10(repere) : repere);
        d.line(gl, py, gl + pw, py, "#ff9f68", 1.6, "6 4");
        d.text(gl + pw - 6, py - 6, txt_repere, 12, "#ff9f68", "end");
    }
    return d.rendre();
}

// ---------------------------------------------------------------- géométrie 3D
struct V3 { double x, y, z; };
inline V3 rot(V3 p, double ax, double ay) {  // rotation autour de x puis y
    double c = std::cos(ax), s = std::sin(ax);
    V3 q{p.x, p.y * c - p.z * s, p.y * s + p.z * c};
    c = std::cos(ay); s = std::sin(ay);
    return {q.x * c + q.z * s, q.y, -q.x * s + q.z * c};
}
struct Quad {
    V3 p[4];
    std::string couleur;
    double opa = 1;
};
// Dessin painter's algorithm d'une liste de quadrilatères, projection perspective légère.
inline void dessiner_quads(Doc& d, std::vector<Quad> qs, double cx, double cy, double echelle, double ax, double ay,
                           double dist = 6.0, const std::string& contour = "#00000055") {
    struct Proj { double z; std::vector<std::pair<double, double>> pts; std::string c; double opa; };
    std::vector<Proj> ps;
    for (auto& q : qs) {
        Proj pr;
        pr.c = q.couleur; pr.opa = q.opa;
        double zm = 0;
        for (int i = 0; i < 4; ++i) {
            V3 r = rot(q.p[i], ax, ay);
            double k = dist / (dist + r.z);
            pr.pts.push_back({cx + echelle * r.x * k, cy - echelle * r.y * k});
            zm += r.z;
        }
        pr.z = zm / 4;
        ps.push_back(std::move(pr));
    }
    std::sort(ps.begin(), ps.end(), [](const Proj& a, const Proj& b) { return a.z > b.z; });
    for (auto& p : ps) d.polygon(p.pts, p.c, contour, p.opa, 0.3);
}

}  // namespace svg
