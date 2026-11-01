#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "scanner.h"

static bool starts_with(const char *s, const char *prefix)
{
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

static bool is_launchable_app(const char *file_path)
{
    FILE *file = fopen(file_path, "r");
    if (!file)
        return false;

    bool is_app = false;
    bool is_hidden = false;
    bool has_exec = false;

    char line[1024];
    bool at_line_start = true;   /* guards against long lines being split by fgets */

    while (fgets(line, sizeof line, file)) {
        if (at_line_start) {
            if (starts_with(line, "Type=Application"))
                is_app = true;
            if (starts_with(line, "NoDisplay=true"))
                is_hidden = true;
            if (starts_with(line, "Exec="))
                has_exec = true;
        }
        at_line_start = (strchr(line, '\n') != NULL);
    }

    fclose(file);
    return is_app && !is_hidden && has_exec;
}

static bool push_app(application **apps, size_t *count, size_t *cap, const char *name)
{
    if (*count + 1 >= *cap) {            /* +1 keeps room for the terminator */
        size_t new_cap = *cap ? *cap * 2 : 32;
        application *tmp = realloc(*apps, new_cap * sizeof **apps);
        if (!tmp)
            return false;
        *apps = tmp;
        *cap = new_cap;
    }
    strncpy((*apps)[*count].appName, name, MAX_APP_NAME - 1);
    (*apps)[*count].appName[MAX_APP_NAME - 1] = '\0';
    (*count)++;
    return true;
}

static void scan_apps_in_directory(const char *dir_path,
                                   application **apps,
                                   size_t *count,
                                   size_t *cap)
{
    DIR *dir = opendir(dir_path);
    if (!dir)
        return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        const char *filename = entry->d_name;

        if (strcmp(filename, "mimeinfo.cache") == 0)
            continue;

        char full_path[4096];
        int n = snprintf(full_path, sizeof full_path, "%s/%s", dir_path, filename);
        if (n < 0 || (size_t)n >= sizeof full_path)
            continue;

        struct stat st;
        if (stat(full_path, &st) != 0 || !S_ISREG(st.st_mode))
            continue;

        if (!is_launchable_app(full_path))
            continue;

        if (!push_app(apps, count, cap, filename))
            break;
    }

    closedir(dir);
}

application *send_apps_in_directory(int request)
{
    application *apps = NULL;
    size_t count = 0, cap = 0;

    if (request == 0) {
        const char *xdg_dirs = getenv("XDG_DATA_DIRS");
        if (xdg_dirs == NULL)
            return NULL;

        /* strtok_r modifies its input, so work on a copy */
        char *copy = strdup(xdg_dirs);
        if (!copy)
            return NULL;

        char *saveptr = NULL;
        for (char *base_dir = strtok_r(copy, ":", &saveptr);
             base_dir != NULL;
             base_dir = strtok_r(NULL, ":", &saveptr)) {

            char app_dir[4096];
            int n = snprintf(app_dir, sizeof app_dir, "%s/applications", base_dir);
            if (n < 0 || (size_t)n >= sizeof app_dir)
                continue;

            scan_apps_in_directory(app_dir, &apps, &count, &cap);
        }
        free(copy);
    }

    /* terminator entry so the caller knows where the array ends */
    if (push_app(&apps, &count, &cap, "")) {
        /* ok */
    }
    return apps;
}
