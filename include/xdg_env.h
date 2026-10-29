#ifndef XDG_ENV_H
#define XDG_ENV_H

#include "strutil.h"

/* Reads LANG etc. and fills 'out' with the Name[xx] keys to prefer,
 * best first. Example: LANG=en_US.UTF-8  ->  {"en_US", "en"} */
void locale_candidates(StrList *out);

/* Fills 'out' with the folders that may contain an "applications" folder,
 * highest priority first (user folder, then system folders, then Flatpak). */
void data_dirs(StrList *out);

#endif
