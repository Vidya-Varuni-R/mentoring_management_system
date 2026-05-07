/*
 * mentor.c — Mentor Module
 * SSN College of Engineering, Dept. of IT
 *
 * Usage (from c_backend/):
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
 *
 * Data structures:
 *   Mentee BST      — key = roll, used for search / update / delete
 *   Meeting list    — linked list, appended for new meetings
 *   Note list       — linked list, appended for new notes
 *   Request queue   — FIFO, respond changes status in place
 */

#include "structures.h"


/* =============================================================
   MENTEE OPERATIONS (BST)
============================================================= */

void viewMentees(struct MenteeNode *root, const char *mentor_id) {
    /* We need to traverse the whole BST and print matching mentor_id.
       We do an inorder traversal using a helper with a closure variable. */
    if (root == NULL) return;
    viewMentees(root->left, mentor_id);
    if (strcmp(root->data.mentor_id, mentor_id) == 0) {
        printf("DATA:%s,%s,%s,%s,%s,%s,%s\n",
               root->data.roll, root->data.name, root->data.dept,
               root->data.cgpa, root->data.attendance,
               root->data.cat1, root->data.cat2);
    }
    viewMentees(root->right, mentor_id);
}

int addMentee(struct MenteeNode **root, const char *mentor_id,
              const char *name, const char *dept, const char *roll,
              const char *cat1, const char *cat2,
              const char *cgpa, const char *attendance) {

    if (menteeBstSearch(*root, roll) != NULL) {
        printf("ERROR:duplicate_roll\n");
        return 0;
    }

    struct Mentee m;
    copyField(m.roll,       roll,       MAX_LEN);
    copyField(m.name,       name,       MAX_LEN);
    copyField(m.dept,       dept,       MAX_LEN);
    copyField(m.cat1,       cat1,       MAX_LEN);
    copyField(m.cat2,       cat2,       MAX_LEN);
    copyField(m.cgpa,       cgpa,       MAX_LEN);
    copyField(m.attendance, attendance, MAX_LEN);
    copyField(m.mentor_id,  mentor_id,  MAX_LEN);

    *root = menteeBstInsert(*root, m);
    printf("SUCCESS:mentee_added\n");
    return 1;
}

void searchMentee(struct MenteeNode *root, const char *roll) {
    struct MenteeNode *found = menteeBstSearch(root, roll);
    if (found == NULL) {
        printf("NOT_FOUND:%s\n", roll);
        return;
    }
    printf("FOUND:%s,%s,%s,%s,%s,%s,%s,%s\n",
           found->data.roll,  found->data.name, found->data.dept,
           found->data.cgpa,  found->data.attendance, found->data.mentor_id,
           found->data.cat1,  found->data.cat2);
}

int updateMentee(struct MenteeNode *root, const char *roll,
                 const char *name, const char *dept,
                 const char *cat1, const char *cat2,
                 const char *cgpa, const char *attendance) {

    struct MenteeNode *found = menteeBstSearch(root, roll);
    if (found == NULL) {
        printf("NOT_FOUND:%s\n", roll);
        return 0;
    }

    /* Update fields directly in the BST node */
    copyField(found->data.name,       name,       MAX_LEN);
    copyField(found->data.dept,       dept,       MAX_LEN);
    copyField(found->data.cat1,       cat1,       MAX_LEN);
    copyField(found->data.cat2,       cat2,       MAX_LEN);
    copyField(found->data.cgpa,       cgpa,       MAX_LEN);
    copyField(found->data.attendance, attendance, MAX_LEN);

    printf("SUCCESS:mentee_updated\n");
    return 1;
}

int deleteMentee(struct MenteeNode **root, const char *roll) {
    if (menteeBstSearch(*root, roll) == NULL) {
        printf("NOT_FOUND:%s\n", roll);
        return 0;
    }
    *root = menteeBstDelete(*root, roll);
    printf("SUCCESS:mentee_deleted\n");
    return 1;
}


/* =============================================================
   NOTE OPERATIONS (Linked List)
============================================================= */

int addNote(struct Note **head, struct Note **tail,
            const char *mentor_id, const char *roll, const char *note_text) {

    /* Find max id */
    int maxId = 0;
    for (struct Note *n = *head; n != NULL; n = n->next) {
        if (n->id > maxId) maxId = n->id;
    }

    struct Note *n = calloc(1, sizeof(struct Note));
    n->id = maxId + 1;
    copyField(n->roll,      roll,       MAX_LEN);
    copyField(n->mentor_id, mentor_id,  MAX_LEN);
    copyField(n->note,      note_text,  MAX_NOTE_LEN);
    getToday(n->date);

    noteAppend(head, tail, n);
    printf("SUCCESS:note_added\n");
    return 1;
}

