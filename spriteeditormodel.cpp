/*
 * Team             |   Chandler
 * Members          |   Braden Fiedel, Caden Erickson, Connor Blood, Josie Fiedel
 * Class            |   CS 3505
 * Project          |   A7: Sprite Editor Implementation
 * Last modified    |   April 5, 2023
 * Style Reviewer   |   Caden Erickson
 *
 * This file contains the implementation of the class definition located in spriteeditormodel.h.
 */


#include "spriteeditormodel.h"
#include <QTimer>


/**
 * @brief SpriteEditorModel::SpriteEditorModel
 * Constructor. Creates a new SpriteEditorModel object, using
 * default values (for now).
 *
 * @param parent -- QWidget parent object
 */
SpriteEditorModel::SpriteEditorModel(QWidget* parent)
    : QWidget{parent}
    , numFrames{0}
    , currentFrameIndex{0}
    , animationRunning{false}
    , currentEdit{SpriteEdit(0)}
{
    // Create the default brush color (black) and add it to the recent colors list.
    QColor color;
    color.setHsv(359, 3, 4);
    recentColors.append(color);

    // Set up a timer to animate the preview frame
    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setTimerType(Qt::PreciseTimer);
    timer->setInterval(0);

    connect(timer, &QTimer::timeout,
            this, &SpriteEditorModel::animatePreviewFrame);
}


// ===================================================
// ===             SETTERS AND GETTERS             ===
// ===================================================

/**
 * @brief SpriteEditorModel::setCanvasSize
 * Sets the side length (size) of the canvas to the given size in QImage "pixels"
 *
 * @param size -- the value to which to set the canvas side length (size)
 */
void SpriteEditorModel::setCanvasSize(int size)
{
    canvasSize = size;
}

/**
 * @brief SpriteEditorModel::getCanvasSize
 *
 * @return the side length (size) of the canvas, in QImage "pixels"
 */
int SpriteEditorModel::getCanvasSize()
{
    return canvasSize;
}

/**
 * @brief SpriteEditorModel::setCurrentColor
 *
 * @param color -- the new color of the drawing tool
 */
void SpriteEditorModel::setCurrentColor(QColor color)
{
    currentColor = color;
}

/**
 * @brief SpriteEditorModel::getCurrentColor
 *
 * @return the current color of the drawing tool
 */
QColor SpriteEditorModel::getCurrentColor()
{
    return currentColor;
}

/**
 * @brief SpriteEditorModel::getCurrentFrameIndex
 *
 * @return the index of the currently active frame
 */
int SpriteEditorModel::getCurrentFrameIndex()
{
    return currentFrameIndex;
}

/**
 * @brief SpriteEditorModel::getFrame
 *
 * @param frameIndex -- the index of the frame to get
 * @return a pointer to the frame of the given index
 */
QImage* SpriteEditorModel::getFrame(int frameIndex)
{
    return &frames[frameIndex];
}

/**
 * @brief SpriteEditorModel::getFrameButton
 *
 * @param frameButtonIndex -- the index of the frame button to get
 * @return a pointer to the frame of the given index
 */
QPushButton* SpriteEditorModel::getFrameButton(int frameButtonIndex)
{
    return frameButtons[frameButtonIndex];
}


// ===================================================
// ===              FILE MANIPULATION              ===
// ===================================================

/**
 * @brief SpriteEditorModel::saveFile
 * Attempts to save a file at the specified directory.
 *
 * @param fileDir -- the file directory at which a file is to be saved.
 */
void SpriteEditorModel::saveFile(QString fileDir)
{
    QJsonObject editorInstance;

    // Serialize the height, width, and number of frames.
    editorInstance["height"] = canvasSize;
    editorInstance["width"] = canvasSize ;
    editorInstance["numberOfFrames"] = frames.count();

    QJsonObject framePixels;
    // Loop through every frame.
    for (int i = 0; i < frames.count(); i++)
    {
        QJsonArray pixels;
        // Loop through every row in the frame.
        for (int y = 0; y < canvasSize; y++)
        {
            QJsonArray row;
            // Loop through every pixel in the row.
            for (int x = 0; x < canvasSize; x++) {
                QRgb pixel = frames[i].pixel(x, y);
                QJsonArray pixelColors;
                pixelColors.append(qRed(pixel));
                pixelColors.append(qGreen(pixel));
                pixelColors.append(qBlue(pixel));
                pixelColors.append(qAlpha(pixel));
                row.append(pixelColors);
            }
            pixels.append(row);
        }
        framePixels["frame" + QString::number(i)] = pixels;
    }
    editorInstance["frames"] = framePixels;

    // Write the serialized data to a file.
    QFile file(fileDir);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument editorInstanceDoc(editorInstance);
        file.write(editorInstanceDoc.toJson());
        file.close();

        // Update the stored save directory if needed.
        if (saveDir.isNull())
            saveDir = fileDir;
        else if (saveDir != fileDir)
            saveDir = fileDir;
    }
}

