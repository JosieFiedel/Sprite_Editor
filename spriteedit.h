/*
 * Team             |   Chandler
 * Members          |   Braden Fiedel, Caden Erickson, Connor Blood, Josie Fiedel
 * Class            |   CS 3505
 * Project          |   A7: Sprite Editor Implementation
 * Last modified    |   March 30, 2023
 * Style Reviewer   |   Braden Fiedel
 *
 * This file contains the class definition for the SpriteEdit class.
 */


#ifndef SPRITEEDIT_H
#define SPRITEEDIT_H

#include <QObject>
#include <QPoint>
#include "qcolor.h"

using std::tuple;


/**
 * @brief The SpriteEdit class
 * This class represents an "edit" made in the sprite editor. Objects of this class
 * are used to enable undo/redo functionality in the editor, and store data about
 * each pixel affected by the edit, as well as the frame in which the edit took place.
 */
class SpriteEdit
{
public:
    explicit SpriteEdit(int);
    void addEditComponent(QPoint, QColor, QColor);
    bool isEmpty();
    int getFrameIndex();
    void incrementFrameIndex();
    void decrementFrameIndex();
    QList<tuple<QPoint, QColor, QColor>> getComponents();


private:
    int frameIndex;
    QList<tuple<QPoint, QColor, QColor>> editComponents;

};

#endif // SPRITEEDIT_H
