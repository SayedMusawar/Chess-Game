#include "mainwindow.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QStatusBar>
#include <QTimer>
#include <QFont>
#include <QDebug>

// ── board palette (classic wooden look) ──────────────────────────────────────
static const QColor kLightSquare (240, 217, 181);   // cream
static const QColor kDarkSquare  (181, 136,  99);   // walnut
static const QColor kBorder      ( 99,  66,  32);   // dark wood border
static const QColor kSelectedSq  (255, 215,   0, 180);   // gold – selected piece
static const QColor kMoveSq      ( 80, 200,  80, 140);   // green – empty valid move
static const QColor kCaptureSq   (210,  40,  40, 200);   // RED  – enemy piece (will be captured)
static const QColor kCheckSq     (255,  50,  50, 220);   // bright red – king in check

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      selectedPos(-1, -1),
      updateTimer(new QTimer(this)) {

    // Board area: 8×70 = 560px + 20px border each side = 600px wide
    // Height: 560 + 50 top (status) + 50 bottom (coords+bar) = 660
    setFixedSize(620, 680);

    setStyleSheet(R"(
        QMainWindow { background: #6b4423; }
        QStatusBar  {
            background: #3d2008;
            color: #f5deb3;
            font-size: 14px;
            font-weight: bold;
            padding: 4px 12px;
        }
        QMessageBox { background-color: #f0ece4; color: #2a1a0a; }
        QMessageBox QPushButton {
            background-color: #8b5e3c;
            color: white;
            padding: 6px 14px;
            border: none;
            border-radius: 4px;
            font-weight: bold;
        }
        QMessageBox QPushButton:hover { background-color: #a0714f; }
    )");

    updateTimer->setSingleShot(true);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateGameStatus);
    updateGameStatus();
}

MainWindow::~MainWindow() {}

// ── painting ──────────────────────────────────────────────────────────────────

void MainWindow::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    drawBoard(painter);
    drawCoordinates(painter);
    drawHighlights(painter);
    drawPieces(painter);
}

// Board offset: leave 30px left for rank numbers, 55px top for title bar area
static const int OX = 30;   // x offset
static const int OY = 55;   // y offset

void MainWindow::drawBoard(QPainter& painter) {
    // Outer wooden border
    painter.setPen(Qt::NoPen);
    painter.setBrush(kBorder);
    painter.drawRoundedRect(OX - 8, OY - 8,
                            8*70 + 16, 8*70 + 16, 6, 6);

    // Squares
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c) {
            painter.setBrush((r + c) % 2 == 0 ? kLightSquare : kDarkSquare);
            painter.drawRect(OX + c*70, OY + (7-r)*70, 70, 70);
        }
}

void MainWindow::drawCoordinates(QPainter& painter) {
    QFont font("Arial", 9, QFont::Bold);
    painter.setFont(font);

    for (int i = 0; i < 8; ++i) {
        // Rank numbers (1-8) on the left
        bool lightRank = (i % 2 == 0);
        painter.setPen(lightRank ? kDarkSquare : kLightSquare);
        painter.drawText(OX - 18,
                         OY + (7-i)*70 + 38,
                         QString::number(i + 1));

        // File letters (a-h) on the bottom
        bool lightFile = (i % 2 == 0);
        painter.setPen(lightFile ? kLightSquare : kDarkSquare);
        painter.drawText(OX + i*70 + 28,
                         OY + 8*70 + 16,
                         QString(QChar('a' + i)));
    }
}

void MainWindow::drawHighlights(QPainter& painter) {
    painter.setPen(Qt::NoPen);

    // Selected square — gold
    if (selectedPos.isValid()) {
        painter.setBrush(kSelectedSq);
        painter.drawRect(OX + selectedPos.col*70,
                         OY + (7 - selectedPos.row)*70,
                         70, 70);
    }

    // Valid moves: green for empty squares, RED for captures
    for (const auto& m : validMoves) {
        bool isCapture = std::find(captureMoves.begin(),
                                   captureMoves.end(), m) != captureMoves.end();
        painter.setBrush(isCapture ? kCaptureSq : kMoveSq);
        painter.drawRect(OX + m.col*70, OY + (7 - m.row)*70, 70, 70);
    }

    // King-in-check: flash the king's square bright red
    PieceColor toMove = game.getCurrentTurn();
    if (game.isKingInCheck(toMove)) {
        // Find the king square
        for (int r = 0; r < 8; ++r)
            for (int c = 0; c < 8; ++c) {
                Piece* p = game.getPieceAt(Position(r, c));
                if (p && p->getType() == PieceType::King
                      && p->getColor() == toMove) {
                    painter.setBrush(kCheckSq);
                    painter.drawRect(OX + c*70, OY + (7-r)*70, 70, 70);
                }
            }
    }

    // Small dots on empty valid-move squares (chess.com style)
    painter.setBrush(QColor(0, 0, 0, 50));
    for (const auto& m : validMoves) {
        bool isCapture = std::find(captureMoves.begin(),
                                   captureMoves.end(), m) != captureMoves.end();
        if (!isCapture) {
            // Draw a small centered dot
            int cx = OX + m.col*70 + 35;
            int cy = OY + (7 - m.row)*70 + 35;
            painter.drawEllipse(cx - 10, cy - 10, 20, 20);
        }
    }
}

