#ifndef SCANNER_STRUCTS_H
#define SCANNER_STRUCTS_H

/* Pull in the Wayland and xdg-shell types used by the struct below */
#include "w11-client.h"
#include "xdg-shell-client-protocol.h"

typedef struct {
    char **items;
    size_t len, cap;
} StrVec; /* growable list of owned strings */

typedef struct {
    char *id, *name, *exec, *try_exec, *only_show_in, *not_show_in;
    bool is_application, hidden, no_display, dbus_activatable;
} DesktopEntry;

typedef struct {
    DesktopEntry *items;
    size_t len, cap;
} EntryVec; /* growable list of entries */


typedef struct {
    StrVec seen;    /* IDs already claimed by a higher-priority directory */
    StrVec locales; /* e.g. {"en_US", "en"} */
    EntryVec entries;
} Loader;
#endif 
