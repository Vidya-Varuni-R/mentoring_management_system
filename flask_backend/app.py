from flask import Flask, render_template, request, redirect, url_for, session
import subprocess, os

BASE_DIR     = os.path.dirname(os.path.abspath(__file__))
TEMPLATE_DIR = os.path.abspath(os.path.join(BASE_DIR, "..", "frontend", "templates"))
STATIC_DIR   = os.path.abspath(os.path.join(BASE_DIR, "..", "frontend", "static"))
LOGIN_EXE    = os.path.abspath(os.path.join(BASE_DIR, "..", "c_backend", "login.exe"))
MENTOR_EXE   = os.path.abspath(os.path.join(BASE_DIR, "..", "c_backend", "mentor.exe"))

app = Flask(__name__, template_folder=TEMPLATE_DIR, static_folder=STATIC_DIR)
app.secret_key = "mms_secret_key"


# ── run mentor.exe and return its stdout ──────────────────────
def run_mentor(args):
    r = subprocess.run([MENTOR_EXE] + args, capture_output=True, text=True)
    return r.stdout.strip()


# ── parse helpers ─────────────────────────────────────────────
def parse_mentees(out):
    rows = []
    for line in out.splitlines():
        if line.startswith("DATA:"):
            p = line[5:].split(",")
            if len(p) == 3:
                rows.append({"regno": p[0], "name": p[1], "dept": p[2]})
    return rows

def parse_meetings(out, mentees):
    # build a regno -> name lookup from the mentees list
    name_map = {m["regno"]: m["name"] for m in mentees}

    rows = []
    for line in out.splitlines():
        if line.startswith("MEETING:"):
            p = line[8:].split(",", 4)
            if len(p) == 5:
                rows.append({
                    "regno":       p[0],
                    "mentee_name": name_map.get(p[0], p[0]),  # name or fallback to regno
                    "date":        p[1],
                    "time":        p[2],
                    "mode":        p[3],
                    "agenda":      p[4],
                    "status":      "Scheduled",
                    "status_class": "success"
                })
    return rows

def parse_requests(out):
    rows = []
    for line in out.splitlines():
        if line.startswith("REQUEST:"):
            p = line[8:].split(",", 1)
            if len(p) == 2:
                rows.append({"regno": p[0], "message": p[1]})
    return rows

def current_user():
    return {
        "name":     session.get('username', 'Mentor'),
        "initials": session.get('username', 'ME')[:2].upper()
    }


# ══ LOGIN ═════════════════════════════════════════════════════

@app.route('/')
def home():
    return render_template('login.html')

@app.route('/login', methods=['POST'])
def login():
    username = request.form.get('username')
    password = request.form.get('password')
    role     = request.form.get('role')
    try:
        r = subprocess.run([LOGIN_EXE], input=f"{username}\n{password}\n{role}",
                           text=True, capture_output=True)
        out = r.stdout.strip()
        if out.startswith("SUCCESS"):
            session['username'] = username
            session['role']     = out.split(":")[1]
            return redirect(url_for(f"{session['role']}_dashboard"))
        return render_template('login.html', error="Invalid credentials. Please try again.")
    except Exception as e:
        return f"Error: {str(e)}"

@app.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('home'))


# ══ MENTOR DASHBOARD ══════════════════════════════════════════

@app.route('/mentor')
def mentor_dashboard():
    mentees       = parse_mentees(run_mentor(["view"]))
    meetings      = parse_meetings(run_mentor(["meetings"]),mentees)
    notifications = parse_requests(run_mentor(["requests"]))

    return render_template('mentor.html',
                           current_user=current_user(),
                           stats={
                               "total_mentees":  len(mentees),
                               "meetings_month": len(meetings),
                               "pending":        len(notifications)
                           },
                           mentees=mentees,
                           meetings=meetings,
                           notifications=notifications,
                           message=request.args.get('message'),
                           error=request.args.get('error'))


