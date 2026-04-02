#include <stdio.h>
#include <string.h>

int main() {
    char username[50], password[50], role[20];

    // Read input from Flask (stdin)
    scanf("%s", username);
    scanf("%s", password);
    scanf("%s", role);

    // Mentor
    if (strcmp(role, "mentor") == 0) {
        if (strcmp(username, "mentor") == 0 && strcmp(password, "1111") == 0) {
            printf("SUCCESS:mentor");
        } else {
            printf("FAIL:mentor");
        }
    }

    // Mentee
    else if (strcmp(role, "mentee") == 0) {
        if (strcmp(username, "mentee") == 0 && strcmp(password, "2222") == 0) {
            printf("SUCCESS:mentee");
        } else {
            printf("FAIL:mentee");
        }
    }

    // Manager
    else if (strcmp(role, "manager") == 0) {
        if (strcmp(username, "manager") == 0 && strcmp(password, "3333") == 0) {
            printf("SUCCESS:manager");
        } else {
            printf("FAIL:manager");
        }
    }

    else {
        printf("INVALID_ROLE");
    }

    return 0;
}