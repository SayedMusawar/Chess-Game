#ifndef CHESS_H
#define CHESS_H

#include <vector>
#include <memory>
#include <array>
#include <QDebug>

enum class PieceType { None, Pawn, Rook, Knight, Bishop, Queen, King };
enum class PieceColor { White, Black, None };

struct Position {
    int row, col;
    Position(int r = -1, int c = -1) : row(r), col(c) {}
    bool operator==(const Position& o) const { return row == o.row && col == o.col; }
    bool isValid() const { return row >= 0 && row < 8 && col >= 0 && col < 8; }
};

class Piece {
public:
    Piece(PieceType type, PieceColor color, Position position)
        : type_(type), color_(color), position_(position), hasMoved_(false) {}
    virtual ~Piece() = default;

    PieceType  getType()     const { return type_; }
    PieceColor getColor()    const { return color_; }
    Position   getPosition() const { return position_; }
    bool       hasMoved()    const { return hasMoved_; }

    void setPosition(Position pos) { position_ = pos; hasMoved_ = true; }

    virtual std::vector<Position> getPossibleMoves(
        const std::array<std::array<Piece*, 8>, 8>& board) const = 0;
    virtual std::unique_ptr<Piece> clone() const = 0;

protected:
    PieceType  type_;
    PieceColor color_;
    Position   position_;
    bool       hasMoved_;
};

class ChessGame {
public:
    ChessGame();
    ChessGame(const ChessGame& other);
    ChessGame& operator=(const ChessGame& other);

    void initializeBoard();
    bool movePiece(Position from, Position to);
    bool isCheckmate(PieceColor color) const;
    bool isStalemate(PieceColor color) const;
    bool isKingInCheck(PieceColor color) const;

    PieceColor getCurrentTurn() const { return currentTurn_; }
    Piece*     getPieceAt(Position pos) const;
    std::vector<Position> getValidMoves(Position pos) const;

private:
    std::vector<std::unique_ptr<Piece>>   pieces_;
    std::array<std::array<Piece*, 8>, 8>  board_;
    PieceColor currentTurn_;
    Position   whiteKingPos_;
    Position   blackKingPos_;
    Position   enPassantTarget_;   // square a pawn can capture INTO via en passant

    // --- helpers ---
    bool isSquareAttackedBy(Position sq, PieceColor attacker) const;
    bool isMoveLegal(Position from, Position to) const;
    bool hasLegalMoves(PieceColor color) const;
    void removePieceAt(Position pos);
    void handleCastling(Position from, Position to, Piece* king);
    void handleEnPassant(Position from, Position to, Piece* pawn);
    void handlePawnPromotion(Position pos);
    std::vector<Position> getRawMoves(const Piece* piece) const;
};

#endif // CHESS_H