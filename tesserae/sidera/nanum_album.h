/*
 * nanum_album.h — sidus nanum album
 */

#ifndef SIDUS_NANUM_ALBUM_H
#define SIDUS_NANUM_ALBUM_H

#include "../sidus.h"

typedef struct {
    int res;
} nanulum_album_t;

typedef struct {
    tessella_t      avi;
    sidulum_t       pro;
    nanulum_album_t res;
} nanum_album_t;

void reddere_nanum_album(unsigned char *fen, const nanum_album_t *s, const instrumentum_t *instr);
void nanum_album_in_ison(FILE *f, const nanum_album_t *s);
void nanum_album_ex_ison(nanum_album_t *s, const char *ison);

#endif /* SIDUS_NANUM_ALBUM_H */
