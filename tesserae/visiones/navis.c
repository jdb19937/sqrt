
/* ================================================================
 * navis ardens — burning ship fractal
 *
 * z_{n+1} = (|Re(z_n)| + i|Im(z_n)|)^2 + c
 * ================================================================ */

static void navis_reddere(unsigned char *fenestra, const navis_t *n)
{
    double cx  = n->res.centrum_re;
    double cy  = n->res.centrum_im;
    double amp = n->res.amplitudo > 0 ? n->res.amplitudo : 0.02;
    int    max = n->res.iterationes > 0 ? n->res.iterationes : 1000;
    double cyc = n->res.color_cyclus > 0 ? n->res.color_cyclus : 8.0;
    double pha = n->res.color_phase;
    double sat = n->res.saturatio > 0 ? n->res.saturatio : 1.0;

    double x0  = cx - amp * 0.5;
    double y0  = cy - amp * 0.5;
    double pas = amp / (double)FEN;

    for (int py = 0; py < FEN; py++) {
        double ci = y0 + (double)py * pas;
        for (int px = 0; px < FEN; px++) {
            double cr = x0 + (double)px * pas;

            double zr = 0.0, zi = 0.0;
            int iter  = 0;
            while (iter < max) {
                double azr = fabs(zr);
                double azi = fabs(zi);
                double zr2 = azr * azr - azi * azi + cr;
                double zi2 = 2.0 * azr * azi + ci;
                zr         = zr2;
                zi         = zi2;
                double r2  = zr * zr + zi * zi;
                if (r2 > 256.0 || r2 != r2)
                    break;
                iter++;
            }

            int di = (py * FEN + px) * 4;
            if (iter >= max) {
                /* intra fractale — nigrum */
                fenestra[di + 0] = 0;
                fenestra[di + 1] = 0;
                fenestra[di + 2] = 0;
                fenestra[di + 3] = 255;
            } else {
                /* smooth coloring */
                double r2 = zr * zr + zi * zi;
                if (r2 < 1.0)
                    r2 = 1.0;
                double lr = log2(r2);
                if (lr < 1.0)
                    lr = 1.0;
                double mu = (double)iter - log2(lr) + 4.0;
                double t  = mu / cyc + pha;

                /* palette bichroma: caeruleum profundum ↔ ignis aureus */
                double s = fmod(t, 1.0);
                if (s < 0.0)
                    s += 1.0;
                double r, g, b;
                if (s < 0.15) {
                    /* nigrum → caeruleum profundum */
                    double q = s / 0.15;
                    r        = 0.02 * q;
                    g        = 0.04 * q;
                    b        = 0.18 * q;
                } else if (s < 0.4) {
                    /* caeruleum profundum → caeruleum lucidum */
                    double q = (s - 0.15) / 0.25;
                    r        = 0.02 + 0.08 * q;
                    g        = 0.04 + 0.30 * q;
                    b        = 0.18 + 0.55 * q;
                } else if (s < 0.55) {
                    /* caeruleum → aurantium (transitus) */
                    double q = (s - 0.4) / 0.15;
                    r        = 0.10 + 0.70 * q;
                    g        = 0.34 - 0.04 * q;
                    b        = 0.73 - 0.68 * q;
                } else if (s < 0.8) {
                    /* aurantium → flavum lucidum */
                    double q = (s - 0.55) / 0.25;
                    r        = 0.80 + 0.20 * q;
                    g        = 0.30 + 0.50 * q;
                    b        = 0.05 + 0.05 * q;
                } else {
                    /* flavum → album → nigrum (reditus) */
                    double q = (s - 0.8) / 0.2;
                    r        = 1.00 - 0.85 * q;
                    g        = 0.80 - 0.70 * q;
                    b        = 0.10 - 0.05 * q;
                }
                r *= sat;
                g *= sat;
                b *= sat;

                fenestra[di + 0] = gamma_corrigere(r);
                fenestra[di + 1] = gamma_corrigere(g);
                fenestra[di + 2] = gamma_corrigere(b);
                fenestra[di + 3] = 255;
            }
        }
    }
}


