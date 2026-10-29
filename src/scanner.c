/*
 * apps.c - desktop entry scanner (C rewrite of apps.cpp)
 *
 * Build:  gcc -std=c11 -Wall -Wextra apps.c -o apps
 * Run:    ./apps          list launchable apps (name + desktop file ID)
 *         ./apps -v       also log what each main-flow function is doing
 *         ./apps -vv      also log every small helper (very noisy)
 *
 * Logs go to stderr, the app list goes to stdout.
 *
 * Does:  XDG dir discovery (with defaults), recursion, desktop file IDs,
 *        first-ID-wins precedence, group-aware parsing, localized Name,
 *        Hidden / NoDisplay / OnlyShowIn / NotShowIn / TryExec / DBusActivatable.
 * Not yet: Exec field-code expansion, icons, actions.
 */

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_DEPTH 16 /* recursion limit, protects against symlink cycles */

/* ====================================================================
 * Logging
 * ==================================================================== */

static int g_verbose = 0; /* 0 = quiet, 1 = main flow, 2 = everything */

static void log_msg(int level, const char *fn, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

/* Prints "[function_name] message" to stderr when verbosity >= level. */
static void log_msg(int level, const char *fn, const char *fmt, ...) {
    if (g_verbose < level)
        return;
    fprintf(stderr, "[%-24s] ", fn);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

#define LOG(...) log_msg(1, __func__, __VA_ARGS__)  /* shown with -v  */
#define LOG2(...) log_msg(2, __func__, __VA_ARGS__) /* shown with -vv */

/* ====================================================================
 * Types
 * ==================================================================== */

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

/* ====================================================================
 * Memory wrappers (exit on allocation failure)
 * ==================================================================== */

/* malloc that aborts the program instead of returning NULL. */
static void *xmalloc(size_t n) {
    LOG2("allocating %zu bytes", n);
    void *p = malloc(n);
    if (!p) {
        perror("malloc");
        exit(1);
    }
    return p;
}

/* realloc that aborts the program instead of returning NULL. */
static void *xrealloc(void *p, size_t n) {
    LOG2("resizing block to %zu bytes", n);
    void *q = realloc(p, n);
    if (!q) {
        perror("realloc");
        exit(1);
    }
    return q;
}

/* strdup that aborts on failure. */
static char *xstrdup(const char *s) {
    LOG2("duplicating string \"%s\"", s);
    char *d = strdup(s);
    if (!d) {
        perror("strdup");
        exit(1);
    }
    return d;
}

/* strndup that aborts on failure; copies at most n bytes. */
static char *xstrndup(const char *s, size_t n) {
    LOG2("duplicating first %zu bytes of string", n);
    char *d = strndup(s, n);
    if (!d) {
        perror("strndup");
        exit(1);
    }
    return d;
}

/* ====================================================================
 * Small containers and string helpers
 * ==================================================================== */

/* Appends a string the vector takes ownership of (no copy). */
static void sv_push_owned(StrVec *v, char *s) {
    LOG2("appending \"%s\" to string vector (len %zu)", s, v->len);
    if (v->len == v->cap) {
        v->cap = v->cap ? v->cap * 2 : 8;
        v->items = xrealloc(v->items, v->cap * sizeof *v->items);
    }
    v->items[v->len++] = s;
}

/* Appends a copy of s to the vector. */
static void sv_push(StrVec *v, const char *s) {
    LOG2("copying and appending \"%s\"", s);
    sv_push_owned(v, xstrdup(s));
}

/* Linear search: true if the vector holds a string equal to s.
 * O(n) per lookup, which is fine for the few hundred IDs a system has. */
static bool sv_contains(const StrVec *v, const char *s) {
    LOG2("searching %zu strings for \"%s\"", v->len, s);
    for (size_t i = 0; i < v->len; i++)
        if (strcmp(v->items[i], s) == 0)
            return true;
    return false;
}

/* Frees every string and the array itself; leaves the vector empty. */
static void sv_free(StrVec *v) {
    LOG2("freeing string vector of %zu items", v->len);
    for (size_t i = 0; i < v->len; i++)
        free(v->items[i]);
    free(v->items);
    v->items = NULL;
    v->len = v->cap = 0;
}

/* Strips whitespace (including \r) from both ends, IN PLACE.
 * Returns a pointer into the same buffer. */
static char *trim(char *s) {
    LOG2("trimming whitespace from \"%s\"", s);
    while (*s && isspace((unsigned char)*s))
        s++;
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1]))
        s[--n] = '\0';
    return s;
}

