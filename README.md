# Mentoring Management System — FIXED

SSN College of Engineering · Dept. of IT

## Stack
- **Backend logic**  — C (`auth.c`, `mentor.c`, `mentee.c`, `manager.c`)
- **Middleware**     — Python Flask (`flask_backend/app.py`)
- **Frontend**       — HTML (Jinja templates) + single CSS file (`style.css`, green theme)
- **Storage**        — Plain text files in `data/` (no database)

## Project Structure
```
mentoring_management_system/
├── c_backend/
│   ├── structures.h          ← shared structs + load/save helpers
│   ├── auth.c                ← login validation
│   ├── mentor.c              ← mentor module (now with schedule_meeting)
│   ├── mentee.c              ← mentee module
│   ├── manager.c             ← manager module
│   ├── build.sh              ← Linux/macOS build script
│   └── build.bat             ← Windows build script
├── data/                     ← CSV-style flat files (the "database")
│   ├── users.txt             id,username,password,role,entity_id
│   ├── mentees.txt           roll,name,dept,cgpa,attendance,mentor_id
│   ├── meetings.txt          id,roll,mentor_id,date,time,mode,agenda,status
│   ├── requests.txt          id,roll,mentor_id,date,time,mode,purpose,status
│   └── notes.txt             id,roll,mentor_id,note,date
├── flask_backend/
│   └── app.py                ← Flask middleware (subprocess + parse + render)
└── frontend/
    ├── static/style.css
    └── templates/login.html, mentor.html, mentee.html, manager.html
```

## Run
```bash
# 1. Compile C programs
cd c_backend
./build.sh                 # Linux/macOS
# build.bat                # Windows

# 2. Install Flask once
pip install flask

# 3. Run Flask
cd ../flask_backend
python app.py              # http://localhost:5000
```

## Login Credentials (all `pass123`)
| Username  | Role    |
|-----------|---------|
| mentor1   | mentor  |
| mentor2   | mentor  |
| mentee1   | mentee  |
| mentee2   | mentee  |
| manager1  | manager |

## What was fixed (v3 → v3-FIXED)

### Backend / subprocess layer
- **`run()` now captures `stderr` and the C process return code.** Non-zero
  exits no longer fail silently — they surface as `ERROR:returncode_N:<stderr>`
  lines and the dashboard shows a real error banner instead of going blank.
- **Missing executables are detected up front** (`ERROR:exe_missing:<name>`) so
  you immediately see "you forgot to run build.sh" instead of an empty page.
- **Timeouts and exceptions** in `subprocess.run` are caught and reported.
- **Login** validates inputs and falls back to a clear error if `users.txt`
  is missing.

### C ↔ Flask ↔ Template contract (made consistent)
| Source                    | Now emits                                                                                          |
|---------------------------|----------------------------------------------------------------------------------------------------|
| `mentor view_meetings`    | `MEETING:id,roll,mentee_name,date,time,mode,agenda,status` (8 fields — matches `parse_meetings_mentor`) |
| `mentee view_meetings`    | `MEETING:id,date,time,mentor_id,mode,agenda,status` (7 fields — matches `parse_meetings_mentee`)   |
| `manager list_meetings`   | `MEETING:id,roll,mentee_name,mentor_id,date,time,mode,status` (8 fields — matches `parse_meetings_manager`) |
| `mentor view_requests`    | `REQUEST:id,roll,mentee_name,date,time,mode,purpose,status`                                        |
| `mentor view_notes`       | `NOTE:id,roll,mentee_name,note,date`                                                               |

Every parser now uses `split(",", N)` so commas inside the last field don't
break the parse.

### Data hygiene
- All user-supplied free text (notes, agendas, purposes, names, departments)
  has commas replaced with `;` or spaces **before** being passed to the C layer,
  preventing CSV column drift.
- `mentee request_meeting` validates the mentor ID exists.
- `mentor schedule_meeting` validates the mentee belongs to the mentor.
- Capacity overflow now returns `ERROR:capacity_full` instead of silently
  corrupting memory.
- All `strncpy` calls now explicitly null-terminate.

### NEW feature — Mentor can schedule meetings directly
- **C:**   `./mentor schedule_meeting <mentor_id> <roll> <date> <time> <mode> <agenda>`
  - validates the roll belongs to that mentor
  - creates a meeting with status = `confirmed`
- **Flask:** `POST /mentor/schedule`
- **UI:**   "Schedule Meeting" button on the Mentor → Meetings page opens a
            modal with a dropdown of *only* the mentor's own mentees.

### Things left intentionally simple
- Login still reads `users.txt` directly in Flask (rather than spawning
  `auth.exe`). This is faster and removes one moving part for beginners.
  `auth.c` is still included and works if you want to use it.
- File format is unchanged — your existing `data/*.txt` files load as-is.
