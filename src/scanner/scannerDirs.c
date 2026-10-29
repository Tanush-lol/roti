/* scan_dirs.c - the top level of the scan.
 * Decides WHICH folders to look in, then asks scan_subdirs.c to walk each one. */

#include <stdlib.h>
#include "scan_dirs.h"
#include "log.h"
#include "scan_subdirs.h"
#include "xdg_env.h"

void load_all(Loader *L) {
    LOG("starting the scan");

    locale_candidates(&L->locales);            /* which language to prefer */

    StrList bases = {0};
    data_dirs(&bases);                         /* e.g. ~/.local/share, /usr/share, ... */

    /* Order matters: earlier folders have priority over later ones. */
    for (size_t i = 0; i < bases.len; i++) {
        char *apps_dir = join_path(bases.items[i], "applications");
        scan_dir(L, apps_dir, "", 0);          /* "" = start at the top of that folder */
        free(apps_dir);
    }

    strlist_free(&bases);
    LOG("scan finished: %zu apps will be listed", L->entries.len);
}
