/*
    mentor.c - Mentor Module
    SSN College of Engineering, Dept. of IT

    Compile : gcc mentor.c -o mentor.exe

    Commands:
        mentor.exe add      <regno> <n> <dept>
        mentor.exe view
        mentor.exe search   <regno>
        mentor.exe update   <regno> <n> <dept>
        mentor.exe delete   <regno>
        mentor.exe schedule <regno> <date> <time> <mode> <agenda words...>
        mentor.exe meetings
        mentor.exe requests
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"

#define MENTEES_FILE  "../data/mentees.txt"
#define MEETINGS_FILE "../data/meetings.txt"
#define REQUESTS_FILE "../data/requests.txt"


/* ── safe token helper: returns "" instead of crashing on NULL ── */
char* safe_tok(char *src, const char *delim) {
    char *t = strtok(src, delim);
    return t ? t : "";
}


/* ── MENTEE: load / save / free ───────────────────────────────── */

struct Mentee* loadMentees() {
    struct Mentee *head = NULL, *tail = NULL;
    FILE *fp = fopen(MENTEES_FILE, "r");
    if (!fp) return NULL;

    char line[200];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (!strlen(line)) continue;

        struct Mentee *n = malloc(sizeof(struct Mentee));
        n->next = NULL;
        char tmp[200]; strcpy(tmp, line);
        strcpy(n->regno, safe_tok(tmp,  ","));
        strcpy(n->name,  safe_tok(NULL, ","));
        strcpy(n->dept,  safe_tok(NULL, ","));

        if (!head) { head = n; tail = n; }
        else       { tail->next = n; tail = n; }
    }
    fclose(fp);
    return head;
}

void saveMentees(struct Mentee *head) {
    FILE *fp = fopen(MENTEES_FILE, "w");
    if (!fp) { printf("ERROR:cannot_open_file\n"); return; }
    for (struct Mentee *c = head; c; c = c->next)
        fprintf(fp, "%s,%s,%s\n", c->regno, c->name, c->dept);
    fclose(fp);
}

void freeMentees(struct Mentee *head) {
    while (head) { struct Mentee *t = head; head = head->next; free(t); }
}


/* ── MENTEE: operations ───────────────────────────────────────── */

void addMentee(char *regno, char *name, char *dept) {
    struct Mentee *head = loadMentees();

    for (struct Mentee *c = head; c; c = c->next) {
        if (strcmp(c->regno, regno) == 0) {
            printf("ERROR:duplicate_regno\n");
            freeMentees(head); return;
        }
    }

    struct Mentee *n = malloc(sizeof(struct Mentee));
    strcpy(n->regno, regno);
    strcpy(n->name,  name);
    strcpy(n->dept,  dept);
    n->next = NULL;

    if (!head) {
        head = n;
    } else {
        struct Mentee *c = head;
        while (c->next) c = c->next;
        c->next = n;
    }

    saveMentees(head);
    freeMentees(head);
    printf("SUCCESS:mentee_added\n");
}

void viewMentees() {
    struct Mentee *head = loadMentees();
    if (!head) { printf("EMPTY:no_mentees\n"); return; }
    for (struct Mentee *c = head; c; c = c->next)
        printf("DATA:%s,%s,%s\n", c->regno, c->name, c->dept);
    freeMentees(head);
}

void searchMentee(char *regno) {
    struct Mentee *head = loadMentees();
    for (struct Mentee *c = head; c; c = c->next) {
        if (strcmp(c->regno, regno) == 0) {
            printf("FOUND:%s,%s,%s\n", c->regno, c->name, c->dept);
            freeMentees(head); return;
        }
    }
    printf("NOT_FOUND:%s\n", regno);
    freeMentees(head);
}

void updateMentee(char *regno, char *name, char *dept) {
    struct Mentee *head = loadMentees();
    int found = 0;
    for (struct Mentee *c = head; c; c = c->next) {
        if (strcmp(c->regno, regno) == 0) {
            strcpy(c->name, name);
            strcpy(c->dept, dept);
            found = 1; break;
        }
    }
    if (!found) { printf("NOT_FOUND:%s\n", regno); freeMentees(head); return; }
    saveMentees(head);
    freeMentees(head);
    printf("SUCCESS:mentee_updated\n");
}

void deleteMentee(char *regno) {
    struct Mentee *head = loadMentees();
    struct Mentee *prev = NULL, *cur = head;
    int found = 0;

    while (cur) {
        if (strcmp(cur->regno, regno) == 0) {
            if (!prev) head = cur->next;
            else       prev->next = cur->next;
            free(cur);
            found = 1; break;
        }
        prev = cur; cur = cur->next;
    }

    if (!found) { printf("NOT_FOUND:%s\n", regno); freeMentees(head); return; }
    saveMentees(head);
    freeMentees(head);
    printf("SUCCESS:mentee_deleted\n");
}


/* ── MEETING: load / save / free ──────────────────────────────── */

struct Meeting* loadMeetings() {
    struct Meeting *head = NULL, *tail = NULL;
    FILE *fp = fopen(MEETINGS_FILE, "r");
    if (!fp) return NULL;

    char line[300];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (!strlen(line)) continue;

