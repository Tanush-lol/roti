/* entry.c - creating and freeing DesktopEntry / EntryList. */

#include <stdlib.h>
#include "entry.h"
#include "strutil.h"

void entry_free(DesktopEntry *e) {
    free(e->id);
    free(e->name);
    free(e->exec);
    free(e->try_exec);
    free(e->only_show_in);
    free(e->not_show_in);
}

void entrylist_add(EntryList *l, DesktopEntry e) {
    if (l->len == l->cap) {                       /* full: grow the array */
        l->cap = l->cap ? l->cap * 2 : 64;
        l->items = xrealloc(l->items, l->cap * sizeof *l->items);
    }
    l->items[l->len++] = e;                       /* struct is copied in */
}

void entrylist_free(EntryList *l) {
    for (size_t i = 0; i < l->len; i++)
        entry_free(&l->items[i]);
    free(l->items);
    l->items = NULL;
    l->len = l->cap = 0;
}