# ── Schedule Meeting (POST /mentor) ───────────────────────────
@app.route('/mentor', methods=['POST'])
def schedule_meeting():
    regno  = request.form.get('regno',  '').strip()
    date   = request.form.get('date',   '').strip()
    time   = request.form.get('time',   '').strip()
    mode   = request.form.get('mode',   '').strip()
    agenda = request.form.get('agenda', '').strip() or "No agenda"

    if not all([regno, date, time, mode]):
        return redirect(url_for('mentor_dashboard', error="Fill in all required fields."))

    # agenda may contain spaces, so split it into separate args.
    # C's main() will rejoin them with spaces back into one string.
    agenda_args = agenda.split()

    out = run_mentor(["schedule", regno, date, time, mode] + agenda_args)

    if out == "SUCCESS:meeting_scheduled":
        return redirect(url_for('mentor_dashboard', message="Meeting scheduled successfully."))
    return redirect(url_for('mentor_dashboard', error=f"Could not schedule meeting. C said: {out}"))


# ── Add Mentee ────────────────────────────────────────────────
@app.route('/mentor/add', methods=['POST'])
def add_mentee():
    regno = request.form.get('regno', '').strip()
    name  = request.form.get('name',  '').strip()
    dept  = request.form.get('dept',  '').strip()

    if not all([regno, name, dept]):
        return redirect(url_for('mentor_dashboard', error="All fields are required."))

    out = run_mentor(["add", regno, name, dept])

    if out == "SUCCESS:mentee_added":
        return redirect(url_for('mentor_dashboard', message="Mentee added successfully."))
    elif out == "ERROR:duplicate_regno":
        return redirect(url_for('mentor_dashboard', error=f"Register number {regno} already exists."))
    return redirect(url_for('mentor_dashboard', error="Could not add mentee."))


# ── Search Mentee ─────────────────────────────────────────────
@app.route('/mentor/search')
def search_mentee():
    regno  = request.args.get('regno', '').strip()
    result = None
    error  = None

    if regno:
        out = run_mentor(["search", regno])
        if out.startswith("FOUND:"):
            p = out[6:].split(",")
            if len(p) == 3:
                result = {"regno": p[0], "name": p[1], "dept": p[2]}
        else:
            error = f"No mentee found with register number: {regno}"

    return render_template('mentor.html',
                           current_user=current_user(),
                           stats={"total_mentees": 0, "meetings_month": 0, "pending": 0},
                           mentees=[], meetings=[], notifications=[],
                           search_result=result,
                           search_regno=regno,
                           error=error)


# ── Update Mentee ─────────────────────────────────────────────
@app.route('/mentor/update', methods=['POST'])
def update_mentee():
    regno = request.form.get('regno', '').strip()
    name  = request.form.get('name',  '').strip()
    dept  = request.form.get('dept',  '').strip()

    if not all([regno, name, dept]):
        return redirect(url_for('mentor_dashboard', error="All fields are required."))

    out = run_mentor(["update", regno, name, dept])

    if out == "SUCCESS:mentee_updated":
        return redirect(url_for('mentor_dashboard', message="Mentee updated successfully."))
    return redirect(url_for('mentor_dashboard', error=f"Register number {regno} not found."))


# ── Delete Mentee ─────────────────────────────────────────────
@app.route('/mentor/delete', methods=['POST'])
def delete_mentee():
    regno = request.form.get('regno', '').strip()
    if not regno:
        return redirect(url_for('mentor_dashboard', error="Register number is required."))

    out = run_mentor(["delete", regno])

    if out == "SUCCESS:mentee_deleted":
        return redirect(url_for('mentor_dashboard', message="Mentee deleted successfully."))
    return redirect(url_for('mentor_dashboard', error=f"Register number {regno} not found."))


# ══ MENTEE & MANAGER DASHBOARDS ═══════════════════════════════

@app.route('/mentee')
def mentee_dashboard():
    u = {
        "name":        session.get('username', 'Mentee'),
        "initials":    session.get('username', 'ME')[:2].upper(),
        "roll_no":     "-", "year": "-",
        "department":  "Information Technology",
        "mentor_name": "-", "section": "-",
        "cgpa":        "-", "arrears": "-",
        "attendance":  "-", "email":   "-"
    }
    return render_template('mentee.html', current_user=u, meetings=[])

@app.route('/manager')
def manager_dashboard():
    stats = {"total_mentors": 0, "total_mentees": 0, "meetings_month": 0, "flagged": 0}
    return render_template('manager.html', stats=stats, mentors=[], flagged=[])


if __name__ == '__main__':
    app.run(debug=True)