/*
 * TORUS PLANUS CORRUGATUS — ANIMATIO
 * Camera circa torum rotans, imagines PPM scribens
 *
 * Auctore: machina Bernoulli
 * C99, sine ullis dependentiis externis
 *
 * Usus: ./torus_animare [numerus_imaginum]
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PI_GRAECUM  3.14159265358979323846
#define DUO_PI      (2.0 * PI_GRAECUM)

/* Dimensiones animationis */
#define LATITUDO_IMG  640
#define ALTITUDO_IMG  480

/* Discretio superficiei — minor pro celeritate */
#define GRADUS_U  600
#define GRADUS_V  300

#define STRATA_CORRUG  5
#define RADIUS_MAIOR   1.0
#define RADIUS_MINOR   0.42

/* ================================================================ */
typedef struct { double x, y, z; } Vec3;
typedef struct { double r, g, b; } Color;

static inline Vec3 vec3(double x, double y, double z)
{ return (Vec3){x, y, z}; }

static inline Vec3 summa(Vec3 a, Vec3 b)
{ return (Vec3){a.x+b.x, a.y+b.y, a.z+b.z}; }

static inline Vec3 differentia(Vec3 a, Vec3 b)
{ return (Vec3){a.x-b.x, a.y-b.y, a.z-b.z}; }

static inline double productum_scalare(Vec3 a, Vec3 b)
{ return a.x*b.x + a.y*b.y + a.z*b.z; }

static inline Vec3 productum_vectoriale(Vec3 a, Vec3 b)
{ return (Vec3){a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x}; }

static inline Vec3 multiplicare(Vec3 v, double s)
{ return (Vec3){v.x*s, v.y*s, v.z*s}; }

static inline double magnitudo(Vec3 v)
{ return sqrt(productum_scalare(v,v)); }

static inline Vec3 normalizare(Vec3 v)
{ double m=magnitudo(v); if(m<1e-15) return (Vec3){0,0,1}; return multiplicare(v,1.0/m); }

/* ================================================================ */
/*                  SUPERFICIES TORI HEVEA                          */
/* ================================================================ */

static Vec3 superficies_tori(double u, double v)
{
    double corrugatio = 0.0;
    double perturb_u = 0.0, perturb_v = 0.0;

    for (int k = 0; k < STRATA_CORRUG; k++) {
        double freq = 7.0 * pow(2.8, (double)k);
        double amp  = 0.035 / pow(1.8, (double)k);

        if (k % 2 == 0) {
            corrugatio += amp * sin(freq * u + 0.5 * k);
            perturb_v  += amp * 0.15 * cos(freq * u + 0.5 * k);
        } else {
            corrugatio += amp * sin(freq * v + 0.8 * k);
            perturb_u  += amp * 0.15 * cos(freq * v + 0.8 * k);
        }
        double amp2 = amp * 0.3;
        corrugatio += amp2 * sin(freq * u + 1.3 * k)
                           * sin(freq * v + 0.9 * k);
    }

    double uu = u + perturb_u;
    double vv = v + perturb_v;
    double r_eff = RADIUS_MINOR + corrugatio;

    return (Vec3){
        (RADIUS_MAIOR + r_eff * cos(vv)) * cos(uu),
        (RADIUS_MAIOR + r_eff * cos(vv)) * sin(uu),
        r_eff * sin(vv)
    };
}

static Vec3 norma_superficiei(double u, double v)
{
    double h = 5e-5;
    Vec3 du = differentia(superficies_tori(u+h,v), superficies_tori(u-h,v));
    Vec3 dv = differentia(superficies_tori(u,v+h), superficies_tori(u,v-h));
    return normalizare(productum_vectoriale(du, dv));
}

/* ================================================================ */
/*                        CAMERA                                    */
/* ================================================================ */

typedef struct {
    Vec3 positio, ante, dextrum, sursum;
    double focalis;
} Camera;

static Camera cameram_constituere(Vec3 positio, Vec3 scopus)
{
    Camera cam;
    cam.positio = positio;
    cam.ante = normalizare(differentia(scopus, positio));
    Vec3 ms = vec3(0,0,1);
    cam.dextrum = normalizare(productum_vectoriale(cam.ante, ms));
    cam.sursum  = productum_vectoriale(cam.dextrum, cam.ante);
    cam.focalis = 1.6;
    return cam;
}

static int proicere(const Camera *cam, Vec3 p,
                    double *sx, double *sy, double *sz)
{
    Vec3 d = differentia(p, cam->positio);
    double z = productum_scalare(d, cam->ante);
    if (z < 0.05) return 0;
    double f = cam->focalis / z;
    double scala = ALTITUDO_IMG * 0.5;
    *sx = LATITUDO_IMG * 0.5 + productum_scalare(d, cam->dextrum) * f * scala;
    *sy = ALTITUDO_IMG * 0.5 - productum_scalare(d, cam->sursum)  * f * scala;
    *sz = z;
    return 1;
}

