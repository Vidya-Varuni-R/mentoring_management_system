/*
 * mentor.c  —  Mentor Module  (REFACTORED)
 *
 * All feature functions operate on data structures (linked lists, queue,
 * hash tables) passed as arguments. Files are touched only inside main().
 *
 * Usage (run from c_backend/):
 *   ./mentor view_mentees     <mentor_id>
 *   ./mentor add_mentee       <mentor_id> <name> <dept> <roll> <cgpa> <attendance>
 *   ./mentor search_mentee    <roll>
 *   ./mentor update_mentee    <roll> <name> <dept> <cgpa> <attendance>
 *   ./mentor delete_mentee    <roll>
 *   ./mentor add_note         <mentor_id> <roll> <note>
 *   ./mentor view_notes       <mentor_id>
 *   ./mentor view_requests    <mentor_id>
 *   ./mentor respond_request  <request_id> <accepted|rejected>
 *   ./mentor view_meetings    <mentor_id>
 *   ./mentor schedule_meeting <mentor_id> <roll> <date> <time> <mode> <agenda>
 */

#include "structures.h"
#include <time.h>

static void today(char out[MAX_LEN]) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(out, MAX_LEN, "%Y-%m-%d", tm);
}

/* ════════════════════════════════════════════════════════════════
   MENTEES
   ════════════════════════════════════════════════════════════════ */

static void viewMentees(struct Mentee *list, const char *mentor_id) {
    int found = 0;
    for (struct Mentee *m = list; m; m = m->next) {
        if (strcmp(m->mentor_id, mentor_id) == 0) {
            printf("DATA:%s,%s,%s,%s,%s\n",
                m->roll, m->name, m->dept, m->cgpa, m->attendance);
            found = 1;
        }
    }
    if (!found) printf("EMPTY:no_mentees\n");
}

/* Returns 1 if list was changed (so main knows to save). */
static int addMentee(struct Mentee **head, struct Mentee **tail,
                     struct MenteeHash *mh,
                     const char *mentor_id, const char *name,
                     const char *dept, const char *roll,
                     const char *cgpa, const char *attendance) {
    if (menteeHashFind(mh, roll)) {
        printf("ERROR:duplicate_roll\n");
        return 0;
    }
    struct Mentee *m = (struct Mentee *)calloc(1, sizeof(struct Mentee));
    copyField(m->roll,       roll,       MAX_LEN);
    copyField(m->name,       name,       MAX_LEN);
    copyField(m->dept,       dept,       MAX_LEN);
    copyField(m->cgpa,       cgpa,       MAX_LEN);
    copyField(m->attendance, attendance, MAX_LEN);
    copyField(m->mentor_id,  mentor_id,  MAX_LEN);
    appendMentee(head, tail, m);
    menteeHashInsert(mh, m);
    printf("SUCCESS:mentee_added\n");
    return 1;
}

static void searchMentee(struct MenteeHash *mh, const char *roll) {
    struct Mentee *m = menteeHashFind(mh, roll);     /* O(1) hash lookup */
    if (!m) { printf("NOT_FOUND:%s\n", roll); return; }
    printf("FOUND:%s,%s,%s,%s,%s,%s\n",
        m->roll, m->name, m->dept, m->cgpa, m->attendance, m->mentor_id);
}

static int updateMentee(struct MenteeHash *mh, const char *roll,
                        const char *name, const char *dept,
                        const char *cgpa, const char *attendance) {
    struct Mentee *m = menteeHashFind(mh, roll);
    if (!m) { printf("NOT_FOUND:%s\n", roll); return 0; }
    copyField(m->name,       name,       MAX_LEN);
    copyField(m->dept,       dept,       MAX_LEN);
    copyField(m->cgpa,       cgpa,       MAX_LEN);
    copyField(m->attendance, attendance, MAX_LEN);
    printf("SUCCESS:mentee_updated\n");
    return 1;
}

static int deleteMentee(struct Mentee **head, struct Mentee **tail,
                        struct MenteeHash *mh, const char *roll) {
    struct Mentee *prev = NULL, *p = *head;
    while (p) {
        if (strcmp(p->roll, roll) == 0) {
            if (prev) prev->next = p->next; else *head = p->next;
            if (p == *tail) *tail = prev;
            menteeHashRemove(mh, roll);
            free(p);
            printf("SUCCESS:mentee_deleted\n");
            return 1;
        }
        prev = p; p = p->next;
    }
    printf("NOT_FOUND:%s\n", roll);
    return 0;
}

/* ════════════════════════════════════════════════════════════════
   NOTES
   ════════════════════════════════════════════════════════════════ */

