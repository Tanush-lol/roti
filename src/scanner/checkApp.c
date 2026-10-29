
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "check_app.h"
#include "log.h"
#include "strutil.h"

/* Is there a program we're allowed to run called 'cmd'?
 * If cmd contains a '/', it's a path: check it directly.
 * Otherwise look for it in every folder listed in $PATH. */
static bool executable_exists(const char *cmd) {
    LOG("checking that the program \"%s\" exists", cmd);

    if (strchr(cmd, '/'))
        return access(cmd, X_OK) == 0;                 /* X_OK = "is it executable?" */

    const char *path = getenv("PATH");
    if (!path)
        return false;

    StrList dirs = {0};
    strlist_split(path, ':', &dirs);

    bool found = false;
    for (size_t i = 0; i < dirs.len && !found; i++) {
        char *full = join_path(dirs.items[i], cmd);    /* e.g. "/usr/bin/firefox" */
        found = (access(full, X_OK) == 0);
        free(full);
    }
    strlist_free(&dirs);
    return found;
}

/* 'list' looks like "GNOME;KDE;". True if any name in it is also in 'desktops'. */
static bool list_matches(const char *list, const StrList *desktops) {
    StrList names = {0};
    strlist_split(list, ';', &names);

    bool hit = false;
    for (size_t i = 0; i < names.len && !hit; i++)
        hit = strlist_has(desktops, names.items[i]);

    strlist_free(&names);
    return hit;
}

/* Checks OnlyShowIn / NotShowIn against the desktop we are running on
 * (the XDG_CURRENT_DESKTOP variable, e.g. "Hyprland" or "GNOME:Unity"). */
static bool shown_in_current_desktop(const DesktopEntry *e) {
    LOG("checking OnlyShowIn / NotShowIn for \"%s\"", e->name);

    const char *current = getenv("XDG_CURRENT_DESKTOP");
    StrList desktops = {0};
    strlist_split(current ? current : "", ':', &desktops);

    bool ok = true;
    if (e->only_show_in && !list_matches(e->only_show_in, &desktops))
        ok = false;                                    /* "only on X" but we're not on X */
    if (ok && e->not_show_in && list_matches(e->not_show_in, &desktops))
        ok = false;                                    /* "not on X" and we are on X     */

    strlist_free(&desktops);
    return ok;
}

bool is_launchable(const DesktopEntry *e) {
    LOG("deciding whether \"%s\" should be listed", e->name ? e->name : "(unnamed)");

    if (!e->is_application)          { LOG("  no: Type is not Application");        return false; }
    if (e->hidden || e->no_display)  { LOG("  no: Hidden or NoDisplay is set");     return false; }
    if (!e->name || !e->name[0])     { LOG("  no: it has no Name");                 return false; }
    if (!e->exec && !e->dbus_activatable)
                                     { LOG("  no: nothing to run (no Exec)");       return false; }
    if (!shown_in_current_desktop(e)){ LOG("  no: not meant for this desktop");     return false; }
    if (e->try_exec && !executable_exists(e->try_exec))
                                     { LOG("  no: its TryExec program is missing"); return false; }

    LOG("  yes");
    return true;
}
