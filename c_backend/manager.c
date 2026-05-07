/*
 * manager.c — Manager Module (read-only)
 * SSN College of Engineering, Dept. of IT
 *
 * Usage (from c_backend/):
 *   ./manager summary
 *   ./manager list_mentors
 *   ./manager list_mentees
 *   ./manager list_meetings
 *   ./manager export_report
 *
 * Data structures:
 *   User BST   — inorder traversal to list mentors alphabetically
 *   Mentee BST — inorder traversal gives sorted-by-roll display
 *   Meeting LL — iterate for counts and listing
 *
 * No writes: manager is read-only.
 */

#include "structures.h"


/* =============================================================
   SUMMARY
============================================================= */

/* Count users with a given role by walking BST inorder */
static int   _roleCount   = 0;
static char  _targetRole[MAX_LEN];

void _countRoleVisitor(struct User *u) {
    if (strcmp(u->role, _targetRole) == 0) _roleCount++;
}

int countUsersWithRole(struct UserNode *root, const char *role) {
    _roleCount = 0;
    copyField(_targetRole, role, MAX_LEN);
    userBstInorder(root, _countRoleVisitor);
    return _roleCount;
}

void summary(struct UserNode *users, struct MenteeNode *mentees,
             struct MeetingStack *meetings) {
    int mentor_count  = countUsersWithRole(users, "mentor");
    int mentee_count  = countMentees(mentees);
    int meeting_count = countMeetings(meetings);

    /* Count flagged mentees */
    int flagged = 0;
    /* We need to walk the BST — use inorder with a counter */
    /* Simple approach: walk BST recursively and check each node */
    /* Using a helper since we can't use closures in C */

    /* Inline recursive count */
    struct MenteeNode *stack[1024];
    int top = 0;
    struct MenteeNode *cur = mentees;
    while (cur != NULL || top > 0) {
        while (cur != NULL) { stack[top++] = cur; cur = cur->left; }
        cur = stack[--top];
        if (atof(cur->data.attendance) < 75.0f || atof(cur->data.cat1) < 25.0f || atof(cur->data.cat2) < 25.0f) {
            flagged++;
        }
        cur = cur->right;
    }

    printf("SUMMARY:%d,%d,%d,%d\n",
           mentor_count, mentee_count, meeting_count, flagged);
}


/* =============================================================
   LIST MENTORS
============================================================= */

/* We need access to mentees and meetings inside the visitor,
   so we use file-scope pointers (simple approach, no closures in C) */
static struct MenteeNode *_mentorMenteeRoot   = NULL;
static struct MeetingStack *_mentorMeetingStack = NULL;

void _listMentorVisitor(struct User *u) {
    if (strcmp(u->role, "mentor") != 0) return;

    int mentee_count  = 0;
    int meeting_count = 0;

    /* Count mentees for this mentor by walking BST */
    struct MenteeNode *stack[1024];
    int top = 0;
    struct MenteeNode *cur = _mentorMenteeRoot;
    while (cur != NULL || top > 0) {
        while (cur != NULL) { stack[top++] = cur; cur = cur->left; }
        cur = stack[--top];
        if (strcmp(cur->data.mentor_id, u->entity_id) == 0) mentee_count++;
        cur = cur->right;
    }

    for (struct Meeting *m = _mentorMeetingStack->top; m != NULL; m = m->next) {
        if (strcmp(m->mentor_id, u->entity_id) == 0) meeting_count++;
    }

    printf("MENTOR:%s,%s,%d,%d\n",
           u->entity_id, u->username, mentee_count, meeting_count);
}

void listMentors(struct UserNode *users, struct MenteeNode *mentees,
                 struct MeetingStack *meetings) {
    _mentorMenteeRoot   = mentees;
    _mentorMeetingStack = meetings;
    userBstInorder(users, _listMentorVisitor);
}


/* =============================================================
   LIST MENTEES (sorted by roll via BST inorder)
============================================================= */

