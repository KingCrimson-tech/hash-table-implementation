#include <stdio.h>
#include <string.h>

#include "hash_table.h"

static void print_help(void) {
    puts("Commands:");
    puts("  set <key> <value>  Insert or update a value (values may contain spaces)");
    puts("  get <key>          Look up a value");
    puts("  delete <key>       Remove a key");
    puts("  stats              Show item count and bucket capacity");
    puts("  help               Show this message");
    puts("  quit               Exit");
}

int main(void) {
    char line[1024];
    ht_hash_table *ht = ht_new();

    puts("Hash table test console. Type 'help' for commands.");
    while (1) {
        fputs("ht> ", stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        line[strcspn(line, "\n")] = '\0';

        char *command = strtok(line, " ");
        if (command == NULL) continue;
        if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) break;
        if (strcmp(command, "help") == 0) {
            print_help();
        } else if (strcmp(command, "stats") == 0) {
            printf("items: %d, capacity: %d\n", ht_count(ht), ht_capacity(ht));
        } else if (strcmp(command, "get") == 0) {
            char *key = strtok(NULL, " ");
            char *value = key == NULL ? NULL : ht_search(ht, key);
            printf("%s\n", value == NULL ? "(not found)" : value);
        } else if (strcmp(command, "delete") == 0) {
            char *key = strtok(NULL, " ");
            if (key == NULL) puts("usage: delete <key>");
            else {
                ht_delete(ht, key);
                puts("deleted (if present)");
            }
        } else if (strcmp(command, "set") == 0) {
            char *key = strtok(NULL, " ");
            char *value = strtok(NULL, "");
            if (key == NULL || value == NULL || *value == '\0') puts("usage: set <key> <value>");
            else {
                ht_insert(ht, key, value);
                puts("ok");
            }
        } else {
            puts("unknown command; type 'help' for commands");
        }
    }
    ht_del_hash_table(ht);
    return 0;
}
