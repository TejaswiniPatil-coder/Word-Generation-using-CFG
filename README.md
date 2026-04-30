# CFG Simulator — Context-Free Grammar Toolkit
### Group 11

A complete CFG (Context-Free Grammar) simulator featuring:
- **Word Generator** — generates all valid strings up to length N
- **String Validator** — checks if a string belongs to the language
- **Derivation Visualizer** — shows leftmost derivation step-by-step
- **Parse Tree** — ASCII (CLI) and interactive SVG (Web UI)
- **Animated Web UI** — dark-themed, standalone, no install needed
- **Mini Game** — guess if a string is valid or invalid!

---

## 📁 Project Structure

```
Group11_CFG_Project/
├── README.md
│
├── frontend/
│   └── index.html          ← Standalone Web UI (open directly in browser)
│
└── backend/
    ├── main.c              ← CLI entry point
    ├── grammar.c           ← CFG parsing & storage
    ├── generator.c         ← Word generation (recursive DFS)
    ├── parser.c            ← String validation & derivation
    ├── tree.c              ← Parse tree (ASCII + JSON)
    ├── utils.c             ← File I/O helpers
    ├── server.py           ← Flask bridge (optional)
    ├── Makefile
    ├── include/
    │   ├── grammar.h
    │   ├── generator.h
    │   ├── parser.h
    │   └── tree.h
    ├── data/
    │   └── grammar.txt     ← Default grammar (edit this)
    └── output/             ← Auto-created output files
```

---

## 🚀 HOW TO RUN

### Option 1 — Web UI (Recommended, Zero Setup)

Just open the HTML file — no server needed!

```
Open:  frontend/index.html
       (double-click or drag into any browser)
```

The web UI has the full CFG engine built in JavaScript:
- Enter grammar → Click **Parse Grammar**
- Click **Generate** for all valid words
- Enter a string → Click **Validate**
- View animated derivation steps + interactive parse tree
- Play the **Mini Game** to test your understanding!

---

### Option 2 — C Command-Line Interface

**Requirements:** GCC (Linux/Mac/Windows with MinGW)

```bash
cd backend/
make
./cfg_sim          # Linux/Mac
cfg_sim.exe        # Windows
```

**Menu options:**
```
1. Load grammar from file
2. Enter grammar manually
3. Display current grammar
4. Generate words (up to N)
5. Validate a string
6. Show parse tree (ASCII)
7. Export words to file
0. Exit
```

---

### Option 3 — Flask Web Server (C backend + Python frontend)

**Requirements:** Python 3, Flask (`pip install flask`), compiled `cfg_sim`

```bash
cd backend/
make                    # compile C backend
python3 server.py       # start server → opens frontend/index.html
# Open: http://localhost:5000
```

---

## ✏️ Grammar Format

```
S->aSb|#         ← S → aSb | ε
A->aA|a          ← A → aA | a
E->E+T|T         ← E → E+T | T
```

- **Uppercase** = Non-terminals (S, A, B, E …)
- **Lowercase** = Terminals (a, b, c …)
- **#** or **ε** or **epsilon** = empty string
- **|** separates alternatives
- One rule per line

---

## 💡 Key Concepts Implemented

| Feature | Technique |
|---------|-----------|
| CFG Storage | `Production` struct with LHS/RHS array |
| Word Generation | Recursive DFS with depth + length pruning |
| String Validation | Leftmost derivation with backtracking |
| Parse Tree (CLI) | Recursive tree with box-drawing characters |
| Parse Tree (Web) | SVG with auto-layout algorithm |
| Derivation Display | Animated step-by-step reveal |
| Mini Game | Random valid/invalid string guessing challenge |