/**
 * @brief SpriteEditorModel::openFile
 * Attempts to open a file at the specified directory.
 *
 * @param fileDir -- the file directory at which a file is to be opened.
 */
void SpriteEditorModel::openFile(QString fileDir)
{
    // End the animation.
    timer->stop();
    animationRunning = false;
    animationIndex = 0;

    // Read the serialized data from the file.
    QFile file(fileDir);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QJsonObject editorInstance = QJsonDocument::fromJson(file.readAll()).object();
    file.close();

    // Extract the canvas size from the read data.
    canvasSize = editorInstance.value("height").toInt();
    emit canvasSizeChanged();

    // Remove the old frames.
    int oldFrameCount = numFrames;
    for (int i = 0; i < oldFrameCount - 1; i++)
        deleteCurrentFrame();

    // Extract the frames from the read data.
    QJsonObject framePixels = editorInstance.value("frames").toObject();

    // Set the first frame.
    QImage firstFrame(canvasSize, canvasSize, QImage::Format_ARGB32);
    QJsonArray pixels = framePixels.value("frame0").toArray();
    // Loop through every row in the frame.
    for (int y = 0; y < pixels.size(); y++)
    {
        QJsonArray row = pixels.at(y).toArray();
        // Loop through every pixel in the row.
        for (int x = 0; x < row.size(); x++)
        {
            QJsonArray pixel = row.at(x).toArray();
            firstFrame.setPixel(x, y, qRgba(pixel.at(0).toInt(), pixel.at(1).toInt(), pixel.at(2).toInt(), pixel.at(3).toInt()));
        }
    }
    frames[0] = firstFrame;
    emit frameUpdated(0);

    // Set the remaining frames.
    for (int i = 1; i < editorInstance.value("numberOfFrames").toInt(); i++)
    {
        createNewFrame();
        QJsonArray pixels = framePixels.value("frame" + QString::number(i)).toArray();
        // Loop through every row in the frame.
        for (int y = 0; y < pixels.size(); y++)
        {
            QJsonArray row = pixels.at(y).toArray();
            // Loop through every pixel in the row.
            for (int x = 0; x < row.size(); x++)
            {
                QJsonArray pixel = row.at(x).toArray();
                frames[i].setPixel(x, y, qRgba(pixel.at(0).toInt(), pixel.at(1).toInt(), pixel.at(2).toInt(), pixel.at(3).toInt()));
            }
        }
        frameButtons[i]->setChecked(false);
        emit frameUpdated(i);
    }

    // Set focus to the first frame and refresh the drawing canvas
    currentFrameIndex = 0;
    frameButtons[currentFrameIndex]->setChecked(true);
    emit setFocusToIndex(currentFrameIndex);

    // Reset the recent colors, palette, and tool.
    recentColors.clear();
    emit resetColorPalette();
    setTool(PEN);

    // Update the stored save directory.
    saveDir = fileDir;

    // Clear undo/redo stacks.
    edits.clear();
    undoneEdits.clear();

    // Reset the animation preview.
    emit resetPreview();
}

/**
 * @brief SpriteEditorModel::newFile
 * Attempts to create a new file.
 *
 * @param newCanvasSize -- the canvas size of the new file.
 */
void SpriteEditorModel::newFile(int newCanvasSize)
{
    // End the animation.
    timer->stop();
    animationRunning = false;
    animationIndex = 0;

    // Set the canvas size.
    canvasSize = newCanvasSize;
    emit canvasSizeChanged();

    // Remove the old frames.
    int oldFrameCount = numFrames;
    for (int i = 0; i < oldFrameCount - 1; i++)
        deleteCurrentFrame();

    // Set the first frame.
    QImage firstFrame(canvasSize, canvasSize, QImage::Format_ARGB32);
    firstFrame.fill(qRgba(0, 0, 0, 0));
    frames[0] = firstFrame;
    emit frameUpdated(0);

    // Reset the recent colors, palette, and tool.
    recentColors.clear();
    emit resetColorPalette();
    setTool(PEN);

    // Update the stored save directory.
    saveDir = NULL;

    // Clear undo/redo stacks.
    edits.clear();
    undoneEdits.clear();

    // Reset the animation preview.
    emit resetPreview();
}

