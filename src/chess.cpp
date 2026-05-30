#include "chess.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <QDebug>

using namespace std;

// ── local helper (piece move generators use this) ─────────────────────────────
static Piece* at(Position p, const array<array<Piece*, 8>, 8>& b) {
    return p.isValid() ? b[p.row][p.col] : nullptr;
}

// ── Piece subclasses ──────────────────────────────────────────────────────────
class Pawn : public Piece {
public:
    Pawn(PieceColor c, Position p) : Piece(PieceType::Pawn, c, p) {}
    unique_ptr<Piece> clone() const override { return make_unique<Pawn>(*this); }

    // Normal signature — no en passant (used for attack detection)
    vector<Position> getPossibleMoves(const array<array<Piece*, 8>, 8>& board) const override {
        return movesWithEP(board, Position(-1, -1));
    }
    // Full signature including en passant target
    vector<Position> movesWithEP(const array<array<Piece*, 8>, 8>& board,
                                  Position epTarget) const {
        vector<Position> m;
        int dir = (color_ == PieceColor::White) ? 1 : -1;

        // Forward
        Position fwd(position_.row + dir, position_.col);
        if (fwd.isValid() && !at(fwd, board)) {
            m.push_back(fwd);
            if (!hasMoved_) {
                Position dbl(position_.row + 2*dir, position_.col);
                if (!at(dbl, board)) m.push_back(dbl);
            }
        }
        // Diagonal captures + en passant
        for (int dc : {-1, 1}) {
            Position cap(position_.row + dir, position_.col + dc);
            if (!cap.isValid()) continue;
            Piece* tgt = at(cap, board);
            if (tgt && tgt->getColor() != color_)
                m.push_back(cap);                 // normal capture
            else if (cap == epTarget)
                m.push_back(cap);                 // en passant
        }
        return m;
    }
};

class Rook : public Piece {
public:
    Rook(PieceColor c, Position p) : Piece(PieceType::Rook, c, p) {}
    unique_ptr<Piece> clone() const override { return make_unique<Rook>(*this); }
    vector<Position> getPossibleMoves(const array<array<Piece*, 8>, 8>& board) const override {
        vector<Position> m;
        for (auto [dr, dc] : vector<pair<int,int>>{{1,0},{-1,0},{0,1},{0,-1}}) {
            for (int s = 1; s < 8; ++s) {
                Position np(position_.row+s*dr, position_.col+s*dc);
                if (!np.isValid()) break;
                Piece* p = at(np, board);
                if (!p) { m.push_back(np); }
                else { if (p->getColor() != color_) m.push_back(np); break; }
            }
        }
        return m;
    }
};

class Knight : public Piece {
public:
    Knight(PieceColor c, Position p) : Piece(PieceType::Knight, c, p) {}
    unique_ptr<Piece> clone() const override { return make_unique<Knight>(*this); }
    vector<Position> getPossibleMoves(const array<array<Piece*, 8>, 8>& board) const override {
        vector<Position> m;
        for (auto [dr,dc] : vector<pair<int,int>>{{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}}) {
            Position np(position_.row+dr, position_.col+dc);
            if (np.isValid()) {
                Piece* p = at(np, board);
                if (!p || p->getColor() != color_) m.push_back(np);
            }
        }
        return m;
    }
};

class Bishop : public Piece {
public:
    Bishop(PieceColor c, Position p) : Piece(PieceType::Bishop, c, p) {}
    unique_ptr<Piece> clone() const override { return make_unique<Bishop>(*this); }
    vector<Position> getPossibleMoves(const array<array<Piece*, 8>, 8>& board) const override {
        vector<Position> m;
        for (auto [dr,dc] : vector<pair<int,int>>{{1,1},{1,-1},{-1,1},{-1,-1}}) {
            for (int s = 1; s < 8; ++s) {
                Position np(position_.row+s*dr, position_.col+s*dc);
                if (!np.isValid()) break;
                Piece* p = at(np, board);
                if (!p) { m.push_back(np); }
                else { if (p->getColor() != color_) m.push_back(np); break; }
            }
        }
        return m;
    }
};

