## ♟️ **Chess Game — Qt C++ Desktop Application** 

A fully-featured two-player chess game built with **C++17** and **Qt 6 (Widgets)** , featuring a polished wooden-board aesthetic, complete rule enforcement, and chess.com-style move highlighting. 

## **Screenshots** 📸 

**Main Board — Starting Position** 

**Move Highlighting — Valid Moves & Captures** 

## **King in Check** 

## **Checkmate / Game Over Dialog** 

## **Features** ✨ 

- **Complete chess rule enforcement** — legal-only moves, including all edge-case rules 

- **Castling** (kingside and queenside) — with correct "king cannot pass through check" validation 

- **En passant** — double-pawn-push detection and diagonal capture on the following move 

- **Automatic pawn promotion** — pawns reaching the back rank instantly become queens 

- **Check detection** — king's square highlights bright red; status bar announces the threat 

- **Checkmate & stalemate detection** — game-over dialog with winner announcement 

##  **chess.com-style highlighting:** 

   - 🟡 Gold — currently selected piece 

   - 🟢 Green dot — valid move to an empty square 

   - 🔴 Red — enemy piece that can be captured (including en passant targets) 

   - 🔴 Bright red — king currently in check 

- **Coordinate labels** — a–h file letters and 1–8 rank numbers on the board border 

- **Wooden board aesthetic** — cream/walnut squares with a dark wood border and styled status bar 

## 🗂️ **Project Structure** 

```
ChessGameProject/
├── src/
│   ├── main.cpp          # Application entry point
│   ├── chess.cpp         # Game engine: all piece logic, move validation,
special rules
│   └── mainwindow.cpp    # Qt UI: painting, input handling, status updates
├── include/
│   ├── chess.h           # Piece class hierarchy, ChessGame interface
│   └── mainwindow.h      # MainWindow declaration
├── assets/               # PNG piece images (12 files: white/black × 6 piece
types)
├── chess.qrc             # Qt resource file embedding all piece images
├── mainwindow.ui         # Qt Designer UI form
└── ChessGameProject.pro  # qmake project file
```

## **Architecture** 🏗️ 

## **Game Engine (** **`chess.h` /** **`chess.cpp` )** 

The engine is fully decoupled from Qt — it is pure C++17 and could be used with any frontend. 

## _**Class Hierarchy**_ 

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

- `clone()` — deep copy via `std::unique_ptr<Piece>` , used by the move-simulation engine 

## _**`ChessGame` — Central Game State**_ 

|**_`ChessGame` — Central Game State_**||
|---|---|
|**Responsibility**|**Detail**|
|Board state|`std::array<std::array<Piece*,`<br>`8>, 8>`for O(1) lookup +<br>`std::vector<std::unique_ptr<Piec`<br>`e>>`for ownership|
|Legal move filtering|`isMoveLegal()`deep-copies the entire board,<br>simulates the move, and checks if the moving<br>side's king is in check|
|En passant|`enPassantTarget_`stores the square a pawn<br>can capture into; cleared after every non-double-<br>push move|
|Castling|Validated entirely in`getValidMoves()`—<br>checks piece`hasMoved`flags and that the king<br>doesn't pass through or land on an attacked<br>square|
|Check detection|`isKingInCheck()`calls<br>`isSquareAttackedBy()`which iterates all<br>opponent pieces|
|Checkmate / stalemate|`hasLegalMoves()`iterates all pieces and<br>tests every move; no legal moves → checkmate<br>(in check) or stalemate (not in check)|
|Copy semantics|Full copy constructor and assignment operator<br>for safe board simulation — correctly rebuilds<br>`board_`raw pointer array from cloned<br>`pieces_`vector|



## **UI Layer (** **`mainwindow.h` /** **`mainwindow.cpp` )** 

Built on `QMainWindow` with pure `QPainter` rendering — no QGraphicsScene. 

|Built on`QMainWindow`with pure`QPainter`re|ndering — no QGraphicsScene.|
|---|---|
|**Method**|**Role**|
|`drawBoard()`|Renders the 8×8 grid with cream/walnut squares<br>inside a rounded dark-wood border|
|`drawCoordinates()`|Draws a–h / 1–8 labels; square color determines<br>label color for contrast|
|`drawHighlights()`|Renders selected square (gold), move dots<br>(green), capture squares (red), and check square<br>(bright red)|
|`drawPieces()`|Loads piece PNGs from Qt resources<br>(`:Images/assets/`) and draws them 64×64<br>with a 3 px inset padding|
|`mousePressEvent()`|Converts pixel coordinates to board row/col;<br>either executes a pending move or selects a new<br>piece and computes`validMoves`+|



|**Method**|**Role**|
|---|---|
||`captureMoves`|
|`updateGameStatus()`|Checks for checkmate/stalemate (shows<br>`QMessageBox`), or updates the status bar with<br>whose turn it is and whether the king is in check|



The `captureMoves` vector (a subset of `validMoves` ) is populated in `mousePressEvent` by checking whether a destination square holds an enemy piece **or** is an en passant diagonal from a pawn to an empty square. 

## 🔧 **Building** 

## **Prerequisites** 

|**Prerequisites**||
|---|---|
|**Tool**|**Version**|
|Qt|6.x (Qt Widgets module)|
|Compiler|GCC / Clang / MSVC with C++17 support|
|qmake|Bundled with Qt|



## **Build Steps** 

```
# 1. Clone / copy the project
cd ChessGameProject
# 2. Generate Makefile
qmake ChessGameProject.pro
```

```
# 3. Compile
make          # Linux/macOS
nmake         # Windows (MSVC)
# 4. Run
./ChessGameProject
```

Alternatively, open `ChessGameProject.pro` directly in **Qt Creator** and press **Run (Ctrl+R)** . 

## 🖼️ **Asset Requirements** 

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

Images are loaded at path `:Images/assets/{color}_{type}.png` . Missing images print a 

warning to `qDebug()` but do not crash the application. 

## 🎮 **How to Play** 

1. **White moves first.** The status bar shows whose turn it is. 

2. **Click a piece** to select it — its square turns gold and legal moves appear. 

## 3. **Click a highlighted square** to move: 

   - Green dot → move to empty square 

   - Red highlight → capture enemy piece 

4. Special moves happen automatically: 

   - **Castling** — click the king two squares toward a rook 

   - **En passant** — the diagonal pawn capture square is highlighted red on the turn it is available 

   - **Promotion** — a pawn reaching rank 8 (white) or rank 1 (black) is automatically promoted to a queen 

5. When a king is in **check** , its square flashes red and the status bar announces it. 

6. **Checkmate or stalemate** triggers a game-over dialog. 

## **Technical Notes** 🔩 

- **Move simulation for legality** — `isMoveLegal` performs a full deep copy of the game state per candidate move. This is safe and correct, but O(n) copies per legal-move query. For a future AI engine, incremental make/unmake would be more efficient. 

- **No threat cache** — `isKingInCheck` always recomputes from scratch to avoid stale data in copied game states. 

- **Pawn en passant distinction** — `Pawn::movesWithEP()` is separate from `getPossibleMoves()` so attack-detection (used in castling and check checks) doesn't incorrectly treat en passant squares as attacked squares. 

- **Board offset constants** — `OX = 30` , `OY = 55` are file-scope constants shared between all drawing and hit-testing methods. 

## **License** 📄 

This project is provided for educational and personal use. See `LICENSE` if present, or contact the author for usage terms. 

