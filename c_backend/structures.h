/*
 * structures.h
 * Mentoring Management System — SSN College of Engineering
 *
 * Data structures used:
 *   BST          : Users (key = username), Mentees (key = roll)
 *   Linked list  : Meetings, Notes
 *   Queue        : Requests (FIFO, linked list)
 *
 * Rules:
 *   No typedef, no inline, no complex macros.
 *   Separate data struct and BST node struct for each entity.
 *   Load from file -> operate on data structures -> save to file.
 */

#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ── Limits ───────────────────────────────────────────────── */
#define MAX_LEN      64
#define MAX_NOTE_LEN 256

/* ── File Paths ───────────────────────────────────────────── */
#define USERS_FILE    "../data/users.txt"
#define MENTEES_FILE  "../data/mentees.txt"
#define MEETINGS_FILE "../data/meetings.txt"
#define NOTES_FILE    "../data/notes.txt"
#define REQUESTS_FILE "../data/requests.txt"


/* =============================================================
   DATA STRUCTS
   Each struct holds only the data fields for that entity.
   The BST node struct (below) wraps these with tree pointers.
============================================================= */

struct User {
    int  id;
    char username[MAX_LEN];
    char password[MAX_LEN];
    char role[MAX_LEN];
    char entity_id[MAX_LEN];
};

struct Mentee {
    char roll[MAX_LEN];
    char name[MAX_LEN];
    char dept[MAX_LEN];
    char cgpa[MAX_LEN];
    char attendance[MAX_LEN];
    char mentor_id[MAX_LEN];
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
    struct Request *next;
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


/* =============================================================
   BST NODE STRUCTS
   Separate node struct wraps data + left/right pointers.
   This keeps data clean and tree structure separate.
============================================================= */

struct UserNode {
    struct User     data;
    struct UserNode *left;
    struct UserNode *right;
};

struct MenteeNode {
    struct Mentee     data;
    struct MenteeNode *left;
    struct MenteeNode *right;
};


/* =============================================================
   QUEUE STRUCT (for Requests — FIFO)
============================================================= */

struct RequestQueue {
    struct Request *head;
    struct Request *tail;
};


/* =============================================================
   HELPER: safe string copy
============================================================= */

void copyField(char *dst, const char *src, int max) {
    strncpy(dst, src, max - 1);
    dst[max - 1] = '\0';
}


/* =============================================================
   HELPER: today's date string
============================================================= */

void getToday(char out[MAX_LEN]) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(out, MAX_LEN, "%Y-%m-%d", tm);
}


/* =============================================================
   BST OPERATIONS — User (key = username)
============================================================= */

struct UserNode *userBstInsert(struct UserNode *root, struct User data) {
    if (root == NULL) {
        struct UserNode *n = calloc(1, sizeof(struct UserNode));
        n->data  = data;
        n->left  = NULL;
        n->right = NULL;
        return n;
    }
    int cmp = strcmp(data.username, root->data.username);
    if (cmp < 0) {
        root->left  = userBstInsert(root->left,  data);
    } else if (cmp > 0) {
        root->right = userBstInsert(root->right, data);
    }
    return root;
}

struct UserNode *userBstSearch(struct UserNode *root, const char *username) {
    if (root == NULL) return NULL;
    int cmp = strcmp(username, root->data.username);
    if (cmp == 0) return root;
    if (cmp < 0)  return userBstSearch(root->left,  username);
    return             userBstSearch(root->right, username);
}

/* Search by entity_id — linear scan because key is username not entity_id */
struct UserNode *userBstSearchByEntity(struct UserNode *root, const char *entity_id) {
    if (root == NULL) return NULL;
    if (strcmp(root->data.entity_id, entity_id) == 0) return root;
    struct UserNode *left  = userBstSearchByEntity(root->left,  entity_id);
    if (left) return left;
    return                  userBstSearchByEntity(root->right, entity_id);
}

void userBstFree(struct UserNode *root) {
    if (root == NULL) return;
    userBstFree(root->left);
    userBstFree(root->right);
    free(root);
}

