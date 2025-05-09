
#include "chess.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <QDebug>

using namespace std;

Piece* getPieceAt(Position pos, const std::array<std::array<Piece*, 8>, 8>& board) {
    if (!pos.isValid()) return nullptr;
    return board[pos.row][pos.col];
}

class Pawn : public Piece {
public:
    Pawn(PieceColor color, Position position) : Piece(PieceType::Pawn, color, position) {}
    std::vector<Position> getPossibleMoves(const std::array<std::array<Piece*, 8>, 8>& board) const override;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Pawn>(*this); }
};

class Rook : public Piece {
public:
    Rook(PieceColor color, Position position) : Piece(PieceType::Rook, color, position) {}
    std::vector<Position> getPossibleMoves(const std::array<std::array<Piece*, 8>, 8>& board) const override;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Rook>(*this); }
};

class Knight : public Piece {
public:
    Knight(PieceColor color, Position position) : Piece(PieceType::Knight, color, position) {}
    std::vector<Position> getPossibleMoves(const std::array<std::array<Piece*, 8>, 8>& board) const override;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Knight>(*this); }
};

class Bishop : public Piece {
public:
    Bishop(PieceColor color, Position position) : Piece(PieceType::Bishop, color, position) {}
    std::vector<Position> getPossibleMoves(const std::array<std::array<Piece*, 8>, 8>& board) const override;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Bishop>(*this); }
};

class Queen : public Piece {
public:
    Queen(PieceColor color, Position position) : Piece(PieceType::Queen, color, position) {}
    std::vector<Position> getPossibleMoves(const std::array<std::array<Piece*, 8>, 8>& board) const override;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Queen>(*this); }
};

class King : public Piece {
public:
    King(PieceColor color, Position position) : Piece(PieceType::King, color, position) {}
    std::vector<Position> getPossibleMoves(const std::array<std::array<Piece*, 8>, 8>& board) const override;
    std::unique_ptr<Piece> clone() const override { return std::make_unique<King>(*this); }
    bool isInCheck(const std::array<std::array<Piece*, 8>, 8>& /*board*/) const override;
};

vector<Position> Pawn::getPossibleMoves(const array<array<Piece*, 8>, 8>& board) const {
    vector<Position> moves;
    int direction = (color_ == PieceColor::White) ? 1 : -1;

    Position forward(position_.row + direction, position_.col);
    if (forward.isValid() && !getPieceAt(forward, board)) {
        moves.push_back(forward);

        if (!hasMoved_) {
            Position doubleForward(position_.row + 2 * direction, position_.col);
            if (!getPieceAt(doubleForward, board)) {
                moves.push_back(doubleForward);
            }
        }
    }

    for (int colOffset : {-1, 1}) {
        Position capturePos(position_.row + direction, position_.col + colOffset);
        if (capturePos.isValid()) {
            Piece* target = getPieceAt(capturePos, board);
            if (target && target->getColor() != color_) {
                moves.push_back(capturePos);
            }
        }
    }

    return moves;
}

vector<Position> Rook::getPossibleMoves(const array<array<Piece*, 8>, 8>& board) const {
    vector<Position> moves;

    const vector<pair<int, int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    for (const auto& dir : directions) {
        for (int step = 1; step < 8; ++step) {
            Position newPos(position_.row + step * dir.first,
                            position_.col + step * dir.second);

            if (!newPos.isValid()) break;

            Piece* piece = getPieceAt(newPos, board);
            if (!piece) {
                moves.push_back(newPos);
            } else {
                if (piece->getColor() != color_) {
                    moves.push_back(newPos);
                }
                break;
            }
        }
    }

    return moves;
}

vector<Position> Knight::getPossibleMoves(const array<array<Piece*, 8>, 8>& board) const {
    vector<Position> moves;

    const vector<pair<int, int>> offsets = {
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
    };

    for (const auto& offset : offsets) {
        Position newPos(position_.row + offset.first,
                        position_.col + offset.second);

        if (newPos.isValid()) {
            Piece* piece = getPieceAt(newPos, board);
            if (!piece || piece->getColor() != color_) {
                moves.push_back(newPos);
            }
        }
    }

    return moves;
}