        struct Meeting *n = malloc(sizeof(struct Meeting));
        n->next = NULL;
        char tmp[300]; strcpy(tmp, line);
        strcpy(n->regno,  safe_tok(tmp,  ","));
        strcpy(n->date,   safe_tok(NULL, ","));
        strcpy(n->time,   safe_tok(NULL, ","));
        strcpy(n->mode,   safe_tok(NULL, ","));
        /* agenda is everything after the 4th comma */
        char *ag = strtok(NULL, "");
        strcpy(n->agenda, ag ? ag : "");

        if (!head) { head = n; tail = n; }
        else       { tail->next = n; tail = n; }
    }
    fclose(fp);
    return head;
}

void saveMeetings(struct Meeting *head) {
    FILE *fp = fopen(MEETINGS_FILE, "w");
    if (!fp) { printf("ERROR:cannot_open_file\n"); return; }
    for (struct Meeting *c = head; c; c = c->next)
        fprintf(fp, "%s,%s,%s,%s,%s\n", c->regno, c->date, c->time, c->mode, c->agenda);
    fclose(fp);
}

void freeMeetings(struct Meeting *head) {
    while (head) { struct Meeting *t = head; head = head->next; free(t); }
}


/* ── MEETING: operations ──────────────────────────────────────── */

/*
    scheduleMeeting
    ---------------
    argc and argv are passed in so we can join argv[6], argv[7], ...
    into one agenda string. This handles agendas with spaces like
    "Discuss CGPA improvement" which would otherwise split into
    multiple separate arguments.
*/
void scheduleMeeting(int argc, char *argv[]) {
    char *regno = argv[2];
    char *date  = argv[3];
    char *time  = argv[4];
    char *mode  = argv[5];

    /* join all remaining args into one agenda string */
    char agenda[200] = "";
    for (int i = 6; i < argc; i++) {
        if (i > 6) strcat(agenda, " ");
        strcat(agenda, argv[i]);
    }
    if (strlen(agenda) == 0) strcpy(agenda, "No agenda");

    struct Meeting *head = loadMeetings();

    struct Meeting *n = malloc(sizeof(struct Meeting));
    strcpy(n->regno,  regno);
    strcpy(n->date,   date);
    strcpy(n->time,   time);
    strcpy(n->mode,   mode);
    strcpy(n->agenda, agenda);
    n->next = NULL;

    if (!head) {
        head = n;
    } else {
        struct Meeting *c = head;
        while (c->next) c = c->next;
        c->next = n;
    }

    saveMeetings(head);
    freeMeetings(head);
    printf("SUCCESS:meeting_scheduled\n");
}

void viewMeetings() {
    struct Meeting *head = loadMeetings();
    if (!head) { printf("EMPTY:no_meetings\n"); return; }
    for (struct Meeting *c = head; c; c = c->next)
        printf("MEETING:%s,%s,%s,%s,%s\n", c->regno, c->date, c->time, c->mode, c->agenda);
    freeMeetings(head);
}


/* ── REQUEST: load / free / view ──────────────────────────────── */

struct Request* loadRequests() {
    struct Request *head = NULL, *tail = NULL;
    FILE *fp = fopen(REQUESTS_FILE, "r");
    if (!fp) return NULL;

    char line[300];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (!strlen(line)) continue;

        struct Request *n = malloc(sizeof(struct Request));
        n->next = NULL;
        char tmp[300]; strcpy(tmp, line);
        strcpy(n->regno,   safe_tok(tmp,  ","));
        char *msg = strtok(NULL, "");
        strcpy(n->message, msg ? msg : "");

        if (!head) { head = n; tail = n; }
        else       { tail->next = n; tail = n; }
    }
    fclose(fp);
    return head;
}

void freeRequests(struct Request *head) {
    while (head) { struct Request *t = head; head = head->next; free(t); }
}

void viewRequests() {
    struct Request *head = loadRequests();
    if (!head) { printf("EMPTY:no_requests\n"); return; }
    for (struct Request *c = head; c; c = c->next)
        printf("REQUEST:%s,%s\n", c->regno, c->message);
    freeRequests(head);
}


/* ── MAIN ─────────────────────────────────────────────────────── */

int main(int argc, char *argv[]) {
    if (argc < 2) { printf("ERROR:no_command\n"); return 1; }

    if      (strcmp(argv[1], "add")      == 0 && argc >= 5) addMentee(argv[2], argv[3], argv[4]);
    else if (strcmp(argv[1], "view")     == 0)               viewMentees();
    else if (strcmp(argv[1], "search")   == 0 && argc >= 3) searchMentee(argv[2]);
    else if (strcmp(argv[1], "update")   == 0 && argc >= 5) updateMentee(argv[2], argv[3], argv[4]);
    else if (strcmp(argv[1], "delete")   == 0 && argc >= 3) deleteMentee(argv[2]);
    else if (strcmp(argv[1], "schedule") == 0 && argc >= 6) scheduleMeeting(argc, argv);
    else if (strcmp(argv[1], "meetings") == 0)               viewMeetings();
    else if (strcmp(argv[1], "requests") == 0)               viewRequests();
    else printf("ERROR:unknown_command\n");

    return 0;
}