/* Inorder traversal: visits every user in ascending username order */
void userBstInorder(struct UserNode *root, void (*visit)(struct User *)) {
    if (root == NULL) return;
    userBstInorder(root->left,  visit);
    visit(&root->data);
    userBstInorder(root->right, visit);
}

/* Save helper used by userBstSave */
static FILE *_userSaveFp = NULL;
void _userSaveVisitor(struct User *u) {
    fprintf(_userSaveFp, "%d,%s,%s,%s,%s\n",
            u->id, u->username, u->password, u->role, u->entity_id);
}

void userBstSave(struct UserNode *root) {
    _userSaveFp = fopen(USERS_FILE, "w");
    if (!_userSaveFp) return;
    userBstInorder(root, _userSaveVisitor);
    fclose(_userSaveFp);
    _userSaveFp = NULL;
}


/* =============================================================
   BST OPERATIONS — Mentee (key = roll)
============================================================= */

struct MenteeNode *menteeBstInsert(struct MenteeNode *root, struct Mentee data) {
    if (root == NULL) {
        struct MenteeNode *n = calloc(1, sizeof(struct MenteeNode));
        n->data  = data;
        n->left  = NULL;
        n->right = NULL;
        return n;
    }
    int cmp = strcmp(data.roll, root->data.roll);
    if (cmp < 0) {
        root->left  = menteeBstInsert(root->left,  data);
    } else if (cmp > 0) {
        root->right = menteeBstInsert(root->right, data);
    }
    return root;
}

struct MenteeNode *menteeBstSearch(struct MenteeNode *root, const char *roll) {
    if (root == NULL) return NULL;
    int cmp = strcmp(roll, root->data.roll);
    if (cmp == 0) return root;
    if (cmp < 0)  return menteeBstSearch(root->left,  roll);
    return              menteeBstSearch(root->right, roll);
}

/* Find the smallest node (leftmost) — used during delete */
struct MenteeNode *menteeBstMin(struct MenteeNode *root) {
    while (root->left != NULL) root = root->left;
    return root;
}

struct MenteeNode *menteeBstDelete(struct MenteeNode *root, const char *roll) {
    if (root == NULL) return NULL;
    int cmp = strcmp(roll, root->data.roll);
    if (cmp < 0) {
        root->left  = menteeBstDelete(root->left,  roll);
    } else if (cmp > 0) {
        root->right = menteeBstDelete(root->right, roll);
    } else {
        /* Found the node to delete */
        if (root->left == NULL) {
            struct MenteeNode *tmp = root->right;
            free(root);
            return tmp;
        }
        if (root->right == NULL) {
            struct MenteeNode *tmp = root->left;
            free(root);
            return tmp;
        }
        /* Two children: replace with inorder successor (min of right subtree) */
        struct MenteeNode *successor = menteeBstMin(root->right);
        root->data  = successor->data;
        root->right = menteeBstDelete(root->right, successor->data.roll);
    }
    return root;
}

void menteeBstFree(struct MenteeNode *root) {
    if (root == NULL) return;
    menteeBstFree(root->left);
    menteeBstFree(root->right);
    free(root);
}

/* Inorder traversal: visits every mentee in ascending roll order */
void menteeBstInorder(struct MenteeNode *root, void (*visit)(struct Mentee *)) {
    if (root == NULL) return;
    menteeBstInorder(root->left,  visit);
    visit(&root->data);
    menteeBstInorder(root->right, visit);
}

/* Save helper used by menteeBstSave */
static FILE *_menteeSaveFp = NULL;
void _menteeSaveVisitor(struct Mentee *m) {
    fprintf(_menteeSaveFp, "%s,%s,%s,%s,%s,%s\n",
            m->roll, m->name, m->dept, m->cgpa, m->attendance, m->mentor_id);
}

void menteeBstSave(struct MenteeNode *root) {
    _menteeSaveFp = fopen(MENTEES_FILE, "w");
    if (!_menteeSaveFp) return;
    menteeBstInorder(root, _menteeSaveVisitor);
    fclose(_menteeSaveFp);
    _menteeSaveFp = NULL;
}


/* =============================================================
   QUEUE OPERATIONS — Request (FIFO linked list)
============================================================= */