/* Splits s on sep and appends each non-empty piece to out. */
static void split(const char *s, char sep, StrVec *out) {
    LOG2("splitting \"%s\" on '%c'", s, sep);
    const char *start = s;
    for (const char *p = s;; p++) {
        if (*p == sep || *p == '\0') {
            if (p > start)
                sv_push_owned(out, xstrndup(start, (size_t)(p - start)));
            if (*p == '\0')
                break;
            start = p + 1;
        }
    }
}

/* True if s ends with suffix. */
static bool has_suffix(const char *s, const char *suffix) {
    LOG2("checking whether \"%s\" ends with \"%s\"", s, suffix);
    size_t ls = strlen(s), lx = strlen(suffix);
    return ls >= lx && strcmp(s + ls - lx, suffix) == 0;
}

/* Returns a new "a/b" string. Caller frees. */
static char *join_path(const char *a, const char *b) {
    LOG2("joining \"%s\" and \"%s\"", a, b);
    size_t n = strlen(a) + strlen(b) + 2;
    char *r = xmalloc(n);
    snprintf(r, n, "%s/%s", a, b);
    return r;
}

/* Replaces *dst with a copy of val (frees the old value). */
static void set_str(char **dst, const char *val) {
    LOG2("storing value \"%s\"", val);
    free(*dst);
    *dst = xstrdup(val);
}

/* Desktop file ID: the path relative to applications/ with '/' -> '-'.
 * "kde4/foo.desktop" becomes "kde4-foo.desktop". Caller frees. */
static char *make_id(const char *rel_path) {
    LOG2("deriving desktop file ID from \"%s\"", rel_path);
    char *id = xstrdup(rel_path);
    for (char *p = id; *p; p++)
        if (*p == '/')
            *p = '-';
    return id;
}

/* ====================================================================
 * Entry storage
 * ==================================================================== */

/* Frees all strings owned by one entry. */
static void entry_free(DesktopEntry *e) {
    LOG2("freeing entry \"%s\"", e->name ? e->name : "(unnamed)");
    free(e->id);
    free(e->name);
    free(e->exec);
    free(e->try_exec);
    free(e->only_show_in);
    free(e->not_show_in);
}

/* Appends an entry (struct is copied; the vector takes over its strings). */
static void entry_vec_push(EntryVec *v, DesktopEntry e) {
    LOG2("appending entry \"%s\" (len %zu)", e.name, v->len);
    if (v->len == v->cap) {
        v->cap = v->cap ? v->cap * 2 : 64;
        v->items = xrealloc(v->items, v->cap * sizeof *v->items);
    }
    v->items[v->len++] = e;
}

/* Frees every entry and the array itself. */
static void entry_vec_free(EntryVec *v) {
    LOG2("freeing entry vector of %zu items", v->len);
    for (size_t i = 0; i < v->len; i++)
        entry_free(&v->items[i]);
    free(v->items);
    v->items = NULL;
    v->len = v->cap = 0;
}

/* ====================================================================
 * Environment: locale, PATH, XDG dirs
 * ==================================================================== */

/* Reads LC_ALL / LC_MESSAGES / LANG and produces lookup keys in priority
 * order, e.g. "en_US.UTF-8" -> {"en_US", "en"}. (@modifier ignored.) */
