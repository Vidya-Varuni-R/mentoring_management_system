/*
 * structures.h  —  Shared Data Structures (REFACTORED)
 * Mentoring Management System, SSN College of Engineering
 *
 * Design rules followed by every module:
 *   1. main() loads ALL data files ONCE into linked lists.
 *   2. Hash tables and (optional) BSTs are built from those lists.
 *   3. All operations work IN MEMORY on the data structures.
 *   4. main() saves the (modified) lists back to disk ONCE before exit.
 *
 * No function (other than the load/save helpers used by main) touches files.
 *
 * Data structures used:
 *   - Singly linked lists  : primary storage for users, mentees, notes,
 *                            meetings.
 *   - Queue (linked list)  : meeting requests, processed FIFO.
 *   - Hash tables (chaining): fast lookup of mentees by roll and users
 *                             by username / entity_id.
 *   - Binary Search Tree   : optional, used by manager.c only to print
 *                            mentees in sorted order (by roll).
 *
 * Style: plain C, no typedef, simple comments, easy to follow.
 */

#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── Limits ──────────────────────────────────────────────────── */
#define MAX_LEN       64
#define MAX_NOTE_LEN 256
#define HASH_SIZE     53     /* small prime, fine for student project */

/* ── File Paths (relative to c_backend/) ─────────────────────── */
#define USERS_FILE    "../data/users.txt"
#define MENTEES_FILE  "../data/mentees.txt"
#define MEETINGS_FILE "../data/meetings.txt"
#define NOTES_FILE    "../data/notes.txt"
#define REQUESTS_FILE "../data/requests.txt"

/* ════════════════════════════════════════════════════════════════
   RECORD STRUCTS  (kept identical in fields to original version)
   ════════════════════════════════════════════════════════════════ */

struct User {
    int  id;
    char username[MAX_LEN];
    char password[MAX_LEN];
    char role[MAX_LEN];
    char entity_id[MAX_LEN];
    struct User *next;            /* linked-list pointer  */
    struct User *hnext_user;      /* hash chain (by username)  */
    struct User *hnext_entity;    /* hash chain (by entity_id) */
};

struct Mentee {
    char roll[MAX_LEN];
    char name[MAX_LEN];
    char dept[MAX_LEN];
    char cgpa[MAX_LEN];
    char attendance[MAX_LEN];
    char mentor_id[MAX_LEN];
    struct Mentee *next;          /* linked-list pointer */
    struct Mentee *hnext;         /* hash chain (by roll) */
};

struct Note {
    int  id;
    char roll[MAX_LEN];
    char mentor_id[MAX_LEN];
    char note[MAX_NOTE_LEN];
    char date[MAX_LEN];
    struct Note *next;
};

struct Request {
    int  id;
    char roll[MAX_LEN];
    char mentor_id[MAX_LEN];
    char date[MAX_LEN];
    char time[MAX_LEN];
    char mode[MAX_LEN];
    char purpose[MAX_NOTE_LEN];
    char status[MAX_LEN];
    struct Request *next;         /* queue link (FIFO) */
};

struct Meeting {
    int  id;
    char roll[MAX_LEN];
    char mentor_id[MAX_LEN];
    char date[MAX_LEN];
    char time[MAX_LEN];
    char mode[MAX_LEN];
    char agenda[MAX_NOTE_LEN];
    char status[MAX_LEN];
    struct Meeting *next;
};

/* ── Queue (for meeting requests) ────────────────────────────── */
struct RequestQueue {
    struct Request *head;   /* dequeue from head */
    struct Request *tail;   /* enqueue at tail   */
};

/* ── Hash tables ─────────────────────────────────────────────── */
struct MenteeHash { struct Mentee *bucket[HASH_SIZE]; };
struct UserHash   { struct User   *byUsername[HASH_SIZE];
                    struct User   *byEntity[HASH_SIZE]; };

/* ── BST node (manager display only) ─────────────────────────── */
struct BstNode {
    struct Mentee  *m;       /* points to mentee in the linked list */
    struct BstNode *left;
    struct BstNode *right;
};

/* ════════════════════════════════════════════════════════════════
   SIMPLE HELPERS
   ════════════════════════════════════════════════════════════════ */

