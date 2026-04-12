#include "../sidus_communia.h"

void galaxia_ex_ison(galaxia_t *s, const char *ison)
{
    s->pro.magnitudo   = ison_da_f(ison, "sidulum.magnitudo", 5.0);
    s->pro.temperatura = ison_da_f(ison, "sidulum.temperatura", 5000);
    s->res.morphologia = (galaxia_morphologia_t)(int)ison_da_f(ison, "galaxiola.morphologia", 0);
    s->res.angulus     = ison_da_f(ison, "galaxiola.angulus", 0.0);
}

/*
 * reddere_galaxia — galaxia distans in fenestra 64×64.
 *
 * Galaxiae ut objecta extendida redduntur, non ut puncta.
 * Morphologia (in campo "phase" sideris) formam determinat:
 *
 *   GALAXIA_ELLIPTICA:
 *     Profilo de Vaucouleurs (1948, "R^{1/4} law"):
 *       I(r) = I_e · exp(-7.67·((r/r_e)^{1/4} - 1))
 *     ubi I_e = luminositas superficialis ad r_e.
 *     Sérsic (1963) generalizavit: exp(-b_n·((r/r_e)^{1/n}-1))
 *     ubi n=4 dat de Vaucouleurs, n=1 dat exponentialem.
 *     Ellipticitas ex inclinatione et forma intrinseca:
 *     galaxiae vere triaxiales (Binney 1978, Franx+ 1991).
 *     Colores: rubri-aurei, T_eff effectivus ~4000-5000 K
 *     (dominantur stellae K gigantes, Worthey 1994).
 *
 *   GALAXIA_SPIRALIS:
 *     Nucleus (bulge): profilo Sérsic n~2-3.
 *     Discus: profilo exponentiale I(r) = I_0·exp(-r/h_r)
 *     ubi h_r = scala longitudinis (Freeman 1970).
 *     Brachia: perturbatio logarithmica r(θ) = a·exp(b·θ)
 *     (Ringermacher & Mead 2009). numerus brachiorum typice
 *     m=2 (dominans) vel m=4 (flocculent).
 *     Face-on: brachia caerulea visibilia.
 *     Edge-on: discus tenuis cum fascia pulveris centrali
 *     (sicut NGC 891, van der Kruit & Searle 1981).
 *     Inclinatio continua inter haec extrema.
 *
 *   GALAXIA_SPIRALIS_BARRATA:
 *     Ut spiralis sed cum barra stellari centrali.
 *     Barra: structura elongata cum profilo Ferrers (1877):
 *       ρ(m) ∝ (1 - m²)^n ubi m = distantia normalizata.
 *     Brachia ex extremis barrae oriuntur, non ex nucleo.
 *     Resonantiae orbitales (Contopoulos & Papayannopoulos
 *     1980): orbita x1 sustinet barram, x2 perpendicularis
 *     destruit eam — transitus inter regimina determinat
 *     longitudinem barrae.
 *
 *   GALAXIA_LENTICULARIS:
 *     Discus sine brachiis + nucleus prominens.
 *     Profilo compositus: Sérsic (nucleus, n~2-4) +
 *     exponentiale (discus). similaris spirali edge-on
 *     sed sine fascia pulveris et sine nodis formationis.
 *     Laurikainen+ (2010): pleraeque S0 habent etiam
 *     structuras barrae debiles ("S0/a").
 *
 *   GALAXIA_IRREGULARIS:
 *     Sine symmetria regulari. nodi starburst (luminosi,
 *     caerulei) distributi asymmetrice. Hunter & Elmegreen
 *     (2004): formatio stellarum in irregularibus non in
 *     brachiis sed in regionibus stochasticis compressionis.
 *     Exempla: LMC (cum structura barrae residua), SMC,
 *     NGC 1427A (in Fornax, cum cauda mareali).
 *
 * Distributio magnitudinis sequitur Schechter (1976):
 *   φ(L) ∝ (L/L*)^α · exp(-L/L*), α ≈ -1.25.
 *   implementatio: mag = mag_min + Δmag · (1-r^0.4)
 *   ubi r uniformis — pleraeque debiles, paucae lucidae.
 */
