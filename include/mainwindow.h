#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chess.h"
#include <QMainWindow>
#include <QTimer>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void updateGameStatus();

private:
    ChessGame             game;
    Position              selectedPos;
    std::vector<Position> validMoves;
    std::vector<Position> captureMoves;   // ← NEW: subset of validMoves that capture an enemy
    QTimer*               updateTimer;
    const int             squareSize = 70;

    void drawBoard(QPainter& p);
    void drawPieces(QPainter& p);
    void drawHighlights(QPainter& p);
    void drawCoordinates(QPainter& p);    // ← NEW: a-h / 1-8 labels on the border
};

#endif // MAINWINDOW_H