#ifndef ENTRY_H
#define ENTRY_H

#include <stdbool.h>
#include <stddef.h>

/* Everything we remember about ONE .desktop file. */
typedef struct {
    char *id;             /* "firefox.desktop" (or "kde4-foo.desktop" for subfolders) */
    char *name;           /* what the user sees: "Firefox"                            */
    char *exec;           /* command to run (not expanded yet)                        */
    char *try_exec;       /* if set, this program must exist or the app is hidden     */
    char *only_show_in;   /* "GNOME;KDE;" -> only show on these desktops              */
    char *not_show_in;    /* "GNOME;"     -> hide on these desktops                   */
    bool is_application;  /* Type=Application                                         */
    bool hidden;          /* Hidden=true     ("deleted")                              */
    bool no_display;      /* NoDisplay=true  ("don't list it")                        */
    bool dbus_activatable;/* can be launched over D-Bus even without Exec             */
} DesktopEntry;

/* A list of entries that grows as you add to it. */
typedef struct {
    DesktopEntry *items;
    size_t len;
    size_t cap;
} EntryList;

void entry_free(DesktopEntry *e);               /* frees the strings inside one entry */
void entrylist_add(EntryList *l, DesktopEntry e);
void entrylist_free(EntryList *l);              /* frees every entry + the array      */

#endif
