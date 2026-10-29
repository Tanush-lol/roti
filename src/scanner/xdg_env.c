/* xdg_env.c - reads environment variables to answer two questions:
 *   1. Which language should app names be shown in?
 *   2. Which folders should we look for apps in?                    */

#include <stdlib.h>
#include <string.h>
#include "xdg_env.h"
#include "log.h"

void locale_candidates(StrList *out) {
    LOG("working out which Name[xx] keys to prefer, from the language settings");

    /* Same priority order as the C library: LC_ALL beats LC_MESSAGES beats LANG.
     * getenv() gives NULL if the variable doesn't exist; *lang == '\0' means
     * it exists but is empty. Both count as "not set". */
    const char *lang = getenv("LC_ALL");
    if (!lang || !*lang) lang = getenv("LC_MESSAGES");
    if (!lang || !*lang) lang = getenv("LANG");

    if (!lang || !*lang) {
        LOG("no language variable set, so only the plain Name will be used");
        return;
    }

    /* Work on a copy: we're about to modify the text, and the original
     * belongs to the program's environment, which we must not damage. */
    char *copy = xstrdup(lang);

    /* "en_US.UTF-8" -> "en_US"   (cut at the first '.' or '@') */
    char *cut = strpbrk(copy, ".@");
    if (cut)
        *cut = '\0';

    /* "C" and "POSIX" mean "no translation", so there's nothing to look up. */
    if (copy[0] != '\0' && strcmp(copy, "C") != 0 && strcmp(copy, "POSIX") != 0) {
        strlist_add(out, copy);                    /* first choice:  "en_US" */

        char *underscore = strchr(copy, '_');
        if (underscore) {
            *underscore = '\0';                    /* "en_US" -> "en"        */
            strlist_add(out, copy);                /* second choice: "en"    */
        }
        LOG("preferred name key is Name[%s]", out->items[0]);
    } else {
        LOG("language is C/POSIX, so only the plain Name will be used");
    }

    free(copy);
}

void data_dirs(StrList *out) {
    LOG("building the list of folders that may contain applications/");

    const char *home = getenv("HOME");
    const char *data_home = getenv("XDG_DATA_HOME");

    /* 1. The user's own folder comes first = highest priority. */
    if (data_home && *data_home)
        strlist_add(out, data_home);
    else if (home)
        strlist_add_owned(out, join_path(home, ".local/share"));

    /* 2. System folders. The spec says: if XDG_DATA_DIRS isn't set, use this default. */
    const char *system_dirs = getenv("XDG_DATA_DIRS");
    if (!system_dirs || !*system_dirs) {
        LOG("XDG_DATA_DIRS not set, using the default /usr/local/share:/usr/share");
        system_dirs = "/usr/local/share:/usr/share";
    }
    strlist_split(system_dirs, ':', out);

    /* 3. Flatpak folders. Login scripts normally add these to XDG_DATA_DIRS,
     *    but a launcher started from a window-manager config can miss them. */
    if (home)
        strlist_add_owned(out, join_path(home, ".local/share/flatpak/exports/share"));
    strlist_add(out, "/var/lib/flatpak/exports/share");

    LOG("%zu folders to look in", out->len);
}
