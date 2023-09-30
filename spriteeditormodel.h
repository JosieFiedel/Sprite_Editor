/*
 * Team             |   Chandler
 * Members          |   Braden Fiedel, Caden Erickson, Connor Blood, Josie Fiedel
 * Class            |   CS 3505
 * Project          |   A7: Sprite Editor Implementation
 * Last modified    |   April 5, 2023
 * Style Reviewer   |   Caden Erickson
 *
 * This file contains the class definition for the SpriteEditorModel class.
 */


#ifndef SPRITEEDITORMODEL_H
#define SPRITEEDITORMODEL_H

#include "spriteedit.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QStack>
#include <QTimer>
#include <QWidget>

/**
 * @brief The SpriteEditorModel class.
 * This class stores "state" data for an instance of the Sprite Editor program.
 */
class SpriteEditorModel : public QWidget
{
    Q_OBJECT

public:
    explicit SpriteEditorModel(QWidget *parent = nullptr);

    enum Tool { PEN, ERASER };
    int getCanvasSize();
    QColor getCurrentColor();
    int getCurrentFrameIndex();
    QImage* getFrame(int);
    QPushButton* getFrameButton(int);

    void saveFile(QString);
    void openFile(QString);
    void newFile(int);

    void createNewFrame();

    void beginEdit();
    void addToEdit(QPoint, QColor, QColor);
    void endEdit();
    void clearEditsOnCurrentFrame();

private:
    QColor currentColor;
    QList<QColor> recentColors;
    int canvasSize = 16;
    int numFrames;
    int currentFrameIndex;
    QList<QImage> frames;
    QList<QPushButton*> frameButtons;

    QString saveDir;

    QTimer *timer;
    int animationIndex = 0;
    bool animationRunning;

    SpriteEdit currentEdit;
    QStack<SpriteEdit> edits;
    QStack<SpriteEdit> undoneEdits;
    void adjustEditsUpFromIndex(int);
    void adjustEditsDownFromIndex(int);

    void setCanvasSize(int);
    bool areSimilarColors(QColor, QColor);

public slots:
    void saveClicked();
    void deleteCurrentFrame();
    void duplicateCurrentFrame();
    void selectNewFrame();
    void undo();
    void redo();
    void animatePreviewFrame();
    void toggleAnimation();
    void changeAnimationSpeed(int);
    void setCurrentColor(QColor);
    void updateRecentColorsList(QColor);
    void setTool(SpriteEditorModel::Tool);

signals:
    void noSaveDirectory();
    void canvasSizeChanged();
    void setUpNewFrame();
    void frameUpdated(int);
    void setFocusToIndex(int);
    void setUpFrameButton();
    void displayPreviewFrame(QImage*);
    void resetPreview();
    void animationStarted();
    void animationStopped();
    void warnAboutDeletion();
    void updateRecentColors(QList<QColor>);
    void resetColorPalette();
};

#endif // SPRITEEDITORMODEL_H
