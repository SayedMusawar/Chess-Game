# ♟️ Chess Game — Qt C++ Desktop Application

A fully-featured two-player chess game built with **C++17** and **Qt 6 (Widgets)**, featuring a polished wooden-board aesthetic, complete rule enforcement, and chess.com-style move highlighting.

---

## 📸 Screenshots

### Main Board — Starting Position
<img width="632" height="711" alt="1" src="https://github.com/user-attachments/assets/2a778421-36fb-49dd-b8c6-74f41bfaa30a" />

<!-- INSERT SCREENSHOT: Full board at game start, showing the wooden aesthetic and piece layout -->
&nbsp;

### Move Highlighting — Valid Moves & Captures
<img width="632" height="711" alt="2" src="https://github.com/user-attachments/assets/2c2ef9c2-ff24-44db-ad74-8d9b20702257" />

<!-- INSERT SCREENSHOT: A piece selected, green dots on empty squares, red highlights on capturable enemy pieces -->
&nbsp;

### King in Check
<img width="632" height="711" alt="3" src="https://github.com/user-attachments/assets/ec0321ff-4587-4096-aa5f-ea5e46d2900d" />

<!-- INSERT SCREENSHOT: The checked king's square flashing bright red with the status bar showing "♔ White is in CHECK!" -->
&nbsp;

### Checkmate / Game Over Dialog
<img width="632" height="711" alt="4" src="https://github.com/user-attachments/assets/373da052-5696-4d17-bba8-f5cbd5952712" />

<!-- INSERT SCREENSHOT: The QMessageBox announcing the winner -->
&nbsp;

---

## ✨ Features

- **Complete chess rule enforcement** — legal-only moves, including all edge-case rules
- **Castling** (kingside and queenside) — with correct "king cannot pass through check" validation
- **En passant** — double-pawn-push detection and diagonal capture on the following move
- **Automatic pawn promotion** — pawns reaching the back rank instantly become queens
- **Check detection** — king's square highlights bright red; status bar announces the threat
- **Checkmate & stalemate detection** — game-over dialog with winner announcement
- **chess.com-style highlighting:**
  - 🟡 Gold — currently selected piece
  - 🟢 Green dot — valid move to an empty square
  - 🔴 Red — enemy piece that can be captured (including en passant targets)
  - 🔴 Bright red — king currently in check
- **Coordinate labels** — a–h file letters and 1–8 rank numbers on the board border
- **Wooden board aesthetic** — cream/walnut squares with a dark wood border and styled status bar

---

## 🗂️ Project Structure

```
ChessGameProject/
├── src/
│   ├── main.cpp          # Application entry point
│   ├── chess.cpp         # Game engine: all piece logic, move validation, special rules
│   └── mainwindow.cpp    # Qt UI: painting, input handling, status updates
├── include/
│   ├── chess.h           # Piece class hierarchy, ChessGame interface
│   └── mainwindow.h      # MainWindow declaration
├── assets/               # PNG piece images (12 files: white/black × 6 piece types)
├── chess.qrc             # Qt resource file embedding all piece images
├── mainwindow.ui         # Qt Designer UI form
└── ChessGameProject.pro  # qmake project file
```

---

## 🏗️ Architecture

### Game Engine (`chess.h` / `chess.cpp`)

The engine is fully decoupled from Qt — it is pure C++17 and could be used with any frontend.

#### Class Hierarchy

```
Piece  (abstract base)
├── Pawn       — forward movement, double push, diagonal captures, en passant
├── Rook       — horizontal/vertical sliding
├── Knight     — L-shaped jumps
├── Bishop     — diagonal sliding
├── Queen      — combined rook + bishop sliding
└── King       — one-square movement (castling handled by ChessGame)
```

Each piece subclass implements:
- `getPossibleMoves()` — raw geometrically reachable squares (ignores check)
- `clone()` — deep copy via `std::unique_ptr<Piece>`, used by the move-simulation engine

#### `ChessGame` — Central Game State