/**
 * @brief SpriteEditorModel::saveClicked
 * Checks if the current file is associated with a file directory.
 * If it is, save to that directory. If not, prompt the user to choose a save location.
 */
void SpriteEditorModel::saveClicked()
{
    if (saveDir.isNull())
        emit noSaveDirectory();
    else
        saveFile(saveDir);
}


// ===================================================
// ===             FRAME MANIPULATION              ===
// ===================================================

/**
 * @brief SpriteEditorModel::createNewFrame
 * Creates a new, blank frame, adds it to the QList of frames, and emits
 * a signal so that the view can redraw accordingly.
 */
void SpriteEditorModel::createNewFrame()
{
    // Create a new QImage frame, add it to the QList of existing frames,
    //  and ensure that it's blank and transparent
    frames.push_back(QImage(canvasSize, canvasSize, QImage::Format_ARGB32));
    numFrames++;
    frames[numFrames - 1].fill(Qt::transparent);

    // If we already have a frame, deselect the last "current frame button"
    if(numFrames > 1)
        frameButtons[currentFrameIndex]->setChecked(false);

    // Set the new frame created as the current frame.
    currentFrameIndex = numFrames - 1;

    // Adding a frame messes with edit indices: adjust them
    adjustEditsUpFromIndex(currentFrameIndex);

    // Set up a new push button that will correspond with the new frame,
    //  and add it to the QList of existing buttons
    QPushButton* button = new QPushButton();
    button->setProperty("id", currentFrameIndex);
    button->setCheckable(true);
    button->setChecked(true);
    button->setStyleSheet("QPushButton:checked { background-color: blue; }");
    frameButtons.push_back(button);

    // Set up a connect call for this new button
    connect(button, &QPushButton::clicked,
            this, &SpriteEditorModel::selectNewFrame);

    emit setUpFrameButton();
    emit frameUpdated(currentFrameIndex);
}

/**
 * @brief SpriteEditorModel::selectNewFrame
 * Changes the current frame and frame button based off
 * which frame button the user pushed.
 */
void SpriteEditorModel::selectNewFrame()
{
    // Check if the id is an int before proceeding
    QVariant idCheck = sender()->property("id");
    if(!idCheck.isValid())
        return;

    int index = idCheck.toInt();

    // Deselect the last frame button if it isn't the same frame that's already focused
    if(!(currentFrameIndex == index))
        frameButtons[currentFrameIndex]->setChecked(false);
    else
        frameButtons[currentFrameIndex]->setChecked(true);

    // Set the current frame to the selected index
    currentFrameIndex = index;

    // Update displays
    emit setFocusToIndex(currentFrameIndex);
}

/**
 * @brief SpriteEditorModel::deleteCurrentFrame
 * Deletes the frame that is currently selected.
 */
void SpriteEditorModel::deleteCurrentFrame()
{
    // Prevent frame deletion if the animation is running
    if(animationRunning)
    {
        emit warnAboutDeletion();
        return;
    }

    // If there is more than one frame, delete the currently selected one -- don't let the user delete the base frame
    if(numFrames > 1)
    {
        // Delete the current frame button
        frameButtons[currentFrameIndex]->deleteLater();

        // If we are deleting frame index 0, set the current frame to the one above it, else progress to the previous frame
        if(currentFrameIndex == 0)
            frameButtons[currentFrameIndex+1]->setChecked(true);
        else
            frameButtons[currentFrameIndex-1]->setChecked(true);

        // Remove the frame and its button from the respective lists
        frames.removeAt(currentFrameIndex);
        frameButtons.removeAt(currentFrameIndex);
        adjustEditsDownFromIndex(currentFrameIndex);

        // Update frame button indexes
        if(currentFrameIndex < numFrames -1)
            for(int i = currentFrameIndex; i < numFrames - 1; i++)
                frameButtons[i]->setProperty("id", i);

        // Decrement the number of frames, and the current frame index
        numFrames--;

        // Decrement the frame index ONLY IF we weren't already at 0
        if(currentFrameIndex > 0)
            currentFrameIndex--;

        // Update display
        emit setFocusToIndex(currentFrameIndex);
    }
}

/**
 * @brief SpriteEditorModel::duplicateCurrentFrame
 * Duplicates the currently selected frame.
 */