static int addNote(struct Note **head, struct Note **tail,
                   const char *mentor_id, const char *roll, const char *note) {
    int maxId = 0;
    for (struct Note *n = *head; n; n = n->next) if (n->id > maxId) maxId = n->id;

    struct Note *n = (struct Note *)calloc(1, sizeof(struct Note));
    n->id = maxId + 1;
    copyField(n->roll,      roll,      MAX_LEN);
    copyField(n->mentor_id, mentor_id, MAX_LEN);
    copyField(n->note,      note,      MAX_NOTE_LEN);
    today(n->date);
    appendNote(head, tail, n);
    printf("SUCCESS:note_added\n");
    return 1;
}

static void viewNotes(struct Note *notes, struct MenteeHash *mh, const char *mentor_id) {
    int found = 0;
    for (struct Note *n = notes; n; n = n->next) {
        if (strcmp(n->mentor_id, mentor_id) == 0) {
            struct Mentee *m = menteeHashFind(mh, n->roll);
            const char *mname = m ? m->name : "-";
            printf("NOTE:%d,%s,%s,%s,%s\n",
                n->id, n->roll, mname, n->note, n->date);
            found = 1;
        }
    }
    if (!found) printf("EMPTY:no_notes\n");
}

/* ════════════════════════════════════════════════════════════════
   REQUESTS  (queue) and MEETINGS
   ════════════════════════════════════════════════════════════════ */

static void viewRequests(struct RequestQueue *q, struct MenteeHash *mh,
                         const char *mentor_id) {
    int found = 0;
    for (struct Request *r = q->head; r; r = r->next) {
        if (strcmp(r->mentor_id, mentor_id) == 0 &&
            strcmp(r->status, "pending") == 0) {
            struct Mentee *m = menteeHashFind(mh, r->roll);
            const char *mname = m ? m->name : "-";
            printf("REQUEST:%d,%s,%s,%s,%s,%s,%s,%s\n",
                r->id, r->roll, mname, r->date, r->time, r->mode,
                r->purpose, r->status);
            found = 1;
        }
    }
    if (!found) printf("EMPTY:no_requests\n");
}

/* writes back pointer to whether requests/meetings changed */
static void respondRequest(struct RequestQueue *q,
                           struct Meeting **mhead, struct Meeting **mtail,
                           const char *req_id_str, const char *response,
                           int *reqChanged, int *meetChanged) {
    int req_id = atoi(req_id_str);
    struct Request *target = NULL;
    for (struct Request *r = q->head; r; r = r->next)
        if (r->id == req_id) { target = r; break; }

    if (!target) { printf("NOT_FOUND:%s\n", req_id_str); return; }
    if (strcmp(response, "accepted") != 0 && strcmp(response, "rejected") != 0) {
        printf("ERROR:invalid_response\n"); return;
    }

    copyField(target->status, response, MAX_LEN);
    *reqChanged = 1;

    if (strcmp(response, "accepted") == 0) {
        int maxId = 0;
        for (struct Meeting *m = *mhead; m; m = m->next)
            if (m->id > maxId) maxId = m->id;
        struct Meeting *nm = (struct Meeting *)calloc(1, sizeof(struct Meeting));
        nm->id = maxId + 1;
        copyField(nm->roll,      target->roll,      MAX_LEN);
        copyField(nm->mentor_id, target->mentor_id, MAX_LEN);
        copyField(nm->date,      target->date,      MAX_LEN);
        copyField(nm->time,      target->time,      MAX_LEN);
        copyField(nm->mode,      target->mode,      MAX_LEN);
        copyField(nm->agenda,    target->purpose,   MAX_NOTE_LEN);
        strcpy(nm->status, "confirmed");
        appendMeeting(mhead, mtail, nm);
        *meetChanged = 1;
        printf("SUCCESS:request_accepted\n");
    } else {
        printf("SUCCESS:request_rejected\n");
    }
}

static void viewMeetings(struct Meeting *meetings, struct MenteeHash *mh,
                         const char *mentor_id) {
    int found = 0;
    for (struct Meeting *m = meetings; m; m = m->next) {
        if (strcmp(m->mentor_id, mentor_id) == 0) {
            struct Mentee *me = menteeHashFind(mh, m->roll);
            const char *mname = me ? me->name : "-";
            printf("MEETING:%d,%s,%s,%s,%s,%s,%s,%s\n",
                m->id, m->roll, mname, m->date, m->time, m->mode,
                m->agenda, m->status);
            found = 1;
        }
    }
    if (!found) printf("EMPTY:no_meetings\n");
}

