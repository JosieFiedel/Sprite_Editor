/*
 * Team             |   Chandler
 * Members          |   Braden Fiedel, Caden Erickson, Connor Blood, Josie Fiedel
 * Class            |   CS 3505
 * Project          |   A7: Sprite Editor Implementation
 * Last modified    |   April 5, 2023
 * Style Reviewer   |   Josie Fiedel
 *
 * This file contains the class definition for the SpriteEditorView class.
 */


#ifndef SPRITEEDITORVIEW_H
#define SPRITEEDITORVIEW_H

#include "spriteeditormodel.h"
#include <QEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QImage>
#include <QMainWindow>
#include <QMouseEvent>
#include <QShortcut>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class SpriteEditorView; }
QT_END_NAMESPACE

/**
 * @brief The SpriteEditorView class. This class represents the UI/View of the Sprite Editor program.
 */
class SpriteEditorView : public QMainWindow
{
    Q_OBJECT

public:
    SpriteEditorView(SpriteEditorModel& modelParam, QWidget *parent = nullptr);
    ~SpriteEditorView();

private:
    SpriteEditorModel* model;

    Ui::SpriteEditorView* ui;
    bool toggleDraw = true;

    QShortcut undoShortcut;
    QShortcut redoShortcut;
    QShortcut newShortcut;
    QShortcut openShortcut;
    QShortcut saveShortcut;
    QShortcut saveAsShortcut;

    void drawPixel(QPoint, QColor);

    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);

    void setUpNewFrame();
    void redrawCurrentFrame();

    void onPenClick();
    void onEraserClick();

public slots:
    void saveAsClicked();
    void openClicked();
    void newClicked();

    void setCanvasBackground(QLabel*);
    void updateCanvas(int);

    void clearCurrentFrame();
    void refreshFrame(int);
    void setUpFrameButton();

    void displayPreviewFrame(QImage*);
    void resetPreview();

private slots:
    void warnAboutClearFrame();
    void warnAboutDeletion();
};

#endif // SPRITEEDITORVIEW_H