void SpriteEditorModel::duplicateCurrentFrame()
{
    // Create a deep copy of the selected frame
    QImage duplicate = frames[currentFrameIndex].copy();

    // Create a new QImage frame, add it to the QList of existing frames. Increment frame counter.
    frames.push_back(duplicate);
    numFrames++;

    // Deselect the old frame
    frameButtons[currentFrameIndex]->setChecked(false);

    // Set the new frame created as the current frame.
    currentFrameIndex = numFrames - 1;

    adjustEditsUpFromIndex(currentFrameIndex);

    // Set up a new push button that will correspond with the new frame
    QPushButton* button = new QPushButton();
    button->setProperty("id", currentFrameIndex);
    button->setCheckable(true);
    button->setChecked(true);
    button->setStyleSheet("QPushButton:checked { background-color: blue; }");
    frameButtons.push_back(button);

    // Set up a connect call for this new button
    connect(button, &QPushButton::clicked,
            this, &SpriteEditorModel::selectNewFrame);

    emit setUpFrameButton();
    emit frameUpdated(currentFrameIndex);
}

// ===================================================
// ===                PREVIEW FRAME                ===
// ===================================================

/**
 * @brief SpriteEditorModel::startAnimation
 * Toggles the animation state between started
 * (if the timer interval is more than 0) and stopped.
 */
void SpriteEditorModel::toggleAnimation()
{
    if(!animationRunning && timer->interval() > 0)
    {
        timer->start();
        animationRunning = true;
        emit animationStarted();
    }
    else if (animationRunning)
    {
        timer->stop();
        animationRunning = false;
        animationIndex = 0;
        animatePreviewFrame();
        emit animationStopped();
    }
}

/**
 * @brief SpriteEditorModel::changeAnimationSpeed
 * If the slider value is greater than 0, sets the timer interval to a frames per second
 * value calculated from the slider value.
 *
 * @param FPS -- the value of the slider in the UI
 */
void SpriteEditorModel::changeAnimationSpeed(int FPS)
{
    // If the slider is set to 0, stop the animation
    if(FPS <= 0)
    {
        timer->setInterval(0);
        timer->stop();
        return;
    }

    // Calculate the FPS and set that as the timer interval
    double fpsScaled = 1/(double)FPS;
    timer->setInterval(fpsScaled * 1000);
    if (animationRunning && !timer->isActive())
        timer->start();
}

/**
 * @brief SpriteEditorModel::animatePreviewFrame
 * Cycles through the frames the user has created, displaying them.
 */
void SpriteEditorModel::animatePreviewFrame()
{
    emit displayPreviewFrame(&frames[animationIndex]);

    if(animationIndex < numFrames - 1)
        animationIndex++;
    else
        animationIndex = 0;
}

// ===================================================
// ===              EDIT MANIPULATION              ===
// ===================================================

/**
 * @brief SpriteEditorModel::undo
 * "Undoes" a previously-made edit, if there are any to be undone,
 * and signals to the view to redraw itself if necessary.
 */
void SpriteEditorModel::undo()
{
    if (edits.isEmpty())
        return;

    SpriteEdit editToUndo = edits.pop();

    // For undoing, set each affected pixel to its "old" color
    for (const auto& [coord, oldColor, newColor] : editToUndo.getComponents())
        frames[editToUndo.getFrameIndex()].setPixel(coord, oldColor.rgba());
    undoneEdits.push(editToUndo);

    emit frameUpdated(editToUndo.getFrameIndex());
}

/**
 * @brief SpriteEditorModel::redo
 * "Redoes" a previously-undone edit, if there are any to be redone,
 * and signals to the view to redraw itself if necessary.
 */
void SpriteEditorModel::redo()
{
    if (undoneEdits.isEmpty())
        return;

    SpriteEdit editToRedo = undoneEdits.pop();

    // For redoing, set each affected pixel to its "new" color
    for (const auto& [coord, oldColor, newColor] : editToRedo.getComponents())
        frames[editToRedo.getFrameIndex()].setPixel(coord, newColor.rgba());
    edits.push(editToRedo);

    emit frameUpdated(editToRedo.getFrameIndex());
}

/**
 * @brief SpriteEditorModel::beginEdit
 * Begins the process of "recording" a new edit made on the canvas.
 */
void SpriteEditorModel::beginEdit()
{
    undoneEdits.clear();
    currentEdit = SpriteEdit(currentFrameIndex);
}

/**
 * @brief SpriteEditorModel::addToEdit
 * Adds a "component" (a changed pixel) to the edit that's currently being tracked.
 *
 * @param editLocation -- the location of the changed pixel
 * @param oldColor -- the old color of the changed pixel
 * @param newColor -- the new color of the changed pixel
 */
void SpriteEditorModel::addToEdit(QPoint editLocation, QColor oldColor, QColor newColor)
{
    currentEdit.addEditComponent(editLocation, oldColor, newColor);
}

