#ifndef LOADER_H
#define LOADER_H

#include "entry.h"
#include "strutil.h"

/* Everything the scanner needs to remember while it works.
 * One of these is created in main() and passed around by pointer. */
typedef struct {
    StrList seen;       /* IDs we've already handled. If the same ID shows up in a
                           lower-priority folder, we ignore it (first one wins). */
    StrList locales;    /* e.g. {"en_US", "en"} - which Name[xx] to prefer        */
    EntryList entries;  /* the final result: apps that should be listed           */
} Loader;

#endif
