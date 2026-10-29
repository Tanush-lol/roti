#ifndef PARSE_APP_H
#define PARSE_APP_H

#include <stdbool.h>
#include "entry.h"
#include "strutil.h"

/* Reads one .desktop file and fills *out.
 * Returns false if the file can't be opened or has no [Desktop Entry] section. */
bool parse_file(const char *path, const StrList *locales, DesktopEntry *out);

#endif
