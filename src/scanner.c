#include <wayland-client.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void scan_apps_in_directory(const char *dir_path);
bool is_launchable_app(const char *file_path);

int main(void) {
    const char *xdg_dirs = getenv("XDG_DATA_DIRS");
    if (!xdg_dirs)
        return 1;

    char *dirs = strdup(xdg_dirs);
    if (!dirs)
        return 1;

    char app_dir[4096];

    for (char *dir = strtok(dirs, ":");
         dir != NULL;
         dir = strtok(NULL, ":")) {

        snprintf(app_dir, sizeof(app_dir),
                 "%s/applications", dir);

        scan_apps_in_directory(app_dir);
    }

    free(dirs);
    return 0;
}
void scan_apps_in_directory(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir)
        return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, "mimeinfo.cache") == 0)
            continue;
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s",dir_path,entry->d_name);

        bool is_app = is_launchable_app(full_path);
        if (is_app) {
            printf("%s\n",entry->d_name);
        }
    }

    closedir(dir);
}


bool is_launchable_app(const char *file_path) {
    FILE *fptr = fopen(file_path, "r");
    if (fptr == NULL) {
        return false;
    }

    bool is_app = false;
    bool is_hidden = false;
    bool has_exec = false;

    char line[4096];
    
    while (fgets(line, sizeof(line), fptr) != NULL) {
        if (strncmp(line, "Type=Application", 16) == 0) { is_app = true; }
        if (strncmp(line, "NoDisplay=true", 14) == 0)   { is_hidden = true; }
        if (strncmp(line, "Exec=", 5) == 0)             { has_exec = true; }
    }

    fclose(fptr);

    return (is_app == true && is_hidden == false && has_exec == true);
}
