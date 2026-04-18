/*
 * auth.c  —  Authentication Module  (REFACTORED to use linked list + hash table)
 * Called by Flask: ./auth <username> <password>
 * Output:
 *   SUCCESS:<role>:<user_id>:<entity_id>
 *   FAIL:invalid_credentials | FAIL:missing_args
 *
 * Data flow:
 *   main() loads users.txt ONCE into a linked list, builds a hash table,
 *   does an O(1) lookup, prints, exits. No saves needed (read-only).
 */
#include "structures.h"

/* Authenticate by hash lookup; password compared in memory only. */
static void authenticate(struct UserHash *uh, const char *username, const char *password) {
    struct User *u = userHashFindByName(uh, username);
    if (u && strcmp(u->password, password) == 0) {
        printf("SUCCESS:%s:%d:%s\n", u->role, u->id, u->entity_id);
        return;
    }
    printf("FAIL:invalid_credentials\n");
}

int main(int argc, char *argv[]) {
    if (argc < 3) { printf("FAIL:missing_args\n"); return 1; }

    /* ── LOAD ONCE ───────────────────────────────────────────── */
    struct User *users = loadUsersList();
    struct UserHash uh;
    buildUserHash(&uh, users);

    /* ── OPERATE IN MEMORY ───────────────────────────────────── */
    authenticate(&uh, argv[1], argv[2]);

    /* ── No save: read-only command. ─────────────────────────── */
    return 0;
}