static inline unsigned int hashStr(const char *s) {
    unsigned int h = 0;
    while (*s) { h = h * 31u + (unsigned char)(*s); s++; }
    return h % HASH_SIZE;
}

static inline void copyField(char *dst, const char *src, int max) {
    strncpy(dst, src, max - 1);
    dst[max - 1] = '\0';
}

/* ════════════════════════════════════════════════════════════════
   LINKED LIST: APPEND HELPERS  (preserve file/insertion order)
   ════════════════════════════════════════════════════════════════ */

static inline void appendUser(struct User **head, struct User **tail, struct User *u) {
    u->next = NULL;
    if (!*head) { *head = *tail = u; } else { (*tail)->next = u; *tail = u; }
}
static inline void appendMentee(struct Mentee **head, struct Mentee **tail, struct Mentee *m) {
    m->next = NULL;
    if (!*head) { *head = *tail = m; } else { (*tail)->next = m; *tail = m; }
}
static inline void appendNote(struct Note **head, struct Note **tail, struct Note *n) {
    n->next = NULL;
    if (!*head) { *head = *tail = n; } else { (*tail)->next = n; *tail = n; }
}
static inline void appendMeeting(struct Meeting **head, struct Meeting **tail, struct Meeting *mt) {
    mt->next = NULL;
    if (!*head) { *head = *tail = mt; } else { (*tail)->next = mt; *tail = mt; }
}

/* ════════════════════════════════════════════════════════════════
   QUEUE OPERATIONS (Request)
   ════════════════════════════════════════════════════════════════ */

static inline void queueInit(struct RequestQueue *q) { q->head = q->tail = NULL; }

static inline void enqueueRequest(struct RequestQueue *q, struct Request *r) {
    r->next = NULL;
    if (!q->head) { q->head = q->tail = r; }
    else          { q->tail->next = r; q->tail = r; }
}

/* dequeue removes and returns head (FIFO); not always needed but provided */
static inline struct Request *dequeueRequest(struct RequestQueue *q) {
    if (!q->head) return NULL;
    struct Request *r = q->head;
    q->head = r->next;
    if (!q->head) q->tail = NULL;
    r->next = NULL;
    return r;
}

/* ════════════════════════════════════════════════════════════════
   HASH TABLE OPERATIONS
   ════════════════════════════════════════════════════════════════ */

static inline void menteeHashInit(struct MenteeHash *h) {
    for (int i = 0; i < HASH_SIZE; i++) h->bucket[i] = NULL;
}
static inline void menteeHashInsert(struct MenteeHash *h, struct Mentee *m) {
    unsigned int idx = hashStr(m->roll);
    m->hnext = h->bucket[idx];
    h->bucket[idx] = m;
}
static inline struct Mentee *menteeHashFind(struct MenteeHash *h, const char *roll) {
    unsigned int idx = hashStr(roll);
    for (struct Mentee *p = h->bucket[idx]; p; p = p->hnext)
        if (strcmp(p->roll, roll) == 0) return p;
    return NULL;
}
static inline void menteeHashRemove(struct MenteeHash *h, const char *roll) {
    unsigned int idx = hashStr(roll);
    struct Mentee *prev = NULL, *p = h->bucket[idx];
    while (p) {
        if (strcmp(p->roll, roll) == 0) {
            if (prev) prev->hnext = p->hnext; else h->bucket[idx] = p->hnext;
            return;
        }
        prev = p; p = p->hnext;
    }
}

static inline void userHashInit(struct UserHash *h) {
    for (int i = 0; i < HASH_SIZE; i++) { h->byUsername[i] = NULL; h->byEntity[i] = NULL; }
}
static inline void userHashInsert(struct UserHash *h, struct User *u) {
    unsigned int a = hashStr(u->username);
    u->hnext_user = h->byUsername[a];
    h->byUsername[a] = u;
    unsigned int b = hashStr(u->entity_id);
    u->hnext_entity = h->byEntity[b];
    h->byEntity[b] = u;
}
static inline struct User *userHashFindByName(struct UserHash *h, const char *username) {
    unsigned int idx = hashStr(username);
    for (struct User *p = h->byUsername[idx]; p; p = p->hnext_user)
        if (strcmp(p->username, username) == 0) return p;
    return NULL;
}
static inline struct User *userHashFindByEntity(struct UserHash *h, const char *entity_id) {
    unsigned int idx = hashStr(entity_id);
    for (struct User *p = h->byEntity[idx]; p; p = p->hnext_entity)
        if (strcmp(p->entity_id, entity_id) == 0) return p;
    return NULL;
}

