#ifndef SCAN_SUB_DIRS_H
#define SCAN_SUB_DIRS_H

#include "loader.h"

/* Walks ONE "applications" folder, including every subfolder inside it.
 *   root  = the applications folder, e.g. "/usr/share/applications"
 *   rel   = which subfolder we're in right now ("" at the top, "kde4" below it)
 *   depth = how many levels down we are (starts at 0) */
void scan_dir(Loader *L, const char *root, const char *rel, int depth);

#endif