static void locale_candidates(StrVec *out) {
    LOG("figuring out which locale to use for Name[xx] lookups");
    const char *l = getenv("LC_ALL");
    if (!l || !*l)
        l = getenv("LC_MESSAGES");
    if (!l || !*l)
        l = getenv("LANG");
    if (!l || !*l) {
        LOG("no locale variable set; will use the plain Name key");
        return;
    }

    char *s = xstrdup(l);
    char *cut = strpbrk(s, ".@"); /* drop ".UTF-8" and "@modifier" */
    if (cut)
        *cut = '\0';

    if (*s && strcmp(s, "C") != 0 && strcmp(s, "POSIX") != 0) {
        sv_push(out, s); /* lang_COUNTRY */
        char *us = strchr(s, '_');
        if (us) {
            *us = '\0';
            sv_push(out, s); /* lang */
        }
        LOG("locale lookup keys: %zu candidate(s), first is \"%s\"", out->len,
            out->items[0]);
    } else {
        LOG("locale is C/POSIX; will use the plain Name key");
    }
    free(s);
}

/* True if cmd is executable: checked directly if it contains '/',
 * otherwise searched for along $PATH. Used for TryExec. */
static bool executable_exists(const char *cmd) {
    LOG("checking whether \"%s\" is an executable we can run", cmd);
    if (strchr(cmd, '/'))
        return access(cmd, X_OK) == 0;

    const char *path = getenv("PATH");
    if (!path)
        return false;

    StrVec dirs = {0};
    split(path, ':', &dirs);
    bool found = false;
    for (size_t i = 0; i < dirs.len && !found; i++) {
        char *full = join_path(dirs.items[i], cmd);
        found = access(full, X_OK) == 0;
        free(full);
    }
    sv_free(&dirs);
    return found;
}

/* Builds the ordered list of base data dirs: $XDG_DATA_HOME (or
 * ~/.local/share), then $XDG_DATA_DIRS (or its default), then the Flatpak
 * export dirs that profile scripts normally add. Earlier = higher priority. */
static void data_dirs(StrVec *out) {
    LOG("building the ordered list of XDG data directories");
    const char *home = getenv("HOME");
    const char *data_home = getenv("XDG_DATA_HOME");

    if (data_home && *data_home) {
        sv_push(out, data_home);
    } else if (home) {
        sv_push_owned(out, join_path(home, ".local/share"));
    }

    const char *xdg = getenv("XDG_DATA_DIRS");
    if (!xdg || !*xdg) {
        LOG("XDG_DATA_DIRS unset; using spec default /usr/local/share:/usr/share");
        xdg = "/usr/local/share:/usr/share";
    }
    split(xdg, ':', out);

    if (home)
        sv_push_owned(out, join_path(home, ".local/share/flatpak/exports/share"));
    sv_push(out, "/var/lib/flatpak/exports/share");
    LOG("%zu base directories to scan", out->len);
}

/* ====================================================================
 * Parsing
 * ==================================================================== */

/* Parses one .desktop file into *out. Only keys inside [Desktop Entry] are
 * read, so keys in [Desktop Action ...] groups can't leak into the entry.
 * Handles CRLF, spaces around '=', comments, and picks the best Name[xx].
 * Returns false if the file can't be read or has no [Desktop Entry] group. */
