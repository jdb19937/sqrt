/*
 * magnetar.h — sidus magnetar
 *
 * Campus magneticus extremus (B ~10⁹-10¹¹ T).
 * Phase ad semen aleatorium adhibetur pro orientatione jetuum.
 */

#ifndef SIDUS_MAGNETAR_H
#define SIDUS_MAGNETAR_H

#include "../sidus.h"

typedef struct {
    double phase;           /* semen aleatorium pro orientatione */
} magnetarulum_t;

typedef struct {
    tessella_t     avi;
    sidulum_t      pro;
    magnetarulum_t res;
} magnetar_t;

#endif /* SIDUS_MAGNETAR_H */