vector<Position> Bishop::getPossibleMoves(const array<array<Piece*, 8>, 8>& board) const {
    vector<Position> moves;

    const vector<pair<int, int>> directions = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    for (const auto& dir : directions) {
        for (int step = 1; step < 8; ++step) {
            Position newPos(position_.row + step * dir.first,
                            position_.col + step * dir.second);

            if (!newPos.isValid()) break;

            Piece* piece = getPieceAt(newPos, board);
            if (!piece) {
                moves.push_back(newPos);
            } else {
                if (piece->getColor() != color_) {
                    moves.push_back(newPos);
                }
                break;
            }
        }
    }

    return moves;
}

vector<Position> Queen::getPossibleMoves(const array<array<Piece*, 8>, 8>& board) const {
    vector<Position> moves;

    const vector<pair<int, int>> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    for (const auto& dir : directions) {
        for (int step = 1; step < 8; ++step) {
            Position newPos(position_.row + step * dir.first,
                            position_.col + step * dir.second);

            if (!newPos.isValid()) break;

            Piece* piece = getPieceAt(newPos, board);
            if (!piece) {
                moves.push_back(newPos);
            } else {
                if (piece->getColor() != color_) {
                    moves.push_back(newPos);
                }
                break;
            }
        }
    }

    return moves;
}

vector<Position> King::getPossibleMoves(const array<array<Piece*, 8>, 8>& board) const {
    vector<Position> moves;

    for (int row = -1; row <= 1; ++row) {
        for (int col = -1; col <= 1; ++col) {
            if (row == 0 && col == 0) continue;

            Position newPos(position_.row + row, position_.col + col);
            if (newPos.isValid()) {
                Piece* piece = getPieceAt(newPos, board);
                if (!piece || piece->getColor() != color_) {
                    moves.push_back(newPos);
                }
            }
        }
    }

    if (!hasMoved_) {
        bool kingsideClear = true;
        for (int col = position_.col + 1; col < 7; ++col) {
            Position pos(position_.row, col);
            if (getPieceAt(pos, board)) {
                kingsideClear = false;
                break;
            }
        }

        if (kingsideClear) {
            Piece* rook = getPieceAt(Position(position_.row, 7), board);
            if (rook && rook->getType() == PieceType::Rook && !rook->hasMoved()) {
                moves.push_back(Position(position_.row, position_.col + 2));
            }
        }

        bool queensideClear = true;
        for (int col = position_.col - 1; col > 0; --col) {
            Position pos(position_.row, col);
            if (getPieceAt(pos, board)) {
                queensideClear = false;
                break;
            }
        }

        if (queensideClear) {
            Piece* rook = getPieceAt(Position(position_.row, 0), board);
            if (rook && rook->getType() == PieceType::Rook && !rook->hasMoved()) {
                moves.push_back(Position(position_.row, position_.col - 2));
            }
        }
    }

    return moves;
}

bool King::isInCheck(const array<array<Piece*, 8>, 8>& /*board*/) const {
    return false;
}

ChessGame::ChessGame() : currentTurn_(PieceColor::White), isCheckingLegalMove_(false), threatCacheValid_(false), threatCacheColor_(PieceColor::None) {
    whiteKingPos_ = Position(-1, -1);
    blackKingPos_ = Position(-1, -1);
    for (auto& row : board_) {
        for (auto& cell : row) {
            cell = nullptr;
        }
    }
    initializeBoard();
}