void viewNotes(struct Note *head, struct MenteeNode *menteeRoot,
               const char *mentor_id) {
    int found = 0;
    for (struct Note *n = head; n != NULL; n = n->next) {
        if (strcmp(n->mentor_id, mentor_id) != 0) continue;

        /* Look up mentee name from BST */
        struct MenteeNode *mn = menteeBstSearch(menteeRoot, n->roll);
        const char *mname = (mn != NULL) ? mn->data.name : "-";

        printf("NOTE:%d,%s,%s,%s,%s\n",
               n->id, n->roll, mname, n->note, n->date);
        found = 1;
    }
    if (!found) printf("EMPTY:no_notes\n");
}


/* =============================================================
   REQUEST OPERATIONS (Queue)
============================================================= */

void viewRequests(struct RequestQueue *q, struct MenteeNode *menteeRoot,
                  const char *mentor_id) {
    int found = 0;
    for (struct Request *r = q->head; r != NULL; r = r->next) {
        if (strcmp(r->mentor_id, mentor_id) != 0) continue;
        if (strcmp(r->status, "pending")    != 0) continue;

        struct MenteeNode *mn = menteeBstSearch(menteeRoot, r->roll);
        const char *mname = (mn != NULL) ? mn->data.name : "-";

        printf("REQUEST:%d,%s,%s,%s,%s,%s,%s,%s\n",
               r->id, r->roll, mname, r->date, r->time,
               r->mode, r->purpose, r->status);
        found = 1;
    }
    if (!found) printf("EMPTY:no_requests\n");
}

void respondRequest(struct RequestQueue *q,
                    struct MeetingStack *s,
                    const char *req_id_str, const char *response,
                    int *reqChanged, int *meetChanged) {

    int req_id = atoi(req_id_str);

    if (strcmp(response, "accepted") != 0 && strcmp(response, "rejected") != 0) {
        printf("ERROR:invalid_response\n");
        return;
    }

    struct Request *target = NULL;
    for (struct Request *r = q->head; r != NULL; r = r->next) {
        if (r->id == req_id) { target = r; break; }
    }

    if (target == NULL) {
        printf("NOT_FOUND:%s\n", req_id_str);
        return;
    }

    copyField(target->status, response, MAX_LEN);
    *reqChanged = 1;

    if (strcmp(response, "accepted") == 0) {
        /* Create a confirmed meeting from the request */
        int maxId = 0;
        for (struct Meeting *m = s->top; m != NULL; m = m->next) {
            if (m->id > maxId) maxId = m->id;
        }

        struct Meeting *nm = calloc(1, sizeof(struct Meeting));
        nm->id = maxId + 1;
        copyField(nm->roll,      target->roll,      MAX_LEN);
        copyField(nm->mentor_id, target->mentor_id, MAX_LEN);
        copyField(nm->date,      target->date,      MAX_LEN);
        copyField(nm->time,      target->time,      MAX_LEN);
        copyField(nm->mode,      target->mode,      MAX_LEN);
        copyField(nm->agenda,    target->purpose,   MAX_NOTE_LEN);
        strcpy(nm->status, "confirmed");

        stackPush(s, nm);
        *meetChanged = 1;
        printf("SUCCESS:request_accepted\n");
    } else {
        printf("SUCCESS:request_rejected\n");
    }
}


/* =============================================================
   MEETING OPERATIONS (Stack)
============================================================= */

void viewMeetings(struct MeetingStack *s, struct MenteeNode *menteeRoot,
                  const char *mentor_id) {
    int found = 0;
    for (struct Meeting *m = s->top; m != NULL; m = m->next) {
        if (strcmp(m->mentor_id, mentor_id) != 0) continue;

        struct MenteeNode *mn = menteeBstSearch(menteeRoot, m->roll);
        const char *mname = (mn != NULL) ? mn->data.name : "-";

        printf("MEETING:%d,%s,%s,%s,%s,%s,%s,%s\n",
               m->id, m->roll, mname, m->date,
               m->time, m->mode, m->agenda, m->status);
        found = 1;
    }
    if (!found) printf("EMPTY:no_meetings\n");
}

int scheduleMeeting(struct MeetingStack *s,
                    struct MenteeNode *menteeRoot,
                    const char *mentor_id, const char *roll,
                    const char *date, const char *time_str,
                    const char *mode, const char *agenda) {

    struct MenteeNode *mn = menteeBstSearch(menteeRoot, roll);
    if (mn == NULL || strcmp(mn->data.mentor_id, mentor_id) != 0) {
        printf("ERROR:not_your_mentee\n");
        return 0;
    }

    int maxId = 0;
    for (struct Meeting *m = s->top; m != NULL; m = m->next) {
        if (m->id > maxId) maxId = m->id;
    }

    struct Meeting *nm = calloc(1, sizeof(struct Meeting));
    nm->id = maxId + 1;
    copyField(nm->roll,      roll,      MAX_LEN);
    copyField(nm->mentor_id, mentor_id, MAX_LEN);
    copyField(nm->date,      date,      MAX_LEN);
    copyField(nm->time,      time_str,  MAX_LEN);
    copyField(nm->mode,      mode,      MAX_LEN);
    copyField(nm->agenda,    agenda,    MAX_NOTE_LEN);
    strcpy(nm->status, "confirmed");

    stackPush(s, nm);
    printf("SUCCESS:meeting_scheduled\n");
    return 1;
}

