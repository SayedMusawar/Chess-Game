#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chess.h"
#include <QMainWindow>
#include <QTimer>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void updateGameStatusPart1();
    void updateGameStatusPart2();
    void updateGameStatusPart3();

private:
    ChessGame game;
    Position selectedPos;
    std::vector<Position> validMoves;
    QTimer* updateTimer;
    PieceColor opponentColor;
    bool inCheck;
    const int squareSize = 70;

    void drawBoard(QPainter &painter);
    void drawPieces(QPainter &painter);
    void drawHighlights(QPainter &painter);
};

#endif // MAINWINDOW_H
