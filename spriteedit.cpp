/*
 * Team             |   Chandler
 * Members          |   Braden Fiedel, Caden Erickson, Connor Blood, Josie Fiedel
 * Class            |   CS 3505
 * Project          |   A7: Sprite Editor Implementation
 * Last modified    |   March 30, 2023
 * Style Reviewer   |   Braden Fiedel
 *
 * This file contains the implementation of the class definition located in spriteedit.h.
 */


#include "spriteedit.h"


/**
 * @brief SpriteEdit::SpriteEdit
 * Constructor. Initializes a SpriteEdit object with the given frame index.
 *
 * @param parent -- QObject parent object
 */
SpriteEdit::SpriteEdit(int frameIndex)
    : frameIndex{frameIndex}
{

}

/**
 * @brief SpriteEdit::addEditComponent
 * Adds a "component" (an edited pixel) to the current Edit object
 *
 * @param editLoc -- the coordinates of the edited pixel, within the canvas
 * @param previousColor -- the previous color of that pixel, prior to the edit
 */
void SpriteEdit::addEditComponent(QPoint editLoc, QColor previousColor, QColor newColor)
{
    editComponents.append(tuple(editLoc, previousColor, newColor));
}

/**
 * @brief SpriteEdit::isEmpty
 *
 * @return whether this SpriteEdit contains any edit components
 */
bool SpriteEdit::isEmpty()
{
    return editComponents.isEmpty();
}

/**
 * @brief SpriteEdit::getComponents
 *
 * @return the QList of components that make up this Edit
 */
QList<tuple<QPoint, QColor, QColor>> SpriteEdit::getComponents()
{
    return editComponents;
}

/**
 * @brief SpriteEdit::getFrameIndex
 *
 * @return the index of the frame that this Edit pertains to
 */
int SpriteEdit::getFrameIndex()
{
    return frameIndex;
}

/**
 * @brief SpriteEdit::incrementFrameIndex
 * Increments this Edit's frame index field.
 */
void SpriteEdit::incrementFrameIndex()
{
    frameIndex++;
}

/**
 * @brief SpriteEdit::decrementFrameIndex
 * Decrements this Edit's frame index field.
 */
void SpriteEdit::decrementFrameIndex()
{
    if (frameIndex > 0)
        frameIndex--;
}
