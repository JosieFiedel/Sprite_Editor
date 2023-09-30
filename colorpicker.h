/*
 * Team             |   Chandler
 * Members          |   Braden Fiedel, Caden Erickson, Connor Blood, Josie Fiedel
 * Class            |   CS 3505
 * Project          |   A7: Sprite Editor Implementation
 * Last modified    |   April 3, 2023
 * Style Reviewer   |   Connor Blood
 *
 * This file contains the class definition for the ColorPicker class.
 */

#ifndef COLORPICKER_H
#define COLORPICKER_H

#include "qpushbutton.h"
#include <QMouseEvent>
#include <QPainter>

QT_BEGIN_NAMESPACE
namespace Ui { class ColorPicker; }
QT_END_NAMESPACE

/**
 * @brief The ColorPicker class
 */
class ColorPicker : public QWidget
{
    Q_OBJECT

public:
    explicit ColorPicker(QWidget *parent = nullptr);
    ~ColorPicker();

private slots:
    void changeGradientHue(int);
    void setRecentColorSelection(QPushButton*);

public slots:
    void drawRecentColors(QList<QColor>);
    void resetColorPalette();

private:
    Ui::ColorPicker *ui;

    const int MAX_COLOR_VAL = 255;
    const int MAX_COLOR_HUE = 359;

    QPoint currPt;

    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void paintEvent(QPaintEvent*);

    bool isInGradientBounds(QPoint);
    void updateCurrentColor(QPoint);
    QColor getColorFromCurrentPoint();

signals:
    void updateNewColor(QColor);
};

#endif // COLORPICKER_H