ChessGame::ChessGame(const ChessGame& other)
    : currentTurn_(other.currentTurn_),
    isCheckingLegalMove_(false),
    whiteKingPos_(other.whiteKingPos_),
    blackKingPos_(other.blackKingPos_),
    opponentThreats_(other.opponentThreats_),
    threatCacheValid_(other.threatCacheValid_),
    threatCacheColor_(other.threatCacheColor_) {
    for (const auto& piece : other.pieces_) {
        pieces_.push_back(piece->clone());
    }
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            board_[row][col] = nullptr;
            if (other.board_[row][col]) {
                for (auto& piece : pieces_) {
                    if (piece->getPosition() == Position(row, col)) {
                        board_[row][col] = piece.get();
                        break;
                    }
                }
            }
        }
    }
}

ChessGame& ChessGame::operator=(const ChessGame& other) {
    if (this != &other) {
        pieces_.clear();
        for (auto& row : board_) {
            for (auto& cell : row) {
                cell = nullptr;
            }
        }
        currentTurn_ = other.currentTurn_;
        isCheckingLegalMove_ = false;
        whiteKingPos_ = other.whiteKingPos_;
        blackKingPos_ = other.blackKingPos_;
        opponentThreats_ = other.opponentThreats_;
        threatCacheValid_ = other.threatCacheValid_;
        threatCacheColor_ = other.threatCacheColor_;
        for (const auto& piece : other.pieces_) {
            pieces_.push_back(piece->clone());
        }
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                board_[row][col] = nullptr;
                if (other.board_[row][col]) {
                    for (auto& piece : pieces_) {
                        if (piece->getPosition() == Position(row, col)) {
                            board_[row][col] = piece.get();
                            break;
                        }
                    }
                }
            }
        }
    }
    return *this;
}

void ChessGame::initializeBoard() {
    pieces_.clear();
    for (auto& row : board_) {
        for (auto& cell : row) {
            cell = nullptr;
        }
    }

    for (int col = 0; col < 8; ++col) {
        auto whitePawn = make_unique<Pawn>(PieceColor::White, Position(1, col));
        board_[1][col] = whitePawn.get();
        pieces_.push_back(std::move(whitePawn));

        auto blackPawn = make_unique<Pawn>(PieceColor::Black, Position(6, col));
        board_[6][col] = blackPawn.get();
        pieces_.push_back(std::move(blackPawn));
    }

    auto whiteRook1 = make_unique<Rook>(PieceColor::White, Position(0, 0));
    board_[0][0] = whiteRook1.get();
    pieces_.push_back(std::move(whiteRook1));
    auto whiteRook2 = make_unique<Rook>(PieceColor::White, Position(0, 7));
    board_[0][7] = whiteRook2.get();
    pieces_.push_back(std::move(whiteRook2));
    auto blackRook1 = make_unique<Rook>(PieceColor::Black, Position(7, 0));
    board_[7][0] = blackRook1.get();
    pieces_.push_back(std::move(blackRook1));
    auto blackRook2 = make_unique<Rook>(PieceColor::Black, Position(7, 7));
    board_[7][7] = blackRook2.get();
    pieces_.push_back(std::move(blackRook2));

    auto whiteKnight1 = make_unique<Knight>(PieceColor::White, Position(0, 1));
    board_[0][1] = whiteKnight1.get();
    pieces_.push_back(std::move(whiteKnight1));
    auto whiteKnight2 = make_unique<Knight>(PieceColor::White, Position(0, 6));
    board_[0][6] = whiteKnight2.get();
    pieces_.push_back(std::move(whiteKnight2));
    auto blackKnight1 = make_unique<Knight>(PieceColor::Black, Position(7, 1));
    board_[7][1] = blackKnight1.get();
    pieces_.push_back(std::move(blackKnight1));
    auto blackKnight2 = make_unique<Knight>(PieceColor::Black, Position(7, 6));
    board_[7][6] = blackKnight2.get();
    pieces_.push_back(std::move(blackKnight2));

    auto whiteBishop1 = make_unique<Bishop>(PieceColor::White, Position(0, 2));
    board_[0][2] = whiteBishop1.get();
    pieces_.push_back(std::move(whiteBishop1));
    auto whiteBishop2 = make_unique<Bishop>(PieceColor::White, Position(0, 5));
    board_[0][5] = whiteBishop2.get();
    pieces_.push_back(std::move(whiteBishop2));
    auto blackBishop1 = make_unique<Bishop>(PieceColor::Black, Position(7, 2));
    board_[7][2] = blackBishop1.get();
    pieces_.push_back(std::move(blackBishop1));
    auto blackBishop2 = make_unique<Bishop>(PieceColor::Black, Position(7, 5));
    board_[7][5] = blackBishop2.get();
    pieces_.push_back(std::move(blackBishop2));

    auto whiteQueen = make_unique<Queen>(PieceColor::White, Position(0, 3));
    board_[0][3] = whiteQueen.get();
    pieces_.push_back(std::move(whiteQueen));
    auto blackQueen = make_unique<Queen>(PieceColor::Black, Position(7, 3));
    board_[7][3] = blackQueen.get();
    pieces_.push_back(std::move(blackQueen));

    auto whiteKing = make_unique<King>(PieceColor::White, Position(0, 4));
    board_[0][4] = whiteKing.get();
    pieces_.push_back(std::move(whiteKing));
    auto blackKing = make_unique<King>(PieceColor::Black, Position(7, 4));
    board_[7][4] = blackKing.get();
    pieces_.push_back(std::move(blackKing));

    whiteKingPos_ = Position(0, 4);
    blackKingPos_ = Position(7, 4);
}