void queueInit(struct RequestQueue *q) {
    q->head = NULL;
    q->tail = NULL;
}

void queueEnqueue(struct RequestQueue *q, struct Request *r) {
    r->next = NULL;
    if (q->tail == NULL) {
        q->head = r;
        q->tail = r;
    } else {
        q->tail->next = r;
        q->tail = r;
    }
}

struct Request *queueDequeue(struct RequestQueue *q) {
    if (q->head == NULL) return NULL;
    struct Request *r = q->head;
    q->head = r->next;
    if (q->head == NULL) q->tail = NULL;
    r->next = NULL;
    return r;
}

void queueFree(struct RequestQueue *q) {
    struct Request *cur = q->head;
    while (cur != NULL) {
        struct Request *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    q->head = NULL;
    q->tail = NULL;
}


/* =============================================================
   LINKED LIST HELPERS — Meetings
============================================================= */

void meetingAppend(struct Meeting **head, struct Meeting **tail, struct Meeting *m) {
    m->next = NULL;
    if (*head == NULL) {
        *head = m;
        *tail = m;
    } else {
        (*tail)->next = m;
        *tail = m;
    }
}

void meetingFree(struct Meeting *head) {
    while (head != NULL) {
        struct Meeting *tmp = head;
        head = head->next;
        free(tmp);
    }
}


/* =============================================================
   LINKED LIST HELPERS — Notes
============================================================= */

void noteAppend(struct Note **head, struct Note **tail, struct Note *n) {
    n->next = NULL;
    if (*head == NULL) {
        *head = n;
        *tail = n;
    } else {
        (*tail)->next = n;
        *tail = n;
    }
}

void noteFree(struct Note *head) {
    while (head != NULL) {
        struct Note *tmp = head;
        head = head->next;
        free(tmp);
    }
}


/* =============================================================
   FILE LOADERS — build data structures from files
============================================================= */

/* Load users.txt into a User BST (key = username) */
struct UserNode *loadUsers(void) {
    struct UserNode *root = NULL;
    FILE *fp = fopen(USERS_FILE, "r");
    if (fp == NULL) return NULL;

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;

        struct User u;
        char tmp[256];
        strcpy(tmp, line);

        char *t;
        t = strtok(tmp, ","); if (!t) continue; u.id = atoi(t);
        t = strtok(NULL, ","); if (!t) continue; copyField(u.username,  t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) continue; copyField(u.password,  t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) continue; copyField(u.role,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) continue; copyField(u.entity_id, t, MAX_LEN);

        root = userBstInsert(root, u);
    }
    fclose(fp);
    return root;
}

/* Load mentees.txt into a Mentee BST (key = roll) */
struct MenteeNode *loadMentees(void) {
    struct MenteeNode *root = NULL;
    FILE *fp = fopen(MENTEES_FILE, "r");
    if (fp == NULL) return NULL;

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;

        struct Mentee m;
        char tmp[512];
        strcpy(tmp, line);

        char *t;
        t = strtok(tmp, ","); if (!t) continue; copyField(m.roll,       t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) continue; copyField(m.name,       t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) continue; copyField(m.dept,       t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) continue; copyField(m.cgpa,       t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) continue; copyField(m.attendance, t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) continue; copyField(m.mentor_id,  t, MAX_LEN);

        root = menteeBstInsert(root, m);
    }
    fclose(fp);
    return root;
}

/* Load meetings.txt into a linked list */
struct Meeting *loadMeetings(struct Meeting **tail_out) {
    struct Meeting *head = NULL;
    struct Meeting *tail = NULL;
    FILE *fp = fopen(MEETINGS_FILE, "r");
    if (fp == NULL) { if (tail_out) *tail_out = NULL; return NULL; }

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;

        struct Meeting *mt = calloc(1, sizeof(struct Meeting));
        char tmp[512];
        strcpy(tmp, line);