/* ================================================================ */
/*                      ILLUMINATIO                                 */
/* ================================================================ */

static Color illuminare(Vec3 punct, Vec3 norm, Vec3 oculus)
{
    Vec3 lux_dir[3] = {
        normalizare(vec3( 1.0,-0.6, 1.8)),
        normalizare(vec3(-0.7, 0.8, 0.4)),
        normalizare(vec3( 0.2,-1.0,-0.2))
    };
    Color lux_int[3] = {{0.90,0.80,0.65},{0.20,0.30,0.55},{0.12,0.10,0.08}};
    double mr=0.78, mg=0.55, mb=0.28;
    Color res = {mr*0.06, mg*0.06, mb*0.06};
    Vec3 ao = normalizare(differentia(oculus, punct));
    if (productum_scalare(norm, ao) < 0.0) norm = multiplicare(norm, -1.0);

    for (int i = 0; i < 3; i++) {
        double NdL = productum_scalare(norm, lux_dir[i]);
        if (NdL < 0) NdL = 0;
        Vec3 semi = normalizare(summa(lux_dir[i], ao));
        double NdH = productum_scalare(norm, semi);
        if (NdH < 0) NdH = 0;
        double spec = pow(NdH, 60.0);
        res.r += mr*NdL*lux_int[i].r + spec*lux_int[i].r*0.45;
        res.g += mg*NdL*lux_int[i].g + spec*lux_int[i].g*0.45;
        res.b += mb*NdL*lux_int[i].b + spec*lux_int[i].b*0.45;
    }
    double fresnel = 1.0 - fabs(productum_scalare(norm, ao));
    fresnel = fresnel*fresnel*fresnel*0.35;
    res.r += fresnel*0.4; res.g += fresnel*0.5; res.b += fresnel*0.65;
    return res;
}

/* ================================================================ */
/*                 RASTERIZATIO ET IMAGO                            */
/* ================================================================ */

static unsigned char *tabula_imaginis;
static double *tabula_profunditatis;

static inline unsigned char gamma_c(double v)
{ if(v<0)v=0; if(v>1)v=1; return (unsigned char)(pow(v,1.0/2.2)*255.0+0.5); }

static inline void pixel_scribere(int x, int y, double prof, Color c)
{
    if (x<0||x>=LATITUDO_IMG||y<0||y>=ALTITUDO_IMG) return;
    int idx = y*LATITUDO_IMG+x;
    if (prof >= tabula_profunditatis[idx]) return;
    tabula_profunditatis[idx] = prof;
    tabula_imaginis[idx*3+0] = gamma_c(c.r);
    tabula_imaginis[idx*3+1] = gamma_c(c.g);
    tabula_imaginis[idx*3+2] = gamma_c(c.b);
}

static void triangulum_reddere(
    double sx0,double sy0,double sz0,Vec3 p0,Vec3 n0,
    double sx1,double sy1,double sz1,Vec3 p1,Vec3 n1,
    double sx2,double sy2,double sz2,Vec3 p2,Vec3 n2,
    Vec3 oculus)
{
    double area=(sx1-sx0)*(sy2-sy0)-(sx2-sx0)*(sy1-sy0);
    if(fabs(area)<0.5) return;
    double inv_a=1.0/area;
    int mnx=(int)floor(fmin(fmin(sx0,sx1),sx2));
    int mxx=(int)ceil(fmax(fmax(sx0,sx1),sx2));
    int mny=(int)floor(fmin(fmin(sy0,sy1),sy2));
    int mxy=(int)ceil(fmax(fmax(sy0,sy1),sy2));
    if(mnx<0)mnx=0; if(mxx>=LATITUDO_IMG)mxx=LATITUDO_IMG-1;
    if(mny<0)mny=0; if(mxy>=ALTITUDO_IMG)mxy=ALTITUDO_IMG-1;

    for(int y=mny;y<=mxy;y++){
        double py=y+0.5;
        for(int x=mnx;x<=mxx;x++){
            double px=x+0.5;
            double w0=((sx1-px)*(sy2-py)-(sx2-px)*(sy1-py))*inv_a;
            double w1=((sx2-px)*(sy0-py)-(sx0-px)*(sy2-py))*inv_a;
            double w2=1.0-w0-w1;
            if(w0<-0.001||w1<-0.001||w2<-0.001) continue;
            double z=w0*sz0+w1*sz1+w2*sz2;
            Vec3 pt=summa(summa(multiplicare(p0,w0),multiplicare(p1,w1)),multiplicare(p2,w2));
            Vec3 nm=normalizare(summa(summa(multiplicare(n0,w0),multiplicare(n1,w1)),multiplicare(n2,w2)));
            pixel_scribere(x,y,z,illuminare(pt,nm,oculus));
        }
    }
}

