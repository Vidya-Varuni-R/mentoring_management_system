/*
    manager.c - Manager Module
    SSN College of Engineering, Dept. of IT

    Compile : gcc manager.c -o manager.exe

    Commands:
        manager.exe report      -> summary report (totals, dept counts)
        manager.exe mentees     -> display all mentees
        manager.exe meetings    -> display all meetings
        manager.exe flagged     -> display mentees with no meetings
        manager.exe requests    -> display all pending requests (queue)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"


struct QueueNode {
    char regno[50];
    char message[300];
    struct QueueNode *next;
};

struct Queue {
    struct QueueNode *front;
    struct QueueNode *rear;
    int size;
};
// ...existing code...

#define MENTEES_FILE  "../data/mentees.txt"
#define MEETINGS_FILE "../data/meetings.txt"
#define REQUESTS_FILE "../data/requests.txt"

#define MAX_MENTEES 200
#define MAX_DEPTS   20


/* ═══════════════════════════════════════════════════════════════
   SECTION 1 — LINKED LIST: Load / Free
   ═══════════════════════════════════════════════════════════════ */

/* safe strtok wrapper: returns "" instead of crashing on NULL */
char* safe_tok(char *src, const char *delim) {
    char *t = strtok(src, delim);
    return t ? t : "";
}

/* Load mentees.txt into a linked list */
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

/* Load meetings.txt into a linked list */
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
        char *ag = strtok(NULL, "");
        strcpy(n->agenda, ag ? ag : "");

        if (!head) { head = n; tail = n; }
        else       { tail->next = n; tail = n; }
    }
    fclose(fp);
    return head;
}

/* Free a mentee linked list */
void freeMentees(struct Mentee *head) {
    while (head) {
        struct Mentee *t = head;
        head = head->next;
        free(t);
    }
}

/* Free a meeting linked list */
void freeMeetings(struct Meeting *head) {
    while (head) {
        struct Meeting *t = head;
        head = head->next;
        free(t);
    }
}


/* ═══════════════════════════════════════════════════════════════
   SECTION 2 — ARRAY: Convert Linked List → Array
   ═══════════════════════════════════════════════════════════════ */

/*
   Converts the mentee linked list into a dynamically allocated array.
   Sets *count to the number of elements.
   Caller must free() the returned array.
*/
struct Mentee** linkedListToArray(struct Mentee *head, int *count) {
    /* first pass: count nodes */
    int n = 0;
    for (struct Mentee *c = head; c; c = c->next) n++;
    *count = n;

    if (n == 0) return NULL;

    /* allocate array of pointers */
    struct Mentee **arr = malloc(n * sizeof(struct Mentee *));
    int i = 0;
    for (struct Mentee *c = head; c; c = c->next)
        arr[i++] = c;

    return arr;
}

/* Count total meetings from linked list */
int countMeetings(struct Meeting *head) {
    int n = 0;
    for (struct Meeting *c = head; c; c = c->next) n++;
    return n;
}


/* ═══════════════════════════════════════════════════════════════
   SECTION 3 — QUEUE: Enqueue / Dequeue / Display
   ═══════════════════════════════════════════════════════════════ */

/* Initialise an empty queue */
void initQueue(struct Queue *q) {
    q->front = NULL;
    q->rear  = NULL;
    q->size  = 0;
}

/* Add a request to the rear of the queue (FIFO enqueue) */
void enqueue(struct Queue *q, const char *regno, const char *message) {
    struct QueueNode *n = malloc(sizeof(struct QueueNode));
    strcpy(n->regno,   regno);
    strcpy(n->message, message);
    n->next = NULL;

    if (!q->rear) {
        q->front = n;
        q->rear  = n;
    } else {
        q->rear->next = n;
        q->rear       = n;
    }
    q->size++;
}

/* Remove and return the front node (caller must free it) */
struct QueueNode* dequeue(struct Queue *q) {
    if (!q->front) return NULL;

    struct QueueNode *n = q->front;
    q->front = q->front->next;
    if (!q->front) q->rear = NULL;
    q->size--;
    return n;
}

/* Load requests.txt into a Queue */
void loadRequestsIntoQueue(struct Queue *q) {
    FILE *fp = fopen(REQUESTS_FILE, "r");
    if (!fp) return;

    char line[300];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (!strlen(line)) continue;

        char tmp[300]; strcpy(tmp, line);
        char *regno = safe_tok(tmp, ",");
        char *msg   = strtok(NULL, "");
        enqueue(q, regno, msg ? msg : "");
    }
    fclose(fp);
}

/* Free all remaining nodes in a queue */
void freeQueue(struct Queue *q) {
    struct QueueNode *n;
    while ((n = dequeue(q)) != NULL) free(n);
}


/* ═══════════════════════════════════════════════════════════════
   SECTION 4 — DISPLAY FUNCTIONS
   ═══════════════════════════════════════════════════════════════ */