        char *t;
        t = strtok(tmp, ","); if (!t) { free(mt); continue; } mt->id = atoi(t);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->roll,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->mentor_id, t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->date,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->time,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->mode,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->agenda,    t, MAX_NOTE_LEN);
        t = strtok(NULL, ","); if (!t) { free(mt); continue; } copyField(mt->status,    t, MAX_LEN);

        meetingAppend(&head, &tail, mt);
    }
    fclose(fp);
    if (tail_out) *tail_out = tail;
    return head;
}

/* Load requests.txt into a queue (FIFO) */
void loadRequests(struct RequestQueue *q) {
    queueInit(q);
    FILE *fp = fopen(REQUESTS_FILE, "r");
    if (fp == NULL) return;

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;

        struct Request *r = calloc(1, sizeof(struct Request));
        char tmp[512];
        strcpy(tmp, line);

        char *t;
        t = strtok(tmp, ","); if (!t) { free(r); continue; } r->id = atoi(t);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->roll,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->mentor_id, t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->date,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->time,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->mode,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->purpose,   t, MAX_NOTE_LEN);
        t = strtok(NULL, ","); if (!t) { free(r); continue; } copyField(r->status,    t, MAX_LEN);

        queueEnqueue(q, r);
    }
    fclose(fp);
}

/* Load notes.txt into a linked list */
struct Note *loadNotes(struct Note **tail_out) {
    struct Note *head = NULL;
    struct Note *tail = NULL;
    FILE *fp = fopen(NOTES_FILE, "r");
    if (fp == NULL) { if (tail_out) *tail_out = NULL; return NULL; }

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;

        struct Note *n = calloc(1, sizeof(struct Note));
        char tmp[512];
        strcpy(tmp, line);

        char *t;
        t = strtok(tmp, ","); if (!t) { free(n); continue; } n->id = atoi(t);
        t = strtok(NULL, ","); if (!t) { free(n); continue; } copyField(n->roll,      t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(n); continue; } copyField(n->mentor_id, t, MAX_LEN);
        t = strtok(NULL, ","); if (!t) { free(n); continue; } copyField(n->note,      t, MAX_NOTE_LEN);
        t = strtok(NULL, ","); if (t)  copyField(n->date, t, MAX_LEN);

        noteAppend(&head, &tail, n);
    }
    fclose(fp);
    if (tail_out) *tail_out = tail;
    return head;
}


/* =============================================================
   FILE SAVERS
============================================================= */

void saveMeetings(struct Meeting *head) {
    FILE *fp = fopen(MEETINGS_FILE, "w");
    if (fp == NULL) return;
    for (struct Meeting *m = head; m != NULL; m = m->next) {
        fprintf(fp, "%d,%s,%s,%s,%s,%s,%s,%s\n",
                m->id, m->roll, m->mentor_id, m->date,
                m->time, m->mode, m->agenda, m->status);
    }
    fclose(fp);
}

void saveRequests(struct RequestQueue *q) {
    FILE *fp = fopen(REQUESTS_FILE, "w");
    if (fp == NULL) return;
    for (struct Request *r = q->head; r != NULL; r = r->next) {
        fprintf(fp, "%d,%s,%s,%s,%s,%s,%s,%s\n",
                r->id, r->roll, r->mentor_id, r->date,
                r->time, r->mode, r->purpose, r->status);
    }
    fclose(fp);
}

void saveNotes(struct Note *head) {
    FILE *fp = fopen(NOTES_FILE, "w");
    if (fp == NULL) return;
    for (struct Note *n = head; n != NULL; n = n->next) {
        fprintf(fp, "%d,%s,%s,%s,%s\n",
                n->id, n->roll, n->mentor_id, n->note, n->date);
    }
    fclose(fp);
}


/* =============================================================
   COUNT HELPERS
============================================================= */

int countMeetings(struct Meeting *head) {
    int n = 0;
    for (struct Meeting *m = head; m != NULL; m = m->next) n++;
    return n;
}

/* Count mentees in BST via inorder traversal counter */
static int _menteeCount = 0;
void _countVisitor(struct Mentee *m) { (void)m; _menteeCount++; }

int countMentees(struct MenteeNode *root) {
    _menteeCount = 0;
    menteeBstInorder(root, _countVisitor);
    return _menteeCount;
}

#endif /* STRUCTURES_H */
