/*
 * mentee.c — Mentee Module
 * SSN College of Engineering, Dept. of IT
 *
 * Usage (from c_backend/):
 *   ./mentee view_profile    <roll>
 *   ./mentee request_meeting <roll> <mentor_id> <date> <time> <mode> <purpose>
 *   ./mentee view_meetings   <roll>
 *   ./mentee view_requests   <roll>
 *
 * Data structures:
 *   User BST   — key = username, also searched by entity_id for mentor name
 *   Mentee BST — key = roll, for profile lookup
 *   Meeting LL — filter by roll
 *   Request Q  — enqueue new requests, view by roll
 */

#include "structures.h"


void viewProfile(struct MenteeNode *menteeRoot, struct UserNode *userRoot,
                 const char *roll) {

    struct MenteeNode *mn = menteeBstSearch(menteeRoot, roll);
    if (mn == NULL) { printf("NOT_FOUND:%s\n", roll); return; }

    /* Find mentor name by entity_id in User BST */
    struct UserNode *un = userBstSearchByEntity(userRoot, mn->data.mentor_id);
    const char *mentor_name = (un != NULL) ? un->data.username : "-";

    printf("PROFILE:%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
           mn->data.roll, mn->data.name, mn->data.dept,
           mn->data.cgpa, mn->data.attendance,
           mn->data.cat1, mn->data.cat2,
           mn->data.mentor_id, mentor_name);
}


int requestMeeting(struct RequestQueue *q, struct UserNode *userRoot,
                   const char *roll, const char *mentor_id,
                   const char *date, const char *time_str,
                   const char *mode, const char *purpose) {

    /* Validate mentor exists via BST entity search */
    struct UserNode *un = userBstSearchByEntity(userRoot, mentor_id);
    if (un == NULL || strcmp(un->data.role, "mentor") != 0) {
        printf("ERROR:unknown_mentor\n");
        return 0;
    }

    /* Find max id */
    int maxId = 0;
    for (struct Request *r = q->head; r != NULL; r = r->next) {
        if (r->id > maxId) maxId = r->id;
    }

    struct Request *r = calloc(1, sizeof(struct Request));
    r->id = maxId + 1;
    copyField(r->roll,      roll,      MAX_LEN);
    copyField(r->mentor_id, mentor_id, MAX_LEN);
    copyField(r->date,      date,      MAX_LEN);
    copyField(r->time,      time_str,  MAX_LEN);
    copyField(r->mode,      mode,      MAX_LEN);
    copyField(r->purpose,   purpose,   MAX_NOTE_LEN);
    strcpy(r->status, "pending");

    queueEnqueue(q, r);
    printf("SUCCESS:request_sent\n");
    return 1;
}


void viewMyMeetings(struct MeetingStack *s, const char *roll) {
    int found = 0;
    for (struct Meeting *m = s->top; m != NULL; m = m->next) {
        if (strcmp(m->roll, roll) != 0) continue;
        printf("MEETING:%d,%s,%s,%s,%s,%s,%s\n",
               m->id, m->date, m->time, m->mentor_id,
               m->mode, m->agenda, m->status);
        found = 1;
    }
    if (!found) printf("EMPTY:no_meetings\n");
}


void viewMyRequests(struct RequestQueue *q, const char *roll) {
    int found = 0;
    for (struct Request *r = q->head; r != NULL; r = r->next) {
        if (strcmp(r->roll, roll) != 0) continue;
        printf("REQUEST:%d,%s,%s,%s,%s,%s\n",
               r->id, r->date, r->time, r->mode, r->purpose, r->status);
        found = 1;
    }
    if (!found) printf("EMPTY:no_requests\n");
}


int main(int argc, char *argv[]) {
    if (argc < 2) { printf("ERROR:no_command\n"); return 1; }
    char *cmd = argv[1];

    /* Load */
    struct UserNode     *users    = loadUsers();
    struct MenteeNode   *mentees  = loadMentees();
    struct MeetingStack  meetings;
    loadMeetings(&meetings);
    struct RequestQueue  requests;
    loadRequests(&requests);

    int requestsChanged = 0;

    /* Dispatch */
    if (strcmp(cmd, "view_profile") == 0 && argc >= 3) {
        viewProfile(mentees, users, argv[2]);

    } else if (strcmp(cmd, "request_meeting") == 0 && argc >= 8) {
        requestsChanged = requestMeeting(&requests, users,
                                         argv[2], argv[3], argv[4],
                                         argv[5], argv[6], argv[7]);

    } else if (strcmp(cmd, "view_meetings") == 0 && argc >= 3) {
        viewMyMeetings(&meetings, argv[2]);

    } else if (strcmp(cmd, "view_requests") == 0 && argc >= 3) {
        viewMyRequests(&requests, argv[2]);

    } else {
        printf("ERROR:unknown_or_bad_args:%s\n", cmd);
        return 2;
    }

    /* Save only if changed */
    if (requestsChanged) saveRequests(&requests);

    /* Free */
    userBstFree(users);
    menteeBstFree(mentees);
    stackFree(&meetings);
    queueFree(&requests);

    return 0;
}
