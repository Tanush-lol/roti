/* parse_app.c - reads ONE .desktop file.
 *
 * A .desktop file looks like this:
 *
 *     [Desktop Entry]            <- a "group" header
 *     Type=Application           <- Key=Value lines
 *     Name=Firefox
 *     Name[de]=Feuerfuchs        <- same key, translated
 *     Exec=firefox %u
 *
 *     [Desktop Action new-window]   <- another group; we must IGNORE its keys
 *     Name=New Window
 *     Exec=firefox --new-window
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_app.h"
#include "log.h"

/* Handles a line like "Name[de]=Feuerfuchs".
 * 'locales' lists the languages we prefer, best first. A translation is only
 * kept if it ranks better (earlier in the list) than the one we already have. */
static void pick_localized_name(const char *key, const char *value,
                                const StrList *locales, size_t *best_rank,
                                char **localized_name) {
    size_t key_len = strlen(key);
    if (key[key_len - 1] != ']')
        return;                                        /* not a valid Name[...] */

    char *lang = xstrndup(key + 5, key_len - 6);       /* the text between [ and ] */

    for (size_t i = 0; i < locales->len; i++) {
        if (strcmp(lang, locales->items[i]) == 0 && i < *best_rank) {
            *best_rank = i;
            set_str(localized_name, value);
        }
    }
    free(lang);
}

bool parse_file(const char *path, const StrList *locales, DesktopEntry *out) {
    LOG("reading \"%s\"", path);
    memset(out, 0, sizeof *out);                       /* start with everything empty/false */

    FILE *f = fopen(path, "r");
    if (!f)
        return false;

    char *line = NULL;                 /* getline() allocates this buffer for us */
    size_t line_cap = 0;
    char *plain_name = NULL;           /* from "Name=..."     */
    char *localized_name = NULL;       /* from "Name[xx]=..." */
    size_t best_rank = locales->len;   /* "no translation found yet" */
    bool in_main_group = false;        /* are we currently inside [Desktop Entry]? */
    bool found_main_group = false;

    while (getline(&line, &line_cap, f) != -1) {
        char *text = trim(line);                       /* removes spaces and the \r of Windows line endings */

        if (text[0] == '\0' || text[0] == '#')         /* blank line or comment */
            continue;

        if (text[0] == '[') {                          /* a group header */
            in_main_group = (strcmp(text, "[Desktop Entry]") == 0);
            if (in_main_group)
                found_main_group = true;
            continue;
        }

        if (!in_main_group)                            /* ignore [Desktop Action ...] etc. */
            continue;

        char *equals = strchr(text, '=');              /* split "Key = Value" at the '=' */
        if (!equals)
            continue;
        *equals = '\0';
        char *key = trim(text);
        char *value = trim(equals + 1);

        if      (strcmp(key, "Type") == 0)            out->is_application = (strcmp(value, "Application") == 0);
        else if (strcmp(key, "Name") == 0)            set_str(&plain_name, value);
        else if (strcmp(key, "Exec") == 0)            set_str(&out->exec, value);
        else if (strcmp(key, "TryExec") == 0)         set_str(&out->try_exec, value);
        else if (strcmp(key, "OnlyShowIn") == 0)      set_str(&out->only_show_in, value);
        else if (strcmp(key, "NotShowIn") == 0)       set_str(&out->not_show_in, value);
        else if (strcmp(key, "Hidden") == 0)          out->hidden = (strcmp(value, "true") == 0);
        else if (strcmp(key, "NoDisplay") == 0)       out->no_display = (strcmp(value, "true") == 0);
        else if (strcmp(key, "DBusActivatable") == 0) out->dbus_activatable = (strcmp(value, "true") == 0);
        else if (strncmp(key, "Name[", 5) == 0)       pick_localized_name(key, value, locales, &best_rank, &localized_name);
    }

    free(line);
    fclose(f);

    if (!found_main_group) {                           /* not a real desktop file */
        free(plain_name);
        free(localized_name);
        entry_free(out);
        return false;
    }

    /* Prefer the translated name; fall back to the plain one. */
    if (localized_name) {
        out->name = localized_name;
        free(plain_name);
    } else {
        out->name = plain_name;
    }

    LOG("  Name=\"%s\"  Exec=\"%s\"", out->name ? out->name : "(none)",
        out->exec ? out->exec : "(none)");
    return true;
}
