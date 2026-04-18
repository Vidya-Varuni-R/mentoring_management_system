/*
 * mentee.c  —  Mentee Module  (REFACTORED)
 * All operations work on in-memory linked lists / queue / hash tables.
 * Files are read once at the top of main() and saved once at the end.
 *
 * Usage (run from c_backend/):
 *   ./mentee view_profile    <roll>
 *   ./mentee request_meeting <roll> <mentor_id> <date> <time> <mode> <purpose>
 *   ./mentee view_meetings   <roll>
 *   ./mentee view_requests   <roll>
 */

#include "structures.h"

/* ── view profile ─────────────────────────────────────────────── */
static void viewProfile(struct MenteeHash *mh, struct UserHash *uh, const char *roll) {
    struct Mentee *m = menteeHashFind(mh, roll);
    if (!m) { printf("NOT_FOUND:%s\n", roll); return; }

    const char *mentor_name = "-";
    struct User *u = userHashFindByEntity(uh, m->mentor_id);
    if (u && strcmp(u->role, "mentor") == 0) mentor_name = u->username;

    printf("PROFILE:%s,%s,%s,%s,%s,%s,%s\n",
        m->roll, m->name, m->dept, m->cgpa, m->attendance,
        m->mentor_id, mentor_name);
}

/* ── request meeting (enqueue) ───────────────────────────────── */
static int requestMeeting(struct RequestQueue *q, struct UserHash *uh,
                          const char *roll, const char *mentor_id,
                          const char *date, const char *time_str,
                          const char *mode, const char *purpose) {
    /* Validate mentor via hash lookup */
    struct User *u = userHashFindByEntity(uh, mentor_id);
    if (!u || strcmp(u->role, "mentor") != 0) {
        printf("ERROR:unknown_mentor\n"); return 0;
    }

    /* Find next id by walking the queue (small N; cheap) */
    int maxId = 0;
    for (struct Request *r = q->head; r; r = r->next)
        if (r->id > maxId) maxId = r->id;

    struct Request *r = (struct Request *)calloc(1, sizeof(struct Request));
    r->id = maxId + 1;
    copyField(r->roll,      roll,      MAX_LEN);
    copyField(r->mentor_id, mentor_id, MAX_LEN);
    copyField(r->date,      date,      MAX_LEN);
    copyField(r->time,      time_str,  MAX_LEN);
    copyField(r->mode,      mode,      MAX_LEN);
    copyField(r->purpose,   purpose,   MAX_NOTE_LEN);
    strcpy(r->status, "pending");

    enqueueRequest(q, r);          /* FIFO queue insert */
    printf("SUCCESS:request_sent\n");
    return 1;                      /* changed → save needed */
}

/* ── view meetings of a mentee ───────────────────────────────── */
static void viewMyMeetings(struct Meeting *list, const char *roll) {
    int found = 0;
    for (struct Meeting *m = list; m; m = m->next) {
        if (strcmp(m->roll, roll) == 0) {
            printf("MEETING:%d,%s,%s,%s,%s,%s,%s\n",
                m->id, m->date, m->time, m->mentor_id, m->mode,
                m->agenda, m->status);
            found = 1;
        }
    }
    if (!found) printf("EMPTY:no_meetings\n");
}

/* ── view this mentee's requests ─────────────────────────────── */
static void viewMyRequests(struct RequestQueue *q, const char *roll) {
    int found = 0;
    for (struct Request *r = q->head; r; r = r->next) {
        if (strcmp(r->roll, roll) == 0) {
            printf("REQUEST:%d,%s,%s,%s,%s,%s\n",
                r->id, r->date, r->time, r->mode, r->purpose, r->status);
            found = 1;
        }
    }
    if (!found) printf("EMPTY:no_requests\n");
}

/* ── main: load once, dispatch, save once ────────────────────── */
int main(int argc, char *argv[]) {
    if (argc < 2) { printf("ERROR:no_command\n"); return 1; }
    char *cmd = argv[1];

    /* LOAD ONCE */
    struct User    *users    = loadUsersList();
    struct Mentee  *mentees  = loadMenteesList();
    struct Meeting *meetings = loadMeetingsList();
    struct RequestQueue requests; loadRequestsQueue(&requests);

    /* BUILD HASH TABLES */
    struct UserHash   uh; buildUserHash(&uh, users);
    struct MenteeHash mh; buildMenteeHash(&mh, mentees);

    int requestsChanged = 0;

    /* DISPATCH (in-memory only) */
    if      (strcmp(cmd, "view_profile")    == 0 && argc >= 3) viewProfile(&mh, &uh, argv[2]);
    else if (strcmp(cmd, "request_meeting") == 0 && argc >= 8) requestsChanged = requestMeeting(&requests, &uh, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
    else if (strcmp(cmd, "view_meetings")   == 0 && argc >= 3) viewMyMeetings(meetings, argv[2]);
    else if (strcmp(cmd, "view_requests")   == 0 && argc >= 3) viewMyRequests(&requests, argv[2]);
    else { printf("ERROR:unknown_or_bad_args:%s\n", cmd); return 2; }

    /* SAVE ONCE (only what was actually modified) */
    if (requestsChanged) saveRequestsQueue(&requests);
    return 0;
}
