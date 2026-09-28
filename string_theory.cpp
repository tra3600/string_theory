// string_theory.cpp -- LE LABORATOIRE DES CORDES
//
// Un programme C++17 sans aucune dépendance qui explore la théorie des cordes :
// simulation de cordes vibrantes, spectre quantique, dimensions cachées (Calabi-Yau),
// dualités, études de cas chiffrées, jeux et 18 illustrations SVG générées par le code.
//
//   g++ -std=c++17 -O2 string_theory.cpp -o string_theory
//   ./string_theory --aide
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <unistd.h>

#include "src/calabi.hpp"
#include "src/cas.hpp"
#include "src/corde.hpp"
#include "src/figures.hpp"
#include "src/jeux.hpp"
#include "src/physique.hpp"
#include "src/spectre.hpp"
#include "src/svg.hpp"
#include "src/tests.hpp"

namespace {

void aide(std::ostream& o) {
    o << "LE LABORATOIRE DES CORDES — théorie des cordes en C++\n\n"
         "USAGE : string_theory [options]        (sans option : menu interactif)\n\n"
         "Études de cas (calculées à partir de valeurs physiques) :\n"
         "  --liste                    liste des cas, jeux et figures\n"
         "  --cas ID                   exécute un cas (répétable), ex. --cas regge --cas calabi\n"
         "  --tout                     exécute tous les cas\n"
         "  --Ms=GeV --gs=x --R=x      échelle de corde (défaut 5000), couplage (0.1), rayon en √α' (1)\n"
         "  --x0=x --Gmu=x --masse=kg  point de pincement, tension cosmique, masse d'un trou noir\n"
         "  --lambda=x --n=N           couplage de 't Hooft, nombre de dimensions supplémentaires\n\n"
         "Illustrations :\n"
         "  --figures DOSSIER          écrit les 18 illustrations SVG\n"
         "  --rapport DOSSIER          rapport HTML complet (figures + résultats des cas)\n\n"
         "Ludique :\n"
         "  --jeu quiz|accordeur|pincer|univers|trous_noirs\n"
         "  --quiz N                   quiz de N questions\n"
         "  --anim [N]                 animation ASCII de la corde (N images)\n"
         "  --vue4d                    corde fermée dans R⁴ en ASCII (rotation x-w)\n"
         "  --brut                     coordonnées 4D brutes (format de la première version)\n\n"
         "Divers : --test (auto-tests)   --graine N (jeux reproductibles)   --aide\n";
}

struct Options {
    cas::Params p;
    std::vector<std::string> cas_ids, jeux_ids;
    bool tout = false, liste = false, test = false, anim = false, brut = false, vue4d = false, aide = false;
    int anim_n = 24, quiz_n = 0;
    unsigned graine = std::random_device{}();
    std::string figures, rapport;
};

bool prefixe(const std::string& s, const char* pre, std::string& val) {
    size_t n = std::strlen(pre);
    if (s.compare(0, n, pre) == 0 && s.size() > n && s[n] == '=') { val = s.substr(n + 1); return true; }
    return false;
}

bool analyser(int argc, char** argv, Options& o, std::ostream& err) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i], v;
        auto suivant = [&](std::string& dest) {
            if (i + 1 >= argc) { err << "option " << a << " : valeur manquante\n"; return false; }
            dest = argv[++i];
            return true;
        };
        try {
            if (a == "--aide" || a == "-h" || a == "--help") o.aide = true;
            else if (a == "--liste") o.liste = true;
            else if (a == "--tout") o.tout = true;
            else if (a == "--test") o.test = true;
            else if (a == "--brut") o.brut = true;
            else if (a == "--vue4d") o.vue4d = true;
            else if (a == "--anim") { o.anim = true; if (i + 1 < argc && argv[i + 1][0] != '-') o.anim_n = std::max(1, std::stoi(argv[++i])); }
            else if (a == "--cas") { if (!suivant(v)) return false; o.cas_ids.push_back(v); }
            else if (a == "--jeu") { if (!suivant(v)) return false; o.jeux_ids.push_back(v); }
            else if (a == "--quiz") { if (!suivant(v)) return false; o.quiz_n = std::stoi(v); }
            else if (a == "--figures") { if (!suivant(o.figures)) return false; }
            else if (a == "--rapport") { if (!suivant(o.rapport)) return false; }
            else if (a == "--graine") { if (!suivant(v)) return false; o.graine = (unsigned)std::stoul(v); }
            else if (prefixe(a, "--Ms", v)) o.p.Ms = std::stod(v);
            else if (prefixe(a, "--gs", v)) o.p.gs = std::stod(v);
            else if (prefixe(a, "--R", v)) o.p.R = std::stod(v);
            else if (prefixe(a, "--x0", v)) o.p.x0 = std::stod(v);
            else if (prefixe(a, "--Gmu", v)) o.p.Gmu = std::stod(v);
            else if (prefixe(a, "--masse", v)) o.p.masse = std::stod(v);
            else if (prefixe(a, "--lambda", v)) o.p.lambda = std::stod(v);
            else if (prefixe(a, "--n", v)) o.p.n_extra = std::stoi(v);
            else { err << "option inconnue : " << a << "  (voir --aide)\n"; return false; }
        } catch (const std::exception&) {
            err << "valeur invalide pour " << a << "\n";
            return false;
        }
    }
    if (o.p.Ms <= 0 || o.p.gs <= 0 || o.p.R <= 0 || o.p.masse <= 0 || o.p.lambda <= 0 || o.p.n_extra < 1 || o.p.Gmu <= 0 ||
        o.p.x0 <= 0 || o.p.x0 >= 1) {
        err << "paramètres hors domaine : Ms, gs, R, masse, lambda, Gmu > 0 ; 0 < x0 < 1 ; n >= 1\n";
        return false;
    }
    return true;
}