static bool parse_file(const char *path, const StrVec *locales, DesktopEntry *out) {
    LOG("parsing \"%s\"", path);
    memset(out, 0, sizeof *out);

    FILE *f = fopen(path, "r");
    if (!f)
        return false;

    char *line = NULL, *plain_name = NULL, *localized_name = NULL;
    size_t cap = 0, best = locales->len;
    bool in_main = false, seen_main = false;

    while (getline(&line, &cap, f) != -1) {
        char *t = trim(line);
        if (!*t || *t == '#')
            continue;

        if (*t == '[') { /* group header */
            in_main = strcmp(t, "[Desktop Entry]") == 0;
            if (in_main)
                seen_main = true;
            continue;
        }
        if (!in_main)
            continue;

        char *eq = strchr(t, '=');
        if (!eq)
            continue;
        *eq = '\0';
        char *key = trim(t);
        char *val = trim(eq + 1);

        if (strcmp(key, "Type") == 0)
            out->is_application = strcmp(val, "Application") == 0;
        else if (strcmp(key, "Name") == 0)
            set_str(&plain_name, val);
        else if (strcmp(key, "Exec") == 0)
            set_str(&out->exec, val);
        else if (strcmp(key, "TryExec") == 0)
            set_str(&out->try_exec, val);
        else if (strcmp(key, "OnlyShowIn") == 0)
            set_str(&out->only_show_in, val);
        else if (strcmp(key, "NotShowIn") == 0)
            set_str(&out->not_show_in, val);
        else if (strcmp(key, "Hidden") == 0)
            out->hidden = strcmp(val, "true") == 0;
        else if (strcmp(key, "NoDisplay") == 0)
            out->no_display = strcmp(val, "true") == 0;
        else if (strcmp(key, "DBusActivatable") == 0)
            out->dbus_activatable = strcmp(val, "true") == 0;
        else if (strncmp(key, "Name[", 5) == 0) {
            size_t klen = strlen(key);
            if (key[klen - 1] == ']') {
                char *loc = xstrndup(key + 5, klen - 6);
                for (size_t i = 0; i < locales->len; i++) {
                    if (strcmp(loc, locales->items[i]) == 0 && i < best) {
                        best = i; /* earlier candidate = better match */
                        set_str(&localized_name, val);
                    }
                }
                free(loc);
            }
        }
    }
    free(line);
    fclose(f);

    if (!seen_main) {
        free(plain_name);
        free(localized_name);
        entry_free(out);
        return false;
    }

    if (localized_name) {
        out->name = localized_name;
        free(plain_name);
    } else {
        out->name = plain_name;
    }
    LOG("parsed Name=\"%s\" Exec=\"%s\"", out->name ? out->name : "(none)",
        out->exec ? out->exec : "(none)");
    return true;
}

/* ====================================================================
 * Filtering
 * ==================================================================== */

/* True if any ';'-separated name in list is present in desktops. */
static bool list_matches(const char *list, const StrVec *desktops) {
    LOG2("checking desktop list \"%s\" against current desktop(s)", list);
    StrVec names = {0};
    split(list, ';', &names);
    bool hit = false;
    for (size_t i = 0; i < names.len && !hit; i++)
        hit = sv_contains(desktops, names.items[i]);
    sv_free(&names);
    return hit;
}

/* Applies OnlyShowIn / NotShowIn against $XDG_CURRENT_DESKTOP. */
static bool shown_in_current_desktop(const DesktopEntry *e) {
    LOG("applying OnlyShowIn/NotShowIn for \"%s\"", e->name);
    const char *cur = getenv("XDG_CURRENT_DESKTOP");
    StrVec desktops = {0};
    split(cur ? cur : "", ':', &desktops);

    bool ok = true;
    if (e->only_show_in && !list_matches(e->only_show_in, &desktops))
        ok = false;
    if (ok && e->not_show_in && list_matches(e->not_show_in, &desktops))
        ok = false;
    sv_free(&desktops);
    return ok;
}

/* Decides whether an entry belongs in a launcher list:
 * must be Type=Application, not Hidden/NoDisplay, have a Name and either
 * an Exec or DBusActivatable, pass the desktop filters, and its TryExec
 * (if any) must exist. */
static bool is_launchable(const DesktopEntry *e) {
    LOG("deciding whether \"%s\" should be listed", e->name ? e->name : "(unnamed)");
    if (!e->is_application) {
        LOG("  -> no: Type is not Application");
        return false;
    }
    if (e->hidden || e->no_display) {
        LOG("  -> no: Hidden or NoDisplay");
        return false;
    }
    if (!e->name || !*e->name) {
        LOG("  -> no: missing Name");
        return false;
    }
    if (!e->exec && !e->dbus_activatable) {
        LOG("  -> no: no Exec and not DBusActivatable");
        return false;
    }
    if (!shown_in_current_desktop(e)) {
        LOG("  -> no: filtered by OnlyShowIn/NotShowIn");
        return false;
    }
    if (e->try_exec && !executable_exists(e->try_exec)) {
        LOG("  -> no: TryExec binary not found");
        return false;
    }
    LOG("  -> yes");
    return true;
}