void _listMenteeVisitor(struct Mentee *m) {
    const char *flag = "ok";
    float att  = atof(m->attendance);
    float c1   = atof(m->cat1);
    float c2   = atof(m->cat2);
    if      (att < 75.0f && (c1 < 25.0f || c2 < 25.0f)) flag = "both";
    else if (att < 75.0f)                               flag = "attendance";
    else if (c1 < 25.0f || c2 < 25.0f)                  flag = "cat";

    printf("MENTEE:%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
           m->roll, m->name, m->dept, m->cgpa,
           m->attendance, m->mentor_id, flag,
           m->cat1, m->cat2);
}

void listMentees(struct MenteeNode *root) {
    if (root == NULL) { printf("EMPTY:no_mentees\n"); return; }
    menteeBstInorder(root, _listMenteeVisitor);
}


/* =============================================================
   LIST MEETINGS
============================================================= */

void listMeetings(struct MeetingStack *s, struct MenteeNode *menteeRoot) {
    if (s->top == NULL) { printf("EMPTY:no_meetings\n"); return; }
    for (struct Meeting *m = s->top; m != NULL; m = m->next) {
        struct MenteeNode *mn = menteeBstSearch(menteeRoot, m->roll);
        const char *mname = (mn != NULL) ? mn->data.name : "-";
        printf("MEETING:%d,%s,%s,%s,%s,%s,%s,%s\n",
               m->id, m->roll, mname, m->mentor_id,
               m->date, m->time, m->mode, m->status);
    }
}


/* =============================================================
   EXPORT REPORT
============================================================= */

void exportReport(struct UserNode *users, struct MenteeNode *mentees,
                  struct MeetingStack *meetings) {

    printf("REPORT_START\n");
    printf("Mentoring Management System Report\n");
    printf("SSN College of Engineering, Dept. of IT\n---\n");

    _mentorMenteeRoot   = mentees;
    _mentorMeetingStack = meetings;
    userBstInorder(users, _listMentorVisitor);

    printf("---\nFlagged Students (att<75 or cat<25):\n");

    int any = 0;
    struct MenteeNode *stack[1024];
    int top = 0;
    struct MenteeNode *cur = mentees;
    while (cur != NULL || top > 0) {
        while (cur != NULL) { stack[top++] = cur; cur = cur->left; }
        cur = stack[--top];
        if (atof(cur->data.attendance) < 75.0f || atof(cur->data.cat1) < 25.0f || atof(cur->data.cat2) < 25.0f) {
            printf("  %s | %s | CAT1:%s | CAT2:%s | Att:%s%%\n",
                   cur->data.roll, cur->data.name,
                   cur->data.cat1, cur->data.cat2, cur->data.attendance);
            any = 1;
        }
        cur = cur->right;
    }

    if (!any) printf("  (none)\n");
    printf("REPORT_END\n");
}


/* =============================================================
   MAIN — load once, dispatch, no save (read-only)
============================================================= */

int main(int argc, char *argv[]) {
    if (argc < 2) { printf("ERROR:no_command\n"); return 1; }
    char *cmd = argv[1];

    /* Load */
    struct UserNode   *users    = loadUsers();
    struct MenteeNode *mentees  = loadMentees();
    struct MeetingStack meetings;
    loadMeetings(&meetings);

    /* Dispatch */
    if (strcmp(cmd, "summary") == 0) {
        summary(users, mentees, &meetings);

    } else if (strcmp(cmd, "list_mentors") == 0) {
        listMentors(users, mentees, &meetings);

    } else if (strcmp(cmd, "list_mentees") == 0) {
        listMentees(mentees);

    } else if (strcmp(cmd, "list_meetings") == 0) {
        listMeetings(&meetings, mentees);

    } else if (strcmp(cmd, "export_report") == 0) {
        exportReport(users, mentees, &meetings);

    } else {
        printf("ERROR:unknown_command\n");
        return 2;
    }

    /* Free */
    userBstFree(users);
    menteeBstFree(mentees);
    stackFree(&meetings);

    return 0;
}
