from flask import Flask, render_template, request, redirect, url_for
import subprocess
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

# ✅ Correct paths (based on YOUR structure)
TEMPLATE_DIR = os.path.abspath(os.path.join(BASE_DIR, "..", "frontend", "templates"))
STATIC_DIR = os.path.abspath(os.path.join(BASE_DIR, "..", "frontend", "static"))
LOGIC_PATH = os.path.abspath(os.path.join(BASE_DIR, "..", "c_backend", "login.exe"))

app = Flask(__name__, template_folder=TEMPLATE_DIR, static_folder=STATIC_DIR)


# 🟢 Login Page
@app.route('/')
def home():
    return render_template('login.html')


# 🟢 Handle Login
@app.route('/login', methods=['POST'])
def login():
    username = request.form.get('username')
    password = request.form.get('password')
    role = request.form.get('role')

    try:
        result = subprocess.run(
            [LOGIC_PATH],
            input=f"{username}\n{password}\n{role}",
            text=True,
            capture_output=True
        )

        output = result.stdout.strip()
        print("C Output:", output)

        if output.startswith("SUCCESS"):
            user_role = output.split(":")[1]

            if user_role == "mentor":
                return redirect(url_for('mentor_dashboard'))
            elif user_role == "mentee":
                return redirect(url_for('mentee_dashboard'))
            elif user_role == "manager":
                return redirect(url_for('manager_dashboard'))

        return "Invalid credentials"

    except Exception as e:
        return f"Error: {str(e)}"


# 🟣 Dashboards
@app.route('/mentor')
def mentor_dashboard():
    return render_template('mentor.html')


@app.route('/mentee')
def mentee_dashboard():
    return render_template('mentee.html')


@app.route('/manager')
def manager_dashboard():
    return render_template('manager.html')


# 🟢 Run
if __name__ == '__main__':
    app.run(debug=True)