void reddere_galaxia(
    unsigned char *fen,
    const galaxia_t *s,
    const instrumentum_t *instr
) {
    (void)instr;

    /* semen unicum ex omnibus proprietatibus */
    sidus_semen_g = (unsigned int)(
        s->pro.temperatura * 137
        + s->pro.magnitudo * 100003 + (double)s->res.morphologia * 7001
        + s->res.angulus * 50021
    );

    galaxia_morphologia_t morph = s->res.morphologia;
    double luciditas = pow(10.0, -s->pro.magnitudo * 0.4) * 2.5;
    double ang = s->res.angulus;   /* angulus positionis in caelo */
    double ca = cos(ang), sa = sin(ang);

    /* inclinatio ad lineam visus: 0 = face-on, π/2 = edge-on.
     * distributio uniformis in cos(i) (orientationes aleatoriae
     * in spatio 3D, Hubble 1926). ergo i = arccos(r) ubi r ∈ [0,1].
     * implementamus per "temperatura" campi: T/10000 dat cos(i). */
    double cos_incl = s->pro.temperatura / 10000.0;
    if (cos_incl < 0.05)
        cos_incl = 0.05;
    if (cos_incl > 1.0)
        cos_incl = 1.0;

    /* radius effectivus — galaxiae distantes parvae,
     * proximae (lucidiores) maiores. */
    double r_eff = 1.5 + luciditas * 3.0;
    if (r_eff > 20.0)
        r_eff = 20.0;

    /* ellipticitas apparens ex inclinatione:
     * discus circularis inclinatus: b/a = cos(i).
     * elliptica intrinseca: b/a ∈ [0.3, 1.0]. */
    double ba_ratio;  /* b/a axis ratio */
    double ell_intrin = 0.7 + sidus_alea_f() * 0.3;  /* ellipticae: 0.7-1.0 */

    switch (morph) {
    case GALAXIA_ELLIPTICA:
        ba_ratio = ell_intrin;
        break;
    case GALAXIA_LENTICULARIS:
        ba_ratio = cos_incl * 0.9 + 0.1;
        break;
    default:  /* spirales, barratae, irregulares */
        ba_ratio = cos_incl * 0.95 + 0.05;
        break;
    }

    /* color dominans per morphologiam.
     * ellipticae et lenticulares: stellae veteres, rubrae-aureae.
     *   metalicitas alta, [Fe/H] ~ 0 ad +0.3 (Thomas+ 2005).
     * spirales: nucleus aureus, brachia caerulea (O/B stellae).
     * irregulares: caerulescentes (formatio stellarum activa). */
    color_t col_nuc, col_ext;
    switch (morph) {
    case GALAXIA_ELLIPTICA:
        col_nuc = (color_t){1.0, 0.85, 0.6, 1.0};  /* aureum */
        col_ext = (color_t){0.9, 0.75, 0.55, 1.0};
        break;
    case GALAXIA_LENTICULARIS:
        col_nuc = (color_t){1.0, 0.88, 0.65, 1.0};
        col_ext = (color_t){0.85, 0.78, 0.6, 1.0};
        break;
    case GALAXIA_SPIRALIS:
    case GALAXIA_SPIRALIS_BARRATA:
        col_nuc = (color_t){1.0, 0.9, 0.65, 1.0};  /* nucleus aureus */
        col_ext = (color_t){0.6, 0.75, 1.0, 1.0};  /* brachia caerulea */
        break;
    case GALAXIA_IRREGULARIS:
        col_nuc = (color_t){0.7, 0.8, 1.0, 1.0};   /* caerulea */
        col_ext = (color_t){0.6, 0.7, 1.0, 1.0};
        break;
    default:
        col_nuc = (color_t){1.0, 0.9, 0.7, 1.0};
        col_ext = (color_t){0.8, 0.8, 0.8, 1.0};
        break;
    }

    /* variatio individualis coloris */
    double dR = (sidus_alea_f() - 0.5) * 0.15;
    double dG = (sidus_alea_f() - 0.5) * 0.10;
    double dB = (sidus_alea_f() - 0.5) * 0.10;
    col_nuc.r += dR;
    col_nuc.g += dG;
    col_nuc.b += dB;
    col_ext.r += dR * 0.5;
    col_ext.g += dG * 0.5;
    col_ext.b += dB * 0.5;

    /* --- redditio per morphologiam --- */

    if (morph == GALAXIA_ELLIPTICA) {
        /* profilo de Vaucouleurs: I ∝ exp(-7.67·((r/r_e)^0.25 - 1)).
         * simplificamus ad Gaussianam pro galaxiis parvis,
         * cum forma elliptica ex ba_ratio. */
        for (int dy = -30; dy <= 30; dy++) {
            for (int dx = -30; dx <= 30; dx++) {
                /* coordinatae rotatae per angulum positionis */
                double lx = dx * ca + dy * sa;
                double ly = -dx * sa + dy * ca;
                /* ellipticitas */
                double rx = lx / r_eff;
                double ry = ly / (r_eff * ba_ratio);
                double r2 = rx * rx + ry * ry;
                double r  = sqrt(r2);
                if (r > 5.0)
                    continue;
                /* Sérsic n=4 approximatum: exp(-7.67·(r^0.25 - 1)) */
                double sersic = exp(-7.67 * (pow(r + 0.01, 0.25) - 1.0));
                double f      = luciditas * sersic;
                if (f < 0.002)
                    continue;
                /* color: gradiens ab aureo (centrum) ad rubescente (extra) */
                color_t c;
                double t = r / 3.0;
                if (t > 1.0)
                    t = 1.0;
                c.r = col_nuc.r * (1.0 - t) + col_ext.r * t;
                c.g = col_nuc.g * (1.0 - t) + col_ext.g * t;
                c.b = col_nuc.b * (1.0 - t) + col_ext.b * t;
                c.a = 1.0;
                fen_punctum(fen, SEMI + dx, SEMI + dy, 0.6, c, f);
            }
        }
    } else if (morph == GALAXIA_SPIRALIS || morph == GALAXIA_SPIRALIS_BARRATA) {
        /* nucleus (bulge) — Sérsic n~2 */
        double r_bulge = r_eff * 0.3;
        fen_punctum(fen, SEMI, SEMI, r_bulge, col_nuc, luciditas * 1.5);

        /* barra (solum pro barratis) */
        if (morph == GALAXIA_SPIRALIS_BARRATA) {
            double bar_len = r_eff * (0.4 + sidus_alea_f() * 0.3);
            double bar_wid = r_eff * 0.12;
            for (int i = -40; i <= 40; i++) {
                double t  = (double)i / 40.0;
                double bx = t * bar_len;
                double by = 0;
                /* rotare per ang + applicare inclinationem */
                double px   = bx * ca - by * sa;
                double py   = (bx * sa + by * ca) * ba_ratio;
                double dist = fabs(t);
                double f    = luciditas * 0.6 * exp(-dist * dist * 3.0);
                if (f < 0.002)
                    continue;
                fen_punctum(
                    fen, SEMI + px, SEMI + py, bar_wid * ba_ratio,
                    col_nuc, f
                );
            }
        }

        /* brachia spiralia — r(θ) = a·exp(b·θ), spirala logarithmica.
         * pitch angle typice 10°-30° (Kennicutt 1981):
         *   tan(pitch) = 1/(b·r), ergo b = 1/tan(pitch).
         *   Sa: pitch ~10° (stricta), Sc: pitch ~25° (laxa).
         * duo brachia (m=2) dominans modus per theoriam Lin-Shu. */
        int n_brachia  = 2;
        double pitch   = 0.3 + sidus_alea_f() * 0.4;  /* 0.3-0.7 rad (~17°-40°) */
        double r_start = r_eff * 0.25;
        if (morph == GALAXIA_SPIRALIS_BARRATA)
            r_start = r_eff * 0.45;  /* brachia ex extremis barrae */
        double arm_bright = 0.5 + sidus_alea_f() * 0.3;

        for (int arm = 0; arm < n_brachia; arm++) {
            double theta0 = DUO_PI * arm / n_brachia + sidus_alea_f() * 0.3;
            int n_steps   = 200;
            for (int j = 0; j < n_steps; j++) {
                double t     = (double)j / (double)n_steps;
                double theta = theta0 + t * DUO_PI / tan(pitch);
                double r     = r_start + t * r_eff * 1.8;

                /* spirala logarithmica cum perturbatione */
                double perturb = sidus_alea_f() * 0.15 - 0.075;
                double x_arm   = r * cos(theta + perturb);
                double y_arm   = r * sin(theta + perturb);

                /* proiectio cum inclinatione et rotatione */
                double px = x_arm * ca - y_arm * sa;
                double py = (x_arm * sa + y_arm * ca) * ba_ratio;

                if (fabs(px) > 28 || fabs(py) > 28)
                    continue;

                double fade = (1.0 - t) * (1.0 - t * 0.3);
                double f    = luciditas * arm_bright * fade;
                if (f < 0.002)
                    continue;

                /* regiones HII in brachiis (Kennicutt 1998):
                 * formatio stellarum in undis densitatis.
                 * nodi lucidi stochastice distributi. */
                double nod = 1.0;
                if (sidus_alea_f() < 0.08)
                    nod = 1.5 + sidus_alea_f();
                double wid = 0.5 + t * 0.8;
                fen_punctum(
                    fen, SEMI + px, SEMI + py, wid,
                    col_ext, f * nod
                );
            }
        }

        /* discus diffusus exponentialis (Freeman 1970) */
        for (int dy = -25; dy <= 25; dy++) {
            for (int dx = -25; dx <= 25; dx++) {
                double lx = dx * ca + dy * sa;
                double ly = (-dx * sa + dy * ca) / (ba_ratio + 0.01);
                double r  = sqrt(lx * lx + ly * ly);
                if (r > r_eff * 3.0)
                    continue;
                double disc = exp(-r / r_eff) * 0.3;
                double f    = luciditas * disc;
                if (f < 0.002)
                    continue;
                double t = r / (r_eff * 2.5);
                if (t > 1.0)
                    t = 1.0;
                color_t c;
                c.r = col_nuc.r * (1.0 - t) + col_ext.r * t;
                c.g = col_nuc.g * (1.0 - t) + col_ext.g * t;
                c.b = col_nuc.b * (1.0 - t) + col_ext.b * t;
                c.a = 1.0;
                fen_punctum(fen, SEMI + dx, SEMI + dy, 0.5, c, f);
            }
        }

        /* fascia pulveris pro edge-on (cos_incl < 0.3):
         * in galaxiis edge-on, fascia obscura centralis visibilis
         * (exemplum classicum: NGC 891, van der Kruit & Searle 1981).
         * extinctio per pulverem: A_V ≈ τ_V / (cos(i) + 0.1). */
        if (cos_incl < 0.3) {
            for (int dx = -25; dx <= 25; dx++) {
                double lx   = dx * ca;
                double ly   = dx * sa * ba_ratio;
                double dist = fabs((double)dx) / (r_eff * 2.0);
                if (dist > 1.2)
                    continue;
                double f = 0.4 * (1.0 - dist) * (0.3 - cos_incl) / 0.3;
                if (f < 0.01)
                    continue;
                int px = SEMI + (int)(lx + 0.5);
                int py = SEMI + (int)(ly + 0.5);
                if (px < 0 || px >= FEN || py < 0 || py >= FEN)
                    continue;
                /* obscurare pixeles existentes */
                int idx      = (py * FEN + px) * 4;
                fen[idx + 0] = (unsigned char)(fen[idx + 0] * (1.0 - f));
                fen[idx + 1] = (unsigned char)(fen[idx + 1] * (1.0 - f));
                fen[idx + 2] = (unsigned char)(fen[idx + 2] * (1.0 - f));
            }
        }

    } else if (morph == GALAXIA_LENTICULARIS) {
        /* nucleus prominens Sérsic n~3 */
        fen_punctum(fen, SEMI, SEMI, r_eff * 0.35, col_nuc, luciditas * 1.8);

        /* discus diffusus sine brachiis — exponentiale */
        for (int dy = -25; dy <= 25; dy++) {
            for (int dx = -25; dx <= 25; dx++) {
                double lx = dx * ca + dy * sa;
                double ly = (-dx * sa + dy * ca) / (ba_ratio + 0.01);
                double r  = sqrt(lx * lx + ly * ly);
                if (r > r_eff * 3.0)
                    continue;
                double disc = exp(-r / r_eff) * 0.5;
                double f    = luciditas * disc;
                if (f < 0.002)
                    continue;
                double t = r / (r_eff * 2.0);
                if (t > 1.0)
                    t = 1.0;
                color_t c;
                c.r = col_nuc.r * (1.0 - t) + col_ext.r * t;
                c.g = col_nuc.g * (1.0 - t) + col_ext.g * t;
                c.b = col_nuc.b * (1.0 - t) + col_ext.b * t;
                c.a = 1.0;
                fen_punctum(fen, SEMI + dx, SEMI + dy, 0.5, c, f);
            }
        }

    } else {
        /* GALAXIA_IRREGULARIS — nodi stochastici (Hunter & Elmegreen 2004).
         * distributio spatiosa non symmetrica, nodi luminosi
         * (regiones HII gigantes, 30 Doradus in LMC exemplum)
         * cum fundo diffuso debili. */
        double offset_x = (sidus_alea_f() - 0.5) * r_eff * 0.3;
        double offset_y = (sidus_alea_f() - 0.5) * r_eff * 0.3;

        /* fundus diffusus asymmetricus */
        for (int dy = -20; dy <= 20; dy++) {
            for (int dx = -20; dx <= 20; dx++) {
                double lx = dx - offset_x;
                double ly = dy - offset_y;
                double r  = sqrt(lx * lx + ly * ly);
                if (r > r_eff * 2.5)
                    continue;
                double f = luciditas * 0.3 * exp(-r / r_eff);
                if (f < 0.002)
                    continue;
                fen_punctum(fen, SEMI + dx, SEMI + dy, 0.5, col_ext, f);
            }
        }

        /* nodi starburst — 3-8 regiones luminosae */
        int n_nodi = 3 + (int)(sidus_alea_f() * 6);
        for (int k = 0; k < n_nodi; k++) {
            double nx = (sidus_alea_f() - 0.5) * r_eff * 2.0;
            double ny = (sidus_alea_f() - 0.5) * r_eff * 2.0;
            double nr = 0.5 + sidus_alea_f() * 1.5;
            double nf = luciditas * (0.3 + sidus_alea_f() * 0.5);
            /* colores: caerulei (formatio stellarum) vel rosei (Hα) */
            color_t nc;
            if (sidus_alea_f() < 0.6)
                nc = (color_t){0.5, 0.7, 1.0, 1.0};  /* OB stellae */
            else
                nc = (color_t){1.0, 0.5, 0.6, 1.0};  /* HII Hα */
            fen_punctum(fen, SEMI + nx, SEMI + ny, nr, nc, nf);
        }
    }
}

void galaxia_in_ison(FILE *f, const galaxia_t *s)
{
    fprintf(f, "{\"sidulum\": {\"magnitudo\": %.3f, \"temperatura\": %.1f}", s->pro.magnitudo, s->pro.temperatura);
    fprintf(f, ", \"galaxiola\": {\"morphologia\": %d, \"angulus\": %.3f}",
        (int)s->res.morphologia, s->res.angulus);
    fprintf(f, "}");
}