static int scheduleMeeting(struct Meeting **mhead, struct Meeting **mtail,
                           struct MenteeHash *mh,
                           const char *mentor_id, const char *roll,
                           const char *date, const char *time_str,
                           const char *mode, const char *agenda) {
    struct Mentee *me = menteeHashFind(mh, roll);
    if (!me || strcmp(me->mentor_id, mentor_id) != 0) {
        printf("ERROR:not_your_mentee\n");
        return 0;
    }
    int maxId = 0;
    for (struct Meeting *m = *mhead; m; m = m->next)
        if (m->id > maxId) maxId = m->id;

    struct Meeting *nm = (struct Meeting *)calloc(1, sizeof(struct Meeting));
    nm->id = maxId + 1;
    copyField(nm->roll,      roll,      MAX_LEN);
    copyField(nm->mentor_id, mentor_id, MAX_LEN);
    copyField(nm->date,      date,      MAX_LEN);
    copyField(nm->time,      time_str,  MAX_LEN);
    copyField(nm->mode,      mode,      MAX_LEN);
    copyField(nm->agenda,    agenda,    MAX_NOTE_LEN);
    strcpy(nm->status, "confirmed");
    appendMeeting(mhead, mtail, nm);
    printf("SUCCESS:meeting_scheduled\n");
    return 1;
}

/* ════════════════════════════════════════════════════════════════
   MAIN — load once, dispatch, save once
   ════════════════════════════════════════════════════════════════ */
int main(int argc, char *argv[]) {
    if (argc < 2) { printf("ERROR:no_command\n"); return 1; }
    char *cmd = argv[1];

    /* LOAD ONCE */
    struct Mentee  *mentees      = loadMenteesList();
    struct Note    *notes        = loadNotesList();
    struct Meeting *meetings     = loadMeetingsList();
    struct RequestQueue requests; loadRequestsQueue(&requests);

    /* tail pointers (so we can append in O(1) without rescanning) */
    struct Mentee  *menteeTail  = mentees;
    while (menteeTail && menteeTail->next) menteeTail = menteeTail->next;
    struct Note    *noteTail    = notes;
    while (noteTail && noteTail->next) noteTail = noteTail->next;
    struct Meeting *meetingTail = meetings;
    while (meetingTail && meetingTail->next) meetingTail = meetingTail->next;

    /* HASH TABLE for fast mentee lookup by roll */
    struct MenteeHash mh; buildMenteeHash(&mh, mentees);

    int menteesChanged  = 0;
    int notesChanged    = 0;
    int requestsChanged = 0;
    int meetingsChanged = 0;

    /* DISPATCH */
    if      (strcmp(cmd, "view_mentees")     == 0 && argc >= 3) viewMentees(mentees, argv[2]);
    else if (strcmp(cmd, "add_mentee")       == 0 && argc >= 8) menteesChanged = addMentee(&mentees, &menteeTail, &mh, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
    else if (strcmp(cmd, "search_mentee")    == 0 && argc >= 3) searchMentee(&mh, argv[2]);
    else if (strcmp(cmd, "update_mentee")    == 0 && argc >= 7) menteesChanged = updateMentee(&mh, argv[2], argv[3], argv[4], argv[5], argv[6]);
    else if (strcmp(cmd, "delete_mentee")    == 0 && argc >= 3) menteesChanged = deleteMentee(&mentees, &menteeTail, &mh, argv[2]);
    else if (strcmp(cmd, "add_note")         == 0 && argc >= 5) notesChanged   = addNote(&notes, &noteTail, argv[2], argv[3], argv[4]);
    else if (strcmp(cmd, "view_notes")       == 0 && argc >= 3) viewNotes(notes, &mh, argv[2]);
    else if (strcmp(cmd, "view_requests")    == 0 && argc >= 3) viewRequests(&requests, &mh, argv[2]);
    else if (strcmp(cmd, "respond_request")  == 0 && argc >= 4) respondRequest(&requests, &meetings, &meetingTail, argv[2], argv[3], &requestsChanged, &meetingsChanged);
    else if (strcmp(cmd, "view_meetings")    == 0 && argc >= 3) viewMeetings(meetings, &mh, argv[2]);
    else if (strcmp(cmd, "schedule_meeting") == 0 && argc >= 8) meetingsChanged = scheduleMeeting(&meetings, &meetingTail, &mh, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
    else { printf("ERROR:unknown_or_bad_args:%s\n", cmd); return 2; }

    /* SAVE ONCE (only files that changed) */
    if (menteesChanged)  saveMenteesList(mentees);
    if (notesChanged)    saveNotesList(notes);
    if (requestsChanged) saveRequestsQueue(&requests);
    if (meetingsChanged) saveMeetingsList(meetings);
    return 0;
}