void liste(std::ostream& o) {
    o << "ÉTUDES DE CAS (--cas ID) :\n";
    for (auto& c : cas::tous()) o << "  " << cas::pad(c.id, 12) << c.titre << " — " << c.resume << "\n";
    o << "\nJEUX (--jeu ID) :\n"
         "  quiz         le défi de la corde (20 questions)\n"
         "  accordeur    trouvez les deux rayons de compactification (T-dualité)\n"
         "  pincer       éteignez une harmonique en choisissant où pincer\n"
         "  univers      trouvez un Calabi-Yau à 3 générations\n"
         "  trous_noirs  estimez la température de Hawking\n\nILLUSTRATIONS (--figures DIR) :\n";
    for (auto& f : fig::toutes()) o << "  " << f.id << "  " << f.titre << "\n";
}

const cas::Cas* trouver(const std::string& id) {
    for (auto& c : cas::tous()) if (c.id == id) return &c;
    return nullptr;
}

bool lancer_jeu(jeux::Contexte& c, const std::string& id, int nq) {
    if (id == "quiz") jeux::quiz(c, nq > 0 ? nq : 10);
    else if (id == "accordeur") jeux::accordeur(c);
    else if (id == "pincer") jeux::pincer(c);
    else if (id == "univers") jeux::univers(c);
    else if (id == "trous_noirs") jeux::trous_noirs(c);
    else return false;
    return true;
}

