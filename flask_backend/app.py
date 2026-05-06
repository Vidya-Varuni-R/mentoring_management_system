"""
app.py  —  Flask Middleware  (FIXED)
Mentoring Management System, SSN College of Engineering

Architecture:
  Flask  ─►  subprocess.run(C_exe, cwd=C_DIR, capture stdout/stderr/returncode)
         ─►  parse line-prefixed protocol (DATA: / MEETING: / NOTE: / ...)
         ─►  render Jinja templates

Run:  python app.py    (from flask_backend/)
Open: http://localhost:5000
"""

from flask import Flask, render_template, request, redirect, url_for, session, Response
import subprocess
import os
import sys
from flask import Response

# ── Paths ──────────────────────────────────────────────────────
BASE_DIR     = os.path.dirname(os.path.abspath(__file__))
TEMPLATE_DIR = os.path.abspath(os.path.join(BASE_DIR, "..", "frontend", "templates"))
STATIC_DIR   = os.path.abspath(os.path.join(BASE_DIR, "..", "frontend", "static"))
C_DIR        = os.path.abspath(os.path.join(BASE_DIR, "..", "c_backend"))
DATA_DIR     = os.path.abspath(os.path.join(BASE_DIR, "..", "data"))

# Windows-friendly: try .exe first, fall back to no-extension binary
def _exe(name: str) -> str:
    win = os.path.join(C_DIR, name + ".exe")
    nix = os.path.join(C_DIR, name)
    return win if os.path.exists(win) else nix

AUTH_EXE    = _exe("auth")
MENTOR_EXE  = _exe("mentor")
MENTEE_EXE  = _exe("mentee")
MANAGER_EXE = _exe("manager")

app = Flask(__name__, template_folder=TEMPLATE_DIR, static_folder=STATIC_DIR)
app.secret_key = "mms_ssn_2026"


# ══════════════════════════════════════════════════════════════
# SUBPROCESS HELPER  (now captures stderr + return code)
# ══════════════════════════════════════════════════════════════

def run(exe, args):
    """
    Execute a C binary. Returns a list of stdout lines.
    Always inserts a synthetic line on error so callers can
    surface a meaningful message instead of a blank UI.
    """
    if not os.path.exists(exe):
        msg = f"ERROR:exe_missing:{os.path.basename(exe)}"
        print(f"[run] {msg}", file=sys.stderr)
        return [msg]
    try:
        result = subprocess.run(
            [exe] + [str(a) for a in args],
            capture_output=True, text=True, timeout=8,
            cwd=C_DIR,
        )
    except subprocess.TimeoutExpired:
        return ["ERROR:timeout"]
    except Exception as e:
        return [f"ERROR:exception:{e}"]

    stdout = (result.stdout or "").strip()
    stderr = (result.stderr or "").strip()
    if result.returncode != 0:
        # Surface but don't hide stdout (the C code may still have written useful lines)
        print(f"[run] {os.path.basename(exe)} {args} rc={result.returncode} stderr={stderr}",
              file=sys.stderr)
        lines = stdout.splitlines() if stdout else []
        lines.append(f"ERROR:returncode_{result.returncode}:{stderr or 'no_stderr'}")
        return lines
    if stderr:
        print(f"[run] {os.path.basename(exe)} stderr: {stderr}", file=sys.stderr)
    return stdout.splitlines()


def first_status(lines):
    """Return the first SUCCESS:/ERROR:/NOT_FOUND: line, or '' if none."""
    for line in lines:
        if line.startswith(("SUCCESS:", "ERROR:", "NOT_FOUND:", "FAIL:")):
            return line
    return lines[0] if lines else ""


# ══════════════════════════════════════════════════════════════
# PARSERS
# ══════════════════════════════════════════════════════════════

def parse_mentees(lines):
    out = []
    for line in lines:
        if line.startswith("DATA:"):
            p = line[5:].split(",")
            if len(p) >= 5:
                out.append({"roll": p[0], "name": p[1], "dept": p[2],
                            "cgpa": p[3], "attendance": p[4]})
    return out


def parse_meetings_mentor(lines):
    """C: MEETING:id,roll,mentee_name,date,time,mode,agenda,status"""
    out = []
    for line in lines:
        if line.startswith("MEETING:"):
            p = line[8:].split(",", 7)   # keep agenda commas (already sanitized) just in case
            if len(p) >= 8:
                out.append({"id": p[0], "roll": p[1], "mentee_name": p[2],
                            "date": p[3], "time": p[4], "mode": p[5],
                            "agenda": p[6], "status": p[7],
                            "status_class": p[7]})
    return out


