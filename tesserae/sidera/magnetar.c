#include "../sidus_communia.h"

void magnetar_ex_ison(magnetar_t *s, const char *ison)
{
    s->pro.magnitudo   = ison_da_f(ison, "sidulum.magnitudo", 5.0);
    s->pro.temperatura = ison_da_f(ison, "sidulum.temperatura", 20000);
    s->res.phase       = ison_da_f(ison, "magnetarulum.phase", 0.0);
}

/* Magnetar: stella neutronium cum B ~10^9-10^11 T.
 * Jets relativistici bipolares spirant circa lineas campi magnetici.
 * Materia in jet accelerata ad ~0.3c per processum Blandford-Znajek.
 * Emissio synchrotronum: spectrum continuum, polarizatum,
 * color caeruleum-album (potentia spectri ∝ ν^(-0.7)).
 * Precession geodaetica causat jets spirales (Lense-Thirring).
 * Halo birefringens: vacuum QED prope B_Schwinger lucem
 * in duos modos polares separat (Heisenberg-Euler 1936). */
void reddere_magnetar(
    unsigned char *fen,
    const magnetar_t *s,
    const instrumentum_t *instr
) {
    (void)instr;
    double luciditas = pow(10.0, -s->pro.magnitudo * 0.4) * 3.0;

    /* semen ex magnitudo et temperatura derivatum — unicum per sidus.
     * antea: semen ex sola temperatura, sed magnetares omnes T=5×10⁶ K
     * habent, ergo omnes eandem orientationem reddebant.
     * nunc magnitudinem addimus ut variatio inter magnetares oriatur. */
    sidus_semen_g = (unsigned int)(
        s->pro.temperatura * 71
        + s->pro.magnitudo * 100003 + s->res.phase * 70001
    );

    /* axis magneticus — angulus aleatorius per plenum circulum.
     * in magnetaribus veris, axis magneticus non cum axe rotationis
     * congruere necesse est (obliquitas typica 10°-60°, Lander & Jones
     * 2009). axis proiectus in planum caeli quemlibet angulum habere
     * potest, ergo DUO_PI non PI_GRAECUM adhibemus. */
    double axis_ang = sidus_alea_f() * DUO_PI;
    double ax       = cos(axis_ang), ay = sin(axis_ang);

    /* variatio individualis magnetaris:
     * campus magneticus B variat 10⁹-10¹¹ T (Duncan & Thompson 1992).
     * B fortior → jets longiores et collimatiores (Bucciantini+ 2006).
     * periodus rotationis P = 2-12 s → precession rate variat.
     * aetas τ = P/(2Ṗ) ~ 10³-10⁵ a → jets seniores latiores et debiliores. */
    double jet_long     = 16.0 + sidus_alea_f() * 20.0;    /* 16-36 pixeles */
    double jet_apertura = 3.0 + sidus_alea_f() * 6.0;  /* amplitudo spiralis */
    double jet_freq     = 8.0 + sidus_alea_f() * 10.0;     /* frequentia spiralis */
    int    jet_fil      = 2 + (int)(sidus_alea_f() * 3);     /* 2-4 filamenta */
    double halo_long    = 2.0 + sidus_alea_f() * 3.0;     /* elongatio hali */

    /* nucleus: album intensum cum halo asymmetrico (birefringentia) */
    color_t album = {1.0, 1.0, 1.0, 1.0};
    fen_punctum(fen, SEMI, SEMI, 0.5, album, luciditas * 2.0);

    /* halo elongatum per axem magneticum (birefringentia vacui).
     * QED in campo magnetico extremo (B > B_Schwinger = m²c³/eℏ =
     * 4.414×10⁹ T) vacuum birefringens fit: duae polarizationes
     * cum indicibus refractionis diversis propagantur (Heisenberg &
     * Euler 1936, Adler 1971). hoc imaginem stellae elongat
     * perpendiculare ad campum magneticum proiectum (Heyl & Shaviv
     * 2000, 2002). in data Chandra, RX J1856.5-3754 elongationem
     * consistentem cum birefringentia vacui ostendit (Mignani+ 2017,
     * confirmatio prima effectus QED in campo astronomico). */
    for (int i = -8; i <= 8; i++) {
        double t  = (double)i / 8.0;
        double px = SEMI + t * halo_long * ax;
        double py = SEMI + t * halo_long * ay;
        double f  = luciditas * 0.5 * exp(-t * t * 2.0);
        color_t hc = {0.7, 0.8, 1.0, 1.0};
        fen_punctum(fen, px, py, 1.0, hc, f);
    }

    /* jets bipolares relativistici spirantes circa lineas campi.
     *
     * mechanismus: materia accreti per campum magneticum ad polos
     * dirigitur (Goldreich & Julian 1969). particuli relativistici
     * in campo magnetico radiationem synchrotron emittunt cum
     * spectro potentiae F_ν ∝ ν^α ubi α ≈ -0.6 ad -0.8
     * (Rybicki & Lightman 1979). color ergo caeruleus-albus.
     *
     * precession Lense-Thirring (Lense & Thirring 1918):
     * stella neutronium rotans spatium-tempus circa se trahit
     * (frame dragging). axis praecessionis Ω_LT = 2GJ/(c²r³)
     * ubi J = momentum angulare. hoc causat jets spirales —
     * helicam cum radio crescente a polo.
     *
     * collimatio: jets magnetaris typice apertura ~5°-30°
     * (Granot+ 2017), strictiores quam jets AGN (~15°-60°). */
    color_t jet_colores[4] = {
        {0.4, 0.6, 1.0, 1.0},    /* synchrotron caeruleum */
        {0.7, 0.5, 1.0, 1.0},    /* violaceum */
        {0.3, 0.9, 0.9, 1.0},    /* cyaneum (Compton inversus) */
        {0.5, 0.4, 1.0, 1.0}     /* indicum */
    };

    for (int pol = -1; pol <= 1; pol += 2) {
        double dir = (double)pol;

        for (int fil = 0; fil < jet_fil; fil++) {
            double phase_offset = DUO_PI * fil / (double)jet_fil;
            color_t jc = jet_colores[fil % 4];

            int n_steps = 120;
            for (int j = 0; j <= n_steps; j++) {
                double t    = (double)j / (double)n_steps;
                double dist = t * jet_long;

                double px_ax = SEMI + dir * dist * ax;
                double py_ax = SEMI + dir * dist * ay;

                double spiral_r   = t * t * jet_apertura;
                double spiral_ang = phase_offset + t * jet_freq;
                double perp_x     = -ay, perp_y = ax;

                double sx = spiral_r * (
                    cos(spiral_ang) * perp_x
                    - sin(spiral_ang) * dir * ax
                );
                double sy = spiral_r * (
                    cos(spiral_ang) * perp_y
                    - sin(spiral_ang) * dir * ay
                );

                double px = px_ax + sx;
                double py = py_ax + sy;

                if (px < 0 || px >= FEN || py < 0 || py >= FEN)
                    continue;

                double f = luciditas * 0.35 * (1.0 - t) * (1.0 - t * 0.5);
                fen_punctum(fen, px, py, 0.4 + t * 0.3, jc, f);
            }
        }
    }

    /* nodi lucidi in jets (shocks interni).
     * Rees (1978) primus proposuit fluctuationes in fluxu jet
     * shells cum velocitatibus diversis creare quae collisionibus
     * "shocks internos" producunt. hoc in blazaribus (Marscher &
     * Gear 1985) et in GRBs (Rees & Mészáros 1994) confirmatum.
     * in magnetaribus, giant flares (ut SGR 1806-20, Palmer+ 2005)
     * nodos similes per eruptiones in jets creant. */
    int n_nodi = 2 + (int)(sidus_alea_f() * 3);
    for (int pol = -1; pol <= 1; pol += 2) {
        double dir = (double)pol;
        for (int k = 0; k < n_nodi; k++) {
            double t    = 0.15 + sidus_alea_f() * 0.6;
            double dist = t * jet_long * 0.9;
            double px   = SEMI + dir * dist * ax;
            double py   = SEMI + dir * dist * ay;
            fen_punctum(
                fen, px, py, 0.8 + sidus_alea_f() * 0.5, album,
                luciditas * (0.2 + sidus_alea_f() * 0.3)
            );
        }
    }
}

void magnetar_in_ison(FILE *f, const magnetar_t *s)
{
    fprintf(f, "{\"sidulum\": {\"magnitudo\": %.3f, \"temperatura\": %.1f}", s->pro.magnitudo, s->pro.temperatura);
    fprintf(f, ", \"magnetarulum\": {\"phase\": %.3f}", s->res.phase);
    fprintf(f, "}");
}