Piece* ChessGame::getPieceAt(Position pos) const {
    if (!pos.isValid()) return nullptr;
    return board_[pos.row][pos.col];
}

void ChessGame::updateThreatCache(PieceColor kingColor) const {
    opponentThreats_.clear();
    threatCacheColor_ = kingColor;
    PieceColor opponentColor = (kingColor == PieceColor::White) ? PieceColor::Black : PieceColor::White;

    for (const auto& piece : pieces_) {
        if (piece->getColor() == opponentColor) {
            auto moves = piece->getPossibleMoves(board_);
            for (const auto& move : moves) {
                opponentThreats_.push_back(move);
            }
        }
    }

    threatCacheValid_ = true;
}

bool ChessGame::isKingInCheck(PieceColor color) const {
    if (!threatCacheValid_ || threatCacheColor_ != color) {
        updateThreatCache(color);
    }
    Position kingPos = (color == PieceColor::White) ? whiteKingPos_ : blackKingPos_;
    return find(opponentThreats_.begin(), opponentThreats_.end(), kingPos) != opponentThreats_.end();
}

vector<Position> ChessGame::getValidMoves(Position pos) const {
    vector<Position> validMoves;
    Piece* piece = getPieceAt(pos);

    if (piece && piece->getColor() == currentTurn_) {
        auto possibleMoves = piece->getPossibleMoves(board_);
        for (const auto& move : possibleMoves) {
            if (isMoveLegal(pos, move)) {
                validMoves.push_back(move);
            }
        }
    }

    return validMoves;
}