int rapport(const std::string& dossier, const cas::Params& p) {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::create_directories(dossier, ec);
    if (fig::ecrire_toutes(dossier) < 0) {
        std::cerr << "impossible d'écrire dans " << dossier << "\n";
        return 1;
    }
    std::ostringstream h;
    h << "<!doctype html><html lang='fr'><head><meta charset='utf-8'>"
         "<meta name='viewport' content='width=device-width,initial-scale=1'><title>Le laboratoire des cordes</title><style>"
         ":root{--bg:#0b1020;--pn:#131a33;--tx:#e6e9f5;--t2:#9aa4c7;--ac:#5cc8ff}"
         "body{margin:0;background:var(--bg);color:var(--tx);font:16px/1.55 Helvetica,Arial,sans-serif}"
         "main{max-width:1000px;margin:auto;padding:0 16px 60px}h1{font-size:2.2em;margin:.8em 0 .1em}"
         "h2{margin-top:2.2em;border-bottom:1px solid #2a3358;padding-bottom:.2em}p.sub{color:var(--t2)}"
         "figure{margin:1.2em 0;background:var(--pn);border-radius:10px;padding:10px}figure img{width:100%;height:auto;border-radius:6px}"
         "figcaption{color:var(--t2);font-size:.92em;padding:6px 4px}"
         "pre{background:#070b17;border:1px solid #2a3358;border-radius:8px;padding:12px;overflow:auto;font-size:12.5px;line-height:1.4}"
         "nav a{color:var(--ac);margin-right:12px;white-space:nowrap}details{margin:.8em 0}summary{cursor:pointer;color:var(--ac);font-weight:bold}"
         "</style></head><body><main><h1>Le laboratoire des cordes</h1>"
         "<p class='sub'>Rapport généré par <code>string_theory --rapport</code> : chaque valeur est calculée par le programme "
         "(paramètres : M<sub>s</sub> = "
      << phys::nb(p.Ms) << " GeV, g<sub>s</sub> = " << phys::nb(p.gs) << ").</p><nav>";
    for (auto& f : fig::toutes()) h << "<a href='#" << f.id << "'>" << svg::echapper(f.titre) << "</a> ";
    h << "</nav><h2>Illustrations</h2>";
    for (auto& f : fig::toutes())
        h << "<figure id='" << f.id << "'><img src='" << f.id << ".svg' alt='" << svg::echapper(f.titre) << "'><figcaption><b>"
          << svg::echapper(f.titre) << "</b> — " << svg::echapper(f.legende) << "</figcaption></figure>\n";
    h << "<h2>Études de cas</h2>";
    for (auto& c : cas::tous()) {
        std::ostringstream o;
        c.executer(o, p);
        h << "<details><summary>" << svg::echapper(c.titre) << "</summary><pre>" << svg::echapper(o.str()) << "</pre></details>\n";
    }
    std::ostringstream t;
    tests::lancer(t);
    h << "<h2>Auto-tests</h2><pre>" << svg::echapper(t.str()) << "</pre></main></body></html>\n";
    if (!svg::ecrire(dossier + "/index.html", h.str())) return 1;
    std::cout << "rapport écrit : " << dossier << "/index.html  (" << fig::toutes().size() << " figures)\n";
    return 0;
}

