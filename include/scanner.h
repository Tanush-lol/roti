#ifndef SCANNER_H
#define SCANNER_H

#include <stddef.h>

#define MAX_APP_NAME 256

typedef struct {
    char appName[MAX_APP_NAME];
    //future for  app execution path
} application;

application *send_apps_in_directory(int request);

#endif /* SCANNER_H */