/* ====================================================================
 * Discovery
 * ==================================================================== */

/* Handles one .desktop file: derive its ID, skip it if a higher-priority
 * directory already claimed that ID, parse it, claim the ID, then keep the
 * entry only if it is launchable. The ID is claimed even for hidden entries
 * so a Hidden=true user file shadows the system one. */
static void handle_file(Loader *L, const char *full_path, const char *rel_path) {
    char *id = make_id(rel_path);
    LOG("considering \"%s\" (id=%s)", full_path, id);

    if (sv_contains(&L->seen, id)) {
        LOG("  id already claimed by a higher-priority dir; skipping");
        free(id);
        return;
    }

    DesktopEntry e;
    if (!parse_file(full_path, &L->locales, &e)) {
        LOG("  not a valid desktop file; skipping");
        free(id);
        return;
    }

    sv_push(&L->seen, id);
    if (!is_launchable(&e)) {
        entry_free(&e);
        free(id);
        return;
    }

    e.id = id;
    entry_vec_push(&L->entries, e);
}

/* Recursively walks root/rel. stat() follows symlinks, so symlinked files
 * and dirs (Nix, stow) work; depth is capped to survive symlink loops. */
static void scan_dir(Loader *L, const char *root, const char *rel, int depth) {
    char *dirpath = *rel ? join_path(root, rel) : xstrdup(root);
    LOG("scanning directory \"%s\" (depth %d)", dirpath, depth);

    DIR *d = opendir(dirpath);
    if (!d) {
        LOG("  cannot open (missing or no permission); skipping");
        free(dirpath);
        return;
    }

    struct dirent *de;
    while ((de = readdir(d)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        char *child_rel = *rel ? join_path(rel, de->d_name) : xstrdup(de->d_name);
        char *full = join_path(root, child_rel);

        struct stat st;
        if (stat(full, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                if (depth < MAX_DEPTH)
                    scan_dir(L, root, child_rel, depth + 1);
            } else if (S_ISREG(st.st_mode) && has_suffix(de->d_name, ".desktop")) {
                handle_file(L, full, child_rel);
            }
        }
        free(full);
        free(child_rel);
    }
    closedir(d);
    free(dirpath);
}

static void load_all(Loader *L) {
    LOG("loading all applications");
    locale_candidates(&L->locales);//function

    StrVec bases = {0}; //struct
    data_dirs(&bases);//function
    for (size_t i = 0; i < bases.len; i++) {
        char *root = join_path(bases.items[i], "applications");
        scan_dir(L, root, "", 0);
        free(root);
    }
    sv_free(&bases);//function
    LOG("done: %zu launchable applications found", L->entries.len);
}

/* ====================================================================
 * Output
 * ==================================================================== */

/* qsort comparator: case-insensitive by display name. */
static int cmp_entries(const void *a, const void *b) {
    LOG2("comparing two entries by name");
    const DesktopEntry *x = a, *y = b;
    return strcasecmp(x->name, y->name);
}

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            g_verbose = 1;// const
        } else if (strcmp(argv[i], "-vv") == 0) {
            g_verbose = 2;// const
        } else {
            fprintf(stderr, "usage: %s [-v | -vv]\n", argv[0]);
            return strcmp(argv[i], "-h") == 0 ? 0 : 1;
        }
    }
    LOG("starting; verbosity level %d", g_verbose);

    Loader L = {0};
    load_all(&L);//function

    LOG("sorting %zu entries by name", L.entries.len);
    qsort(L.entries.items, L.entries.len, sizeof *L.entries.items, cmp_entries);

    for (size_t i = 0; i < L.entries.len; i++)
        printf("application :%s  (%s)\n", L.entries.items[i].name, L.entries.items[i].id);// where applications are getting logged
    fprintf(stderr, "fprint output :%zu applications\n", L.entries.len);

    entry_vec_free(&L.entries);
    sv_free(&L.seen);
    sv_free(&L.locales);
    return 0;
}
