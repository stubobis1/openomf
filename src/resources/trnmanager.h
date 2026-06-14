#ifndef TRNMANAGER_H
#define TRNMANAGER_H

#include "formats/tournament.h"
#include "utils/vector.h"

void trnlist_init(vector *trnlist);
void trnlist_free(vector *trnlist);
int trn_load(sd_tournament_file *trn, const char *trn_name);
void trn_strip_pilot_requirements(sd_tournament_file *trn);

#endif // TRNMANAGER_H