def parse_meetings_mentee(lines):
    """C: MEETING:id,date,time,mentor_id,mode,agenda,status"""
    out = []
    for line in lines:
        if line.startswith("MEETING:"):
            p = line[8:].split(",", 6)
            if len(p) >= 7:
                out.append({"id": p[0], "date": p[1], "time": p[2],
                            "mentor_id": p[3], "mode": p[4],
                            "agenda": p[5], "status": p[6],
                            "status_class": p[6]})
    return out


def parse_meetings_manager(lines):
    """C: MEETING:id,roll,mentee_name,mentor_id,date,time,mode,status"""
    out = []
    for line in lines:
        if line.startswith("MEETING:"):
            p = line[8:].split(",", 7)
            if len(p) >= 8:
                out.append({"id": p[0], "roll": p[1], "mentee_name": p[2],
                            "mentor_id": p[3], "date": p[4], "time": p[5],
                            "mode": p[6], "status": p[7],
                            "status_class": p[7]})
    return out


def parse_notes(lines):
    out = []
    for line in lines:
        if line.startswith("NOTE:"):
            p = line[5:].split(",", 4)
            if len(p) >= 5:
                out.append({"id": p[0], "roll": p[1], "mentee_name": p[2],
                            "note": p[3], "date": p[4]})
    return out


def parse_requests_mentor(lines):
    """C: REQUEST:id,roll,mentee_name,date,time,mode,purpose,status"""
    out = []
    for line in lines:
        if line.startswith("REQUEST:"):
            p = line[8:].split(",", 7)
            if len(p) >= 8:
                out.append({"id": p[0], "roll": p[1], "mentee_name": p[2],
                            "date": p[3], "time": p[4], "mode": p[5],
                            "purpose": p[6], "status": p[7]})
    return out


def parse_requests_mentee(lines):
    """C: REQUEST:id,date,time,mode,purpose,status"""
    out = []
    for line in lines:
        if line.startswith("REQUEST:"):
            p = line[8:].split(",", 5)
            if len(p) >= 6:
                out.append({"id": p[0], "date": p[1], "time": p[2],
                            "mode": p[3], "purpose": p[4], "status": p[5],
                            "status_class": p[5]})
    return out


def collect_errors(*lists):
    """Surface ERROR:/exe_missing lines as a single human message."""
    msgs = []
    for lst in lists:
        for line in lst:
            if line.startswith("ERROR:"):
                msgs.append(line[6:])
    return "; ".join(sorted(set(msgs))) if msgs else None


def login_required(role=None):
    if "user_id" not in session:
        return False
    if role and session.get("role") != role:
        return False
    return True


# ══════════════════════════════════════════════════════════════
# AUTH ROUTES
# ══════════════════════════════════════════════════════════════

@app.route("/")
def home():
    if "user_id" in session:
        return redirect(url_for(f"{session['role']}_dashboard"))
    return render_template("login.html")


@app.route("/login", methods=["POST"])
def login():
    username = request.form.get("username", "").strip()
    password = request.form.get("password", "").strip()
    if not username or not password:
        return render_template("login.html", error="Username and password are required.")

    # Read users.txt directly — robust even if auth.exe is not compiled.
    users_file = os.path.join(DATA_DIR, "users.txt")
    matched = None
    try:
        with open(users_file, "r") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                parts = line.split(",")
                if len(parts) >= 5:
                    uid, uname, upass, urole, entity_id = parts[:5]
                    if uname == username and upass == password:
                        matched = {"user_id": uid, "role": urole, "entity_id": entity_id}
                        break
    except FileNotFoundError:
        return render_template("login.html",
            error=f"users.txt not found at {users_file}")
    except Exception as e:
        return render_template("login.html", error=f"Server error: {e}")

    if matched:
        session.clear()
        session["role"]      = matched["role"]
        session["user_id"]   = matched["user_id"]
        session["entity_id"] = matched["entity_id"]
        session["username"]  = username
        return redirect(url_for(f"{matched['role']}_dashboard"))

    return render_template("login.html", error="Incorrect username or password.")


