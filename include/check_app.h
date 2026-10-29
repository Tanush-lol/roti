#ifndef CHECK_APP_H
#define CHECK_APP_H

#include <stdbool.h>
#include "entry.h"

/* Should this app appear in a launcher list? (yes/no) */
bool is_launchable(const DesktopEntry *e);

#endif
