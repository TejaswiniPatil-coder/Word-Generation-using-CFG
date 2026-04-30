"""
CFG Simulator — Flask bridge between C backend and Web UI
Run: python3 server.py  (from inside the backend/ folder)
Then open: http://localhost:5000
"""
import subprocess, json, os
from flask import Flask, request, jsonify, send_from_directory

# frontend/ is one level up from backend/
FRONTEND_DIR = os.path.join(os.path.dirname(__file__), '..', 'frontend')

app = Flask(__name__, static_folder=FRONTEND_DIR)

CFG_SIM  = os.path.join(os.path.dirname(__file__), 'cfg_sim')
DATA_DIR = os.path.join(os.path.dirname(__file__), 'data')
OUT_DIR  = os.path.join(os.path.dirname(__file__), 'output')

os.makedirs(DATA_DIR, exist_ok=True)
os.makedirs(OUT_DIR,  exist_ok=True)

def run_cfg(inputs: str) -> str:
    """Send stdin to cfg_sim and capture stdout."""
    try:
        result = subprocess.run(
            [CFG_SIM],
            input=inputs,
            capture_output=True, text=True, timeout=10
        )
        return result.stdout
    except Exception as e:
        return f"Error: {e}"

@app.route('/')
def index():
    return send_from_directory(FRONTEND_DIR, 'index.html')

@app.route('/api/parse', methods=['POST'])
def api_parse():
    data = request.json
    grammar_text = data.get('grammar', '')
    gfile = os.path.join(DATA_DIR, 'grammar.txt')
    with open(gfile, 'w') as f:
        f.write(grammar_text)
    inp = f"1\n{gfile}\n3\n0\n"
    out = run_cfg(inp)
    return jsonify({'output': out, 'ok': True})

@app.route('/api/generate', methods=['POST'])
def api_generate():
    data = request.json
    grammar_text = data.get('grammar', '')
    max_len = int(data.get('maxLen', 8))
    gfile = os.path.join(DATA_DIR, 'grammar.txt')
    with open(gfile, 'w') as f:
        f.write(grammar_text)
    inp = f"1\n{gfile}\n4\n{max_len}\n0\n"
    run_cfg(inp)
    wfile = os.path.join(OUT_DIR, 'words.txt')
    words = []
    if os.path.exists(wfile):
        with open(wfile) as f:
            for line in f.readlines()[1:]:
                line = line.strip()
                if line:
                    words.append('' if line == 'epsilon' else line)
    return jsonify({'words': words})

@app.route('/api/validate', methods=['POST'])
def api_validate():
    data = request.json
    grammar_text = data.get('grammar', '')
    string = data.get('string', '')
    gfile = os.path.join(DATA_DIR, 'grammar.txt')
    with open(gfile, 'w') as f:
        f.write(grammar_text)
    inp = f"1\n{gfile}\n5\n{string}\n0\n"
    out = run_cfg(inp)
    valid = '✔' in out
    steps = []
    in_deriv = False
    for line in out.split('\n'):
        if 'Derivation' in line: in_deriv = True; continue
        if in_deriv and ('─' in line or '└' in line): in_deriv = False; continue
        if in_deriv and line.strip():
            steps.append(line.strip())
    return jsonify({'valid': valid, 'steps': steps, 'output': out})

if __name__ == '__main__':
    if not os.path.exists(CFG_SIM):
        print("ERROR: cfg_sim not compiled. Run: make  (inside backend/ folder)")
        exit(1)
    print("CFG Simulator server running at http://localhost:5000")
    app.run(debug=True, port=5000)
