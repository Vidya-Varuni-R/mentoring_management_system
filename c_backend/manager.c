/*
 * manager.c  —  Manager Module  (REFACTORED)
 *
 * Read-only commands. Loads everything ONCE into linked lists + hash table,
 * uses an OPTIONAL Binary Search Tree to print mentees in roll-sorted order
 * for the "list_mentees" command (display purpose only — primary storage
 * is still the linked list).
 *
 * Usage:
 *   ./manager summary
 *   ./manager list_mentors
 *   ./manager list_mentees
 *   ./manager list_meetings
 *   ./manager export_report
 */

#include "structures.h"

/* ── per-mentor counts via single pass over lists ────────────── */
struct MentorCounts { int mentees; int meetings; };

static struct MentorCounts countForMentor(struct Mentee *menteeList,
                                          struct Meeting *meetingList,
                                          const char *entity_id) {
    struct MentorCounts c = {0, 0};
    for (struct Mentee  *m = menteeList;  m; m = m->next)
        if (strcmp(m->mentor_id,  entity_id) == 0) c.mentees++;
    for (struct Meeting *m = meetingList; m; m = m->next)
        if (strcmp(m->mentor_id,  entity_id) == 0) c.meetings++;
    return c;
}

/* ── summary ─────────────────────────────────────────────────── */
static void summary(struct User *users, struct Mentee *mentees, struct Meeting *meetings) {
    int mentor_count = 0;
    for (struct User *u = users; u; u = u->next)
        if (strcmp(u->role, "mentor") == 0) mentor_count++;

    int nm = countMentees(mentees);
    int nmt = countMeetings(meetings);

    int flagged = 0;
    for (struct Mentee *m = mentees; m; m = m->next) {
        float att  = atof(m->attendance);
        float cgpa = atof(m->cgpa);
        if (att < 75.0f || cgpa < 7.0f) flagged++;
    }
    printf("SUMMARY:%d,%d,%d,%d\n", mentor_count, nm, nmt, flagged);
}

/* ── list mentors ────────────────────────────────────────────── */
static void listMentors(struct User *users, struct Mentee *mentees, struct Meeting *meetings) {
    int printed = 0;
    for (struct User *u = users; u; u = u->next) {
        if (strcmp(u->role, "mentor") != 0) continue;
        struct MentorCounts c = countForMentor(mentees, meetings, u->entity_id);
        printf("MENTOR:%s,%s,%d,%d\n", u->entity_id, u->username, c.mentees, c.meetings);
        printed++;
    }
    if (!printed) printf("EMPTY:no_mentors\n");
}

/* ── list mentees (BST inorder for sorted-by-roll display) ────── */
static void printMenteeRow(struct Mentee *m) {
    float att  = atof(m->attendance);
    float cgpa = atof(m->cgpa);
    const char *flag = "ok";
    if      (att < 75.0f && cgpa < 7.0f) flag = "both";
    else if (att < 75.0f)                 flag = "attendance";
    else if (cgpa < 7.0f)                 flag = "cgpa";
    printf("MENTEE:%s,%s,%s,%s,%s,%s,%s\n",
        m->roll, m->name, m->dept, m->cgpa, m->attendance, m->mentor_id, flag);
}

static void listMentees(struct Mentee *mentees) {
    if (!mentees) { printf("EMPTY:no_mentees\n"); return; }

    /* Build a BST keyed by roll, then inorder traversal = sorted output.
     * BST is OPTIONAL and used only for display; primary storage is the list. */
    struct BstNode *root = NULL;
    for (struct Mentee *m = mentees; m; m = m->next)
        root = bstInsert(root, m);

    bstInorder(root, printMenteeRow);
    bstFree(root);
}

/* ── list meetings ───────────────────────────────────────────── */
static void listMeetings(struct Meeting *meetings, struct MenteeHash *mh) {
    if (!meetings) { printf("EMPTY:no_meetings\n"); return; }
    for (struct Meeting *mt = meetings; mt; mt = mt->next) {
        struct Mentee *me = menteeHashFind(mh, mt->roll);
        const char *mname = me ? me->name : "-";
        /* Format matches Flask parser: id,roll,mentee_name,mentor_id,date,time,mode,status */
        printf("MEETING:%d,%s,%s,%s,%s,%s,%s,%s\n",
            mt->id, mt->roll, mname, mt->mentor_id,
            mt->date, mt->time, mt->mode, mt->status);
    }
}

/* ── export report ───────────────────────────────────────────── */
static void exportReport(struct User *users, struct Mentee *mentees, struct Meeting *meetings) {
    printf("REPORT_START\n");
    printf("Mentoring Management System Report\n");
    printf("SSN College of Engineering, Dept. of IT\n---\n");
    for (struct User *u = users; u; u = u->next) {
        if (strcmp(u->role, "mentor") != 0) continue;
        struct MentorCounts c = countForMentor(mentees, meetings, u->entity_id);
        printf("Mentor: %s | Mentees: %d | Meetings: %d\n",
               u->username, c.mentees, c.meetings);
    }
    printf("---\nFlagged Students (att<75 or cgpa<7.0):\n");
    int any = 0;
    for (struct Mentee *m = mentees; m; m = m->next) {
        float att  = atof(m->attendance);
        float cgpa = atof(m->cgpa);
        if (att < 75.0f || cgpa < 7.0f) {
            printf("  %s | %s | CGPA:%s | Att:%s%%\n",
                m->roll, m->name, m->cgpa, m->attendance);
            any = 1;
        }
    }
    if (!any) printf("  (none)\n");
    printf("REPORT_END\n");
}

/* ── main: load once, dispatch, no save (read-only module) ───── */
int main(int argc, char *argv[]) {
    if (argc < 2) { printf("ERROR:no_command\n"); return 1; }
    char *cmd = argv[1];

    /* LOAD ONCE */
    struct User    *users    = loadUsersList();
    struct Mentee  *mentees  = loadMenteesList();
    struct Meeting *meetings = loadMeetingsList();

    /* HASH TABLE for fast mentee-name lookup */
    struct MenteeHash mh; buildMenteeHash(&mh, mentees);

    if      (strcmp(cmd, "summary")       == 0) summary(users, mentees, meetings);
    else if (strcmp(cmd, "list_mentors")  == 0) listMentors(users, mentees, meetings);
    else if (strcmp(cmd, "list_mentees")  == 0) listMentees(mentees);
    else if (strcmp(cmd, "list_meetings") == 0) listMeetings(meetings, &mh);
    else if (strcmp(cmd, "export_report") == 0) exportReport(users, mentees, meetings);
    else { printf("ERROR:unknown_command\n"); return 2; }

    /* No save: manager is read-only. */
    return 0;
}