void MainWindow::drawPieces(QPainter& painter) {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Piece* piece = game.getPieceAt(Position(r, c));
            if (!piece) continue;

            QString color = (piece->getColor() == PieceColor::White) ? "white" : "black";
            QString type;
            switch (piece->getType()) {
            case PieceType::Pawn:   type = "pawn";   break;
            case PieceType::Rook:   type = "rook";   break;
            case PieceType::Knight: type = "knight"; break;
            case PieceType::Bishop: type = "bishop"; break;
            case PieceType::Queen:  type = "queen";  break;
            case PieceType::King:   type = "king";   break;
            default: continue;
            }

            QString path = QString(":Images/assets/%1_%2.png").arg(color).arg(type);
            QPixmap px(path);
            if (px.isNull()) { qDebug() << "Missing image:" << path; continue; }

            // Draw with a tiny 3px padding so pieces don't touch square edges
            painter.drawPixmap(OX + c*70 + 3,
                               OY + (7-r)*70 + 3,
                               64, 64, px);
        }
    }
}

// ── input ─────────────────────────────────────────────────────────────────────

void MainWindow::mousePressEvent(QMouseEvent* event) {
    int col = ((int)event->position().x() - OX) / 70;
    int row = 7 - ((int)event->position().y() - OY) / 70;
    Position clicked(row, col);
    if (!clicked.isValid()) return;

    if (std::find(validMoves.begin(), validMoves.end(), clicked) != validMoves.end()) {
        // Execute move
        game.movePiece(selectedPos, clicked);
        selectedPos = Position(-1, -1);
        validMoves.clear();
        captureMoves.clear();
        updateTimer->start(0);
    } else {
        Piece* piece = game.getPieceAt(clicked);
        if (piece && piece->getColor() == game.getCurrentTurn()) {
            selectedPos  = clicked;
            validMoves   = game.getValidMoves(clicked);

            // ── NEW: populate captureMoves ──────────────────────────────────
            // A move is a capture if the destination has an enemy piece,
            // OR it matches the en passant target (pawn diagonal to empty square).
            captureMoves.clear();
            for (const auto& m : validMoves) {
                Piece* target = game.getPieceAt(m);
                if (target && target->getColor() != piece->getColor()) {
                    captureMoves.push_back(m);   // normal capture → RED
                } else if (piece->getType() == PieceType::Pawn &&
                           m.col != clicked.col &&
                           !target) {
                    captureMoves.push_back(m);   // en passant → also RED
                }
            }
            // ───────────────────────────────────────────────────────────────
        } else {
            selectedPos = Position(-1, -1);
            validMoves.clear();
            captureMoves.clear();
        }
    }
    update();
}

// ── game status ───────────────────────────────────────────────────────────────

void MainWindow::updateGameStatus() {
    PieceColor toMove = game.getCurrentTurn();
    QString name = (toMove == PieceColor::White) ? "White" : "Black";
    QString icon = (toMove == PieceColor::White) ? "♔" : "♚";

    if (game.isCheckmate(toMove)) {
        QString winner = (toMove == PieceColor::White) ? "Black ♛" : "White ♕";
        QMessageBox::information(this, "Game Over",
            QString("Checkmate!  %1 wins!").arg(winner));
        return;
    }
    if (game.isStalemate(toMove)) {
        QMessageBox::information(this, "Game Over",
            "Stalemate — the game is a draw!");
        return;
    }
    if (game.isKingInCheck(toMove))
        statusBar()->showMessage(QString("  %1  %2 is in CHECK!").arg(icon).arg(name));
    else
        statusBar()->showMessage(QString("  %1  %2's turn").arg(icon).arg(name));

    update();   // repaint so king-check highlight appears immediately
}