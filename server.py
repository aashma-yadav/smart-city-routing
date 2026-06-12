#!/usr/bin/env python3
"""
Flask wrapper that runs the compiled C++ CLI binary and exposes REST endpoints.
This version:
 - Chooses main.exe on Windows, main on Unix.
 - Increases read timeouts.
 - Prints raw binary output prefixed with [BIN] for debugging.
 - Is robust to slower stdout from the CLI.
"""
import os
import subprocess
import threading
import time
from flask import Flask, jsonify, request, send_from_directory

app = Flask(__name__, static_folder='static', template_folder='templates')

BASEDIR = os.path.dirname(os.path.abspath(__file__))
IS_WINDOWS = os.name == 'nt'
BIN = os.path.join(BASEDIR, "main.exe" if IS_WINDOWS else "main")

proc = None
proc_lock = threading.Lock()
output_buffer = []  # list of lines from binary stdout
read_thread = None

# HOW LONG to wait for the initial menu after starting the binary
INITIAL_PROMPT_TIMEOUT = 12.0
# HOW LONG to wait for a command's output (after sending)
COMMAND_TIMEOUT = 8.0

def compile_binary():
    """Compile main.cpp to BIN if possible. Returns True on success or if BIN already exists."""
    if os.path.exists(BIN):
        return True
    cpp = os.path.join(BASEDIR, "main.cpp")
    if not os.path.exists(cpp):
        print("No main.cpp found to compile at:", cpp)
        return False
    # Use g++ -std=c++11
    cmd = ["g++", "-std=c++11", "main.cpp", "-o", os.path.basename(BIN)]
    print("Compiling", cpp, "->", BIN)
    try:
        rc = subprocess.call(cmd, cwd=BASEDIR)
        if rc != 0:
            print("g++ returned non-zero exit code:", rc)
            return False
    except Exception as e:
        print("Failed to run g++:", e)
        return False
    return os.path.exists(BIN)

def start_proc():
    """Start the binary subprocess and reader thread. Wait until menu prompt appears."""
    global proc, read_thread
    if proc is not None:
        return

    if not compile_binary():
        raise RuntimeError("Failed to compile or find the binary. Ensure g++ and main.cpp exist.")

    # Start process
    proc = subprocess.Popen([BIN], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True, bufsize=1, cwd=BASEDIR)

    # start reader thread
    read_thread = threading.Thread(target=reader_thread, daemon=True)
    read_thread.start()

    # Wait until first prompt appears (the CLI prints its menu); give a generous timeout.
    s = wait_for_prompt(timeout=INITIAL_PROMPT_TIMEOUT)
    # Print what was collected so startup is visible in server console
    if s:
        print("[STARTUP OUTPUT]\n" + s)
    else:
        print("[STARTUP] No prompt detected in initial timeout.")

def reader_thread():
    """Continuously read stdout lines from the binary and append to output_buffer.
    Also print them prefixed with [BIN] for debugging.
    """
    global proc, output_buffer
    try:
        for line in proc.stdout:
            if line is None:
                break
            # print raw binary output for diagnostics (server console)
            try:
                # rstrip to avoid double newlines
                print("[BIN] " + line.rstrip())
            except Exception:
                # best-effort printing
                pass
            with proc_lock:
                output_buffer.append(line)
    except Exception as e:
        print("Reader thread exited with exception:", e)

def read_until_prompt(timeout=COMMAND_TIMEOUT):
    """Collect output_buffer data until we find the menu prompt marker, or timeout.
    Returns the output up to (but not including) the last occurrence of the prompt marker.
    """
    end_time = time.time() + timeout
    collected = ""
    while time.time() < end_time:
        with proc_lock:
            if output_buffer:
                collected += "".join(output_buffer)
                output_buffer.clear()
        # Look for menu prompt. Allow small variations: check for 'Enter choice' substring.
        if "Enter choice:" in collected or "Enter choice" in collected:
            # find last occurrence and return everything BEFORE it
            idx = max(collected.rfind("Enter choice:"), collected.rfind("Enter choice"))
            if idx != -1:
                return collected[:idx]
        time.sleep(0.05)
    # timeout: return whatever we have
    return collected

def wait_for_prompt(timeout=INITIAL_PROMPT_TIMEOUT):
    return read_until_prompt(timeout=timeout)

def send_commands(lines, expect_prompt=True):
    """Send a list of lines (strings) to the binary, flushing after each, then read result."""
    global proc
    if proc is None:
        raise RuntimeError("Binary process not started")

    # clear any previous unread output
    with proc_lock:
        output_buffer.clear()

    for l in lines:
        # write and flush
        try:
            proc.stdin.write(l + "\n")
            proc.stdin.flush()
        except Exception as e:
            print("Failed writing to binary stdin:", e)
            return ""
        # small pause to allow CLI to respond progressively
        time.sleep(0.05)

    # read until next prompt (or timeout)
    out = read_until_prompt(timeout=COMMAND_TIMEOUT)
    return out

