/*
 * auth.c — Authentication Module
 * Usage : ./auth <username> <password>
 * Output: SUCCESS:<role>:<user_id>:<entity_id>
 *         FAIL:invalid_credentials
 *
 * Uses: User BST (key = username) for O(log n) lookup.
 */

#include "structures.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("FAIL:missing_args\n");
        return 1;
    }

    char *username = argv[1];
    char *password = argv[2];

    /* Load users.txt into BST */
    struct UserNode *root = loadUsers();

    /* BST search by username — O(log n) */
    struct UserNode *found = userBstSearch(root, username);

    if (found != NULL && strcmp(found->data.password, password) == 0) {
        printf("SUCCESS:%s:%d:%s\n",
               found->data.role,
               found->data.id,
               found->data.entity_id);
    } else {
        printf("FAIL:invalid_credentials\n");
    }

    userBstFree(root);
    return 0;
}
