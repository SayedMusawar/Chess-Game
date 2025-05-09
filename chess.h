#ifndef CHESS_H
#define CHESS_H

#include <vector>
#include <memory>
#include <array>
#include <QDebug>

enum class PieceType { None, Pawn, Rook, Knight, Bishop, Queen, King };
enum class PieceColor { White, Black, None };

struct Position {
    int row;
    int col;
    Position(int r = -1, int c = -1) : row(r), col(c) {}
    bool operator==(const Position& other) const { return row == other.row && col == other.col; }
    bool isValid() const { return row >= 0 && row < 8 && col >= 0 && col < 8; }
};

class Piece {
public:
    Piece(PieceType type, PieceColor color, Position position)
        : type_(type), color_(color), position_(position), hasMoved_(false) {}

    virtual ~Piece() = default;

    PieceType getType() const { return type_; }
    PieceColor getColor() const { return color_; }
    Position getPosition() const { return position_; }
    bool hasMoved() const { return hasMoved_; }

    void setPosition(Position pos) {
        position_ = pos;
        hasMoved_ = true;
    }

    virtual std::vector<Position> getPossibleMoves(const std::array<std::array<Piece*, 8>, 8>& board) const = 0;
    virtual std::unique_ptr<Piece> clone() const = 0;
    virtual bool isInCheck(const std::array<std::array<Piece*, 8>, 8>& /*board*/) const { return false; }

protected:
    PieceType type_;
    PieceColor color_;
    Position position_;
    bool hasMoved_;
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
    bool isKingInCheck(PieceColor color) const; // New public method
    PieceColor getCurrentTurn() const { return currentTurn_; }
    const std::array<std::array<Piece*, 8>, 8>& getBoard() const { return board_; }
    Position whiteKingPos() const { return whiteKingPos_; }
    Position blackKingPos() const { return blackKingPos_; }
    Piece* getPieceAt(Position pos) const;
    std::vector<Position> getValidMoves(Position pos) const;

private:
    std::vector<std::unique_ptr<Piece>> pieces_;
    std::array<std::array<Piece*, 8>, 8> board_;
    PieceColor currentTurn_;
    mutable bool isCheckingLegalMove_;
    Position whiteKingPos_;
    Position blackKingPos_;
    mutable std::vector<Position> opponentThreats_;
    mutable bool threatCacheValid_;
    mutable PieceColor threatCacheColor_;

    void updateThreatCache(PieceColor kingColor) const;
    bool isValidMove(Position from, Position to) const;
    void removeCapturedPiece(Position pos);
    bool isMoveLegal(Position from, Position to) const;
    bool hasLegalMoves(PieceColor color) const;
    void handleCastling(Position from, Position to);
    void handleEnPassant(Position from, Position to);
    void handlePawnPromotion(Position pos);
};

#endif // CHESS_H