@app.route("/logout")
def logout():
    session.clear()
    return redirect(url_for("home"))


# ══════════════════════════════════════════════════════════════
# MENTOR ROUTES
# ══════════════════════════════════════════════════════════════

@app.route("/mentor")
def mentor_dashboard():
    if not login_required("mentor"):
        return redirect(url_for("home"))

    mid = session["entity_id"]
    raw_mentees  = run(MENTOR_EXE, ["view_mentees",  mid])
    raw_meetings = run(MENTOR_EXE, ["view_meetings", mid])
    raw_notes    = run(MENTOR_EXE, ["view_notes",    mid])
    raw_requests = run(MENTOR_EXE, ["view_requests", mid])

    mentees   = parse_mentees(raw_mentees)
    meetings  = parse_meetings_mentor(raw_meetings)
    notes     = parse_notes(raw_notes)
    requests_ = parse_requests_mentor(raw_requests)

    search_regno  = request.args.get("regno", "").strip()
    search_result = None
    search_miss   = False
    if search_regno:
        for line in run(MENTOR_EXE, ["search_mentee", search_regno]):
            if line.startswith("FOUND:"):
                p = line[6:].split(",")
                if len(p) >= 5:
                    search_result = {"roll": p[0], "name": p[1], "dept": p[2],
                                     "cgpa": p[3], "attendance": p[4]}
            elif line.startswith("NOT_FOUND:"):
                search_miss = True

    current_user = {"name": session["username"],
                    "initials": session["username"][:2].upper()}
    stats = {"total_mentees": len(mentees), "meetings_count": len(meetings),
             "pending_req":   len(requests_), "notes_count": len(notes)}

    backend_error = collect_errors(raw_mentees, raw_meetings, raw_notes, raw_requests)
    error = request.args.get("error") or backend_error
    if search_miss and not search_result:
        error = (error + "; " if error else "") + f"No mentee found with roll {search_regno}."

    return render_template("mentor.html",
        current_user=current_user, stats=stats,
        mentees=mentees, meetings=meetings, notes=notes, requests=requests_,
        search_result=search_result, search_regno=search_regno,
        message=request.args.get("message"), error=error)


@app.route("/mentor/add", methods=["POST"])
def mentor_add_mentee():
    if not login_required("mentor"): return redirect(url_for("home"))
    mid  = session["entity_id"]
    roll = request.form.get("roll", "").strip()
    name = request.form.get("name", "").strip().replace(",", " ")
    dept = request.form.get("dept", "").strip().replace(",", " ")
    cgpa = request.form.get("cgpa", "").strip()
    att  = request.form.get("attendance", "").strip()
    if not all([roll, name, dept, cgpa, att]):
        return redirect(url_for("mentor_dashboard", error="All fields are required."))
    out = first_status(run(MENTOR_EXE, ["add_mentee", mid, name, dept, roll, cgpa, att]))
    if out == "SUCCESS:mentee_added":
        return redirect(url_for("mentor_dashboard", message=f"Mentee {name} added."))
    if out == "ERROR:duplicate_roll":
        return redirect(url_for("mentor_dashboard", error=f"Roll {roll} already exists."))
    return redirect(url_for("mentor_dashboard", error=f"Could not add mentee ({out})."))


@app.route("/mentor/update", methods=["POST"])
def mentor_update_mentee():
    if not login_required("mentor"): return redirect(url_for("home"))
    roll = request.form.get("roll", "").strip()
    name = request.form.get("name", "").strip().replace(",", " ")
    dept = request.form.get("dept", "").strip().replace(",", " ")
    cgpa = request.form.get("cgpa", "").strip()
    att  = request.form.get("attendance", "").strip()
    out = first_status(run(MENTOR_EXE, ["update_mentee", roll, name, dept, cgpa, att]))
    if out == "SUCCESS:mentee_updated":
        return redirect(url_for("mentor_dashboard", message="Mentee updated."))
    return redirect(url_for("mentor_dashboard", error=f"Could not update {roll} ({out})."))


@app.route("/mentor/delete", methods=["POST"])
def mentor_delete_mentee():
    if not login_required("mentor"): return redirect(url_for("home"))
    roll = request.form.get("roll", "").strip()
    out = first_status(run(MENTOR_EXE, ["delete_mentee", roll]))
    if out == "SUCCESS:mentee_deleted":
        return redirect(url_for("mentor_dashboard", message=f"Mentee {roll} removed."))
    return redirect(url_for("mentor_dashboard", error=f"Could not delete {roll} ({out})."))