int completeMeeting(struct MeetingStack *s, const char *mentor_id, int meeting_id) {
    for (struct Meeting *m = s->top; m != NULL; m = m->next) {
        if (m->id == meeting_id) {
            if (strcmp(m->mentor_id, mentor_id) != 0) {
                printf("ERROR:not_your_meeting\n");
                return 0;
            }
            if (strcmp(m->status, "completed") == 0) {
                printf("ERROR:already_completed\n");
                return 0;
            }
            strcpy(m->status, "completed");
            printf("SUCCESS:meeting_completed\n");
            return 1;
        }
    }
    printf("ERROR:meeting_not_found\n");
    return 0;
}


/* =============================================================
   MAIN — load once, dispatch, save once
============================================================= */

int main(int argc, char *argv[]) {
    if (argc < 2) { printf("ERROR:no_command\n"); return 1; }
    char *cmd = argv[1];

    /* Load */
    struct MenteeNode   *mentees  = loadMentees();
    struct Note         *notes    = NULL;
    struct Note         *noteTail = NULL;
    struct MeetingStack  meetings;
    struct RequestQueue  requests;

    notes = loadNotes(&noteTail);
    loadMeetings(&meetings);
    loadRequests(&requests);

    int menteesChanged  = 0;
    int notesChanged    = 0;
    int requestsChanged = 0;
    int meetingsChanged = 0;

    /* Dispatch */
    if (strcmp(cmd, "view_mentees") == 0 && argc >= 3) {
        int printed = 0;
        /* viewMentees already loops, but we need to check if anything printed */
        viewMentees(mentees, argv[2]);
        /* If tree is empty or no match, viewMentees prints nothing — add fallback */
        (void)printed;
        if (mentees == NULL) printf("EMPTY:no_mentees\n");

    } else if (strcmp(cmd, "add_mentee") == 0 && argc >= 10) {
        menteesChanged = addMentee(&mentees, argv[2], argv[3], argv[4],
                                   argv[5], argv[6], argv[7], argv[8], argv[9]);

    } else if (strcmp(cmd, "search_mentee") == 0 && argc >= 3) {
        searchMentee(mentees, argv[2]);

    } else if (strcmp(cmd, "update_mentee") == 0 && argc >= 9) {
        menteesChanged = updateMentee(mentees, argv[2], argv[3], argv[4],
                                      argv[5], argv[6], argv[7], argv[8]);

    } else if (strcmp(cmd, "delete_mentee") == 0 && argc >= 3) {
        menteesChanged = deleteMentee(&mentees, argv[2]);

    } else if (strcmp(cmd, "add_note") == 0 && argc >= 5) {
        notesChanged = addNote(&notes, &noteTail, argv[2], argv[3], argv[4]);

    } else if (strcmp(cmd, "view_notes") == 0 && argc >= 3) {
        viewNotes(notes, mentees, argv[2]);

    } else if (strcmp(cmd, "view_requests") == 0 && argc >= 3) {
        viewRequests(&requests, mentees, argv[2]);

    } else if (strcmp(cmd, "respond_request") == 0 && argc >= 4) {
        respondRequest(&requests, &meetings,
                       argv[2], argv[3], &requestsChanged, &meetingsChanged);

    } else if (strcmp(cmd, "view_meetings") == 0 && argc >= 3) {
        viewMeetings(&meetings, mentees, argv[2]);

    } else if (strcmp(cmd, "schedule_meeting") == 0 && argc >= 8) {
        meetingsChanged = scheduleMeeting(&meetings, mentees,
                                          argv[2], argv[3], argv[4],
                                          argv[5], argv[6], argv[7]);

    } else if (strcmp(cmd, "complete_meeting") == 0 && argc >= 4) {
        meetingsChanged = completeMeeting(&meetings, argv[2], atoi(argv[3]));

    } else {
        printf("ERROR:unknown_or_bad_args:%s\n", cmd);
        return 2;
    }

    /* Save only what changed */
    if (menteesChanged)  menteeBstSave(mentees);
    if (notesChanged)    saveNotes(notes);
    if (requestsChanged) saveRequests(&requests);
    if (meetingsChanged) saveMeetings(&meetings);

    /* Free */
    menteeBstFree(mentees);
    noteFree(notes);
    stackFree(&meetings);
    queueFree(&requests);

    return 0;
}