void menu(jeux::Contexte& c, cas::Params& p) {
    std::ostream& o = c.out;
    o << "\n  ╔════════════════════════════════════════════╗\n"
         "  ║      LE LABORATOIRE DES CORDES (C++17)     ║\n"
         "  ╚════════════════════════════════════════════╝\n";
    for (;;) {
        o << "\n  [1] Études de cas   [2] Jeux   [3] Animation ASCII   [4] Vue 4D\n"
             "  [5] Illustrations (SVG)   [6] Régler les paramètres   [7] Auto-tests   [q] Quitter\n";
        std::string s;
        if (!jeux::ligne(c, "  > ", s) || s.empty() || s[0] == 'q' || s[0] == 'Q') break;
        if (s[0] == '1') {
            auto& L = cas::tous();
            for (size_t i = 0; i < L.size(); ++i) o << "   " << i + 1 << ". " << cas::pad(L[i].titre, 40) << L[i].resume << "\n";
            std::string r;
            if (!jeux::ligne(c, "   numéro (ou « t » pour tout) : ", r)) break;
            if (!r.empty() && (r[0] == 't' || r[0] == 'T')) { for (auto& k : L) k.executer(o, p); }
            else {
                int k = 0;
                try { k = std::stoi(r); } catch (...) {}
                if (k >= 1 && k <= (int)L.size()) L[k - 1].executer(o, p);
                else o << "   numéro invalide\n";
            }
        } else if (s[0] == '2') {
            o << "   1. quiz  2. accordeur  3. pincer  4. univers  5. trous noirs\n";
            std::string r;
            if (!jeux::ligne(c, "   jeu : ", r)) break;
            const char* ids[] = {"quiz", "accordeur", "pincer", "univers", "trous_noirs"};
            int k = 0;
            try { k = std::stoi(r); } catch (...) {}
            if (k >= 1 && k <= 5) lancer_jeu(c, ids[k - 1], 10);
            else o << "   choix invalide\n";
        } else if (s[0] == '3') jeux::animation(o, 24, isatty(STDOUT_FILENO));
        else if (s[0] == '4') {
            for (int k = 0; k < 4; ++k) jeux::vue4d(o, k * corde::PI / 4, 0.7);
        } else if (s[0] == '5') {
            std::string d;
            if (!jeux::ligne(c, "   dossier de sortie [figures] : ", d)) break;
            if (d.empty()) d = "figures";
            std::error_code ec;
            std::filesystem::create_directories(d, ec);
            int n = fig::ecrire_toutes(d);
            o << (n < 0 ? "   échec d'écriture\n" : "   " + std::to_string(n) + " illustrations écrites dans " + d + "/\n");
        } else if (s[0] == '6') {
            o << "   Ms=" << p.Ms << " GeV  gs=" << p.gs << "  R=" << p.R << "  x0=" << p.x0 << "  Gmu=" << p.Gmu << "  masse=" << p.masse
              << " kg  lambda=" << p.lambda << "  n=" << p.n_extra << "\n";
            std::string r;
            if (!jeux::ligne(c, "   modifier (ex. Ms=1000) ou vide : ", r) || r.empty()) continue;
            size_t eq = r.find('=');
            if (eq == std::string::npos) { o << "   format nom=valeur\n"; continue; }
            std::string k = r.substr(0, eq);
            double v = 0;
            try { v = std::stod(r.substr(eq + 1)); } catch (...) { o << "   valeur invalide\n"; continue; }
            if (v <= 0) { o << "   valeur positive requise\n"; continue; }
            if (k == "Ms") p.Ms = v; else if (k == "gs") p.gs = v; else if (k == "R") p.R = v;
            else if (k == "x0" && v < 1) p.x0 = v; else if (k == "Gmu") p.Gmu = v; else if (k == "masse") p.masse = v;
            else if (k == "lambda") p.lambda = v; else if (k == "n") p.n_extra = (int)v;
            else o << "   paramètre inconnu\n";
        } else if (s[0] == '7') tests::lancer(o);
        else o << "   choix inconnu\n";
    }
    o << "\nÀ bientôt, dans la 11e dimension !\n";
}

}  // namespace

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    Options opt;
    if (!analyser(argc, argv, opt, std::cerr)) return 2;
    if (opt.aide) { aide(std::cout); return 0; }
    if (opt.liste) { liste(std::cout); return 0; }
    if (opt.test) return tests::lancer(std::cout) == 0 ? 0 : 1;
    jeux::Contexte ctx(std::cin, std::cout, opt.graine);
    bool fait = false;
    if (!opt.figures.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(opt.figures, ec);
        int n = fig::ecrire_toutes(opt.figures);
        if (n < 0) { std::cerr << "impossible d'écrire dans " << opt.figures << "\n"; return 1; }
        std::cout << n << " illustrations écrites dans " << opt.figures << "/\n";
        fait = true;
    }
    if (!opt.rapport.empty()) { if (rapport(opt.rapport, opt.p) != 0) return 1; fait = true; }
    if (opt.brut) { jeux::sortie_brute(std::cout, 100, 10, 0.1); fait = true; }
    if (opt.vue4d) { for (int k = 0; k < 4; ++k) jeux::vue4d(std::cout, k * corde::PI / 4, 0.7); fait = true; }
    if (opt.anim) { jeux::animation(std::cout, opt.anim_n, isatty(STDOUT_FILENO)); fait = true; }
    if (opt.tout) { for (auto& c : cas::tous()) c.executer(std::cout, opt.p); fait = true; }
    for (auto& id : opt.cas_ids) {
        const cas::Cas* c = trouver(id);
        if (!c) { std::cerr << "cas inconnu : " << id << "  (voir --liste)\n"; return 2; }
        c->executer(std::cout, opt.p);
        fait = true;
    }
    bool quiz_demande = std::find(opt.jeux_ids.begin(), opt.jeux_ids.end(), "quiz") != opt.jeux_ids.end();
    if (opt.quiz_n > 0 && !quiz_demande) { jeux::quiz(ctx, opt.quiz_n); fait = true; }
    for (auto& id : opt.jeux_ids) {
        if (!lancer_jeu(ctx, id, opt.quiz_n)) { std::cerr << "jeu inconnu : " << id << "  (voir --liste)\n"; return 2; }
        fait = true;
    }
    if (!fait) menu(ctx, opt.p);
    return 0;
}