bool ChessGame::movePiece(Position from, Position to) {
    Piece* piece = getPieceAt(from);

    if (!piece || piece->getColor() != currentTurn_) {
        return false;
    }

    if (!isValidMove(from, to)) {
        return false;
    }

    if (!isMoveLegal(from, to)) {
        return false;
    }

    board_[from.row][from.col] = nullptr;

    if (piece->getType() == PieceType::King) {
        if (piece->getColor() == PieceColor::White) {
            whiteKingPos_ = to;
        } else {
            blackKingPos_ = to;
        }
    }

    if (piece->getType() == PieceType::King && abs(from.col - to.col) == 2) {
        handleCastling(from, to);
    } else if (piece->getType() == PieceType::Pawn && from.col != to.col && !getPieceAt(to)) {
        handleEnPassant(from, to);
    } else {
        removeCapturedPiece(to);
        piece->setPosition(to);
        board_[to.row][to.col] = piece;
    }

    if (piece->getType() == PieceType::Pawn && (to.row == 0 || to.row == 7)) {
        handlePawnPromotion(to);
    }

    currentTurn_ = (currentTurn_ == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    threatCacheValid_ = false;

    return true;
}

bool ChessGame::isValidMove(Position from, Position to) const {
    if (from == to || !from.isValid() || !to.isValid()) {
        return false;
    }

    Piece* piece = getPieceAt(from);
    if (!piece) {
        return false;
    }

    auto possibleMoves = piece->getPossibleMoves(board_);
    return find(possibleMoves.begin(), possibleMoves.end(), to) != possibleMoves.end();
}

void ChessGame::removeCapturedPiece(Position pos) {
    if (getPieceAt(pos)) {
        pieces_.erase(remove_if(pieces_.begin(), pieces_.end(),
                                [pos](const unique_ptr<Piece>& piece) {
                                    return piece->getPosition() == pos;
                                }), pieces_.end());
        board_[pos.row][pos.col] = nullptr;
    }
}

bool ChessGame::isMoveLegal(Position from, Position to) const {
    if (isCheckingLegalMove_) {
        return true;
    }

    isCheckingLegalMove_ = true;

    ChessGame tempGame(*this);
    Piece* piece = tempGame.getPieceAt(from);
    if (!piece) {
        isCheckingLegalMove_ = false;
        return false;
    }

    tempGame.board_[from.row][from.col] = nullptr;

    if (piece->getType() == PieceType::King) {
        if (piece->getColor() == PieceColor::White) {
            tempGame.whiteKingPos_ = to;
        } else {
            tempGame.blackKingPos_ = to;
        }
    }

    tempGame.removeCapturedPiece(to);
    piece->setPosition(to);
    tempGame.board_[to.row][to.col] = piece;

    bool inCheck = tempGame.isKingInCheck(currentTurn_);

    isCheckingLegalMove_ = false;
    return !inCheck;
}

void ChessGame::handleCastling(Position from, Position to) {
    int row = from.row;
    int kingDirection = (to.col > from.col) ? 1 : -1;

    Piece* king = getPieceAt(from);
    king->setPosition(to);
    board_[to.row][to.col] = king;

    int rookCol = (kingDirection == 1) ? 7 : 0;
    int newRookCol = (kingDirection == 1) ? to.col - 1 : to.col + 1;

    Piece* rook = getPieceAt(Position(row, rookCol));
    rook->setPosition(Position(row, newRookCol));
    board_[row][rookCol] = nullptr;
    board_[row][newRookCol] = rook;
}

void ChessGame::handleEnPassant(Position from, Position to) {
    Position capturedPos(from.row, to.col);
    removeCapturedPiece(capturedPos);

    Piece* pawn = getPieceAt(from);
    pawn->setPosition(to);
    board_[to.row][to.col] = pawn;
}

void ChessGame::handlePawnPromotion(Position pos) {
    Piece* pawn = getPieceAt(pos);
    if (pawn && pawn->getType() == PieceType::Pawn) {
        PieceColor color = pawn->getColor();
        removeCapturedPiece(pos);
        auto newQueen = make_unique<Queen>(color, pos);
        board_[pos.row][pos.col] = newQueen.get();
        pieces_.push_back(std::move(newQueen));
    }
}

bool ChessGame::isCheckmate(PieceColor color) const {
    if (!isKingInCheck(color)) {
        return false;
    }

    return !hasLegalMoves(color);
}

bool ChessGame::isStalemate(PieceColor color) const {
    if (isKingInCheck(color)) {
        return false;
    }

    return !hasLegalMoves(color);
}

bool ChessGame::hasLegalMoves(PieceColor color) const {
    for (const auto& piece : pieces_) {
        if (piece->getColor() == color) {
            auto moves = piece->getPossibleMoves(board_);
            for (const auto& move : moves) {
                if (isMoveLegal(piece->getPosition(), move)) {
                    return true;
                }
            }
        }
    }
    return false;
}