@app.route("/mentor/add-note", methods=["POST"])
def mentor_add_note():
    if not login_required("mentor"): return redirect(url_for("home"))
    mid  = session["entity_id"]
    roll = request.form.get("roll", "").strip()
    note = request.form.get("note", "").strip().replace(",", ";")
    if not roll or not note:
        return redirect(url_for("mentor_dashboard", error="Roll and note are required."))
    out = first_status(run(MENTOR_EXE, ["add_note", mid, roll, note]))
    if out == "SUCCESS:note_added":
        return redirect(url_for("mentor_dashboard", message="Note saved."))
    return redirect(url_for("mentor_dashboard", error=f"Could not save note ({out})."))


@app.route("/mentor/respond-request", methods=["POST"])
def mentor_respond_request():
    if not login_required("mentor"): return redirect(url_for("home"))
    req_id   = request.form.get("request_id", "").strip()
    response = request.form.get("response", "rejected").strip()
    out = first_status(run(MENTOR_EXE, ["respond_request", req_id, response]))
    if out.startswith("SUCCESS:"):
        msg = "Request accepted — meeting scheduled." if response == "accepted" else "Request declined."
        return redirect(url_for("mentor_dashboard", message=msg))
    return redirect(url_for("mentor_dashboard", error=f"Could not process request ({out})."))


@app.route("/mentor/schedule", methods=["POST"])
def mentor_schedule_meeting():
    """NEW: mentor directly schedules a confirmed meeting with one of their mentees."""
    if not login_required("mentor"): return redirect(url_for("home"))
    mid     = session["entity_id"]
    roll    = request.form.get("roll", "").strip()
    date    = request.form.get("date", "").strip()
    time_v  = request.form.get("time", "").strip()
    mode    = request.form.get("mode", "in-person").strip()
    agenda  = request.form.get("agenda", "").strip().replace(",", ";")
    if not all([roll, date, time_v, agenda]):
        return redirect(url_for("mentor_dashboard",
            error="All fields are required to schedule a meeting."))
    out = first_status(run(MENTOR_EXE,
        ["schedule_meeting", mid, roll, date, time_v, mode, agenda]))
    if out == "SUCCESS:meeting_scheduled":
        return redirect(url_for("mentor_dashboard",
            message=f"Meeting with {roll} scheduled for {date} {time_v}."))
    if out == "ERROR:not_your_mentee":
        return redirect(url_for("mentor_dashboard",
            error=f"{roll} is not assigned to you."))
    return redirect(url_for("mentor_dashboard",
        error=f"Could not schedule meeting ({out})."))


# ══════════════════════════════════════════════════════════════
# MENTEE ROUTES
# ══════════════════════════════════════════════════════════════

@app.route("/mentee")
def mentee_dashboard():
    if not login_required("mentee"): return redirect(url_for("home"))

    roll = session["entity_id"]
    raw_profile  = run(MENTEE_EXE, ["view_profile",  roll])
    raw_meetings = run(MENTEE_EXE, ["view_meetings", roll])
    raw_requests = run(MENTEE_EXE, ["view_requests", roll])

    current_user = {"name": session["username"],
                    "initials": session["username"][:2].upper(),
                    "roll_no": roll, "department": "IT",
                    "mentor_name": "-", "mentor_id": "-",
                    "cgpa": "-", "attendance": "-"}
    for line in raw_profile:
        if line.startswith("PROFILE:"):
            p = line[8:].split(",")
            if len(p) >= 7:
                current_user.update({"roll_no": p[0], "name": p[1],
                                     "department": p[2], "cgpa": p[3],
                                     "attendance": p[4], "mentor_id": p[5],
                                     "mentor_name": p[6]})

    meetings  = parse_meetings_mentee(raw_meetings)
    requests_ = parse_requests_mentee(raw_requests)

    backend_error = collect_errors(raw_profile, raw_meetings, raw_requests)
    error = request.args.get("error") or backend_error

    return render_template("mentee.html",
        current_user=current_user, meetings=meetings, requests=requests_,
        message=request.args.get("message"), error=error)