/* ════════════════════════════════════════════════════════════════
   BST OPERATIONS  (manager display only — sort by mentee roll)
   ════════════════════════════════════════════════════════════════ */

static inline struct BstNode *bstInsert(struct BstNode *root, struct Mentee *m) {
    if (!root) {
        struct BstNode *n = (struct BstNode *)malloc(sizeof(struct BstNode));
        n->m = m; n->left = n->right = NULL;
        return n;
    }
    if (strcmp(m->roll, root->m->roll) < 0) root->left  = bstInsert(root->left,  m);
    else                                    root->right = bstInsert(root->right, m);
    return root;
}

/* Caller supplies a visit() callback for each mentee in sorted order. */
static inline void bstInorder(struct BstNode *root, void (*visit)(struct Mentee *)) {
    if (!root) return;
    bstInorder(root->left,  visit);
    visit(root->m);
    bstInorder(root->right, visit);
}

static inline void bstFree(struct BstNode *root) {
    if (!root) return;
    bstFree(root->left);
    bstFree(root->right);
    free(root);
}

/* ════════════════════════════════════════════════════════════════
   LOAD-ONCE FILE READERS
   (called from main() only — never from feature functions)
   ════════════════════════════════════════════════════════════════ */

/* users.txt   :  id,username,password,role,entity_id            */
static inline struct User *loadUsersList(void) {
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) return NULL;
    struct User *head = NULL, *tail = NULL;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (!strlen(line)) continue;
        struct User *u = (struct User *)calloc(1, sizeof(struct User));
        char *t;
        t = strtok(line, ","); if (!t) { free(u); continue; } u->id = atoi(t);
        t = strtok(NULL, ","); if (!t) { free(u); continue; } copyField(u->username,  t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(u); continue; } copyField(u->password,  t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(u); continue; } copyField(u->role,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(u); continue; } copyField(u->entity_id, t, MAX_LEN);
        appendUser(&head, &tail, u);
    }
    fclose(fp);
    return head;
}

/* mentees.txt :  roll,name,dept,cgpa,attendance,mentor_id        */
static inline struct Mentee *loadMenteesList(void) {
    FILE *fp = fopen(MENTEES_FILE, "r");
    if (!fp) return NULL;
    struct Mentee *head = NULL, *tail = NULL;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (!strlen(line)) continue;
        struct Mentee *m = (struct Mentee *)calloc(1, sizeof(struct Mentee));
        char *t;
        t = strtok(line, ","); if (!t) { free(m); continue; } copyField(m->roll,       t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(m); continue; } copyField(m->name,       t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(m); continue; } copyField(m->dept,       t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(m); continue; } copyField(m->cgpa,       t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(m); continue; } copyField(m->attendance, t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(m); continue; } copyField(m->mentor_id,  t, MAX_LEN);
        appendMentee(&head, &tail, m);
    }
    fclose(fp);
    return head;
}

/* notes.txt   :  id,roll,mentor_id,note,date                     */
static inline struct Note *loadNotesList(void) {
    FILE *fp = fopen(NOTES_FILE, "r");
    if (!fp) return NULL;
    struct Note *head = NULL, *tail = NULL;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (!strlen(line)) continue;
        struct Note *n = (struct Note *)calloc(1, sizeof(struct Note));
        char *t;
        t = strtok(line, ","); if (!t) { free(n); continue; } n->id = atoi(t);
        t = strtok(NULL, ","); if (!t) { free(n); continue; } copyField(n->roll,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(n); continue; } copyField(n->mentor_id, t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(n); continue; } copyField(n->note,      t, MAX_NOTE_LEN);
        t = strtok(NULL, ",");
        if (t) copyField(n->date, t, MAX_LEN);
        appendNote(&head, &tail, n);
    }
    fclose(fp);
    return head;
}