class Queen : public Piece {
public:
    Queen(PieceColor c, Position p) : Piece(PieceType::Queen, c, p) {}
    unique_ptr<Piece> clone() const override { return make_unique<Queen>(*this); }
    vector<Position> getPossibleMoves(const array<array<Piece*, 8>, 8>& board) const override {
        vector<Position> m;
        for (auto [dr,dc] : vector<pair<int,int>>{{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}}) {
            for (int s = 1; s < 8; ++s) {
                Position np(position_.row+s*dr, position_.col+s*dc);
                if (!np.isValid()) break;
                Piece* p = at(np, board);
                if (!p) { m.push_back(np); }
                else { if (p->getColor() != color_) m.push_back(np); break; }
            }
        }
        return m;
    }
};

class King : public Piece {
public:
    King(PieceColor c, Position p) : Piece(PieceType::King, c, p) {}
    unique_ptr<Piece> clone() const override { return make_unique<King>(*this); }
    // NOTE: castling squares intentionally NOT included here.
    // They require attack-detection (ChessGame context) and are added by getValidMoves.
    vector<Position> getPossibleMoves(const array<array<Piece*, 8>, 8>& board) const override {
        vector<Position> m;
        for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc) {
                if (!dr && !dc) continue;
                Position np(position_.row+dr, position_.col+dc);
                if (np.isValid()) {
                    Piece* p = at(np, board);
                    if (!p || p->getColor() != color_) m.push_back(np);
                }
            }
        return m;
    }
};

// ── ChessGame implementation ──────────────────────────────────────────────────

ChessGame::ChessGame()
    : currentTurn_(PieceColor::White),
      whiteKingPos_(-1,-1), blackKingPos_(-1,-1),
      enPassantTarget_(-1,-1) {
    for (auto& row : board_) row.fill(nullptr);
    initializeBoard();
}

// FIX: copy constructor always sets threatCacheValid_=false so the copy
//      recomputes on its first isKingInCheck call rather than using stale data.
ChessGame::ChessGame(const ChessGame& other)
    : currentTurn_(other.currentTurn_),
      whiteKingPos_(other.whiteKingPos_),
      blackKingPos_(other.blackKingPos_),
      enPassantTarget_(other.enPassantTarget_) {
    for (auto& row : board_) row.fill(nullptr);

    for (const auto& p : other.pieces_)
        pieces_.push_back(p->clone());

    // Rebuild board_ by matching cloned pieces to original board positions.
    // Match on position+type+color to avoid ambiguity between promoted pieces.
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Piece* orig = other.board_[r][c];
            if (!orig) continue;
            for (auto& cloned : pieces_) {
                if (cloned->getPosition() == Position(r, c) &&
                    cloned->getType()     == orig->getType() &&
                    cloned->getColor()    == orig->getColor()) {
                    board_[r][c] = cloned.get();
                    break;
                }
            }
        }
    }
}

ChessGame& ChessGame::operator=(const ChessGame& other) {
    if (this == &other) return *this;
    pieces_.clear();
    for (auto& row : board_) row.fill(nullptr);
    currentTurn_     = other.currentTurn_;
    whiteKingPos_    = other.whiteKingPos_;
    blackKingPos_    = other.blackKingPos_;
    enPassantTarget_ = other.enPassantTarget_;
    for (const auto& p : other.pieces_) pieces_.push_back(p->clone());
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Piece* orig = other.board_[r][c];
            if (!orig) continue;
            for (auto& cloned : pieces_) {
                if (cloned->getPosition() == Position(r, c) &&
                    cloned->getType()     == orig->getType() &&
                    cloned->getColor()    == orig->getColor()) {
                    board_[r][c] = cloned.get();
                    break;
                }
            }
        }
    }
    return *this;
}