/* Display all mentees (reads from linked list, walks the array) */
void displayAllMentees() {
    struct Mentee *head = loadMentees();
    if (!head) { printf("EMPTY:no_mentees\n"); return; }

    int count = 0;
    struct Mentee **arr = linkedListToArray(head, &count);

    for (int i = 0; i < count; i++)
        printf("MENTEE:%s,%s,%s\n", arr[i]->regno, arr[i]->name, arr[i]->dept);

    free(arr);
    freeMentees(head);
}

/* Display all meetings */
void displayAllMeetings() {
    struct Meeting *head = loadMeetings();
    if (!head) { printf("EMPTY:no_meetings\n"); return; }

    for (struct Meeting *c = head; c; c = c->next)
        printf("MEETING:%s,%s,%s,%s,%s\n",
               c->regno, c->date, c->time, c->mode, c->agenda);

    freeMeetings(head);
}

/*
   Display flagged mentees — those who appear in mentees.txt
   but have NO entry in meetings.txt.
*/
void displayFlaggedMentees() {
    struct Mentee  *mhead = loadMentees();
    struct Meeting *ghead = loadMeetings();

    if (!mhead) { printf("EMPTY:no_mentees\n"); return; }

    int flagged = 0;
    for (struct Mentee *m = mhead; m; m = m->next) {
        int hasMeeting = 0;
        for (struct Meeting *g = ghead; g; g = g->next) {
            if (strcmp(m->regno, g->regno) == 0) {
                hasMeeting = 1;
                break;
            }
        }
        if (!hasMeeting) {
            printf("FLAGGED:%s,%s\n", m->regno, m->name);
            flagged++;
        }
    }
    if (flagged == 0) printf("EMPTY:no_flagged_mentees\n");

    freeMeetings(ghead);
    freeMentees(mhead);
}

/* Display all pending requests from the queue (FIFO order) */
void displayRequestsQueue() {
    struct Queue q;
    initQueue(&q);
    loadRequestsIntoQueue(&q);

    if (q.size == 0) { printf("EMPTY:no_requests\n"); return; }

    /* Walk the queue without destroying it */
    for (struct QueueNode *n = q.front; n; n = n->next)
        printf("REQUEST:%s,%s\n", n->regno, n->message);

    freeQueue(&q);
}


/* ═══════════════════════════════════════════════════════════════
   SECTION 5 — REPORT GENERATOR
   ═══════════════════════════════════════════════════════════════ */

/*
   Generates a full summary report:
     - Total mentees
     - Total meetings
     - Mentee count per department
     - Flagged mentee count
*/
void generateReport() {
    /* ── load data ── */
    struct Mentee  *mhead = loadMentees();
    struct Meeting *ghead = loadMeetings();

    int menteeCount  = 0;
    int meetingCount = countMeetings(ghead);

    /* Convert linked list to array for array-based processing */
    struct Mentee **arr = linkedListToArray(mhead, &menteeCount);

    printf("TOTAL_MENTEES:%d\n",  menteeCount);
    printf("TOTAL_MEETINGS:%d\n", meetingCount);

    /* ── department counts using parallel arrays ── */
    char  deptNames[MAX_DEPTS][50];
    int   deptCounts[MAX_DEPTS];
    int   deptTotal = 0;

    for (int i = 0; i < menteeCount; i++) {
        char *d = arr[i]->dept;
        int found = 0;
        for (int j = 0; j < deptTotal; j++) {
            if (strcmp(deptNames[j], d) == 0) {
                deptCounts[j]++;
                found = 1; break;
            }
        }
        if (!found && deptTotal < MAX_DEPTS) {
            strcpy(deptNames[deptTotal], d);
            deptCounts[deptTotal] = 1;
            deptTotal++;
        }
    }

    for (int j = 0; j < deptTotal; j++)
        printf("DEPT:%s:%d\n", deptNames[j], deptCounts[j]);

    /* ── flagged count ── */
    int flagged = 0;
    for (int i = 0; i < menteeCount; i++) {
        int hasMeeting = 0;
        for (struct Meeting *g = ghead; g; g = g->next) {
            if (strcmp(arr[i]->regno, g->regno) == 0) {
                hasMeeting = 1; break;
            }
        }
        if (!hasMeeting) flagged++;
    }
    printf("FLAGGED_COUNT:%d\n", flagged);

    /* ── cleanup ── */
    free(arr);
    freeMeetings(ghead);
    freeMentees(mhead);
}


/* ═══════════════════════════════════════════════════════════════
   SECTION 6 — MAIN
   ═══════════════════════════════════════════════════════════════ */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("ERROR:no_command\n");
        printf("Usage:\n");
        printf("  manager.exe report\n");
        printf("  manager.exe mentees\n");
        printf("  manager.exe meetings\n");
        printf("  manager.exe flagged\n");
        printf("  manager.exe requests\n");
        return 1;
    }

    if      (strcmp(argv[1], "report")   == 0) generateReport();
    else if (strcmp(argv[1], "mentees")  == 0) displayAllMentees();
    else if (strcmp(argv[1], "meetings") == 0) displayAllMeetings();
    else if (strcmp(argv[1], "flagged")  == 0) displayFlaggedMentees();
    else if (strcmp(argv[1], "requests") == 0) displayRequestsQueue();
    else    printf("ERROR:unknown_command\n");

    return 0;
} 