/* ================================================================ */
/*                       PRINCIPIUM                                 */
/* ================================================================ */

int main(int argc, char **argv)
{
    int numerus_imaginum = 72;
    if (argc > 1) numerus_imaginum = atoi(argv[1]);
    if (numerus_imaginum < 1) numerus_imaginum = 72;

    size_t n_pix = (size_t)LATITUDO_IMG * ALTITUDO_IMG;
    tabula_imaginis = (unsigned char *)malloc(n_pix * 3);
    tabula_profunditatis = (double *)malloc(n_pix * sizeof(double));

    /* Superficiem semel praecomputare */
    fprintf(stderr, "Superficiem computans...\n");
    size_t n_vert = (size_t)(GRADUS_U+1)*(GRADUS_V+1);
    Vec3 *puncta = (Vec3 *)malloc(n_vert * sizeof(Vec3));
    Vec3 *normae = (Vec3 *)malloc(n_vert * sizeof(Vec3));

    for (int i = 0; i <= GRADUS_U; i++) {
        double u = DUO_PI * (double)i / (double)GRADUS_U;
        for (int j = 0; j <= GRADUS_V; j++) {
            double v = DUO_PI * (double)j / (double)GRADUS_V;
            size_t idx = (size_t)i*(GRADUS_V+1)+j;
            puncta[idx] = superficies_tori(u, v);
            normae[idx] = norma_superficiei(u, v);
        }
    }

    fprintf(stderr, "Animationem reddens: %d imagines\n", numerus_imaginum);

    for (int f = 0; f < numerus_imaginum; f++) {
        fprintf(stderr, "Imago %d/%d\n", f+1, numerus_imaginum);

        /* Camera rotans circa axem verticalem */
        double angulus = DUO_PI * (double)f / (double)numerus_imaginum;
        double dist_cam = 3.2;
        double alt_cam  = 1.3;
        Vec3 pos_cam = vec3(dist_cam * cos(angulus),
                            dist_cam * sin(angulus),
                            alt_cam);
        Vec3 scopus = vec3(0.0, 0.0, -0.05);
        Camera cam = cameram_constituere(pos_cam, scopus);

        /* Tabulam purgare */
        for (size_t i = 0; i < n_pix; i++) {
            tabula_profunditatis[i] = 1e30;
            tabula_imaginis[i*3+0] = 8;
            tabula_imaginis[i*3+1] = 8;
            tabula_imaginis[i*3+2] = 14;
        }

        /* Triangula reddere */
        for (int i = 0; i < GRADUS_U; i++) {
            for (int j = 0; j < GRADUS_V; j++) {
                size_t vi[4] = {
                    (size_t)i*(GRADUS_V+1)+j,
                    (size_t)(i+1)*(GRADUS_V+1)+j,
                    (size_t)(i+1)*(GRADUS_V+1)+(j+1),
                    (size_t)i*(GRADUS_V+1)+(j+1)
                };
                double sx[4],sy[4],sz[4];
                int vis=1;
                for(int k=0;k<4;k++){
                    if(!proicere(&cam,puncta[vi[k]],&sx[k],&sy[k],&sz[k]))
                    { vis=0; break; }
                }
                if(!vis) continue;
                triangulum_reddere(
                    sx[0],sy[0],sz[0],puncta[vi[0]],normae[vi[0]],
                    sx[1],sy[1],sz[1],puncta[vi[1]],normae[vi[1]],
                    sx[2],sy[2],sz[2],puncta[vi[2]],normae[vi[2]],
                    cam.positio);
                triangulum_reddere(
                    sx[0],sy[0],sz[0],puncta[vi[0]],normae[vi[0]],
                    sx[2],sy[2],sz[2],puncta[vi[2]],normae[vi[2]],
                    sx[3],sy[3],sz[3],puncta[vi[3]],normae[vi[3]],
                    cam.positio);
            }
        }

        /* PPM scribere */
        char nomen[256];
        snprintf(nomen, sizeof(nomen), "tabulae/imago_%04d.ppm", f);
        FILE *fas = fopen(nomen, "wb");
        if (!fas) {
            fprintf(stderr, "ERROR: %s aperire non possum!\n", nomen);
            return 1;
        }
        fprintf(fas, "P6\n%d %d\n255\n", LATITUDO_IMG, ALTITUDO_IMG);
        fwrite(tabula_imaginis, 1, n_pix*3, fas);
        fclose(fas);
    }

    free(tabula_imaginis);
    free(tabula_profunditatis);
    free(puncta);
    free(normae);

    fprintf(stderr, "Animatio perfecta est.\n");
    return 0;
}
