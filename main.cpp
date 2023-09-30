/*
 * Team             |   Chandler
 * Members          |   Braden Fiedel, Caden Erickson, Connor Blood, Josie Fiedel
 * Class            |   CS 3505
 * Project          |   A7: Sprite Editor Implementation
 * Last modified    |   April 5, 2023
 * Style Reviewer   |   Connor Blood
 *
 * This file contains the class definition for the SpriteEditorModel class.
 */


#include "spriteeditorview.h"
#include <QApplication>


/**
 * @brief Application entry point
 *
 * @param argc -- unused
 * @param argv -- unused
 * @return exit state
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SpriteEditorModel model;
    SpriteEditorView view(model);
    view.show();
    return a.exec();
}
