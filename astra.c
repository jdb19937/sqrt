/*
 * astra.c — corpus caeleste: lector ISON, dispatch phylorum
 */

#include "astra.h"
#include "ison.h"

#include <stdlib.h>
#include <string.h>

/* ================================================================
 * auxiliaria interna
 * ================================================================ */

static sidus_genus_t sidus_genus_ex_nomine(const char *n)
{
    if (!n)                               return SIDUS_SEQUENTIA;
    if (strcmp(n, "nanum_album") == 0)    return SIDUS_NANUM_ALBUM;
    if (strcmp(n, "sequentia") == 0)      return SIDUS_SEQUENTIA;
    if (strcmp(n, "gigas_rubrum") == 0)   return SIDUS_GIGAS_RUBRUM;
    if (strcmp(n, "supergigas") == 0)     return SIDUS_SUPERGIGAS;
    if (strcmp(n, "neutronium") == 0)     return SIDUS_NEUTRONIUM;
    if (strcmp(n, "crystallinum") == 0)   return SIDUS_CRYSTALLINUM;
    if (strcmp(n, "magnetar") == 0)       return SIDUS_MAGNETAR;
    if (strcmp(n, "galaxia") == 0)        return SIDUS_GALAXIA;
    if (strcmp(n, "vagans") == 0)         return SIDUS_VAGANS;
    return SIDUS_SEQUENTIA;
}

static sidus_t sidus_ex_ison(const char *ison)
{
    sidus_t s;
    char *genus_s    = ison_da_chordam(ison, "genus");
    s.genus          = sidus_genus_ex_nomine(genus_s);
    free(genus_s);
    s.magnitudo      = ison_da_f(ison, "magnitudo", 5.0);
    s.temperatura    = ison_da_f(ison, "temperatura", 5000.0);
    s.phase          = ison_da_f(ison, "phase", 0.0);
    s.angulus_phase  = ison_da_f(ison, "angulus_phase", 0.0);
    return s;
}

/* ================================================================
 * astra_ex_ison
 * ================================================================ */

astra_t astra_ex_ison(const char *ison)
{
    astra_t a;
    char *phylum_s = ison_da_chordam(ison, "phylum");
    if (phylum_s && strcmp(phylum_s, "sidus") == 0) {
        a.phylum       = PHYLUM_SIDUS;
        a.corpus.sidus = sidus_ex_ison(ison);
    } else {
        a.phylum         = PHYLUM_PLANETA;
        a.corpus.planeta = planeta_ex_ison(ison);
    }
    free(phylum_s);
    return a;
}