@app.route("/mentee/request-meeting", methods=["POST"])
def mentee_request_meeting():
    if not login_required("mentee"): return redirect(url_for("home"))
    roll    = session["entity_id"]
    mid     = request.form.get("mentor_id", "").strip()
    date    = request.form.get("date", "").strip()
    time_v  = request.form.get("time", "").strip()
    mode    = request.form.get("mode", "in-person").strip()
    purpose = request.form.get("purpose", "").strip().replace(",", ";")
    if not all([mid, date, time_v, purpose]):
        return redirect(url_for("mentee_dashboard", error="All fields are required."))
    out = first_status(run(MENTEE_EXE,
        ["request_meeting", roll, mid, date, time_v, mode, purpose]))
    if out == "SUCCESS:request_sent":
        return redirect(url_for("mentee_dashboard", message="Meeting request sent."))
    if out == "ERROR:unknown_mentor":
        return redirect(url_for("mentee_dashboard", error=f"Unknown mentor ID: {mid}"))
    return redirect(url_for("mentee_dashboard", error=f"Could not send request ({out})."))


# ══════════════════════════════════════════════════════════════
# MANAGER ROUTES
# ══════════════════════════════════════════════════════════════

@app.route("/manager")
def manager_dashboard():
    if not login_required("manager"): return redirect(url_for("home"))

    raw_summary  = run(MANAGER_EXE, ["summary"])
    raw_mentors  = run(MANAGER_EXE, ["list_mentors"])
    raw_mentees  = run(MANAGER_EXE, ["list_mentees"])
    raw_meetings = run(MANAGER_EXE, ["list_meetings"])

    summary_line = ""
    for line in raw_summary:
        if line.startswith("SUMMARY:"): summary_line = line[8:]; break
    sp = summary_line.split(",") if summary_line else []
    stats = {"total_mentors":  sp[0] if len(sp) > 0 else "0",
             "total_mentees":  sp[1] if len(sp) > 1 else "0",
             "total_meetings": sp[2] if len(sp) > 2 else "0",
             "flagged":        sp[3] if len(sp) > 3 else "0"}

    mentors = []
    for line in raw_mentors:
        if line.startswith("MENTOR:"):
            p = line[7:].split(",")
            if len(p) >= 4:
                mentors.append({"mentor_id": p[0], "name": p[1],
                                "mentee_count": p[2], "meeting_count": p[3]})

    all_mentees, flagged = [], []
    for line in raw_mentees:
        if line.startswith("MENTEE:"):
            p = line[7:].split(",")
            if len(p) >= 7:
                m = {"roll": p[0], "name": p[1], "dept": p[2],
                     "cgpa": p[3], "attendance": p[4],
                     "mentor_id": p[5], "flag": p[6]}
                all_mentees.append(m)
                if p[6] != "ok":
                    m["flag_label"] = {"attendance": "Low Attendance",
                                       "cgpa": "Low CGPA",
                                       "both": "Both Issues"}.get(p[6], p[6])
                    m["flag_class"] = "red" if p[6] == "both" else "amber"
                    flagged.append(m)

    all_meetings = parse_meetings_manager(raw_meetings)

    backend_error = collect_errors(raw_summary, raw_mentors, raw_mentees, raw_meetings)
    error = request.args.get("error") or backend_error

    return render_template("manager.html",
        stats=stats, mentors=mentors, all_mentees=all_mentees,
        flagged=flagged, all_meetings=all_meetings,
        message=request.args.get("message"), error=error)




@app.route("/manager/export")
def manager_export():
    if not login_required("manager"):
        return redirect(url_for("home"))

    lines = run(MANAGER_EXE, ["export_report"])
    text = "\n".join(lines)

    return Response(
        text,
        mimetype="text/plain",
        headers={
            "Content-Disposition": "attachment;filename=report.txt"
        }
    )


# ══════════════════════════════════════════════════════════════
if __name__ == "__main__":
    print(f"[mms] C_DIR    = {C_DIR}")
    print(f"[mms] DATA_DIR = {DATA_DIR}")
    for name, path in [("auth", AUTH_EXE), ("mentor", MENTOR_EXE),
                       ("mentee", MENTEE_EXE), ("manager", MANAGER_EXE)]:
        flag = "OK" if os.path.exists(path) else "MISSING (compile it!)"
        print(f"[mms] {name:7s}: {path}  [{flag}]")
    app.run(debug=True, port=5000)