| Responsibility | Detail |
|---|---|
| Board state | `std::array<std::array<Piece*, 8>, 8>` for O(1) lookup + `std::vector<std::unique_ptr<Piece>>` for ownership |
| Legal move filtering | `isMoveLegal()` deep-copies the entire board, simulates the move, and checks if the moving side's king is in check |
| En passant | `enPassantTarget_` stores the square a pawn can capture into; cleared after every non-double-push move |
| Castling | Validated entirely in `getValidMoves()` — checks piece `hasMoved` flags and that the king doesn't pass through or land on an attacked square |
| Check detection | `isKingInCheck()` calls `isSquareAttackedBy()` which iterates all opponent pieces |
| Checkmate / stalemate | `hasLegalMoves()` iterates all pieces and tests every move; no legal moves → checkmate (in check) or stalemate (not in check) |
| Copy semantics | Full copy constructor and assignment operator for safe board simulation — correctly rebuilds `board_` raw pointer array from cloned `pieces_` vector |

### UI Layer (`mainwindow.h` / `mainwindow.cpp`)

Built on `QMainWindow` with pure `QPainter` rendering — no QGraphicsScene.

| Method | Role |
|---|---|
| `drawBoard()` | Renders the 8×8 grid with cream/walnut squares inside a rounded dark-wood border |
| `drawCoordinates()` | Draws a–h / 1–8 labels; square color determines label color for contrast |
| `drawHighlights()` | Renders selected square (gold), move dots (green), capture squares (red), and check square (bright red) |
| `drawPieces()` | Loads piece PNGs from Qt resources (`:Images/assets/`) and draws them 64×64 with a 3 px inset padding |
| `mousePressEvent()` | Converts pixel coordinates to board row/col; either executes a pending move or selects a new piece and computes `validMoves` + `captureMoves` |
| `updateGameStatus()` | Checks for checkmate/stalemate (shows `QMessageBox`), or updates the status bar with whose turn it is and whether the king is in check |

The `captureMoves` vector (a subset of `validMoves`) is populated in `mousePressEvent` by checking whether a destination square holds an enemy piece **or** is an en passant diagonal from a pawn to an empty square.

---

## 🔧 Building

### Prerequisites

| Tool | Version |
|---|---|
| Qt | 6.x (Qt Widgets module) |
| Compiler | GCC / Clang / MSVC with C++17 support |
| qmake | Bundled with Qt |

### Build Steps

```bash
# 1. Clone / copy the project
cd ChessGameProject

# 2. Generate Makefile
qmake ChessGameProject.pro

# 3. Compile
make          # Linux/macOS
nmake         # Windows (MSVC)

# 4. Run
./ChessGameProject
```

Alternatively, open `ChessGameProject.pro` directly in **Qt Creator** and press **Run (Ctrl+R)**.

---

## 🖼️ Asset Requirements

The game expects **12 PNG images** (64×64 px recommended) embedded via Qt resources:

```
assets/
├── white_pawn.png     black_pawn.png
├── white_rook.png     black_rook.png
├── white_knight.png   black_knight.png
├── white_bishop.png   black_bishop.png
├── white_queen.png    black_queen.png
└── white_king.png     black_king.png
```

Images are loaded at path `:Images/assets/{color}_{type}.png`. Missing images print a warning to `qDebug()` but do not crash the application.

---

## 🎮 How to Play

1. **White moves first.** The status bar shows whose turn it is.
2. **Click a piece** to select it — its square turns gold and legal moves appear.
3. **Click a highlighted square** to move:
   - Green dot → move to empty square
   - Red highlight → capture enemy piece
4. Special moves happen automatically:
   - **Castling** — click the king two squares toward a rook
   - **En passant** — the diagonal pawn capture square is highlighted red on the turn it is available
   - **Promotion** — a pawn reaching rank 8 (white) or rank 1 (black) is automatically promoted to a queen
5. When a king is in **check**, its square flashes red and the status bar announces it.
6. **Checkmate or stalemate** triggers a game-over dialog.

---

## 🔩 Technical Notes

- **Move simulation for legality** — `isMoveLegal` performs a full deep copy of the game state per candidate move. This is safe and correct, but O(n) copies per legal-move query. For a future AI engine, incremental make/unmake would be more efficient.
- **No threat cache** — `isKingInCheck` always recomputes from scratch to avoid stale data in copied game states.
- **Pawn en passant distinction** — `Pawn::movesWithEP()` is separate from `getPossibleMoves()` so attack-detection (used in castling and check checks) doesn't incorrectly treat en passant squares as attacked squares.
- **Board offset constants** — `OX = 30`, `OY = 55` are file-scope constants shared between all drawing and hit-testing methods.

---

## 📄 License

This project is provided for educational and personal use. See `LICENSE` if present, or contact the author for usage terms.