void ChessGame::initializeBoard() {
    pieces_.clear();
    for (auto& row : board_) row.fill(nullptr);
    enPassantTarget_ = Position(-1, -1);

    auto place = [&](unique_ptr<Piece> p) {
        Position pos = p->getPosition();
        board_[pos.row][pos.col] = p.get();
        pieces_.push_back(move(p));
    };
    for (int c = 0; c < 8; ++c) {
        place(make_unique<Pawn>(PieceColor::White, Position(1, c)));
        place(make_unique<Pawn>(PieceColor::Black, Position(6, c)));
    }
    place(make_unique<Rook>  (PieceColor::White, Position(0,0)));
    place(make_unique<Rook>  (PieceColor::White, Position(0,7)));
    place(make_unique<Rook>  (PieceColor::Black, Position(7,0)));
    place(make_unique<Rook>  (PieceColor::Black, Position(7,7)));
    place(make_unique<Knight>(PieceColor::White, Position(0,1)));
    place(make_unique<Knight>(PieceColor::White, Position(0,6)));
    place(make_unique<Knight>(PieceColor::Black, Position(7,1)));
    place(make_unique<Knight>(PieceColor::Black, Position(7,6)));
    place(make_unique<Bishop>(PieceColor::White, Position(0,2)));
    place(make_unique<Bishop>(PieceColor::White, Position(0,5)));
    place(make_unique<Bishop>(PieceColor::Black, Position(7,2)));
    place(make_unique<Bishop>(PieceColor::Black, Position(7,5)));
    place(make_unique<Queen> (PieceColor::White, Position(0,3)));
    place(make_unique<Queen> (PieceColor::Black, Position(7,3)));
    place(make_unique<King>  (PieceColor::White, Position(0,4)));
    place(make_unique<King>  (PieceColor::Black, Position(7,4)));
    whiteKingPos_ = Position(0,4);
    blackKingPos_ = Position(7,4);
}

Piece* ChessGame::getPieceAt(Position pos) const {
    return pos.isValid() ? board_[pos.row][pos.col] : nullptr;
}

// Returns all raw moves for a piece, including en passant for pawns.
vector<Position> ChessGame::getRawMoves(const Piece* piece) const {
    if (piece->getType() == PieceType::Pawn)
        return static_cast<const Pawn*>(piece)->movesWithEP(board_, enPassantTarget_);
    return piece->getPossibleMoves(board_);
}

// FIX: check if a square is attacked by any piece of the given color.
// Used for castling legality (king cannot pass through or land on an attacked square).
bool ChessGame::isSquareAttackedBy(Position sq, PieceColor attacker) const {
    for (const auto& p : pieces_) {
        if (p->getColor() != attacker) continue;
        // Use base getPossibleMoves (no ep needed for attack detection)
        auto moves = p->getPossibleMoves(board_);
        if (find(moves.begin(), moves.end(), sq) != moves.end())
            return true;
    }
    return false;
}

