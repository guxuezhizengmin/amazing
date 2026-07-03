#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "common.h"

int save_to_csv(void *ds, DSInterface iface, const char *filename);
int load_from_csv(void *ds, DSInterface iface, const char *filename);

#endif