/**
 * @brief SpriteEditorModel::endEdit
 * Finishes the edit by pushing the completed edit to the "edits" stack.
 */
void SpriteEditorModel::endEdit()
{
    if (!currentEdit.isEmpty())
        edits.push(currentEdit);
}

/**
 * @brief SpriteEditorModel::adjustEditsUpFromIndex
 * Adjusts the edits in the edit stacks when a frame has been added, based
 * on which index the new frame was added at.
 *
 * @param addedFrameIndex -- the index at which the new frame was added
 */
void SpriteEditorModel::adjustEditsUpFromIndex(int addedFrameIndex)
{
    for (SpriteEdit& edit : edits)
        if (edit.getFrameIndex() >= addedFrameIndex)
            edit.incrementFrameIndex();

    for (SpriteEdit& edit : undoneEdits)
        if (edit.getFrameIndex() >= addedFrameIndex)
            edit.incrementFrameIndex();
}

/**
 * @brief SpriteEditorModel::adjustEditsDownFromIndex
 * Adjusts the edits in the edit stacks when a frame has been removed, including
 * removing edits pertaining to the removed frame, based on which index the
 * frame was deleted from.
 *
 * @param removedFrameIndex -- the index from which the removed frame was deleted
 */
void SpriteEditorModel::adjustEditsDownFromIndex(int removedFrameIndex)
{
    for (int i = 0; i < edits.size(); i++)
    {
        if(edits[i].getFrameIndex() == removedFrameIndex)
            edits.removeAt(i);
        else if (edits[i].getFrameIndex() > removedFrameIndex)
            edits[i].decrementFrameIndex();
    }

    for (int i = 0; i < undoneEdits.size(); i++)
    {
        if(undoneEdits[i].getFrameIndex() == removedFrameIndex)
            undoneEdits.removeAt(i);
        else if (undoneEdits[i].getFrameIndex() > removedFrameIndex)
            undoneEdits[i].decrementFrameIndex();
    }
}

/**
 * @brief SpriteEditorModel::clearEditsOnCurrentFrame
 * Removes all edits associated with the current frame.
 */
void SpriteEditorModel::clearEditsOnCurrentFrame()
{
    for (int i = 0; i < edits.size(); i++)
        if(edits[i].getFrameIndex() == currentFrameIndex)
            edits.removeAt(i);

    for (int i = 0; i < undoneEdits.size(); i++)
        if(undoneEdits[i].getFrameIndex() == currentFrameIndex)
            undoneEdits.removeAt(i);
}


// ===================================================
// ===             COLORS AND TOOLS                ===
// ===================================================


/**
 * @brief SpriteEditorModel::updateRecentColorsList
 * Updates the colors in the recent colors list, with a maximum
 * of five colors total.
 *
 * @param color -- the new color to add to the recent colors.
 */
void SpriteEditorModel::updateRecentColorsList(QColor color)
{
    // If a similar color is already in the recent colors list, then that color is
    // removed and added to the front of the colors list.
    for(QColor recentColor: recentColors)
    {
        if(areSimilarColors(recentColor, color))
        {
            recentColors.remove(recentColors.indexOf(recentColor));
            break;
        }
    }

    // The least recently used color is removed if the recent colors list is full.
    if(recentColors.count() == 5)
        recentColors.removeFirst();

    // The new color is added to the recent colors list.
    recentColors.append(color);
    currentColor = color;
    emit updateRecentColors(recentColors);
}

/**
 * @brief SpriteEditorModel::areSimilarColors
 * Determines if the given two colors are similar. That is, they
 * differ by less than 3 in saturation or valu, and they have the same hue.
 *
 * @param colorOne -- first color to compare for similarity
 * @param colorTwo -- second color to compare for similarity
 * @return true if the colors are similar, false otherwise.
 */
bool SpriteEditorModel::areSimilarColors(QColor colorOne, QColor colorTwo)
{
    return abs(colorOne.value() - colorTwo.value()) < 3
        && abs(colorOne.saturation() - colorTwo.saturation()) < 3
        && colorOne.hue() == colorTwo.hue();
}

/**
 * @brief SpriteEditorModel::setTool
 * Sets the current tool and color (transparent if eraser).
 *
 * @param currTool -- the Tool that the model has set as active
 */
void SpriteEditorModel::setTool(Tool currTool)
{
    switch(currTool)
    {
        case PEN:
            setCurrentColor(recentColors.last());
            break;
        case ERASER:
            setCurrentColor(Qt::transparent);
    }
}