/* requests.txt:  id,roll,mentor_id,date,time,mode,purpose,status — into queue */
static inline void loadRequestsQueue(struct RequestQueue *q) {
    queueInit(q);
    FILE *fp = fopen(REQUESTS_FILE, "r");
    if (!fp) return;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (!strlen(line)) continue;
        struct Request *r = (struct Request *)calloc(1, sizeof(struct Request));
        char *t;
        t = strtok(line, ","); if (!t) { free(r); continue; } r->id = atoi(t);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->roll,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->mentor_id, t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->date,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->time,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->mode,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->purpose,   t, MAX_NOTE_LEN);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->status,    t, MAX_LEN);
        enqueueRequest(q, r);
    }
    fclose(fp);
}

/* meetings.txt:  id,roll,mentor_id,date,time,mode,agenda,status  */
static inline struct Meeting *loadMeetingsList(void) {
    FILE *fp = fopen(MEETINGS_FILE, "r");
    if (!fp) return NULL;
    struct Meeting *head = NULL, *tail = NULL;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (!strlen(line)) continue;
        struct Meeting *mt = (struct Meeting *)calloc(1, sizeof(struct Meeting));
        char *t;
        t = strtok(line, ","); if (!t) { free(mt); continue; } mt->id = atoi(t);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->roll,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->mentor_id, t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->date,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->time,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->mode,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->agenda,    t, MAX_NOTE_LEN);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->status,    t, MAX_LEN);
        appendMeeting(&head, &tail, mt);
    }
    fclose(fp);
    return head;
}

/* ════════════════════════════════════════════════════════════════
   SAVE-ONCE FILE WRITERS  (called from main() only)
   ════════════════════════════════════════════════════════════════ */

static inline void saveMenteesList(struct Mentee *head) {
    FILE *fp = fopen(MENTEES_FILE, "w");
    if (!fp) return;
    for (struct Mentee *m = head; m; m = m->next)
        fprintf(fp, "%s,%s,%s,%s,%s,%s\n",
            m->roll, m->name, m->dept, m->cgpa, m->attendance, m->mentor_id);
    fclose(fp);
}

static inline void saveNotesList(struct Note *head) {
    FILE *fp = fopen(NOTES_FILE, "w");
    if (!fp) return;
    for (struct Note *n = head; n; n = n->next)
        fprintf(fp, "%d,%s,%s,%s,%s\n",
            n->id, n->roll, n->mentor_id, n->note, n->date);
    fclose(fp);
}

static inline void saveRequestsQueue(struct RequestQueue *q) {
    FILE *fp = fopen(REQUESTS_FILE, "w");
    if (!fp) return;
    for (struct Request *r = q->head; r; r = r->next)
        fprintf(fp, "%d,%s,%s,%s,%s,%s,%s,%s\n",
            r->id, r->roll, r->mentor_id, r->date,
            r->time, r->mode, r->purpose, r->status);
    fclose(fp);
}

static inline void saveMeetingsList(struct Meeting *head) {
    FILE *fp = fopen(MEETINGS_FILE, "w");
    if (!fp) return;
    for (struct Meeting *m = head; m; m = m->next)
        fprintf(fp, "%d,%s,%s,%s,%s,%s,%s,%s\n",
            m->id, m->roll, m->mentor_id, m->date,
            m->time, m->mode, m->agenda, m->status);
    fclose(fp);
}

/* ── Build hash tables from already-loaded lists ─────────────── */

static inline void buildMenteeHash(struct MenteeHash *h, struct Mentee *list) {
    menteeHashInit(h);
    for (struct Mentee *m = list; m; m = m->next) menteeHashInsert(h, m);
}
static inline void buildUserHash(struct UserHash *h, struct User *list) {
    userHashInit(h);
    for (struct User *u = list; u; u = u->next) userHashInsert(h, u);
}

/* ── Counting helpers (no file IO) ───────────────────────────── */
static inline int countMentees(struct Mentee *head)   { int n=0; for (; head; head=head->next) n++; return n; }
static inline int countMeetings(struct Meeting *head) { int n=0; for (; head; head=head->next) n++; return n; }

#endif /* STRUCTURES_H */
