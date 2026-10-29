#ifndef SCAN_DIRS_H
#define SCAN_DIRS_H

#include "loader.h"

/* Does the whole job: finds every folder that may hold apps, scans them all
 * in priority order, and leaves the result in L->entries. */
void load_all(Loader *L);

#endif