# -------------------- Flask routes --------------------

@app.route("/")
def index():
    return send_from_directory("templates", "index.html")

@app.route("/static/<path:p>")
def static_files(p):
    return send_from_directory("static", p)

@app.route("/summary")
def summary():
    try:
        out = send_commands(["1"])
        return jsonify({"output": out})
    except Exception as e:
        return jsonify({"error": str(e), "output": ""}), 500

@app.route("/stops")
def stops():
    try:
        out = send_commands(["2"])
        return jsonify({"output": out})
    except Exception as e:
        return jsonify({"error": str(e), "output": ""}), 500

@app.route("/buses")
def buses():
    try:
        out = send_commands(["5"])
        return jsonify({"output": out})
    except Exception as e:
        return jsonify({"error": str(e), "output": ""}), 500

@app.route("/move_all", methods=["POST"])
def move_all():
    try:
        out = send_commands(["7"])
        return jsonify({"output": out})
    except Exception as e:
        return jsonify({"error": str(e), "output": ""}), 500

@app.route("/move_bus", methods=["POST"])
def move_bus():
    data = request.get_json() or request.form
    busid = data.get("busId") or data.get("busid") or ""
    if not busid:
        return jsonify({"error":"busId required"}), 400
    try:
        out = send_commands(["8", busid])
        return jsonify({"output": out})
    except Exception as e:
        return jsonify({"error": str(e), "output": ""}), 500

@app.route("/eta_between", methods=["POST"])
def eta_between():
    data = request.get_json() or request.form
    a = data.get("src") or data.get("a") or ""
    b = data.get("dst") or data.get("b") or ""
    if not a or not b:
        return jsonify({"error":"src and dst required"}), 400
    try:
        out = send_commands(["9", a, b])
        return jsonify({"output": out})
    except Exception as e:
        return jsonify({"error": str(e), "output": ""}), 500

@app.route("/eta_for_bus", methods=["POST"])
def eta_for_bus():
    data = request.get_json() or request.form
    bid = data.get("busId") or data.get("busid") or ""
    target = data.get("target") or data.get("targetStop") or ""
    if not bid or not target:
        return jsonify({"error":"busId and target required"}), 400
    try:
        out = send_commands(["10", bid, target])
        return jsonify({"output": out})
    except Exception as e:
        return jsonify({"error": str(e), "output": ""}), 500

@app.route("/shortest_path", methods=["POST"])
def shortest_path():
    data = request.get_json() or request.form
    a = data.get("src") or data.get("a") or ""
    b = data.get("dst") or data.get("b") or ""
    if not a or not b:
        return jsonify({"error":"src and dst required"}), 400
    try:
        out = send_commands(["11", a, b])
        return jsonify({"output": out})
    except Exception as e:
        return jsonify({"error": str(e), "output": ""}), 500

@app.route("/astar", methods=["POST"])
def astar():
    data = request.get_json() or request.form
    a = data.get("src") or data.get("a") or ""
    b = data.get("dst") or data.get("b") or ""
    if not a or not b:
        return jsonify({"error":"src and dst required"}), 400
    try:
        out = send_commands(["12", a, b])
        return jsonify({"output": out})
    except Exception as e:
        return jsonify({"error": str(e), "output": ""}), 500

@app.route("/mst")
def mst():
    try:
        out = send_commands(["13"])
        return jsonify({"output": out})
    except Exception as e:
        return jsonify({"error": str(e), "output": ""}), 500

@app.route("/suggest", methods=["GET"])
def suggest():
    pref = request.args.get("prefix","")
    try:
        out = send_commands(["14", pref])
        return jsonify({"output": out})
    except Exception as e:
        return jsonify({"error": str(e), "output": ""}), 500

@app.route("/history")
def history():
    try:
        out = send_commands(["17"])
        return jsonify({"output": out})
    except Exception as e:
        return jsonify({"error": str(e), "output": ""}), 500

@app.route("/logs")
def logs():
    try:
        out = send_commands(["18"])
        return jsonify({"output": out})
    except Exception as e:
        return jsonify({"error": str(e), "output": ""}), 500

if __name__ == "__main__":
    start_proc()
    print("Server running; binary stdout collector started.")
    # Bind to 0.0.0.0 so local machine can access from browser; keep default Flask behavior otherwise.
    app.run(host="0.0.0.0", port=5000)