// FIX: isKingInCheck always recomputes from scratch (no cache).
//      The old cache was copied into temp games as "valid" and caused wrong results.
bool ChessGame::isKingInCheck(PieceColor color) const {
    Position kingPos = (color == PieceColor::White) ? whiteKingPos_ : blackKingPos_;
    if (!kingPos.isValid()) return false;
    PieceColor opp = (color == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    return isSquareAttackedBy(kingPos, opp);
}

// Returns all fully legal moves for the piece at pos.
vector<Position> ChessGame::getValidMoves(Position pos) const {
    vector<Position> result;
    Piece* piece = getPieceAt(pos);
    if (!piece || piece->getColor() != currentTurn_) return result;

    // Raw moves (includes en passant for pawns)
    for (const auto& to : getRawMoves(piece)) {
        if (isMoveLegal(pos, to))
            result.push_back(to);
    }

    // FIX: Castling — only added here where we can call isSquareAttackedBy safely.
    if (piece->getType() == PieceType::King && !piece->hasMoved() && !isKingInCheck(piece->getColor())) {
        int row = pos.row;
        PieceColor opp = (piece->getColor() == PieceColor::White) ? PieceColor::Black : PieceColor::White;

        // Kingside: squares between king and rook at col 7 must be empty,
        //           king cannot pass through (col+1) or land on (col+2) attacked square.
        {
            bool clear = true;
            for (int c = pos.col+1; c < 7; ++c)
                if (board_[row][c]) { clear = false; break; }
            if (clear) {
                Piece* rook = getPieceAt(Position(row, 7));
                if (rook && rook->getType() == PieceType::Rook &&
                    rook->getColor() == piece->getColor() && !rook->hasMoved() &&
                    !isSquareAttackedBy(Position(row, pos.col+1), opp) &&
                    !isSquareAttackedBy(Position(row, pos.col+2), opp)) {
                    result.push_back(Position(row, pos.col+2));
                }
            }
        }
        // Queenside: squares between king and rook at col 0 must be empty,
        //            king cannot pass through (col-1) or land on (col-2) attacked square.
        {
            bool clear = true;
            for (int c = pos.col-1; c > 0; --c)
                if (board_[row][c]) { clear = false; break; }
            if (clear) {
                Piece* rook = getPieceAt(Position(row, 0));
                if (rook && rook->getType() == PieceType::Rook &&
                    rook->getColor() == piece->getColor() && !rook->hasMoved() &&
                    !isSquareAttackedBy(Position(row, pos.col-1), opp) &&
                    !isSquareAttackedBy(Position(row, pos.col-2), opp)) {
                    result.push_back(Position(row, pos.col-2));
                }
            }
        }
    }

    return result;
}

// FIX: isMoveLegal — simulate the move on a fresh copy.
//      The copy always has a clean state (no stale cache).
//      En passant simulation correctly removes the captured pawn.
bool ChessGame::isMoveLegal(Position from, Position to) const {
    ChessGame tmp(*this);  // copy has no cache (threatCacheValid_ never stored)

    Piece* piece = tmp.getPieceAt(from);
    if (!piece) return false;

    tmp.board_[from.row][from.col] = nullptr;

    // Update king tracking in simulation
    if (piece->getType() == PieceType::King) {
        if (piece->getColor() == PieceColor::White) tmp.whiteKingPos_ = to;
        else                                         tmp.blackKingPos_ = to;
    }

    // FIX: handle en passant in the simulation so the captured pawn is removed
    bool isEP = (piece->getType() == PieceType::Pawn &&
                 from.col != to.col && !tmp.getPieceAt(to));
    if (isEP) {
        tmp.removePieceAt(Position(from.row, to.col));   // remove captured pawn
    } else {
        tmp.removePieceAt(to);                           // normal capture
    }

    piece->setPosition(to);
    tmp.board_[to.row][to.col] = piece;

    return !tmp.isKingInCheck(currentTurn_);
}

bool ChessGame::movePiece(Position from, Position to) {
    Piece* piece = getPieceAt(from);
    if (!piece || piece->getColor() != currentTurn_) return false;

    // Use getValidMoves which covers castling + en passant
    auto valid = getValidMoves(from);
    if (find(valid.begin(), valid.end(), to) == valid.end()) return false;

    // FIX: Save piece pointer BEFORE clearing the board square.
    //      handleCastling and handleEnPassant used to call getPieceAt(from)
    //      after board_[from] was already set to nullptr → null deref crash.
    Piece* movingPiece = piece;
    board_[from.row][from.col] = nullptr;

    // Track king position
    if (movingPiece->getType() == PieceType::King) {
        if (movingPiece->getColor() == PieceColor::White) whiteKingPos_ = to;
        else                                               blackKingPos_ = to;
    }

    bool isCastling = (movingPiece->getType() == PieceType::King &&
                       abs(from.col - to.col) == 2);
    bool isEP       = (movingPiece->getType() == PieceType::Pawn &&
                       from.col != to.col && !getPieceAt(to));

    if (isCastling) {
        handleCastling(from, to, movingPiece);   // FIX: pass saved ptr
    } else if (isEP) {
        handleEnPassant(from, to, movingPiece);  // FIX: pass saved ptr
    } else {
        removePieceAt(to);
        movingPiece->setPosition(to);
        board_[to.row][to.col] = movingPiece;
    }

    // FIX: Update en passant target for next move.
    // Only a double pawn push creates a valid en passant target.
    if (movingPiece->getType() == PieceType::Pawn && abs(from.row - to.row) == 2)
        enPassantTarget_ = Position((from.row + to.row) / 2, from.col);
    else
        enPassantTarget_ = Position(-1, -1);

    // Auto-promote pawn to queen
    if (movingPiece->getType() == PieceType::Pawn && (to.row == 0 || to.row == 7))
        handlePawnPromotion(to);

    currentTurn_ = (currentTurn_ == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    return true;
}

// Removes the piece at pos from both pieces_ vector and board_.
void ChessGame::removePieceAt(Position pos) {
    if (!pos.isValid() || !board_[pos.row][pos.col]) return;
    pieces_.erase(
        remove_if(pieces_.begin(), pieces_.end(),
            [&](const unique_ptr<Piece>& p) { return p->getPosition() == pos; }),
        pieces_.end());
    board_[pos.row][pos.col] = nullptr;
}

// FIX: king pointer is now passed in (was fetched from board after it was cleared → crash).
void ChessGame::handleCastling(Position from, Position to, Piece* king) {
    int row = from.row;
    int dir = (to.col > from.col) ? 1 : -1;

    king->setPosition(to);
    board_[to.row][to.col] = king;

    int rookFromCol = (dir == 1) ? 7 : 0;
    int rookToCol   = (dir == 1) ? to.col - 1 : to.col + 1;

    Piece* rook = getPieceAt(Position(row, rookFromCol));
    if (rook) {
        rook->setPosition(Position(row, rookToCol));
        board_[row][rookFromCol] = nullptr;
        board_[row][rookToCol]   = rook;
    }
}

// FIX: pawn pointer is now passed in (was fetched from board after it was cleared → crash).
void ChessGame::handleEnPassant(Position from, Position to, Piece* pawn) {
    // Remove the captured pawn (same row as 'from', same col as 'to')
    removePieceAt(Position(from.row, to.col));
    pawn->setPosition(to);
    board_[to.row][to.col] = pawn;
}

void ChessGame::handlePawnPromotion(Position pos) {
    Piece* pawn = getPieceAt(pos);
    if (!pawn || pawn->getType() != PieceType::Pawn) return;
    PieceColor color = pawn->getColor();
    removePieceAt(pos);
    auto queen = make_unique<Queen>(color, pos);
    board_[pos.row][pos.col] = queen.get();
    pieces_.push_back(move(queen));
}

bool ChessGame::isCheckmate(PieceColor color) const {
    return isKingInCheck(color) && !hasLegalMoves(color);
}

bool ChessGame::isStalemate(PieceColor color) const {
    return !isKingInCheck(color) && !hasLegalMoves(color);
}

bool ChessGame::hasLegalMoves(PieceColor color) const {
    for (const auto& piece : pieces_) {
        if (piece->getColor() != color) continue;
        for (const auto& to : getRawMoves(piece.get())) {
            if (isMoveLegal(piece->getPosition(), to))
                return true;
        }
        // Also check castling for king (rare but necessary for correctness)
        if (piece->getType() == PieceType::King) {
            auto kingMoves = getValidMoves(piece->getPosition());
            if (!kingMoves.empty()) return true;
        }
    }
    return false;
}