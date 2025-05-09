#include "mainwindow.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QStatusBar>
#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    selectedPos(Position(-1, -1)),
    updateTimer(new QTimer(this)),
    inCheck(false) {
    setFixedSize(600, 650);
    statusBar()->showMessage("White's turn");

    // Define the stylesheet directly
    QString styleSheet = R"(
        QMessageBox {
            background-color: #f0f0f0;
            color: #333;
        }
        QMessageBox QPushButton {
            background-color: #4CAF50;
            color: white;
            padding: 5px 10px;
            border: none;
            border-radius: 3px;
        }
        QMessageBox QPushButton:hover {
            background-color: #45a049;
        }
    )";
    setStyleSheet(styleSheet);

    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateGameStatusPart1);
    updateTimer->setSingleShot(true);
    updateTimer->start(0);
}

MainWindow::~MainWindow() {}

void MainWindow::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
    drawBoard(painter);
    drawHighlights(painter);
    drawPieces(painter);
}

void MainWindow::drawBoard(QPainter &painter) {
    const int offsetX = 20;
    const int offsetY = 50;

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            painter.setBrush((row + col) % 2 == 0 ? Qt::lightGray : Qt::darkGray);
            painter.drawRect(offsetX + col * squareSize, offsetY + (7 - row) * squareSize, squareSize, squareSize);
        }
    }
}

void MainWindow::drawPieces(QPainter &painter) {
    const int offsetX = 20;
    const int offsetY = 50;

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            Piece* piece = game.getPieceAt(Position(row, col));
            if (piece) {
                QString color = piece->getColor() == PieceColor::White ? "white" : "black";
                QString type;
                switch (piece->getType()) {
                case PieceType::Pawn: type = "pawn"; break;
                case PieceType::Rook: type = "rook"; break;
                case PieceType::Knight: type = "knight"; break;
                case PieceType::Bishop: type = "bishop"; break;
                case PieceType::Queen: type = "queen"; break;
                case PieceType::King: type = "king"; break;
                default: continue;
                }
                // Load image directly from file system
                QString imagePath = QString("D:/PROJECTS/C++ Projects/Chess/Images/%1_%2.png").arg(color).arg(type);
                QPixmap pixmap(imagePath);
                if (pixmap.isNull()) {
                    qDebug() << "Failed to load image from file system:" << imagePath;
                } else {
                    painter.drawPixmap(offsetX + col * squareSize, offsetY + (7 - row) * squareSize, squareSize, squareSize, pixmap);
                }
            }
        }
    }
}

void MainWindow::drawHighlights(QPainter &painter) {
    const int offsetX = 20;
    const int offsetY = 50;

    if (selectedPos.isValid()) {
        painter.setBrush(QColor(255, 255, 0, 100));
        painter.drawRect(offsetX + selectedPos.col * squareSize, offsetY + (7 - selectedPos.row) * squareSize, squareSize, squareSize);
        painter.setBrush(QColor(0, 255, 0, 100));
        for (const auto& move : validMoves) {
            painter.drawRect(offsetX + move.col * squareSize, offsetY + (7 - move.row) * squareSize, squareSize, squareSize);
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    const int offsetX = 20;
    const int offsetY = 50;

    int col = (event->x() - offsetX) / squareSize;
    int row = 7 - (event->y() - offsetY) / squareSize;
    Position clickedPos(row, col);

    if (!clickedPos.isValid()) return;

    if (std::find(validMoves.begin(), validMoves.end(), clickedPos) != validMoves.end()) {
        game.movePiece(selectedPos, clickedPos);
        selectedPos = Position(-1, -1);
        validMoves.clear();
        updateTimer->start(0);
    } else {
        Piece* piece = game.getPieceAt(clickedPos);
        if (piece && piece->getColor() == game.getCurrentTurn()) {
            selectedPos = clickedPos;
            validMoves = game.getValidMoves(selectedPos);
        } else {
            selectedPos = Position(-1, -1);
            validMoves.clear();
        }
    }

    update();
}

void MainWindow::updateGameStatusPart1() {
    opponentColor = (game.getCurrentTurn() == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    if (game.isCheckmate(opponentColor)) {
        QString winner = (opponentColor == PieceColor::White) ? "Black" : "White";
        QMessageBox::information(this, "Game Over", QString("Checkmate! %1 wins!").arg(winner));
        return;
    }
    disconnect(updateTimer, nullptr, nullptr, nullptr);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateGameStatusPart2);
    updateTimer->start(10);
}

void MainWindow::updateGameStatusPart2() {
    if (game.isStalemate(opponentColor)) {
        QMessageBox::information(this, "Game Over", "Stalemate! The game is a draw.");
        return;
    }
    disconnect(updateTimer, nullptr, nullptr, nullptr);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateGameStatusPart3);
    updateTimer->start(10);
}

void MainWindow::updateGameStatusPart3() {
    inCheck = false;
    for (int row = 0; row < 8 && !inCheck; ++row) {
        for (int col = 0; col < 8; ++col) {
            Piece* piece = game.getPieceAt(Position(row, col));
            if (piece && piece->getColor() == opponentColor && piece->getType() == PieceType::King) {
                if (game.isKingInCheck(opponentColor)) {
                    statusBar()->showMessage(
                        QString("%1 is in check!").arg(opponentColor == PieceColor::White ? "White" : "Black"));
                    inCheck = true;
                    break;
                }
            }
        }
    }

    if (!inCheck) {
        statusBar()->showMessage(
            QString("%1's turn").arg(game.getCurrentTurn() == PieceColor::White ? "White" : "Black"));
    }
}
