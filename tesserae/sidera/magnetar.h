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
    sidulum_t basis;
    double  phase;           /* semen aleatorium pro orientatione */
} sidus_magnetar_t;

#endif /* SIDUS_MAGNETAR_H */
