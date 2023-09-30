/*
 * Team             |   Chandler
 * Members          |   Braden Fiedel, Caden Erickson, Connor Blood, Josie Fiedel
 * Class            |   CS 3505
 * Project          |   A7: Sprite Editor Implementation
 * Last modified    |   April 5, 2023
 * Style Reviewer   |   Josie Fiedel
 *
 * This file contains the implementation of the class definition located in spriteeditorview.h.
 */


#include "spriteeditorview.h"
#include "ui_spriteeditorview.h"
#include <QMessageBox>


/**
 * @brief SpriteEditorView::SpriteEditorView
 * Constructor. Creates a new SpriteEditorView object, sets up the ui,
 * and establishes connections between different classes and the model.
 *
 * @param parent -- QWidget parent object
 */
SpriteEditorView::SpriteEditorView(SpriteEditorModel& modelParam, QWidget* parent)
    : QMainWindow(parent)
    , model{&modelParam}
    , ui(new Ui::SpriteEditorView)
    , undoShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z), this)
    , redoShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y), this)
    , newShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this)
    , openShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), this)
    , saveShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this)
    , saveAsShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), this)
{
    ui->setupUi(this);
    this->setWindowTitle("Sprite Editor");
    ui->animationStartButton->setStyleSheet("QPushButton {color: #32CD32};");
    ui->penButton->setEnabled(false);

    // Connections for managing the color palette.
    connect(ui->colorPicker, &ColorPicker::updateNewColor,
            model, &SpriteEditorModel::updateRecentColorsList);
    connect(model, &SpriteEditorModel::updateRecentColors,
            ui->colorPicker, &ColorPicker::drawRecentColors);
    connect(model, &SpriteEditorModel::resetColorPalette,
            ui->colorPicker, &ColorPicker::resetColorPalette);

    // Connections for managing brushes.
    connect(ui->colorPicker, &ColorPicker::updateNewColor,
            this, &SpriteEditorView::onPenClick);
    connect(ui->penButton, &QPushButton::clicked,
            this, &SpriteEditorView::onPenClick);
    connect(ui->eraserButton, &QPushButton::clicked,
            this, &SpriteEditorView::onEraserClick);

    // Set up the scroll area for frames with a layout
    ui->scrollAreaWidget->setLayout(ui->scrollLayout);

    // Connections for managing saved data (new, open, save, save as).
    connect(model, &SpriteEditorModel::canvasSizeChanged,
            this, [this](){setCanvasBackground(ui->canvasBackgroundLabel);});
    connect(ui->actionNew, &QAction::triggered,
            this, &SpriteEditorView::newClicked);
    connect(&newShortcut, &QShortcut::activated,
            this, &SpriteEditorView::newClicked);
    connect(ui->actionOpen, &QAction::triggered,
            this, &SpriteEditorView::openClicked);
    connect(&openShortcut, &QShortcut::activated,
            this, &SpriteEditorView::openClicked);
    connect(ui->actionSave, &QAction::triggered,
            model, &SpriteEditorModel::saveClicked);
    connect(&saveShortcut, &QShortcut::activated,
            model, &SpriteEditorModel::saveClicked);
    connect(model, &SpriteEditorModel::noSaveDirectory,
            this, &SpriteEditorView::saveAsClicked);
    connect(ui->actionSaveAs, &QAction::triggered,
            this, &SpriteEditorView::saveAsClicked);
    connect(&saveAsShortcut, &QShortcut::activated,
            this, &SpriteEditorView::saveAsClicked);

    // Connections for managing frames (add, clear, duplicate, etc.)
    connect(ui->addFrame, &QPushButton::clicked,
            model, &SpriteEditorModel::createNewFrame);
    connect(model, &SpriteEditorModel::setUpNewFrame,
            this, &SpriteEditorView::setUpNewFrame);
    connect(ui->clearFrame, &QPushButton::clicked,
            this, &SpriteEditorView::warnAboutClearFrame);
    connect(model, &SpriteEditorModel::warnAboutDeletion,
            this, &SpriteEditorView::warnAboutDeletion);
    connect(ui->deleteFrame, &QPushButton::clicked,
            model, &SpriteEditorModel::deleteCurrentFrame);
    connect(ui->duplicateFrame, &QPushButton::clicked,
            model, &SpriteEditorModel::duplicateCurrentFrame);
    connect(model, &SpriteEditorModel::setFocusToIndex,
            this, &SpriteEditorView::updateCanvas);
    connect(model, &SpriteEditorModel::frameUpdated,
            this, &SpriteEditorView::refreshFrame);
    connect(model, &SpriteEditorModel::setUpFrameButton,
            this, &SpriteEditorView::setUpFrameButton);

    // Connections for managing the preview frame
    connect(model, &SpriteEditorModel::displayPreviewFrame,
            this, &SpriteEditorView::displayPreviewFrame);
    connect(model, &SpriteEditorModel::resetPreview,
            this, &SpriteEditorView::resetPreview);
    connect(ui->animationStartButton, &QPushButton::clicked,
            model, &SpriteEditorModel::toggleAnimation);
    connect(model, &SpriteEditorModel::animationStarted,
            this, [this](){ui->animationStartButton->setText(QString("■"));
                           ui->animationStartButton->setStyleSheet("QPushButton {color: red};");});
    connect(model, &SpriteEditorModel::animationStopped,
            this, [this](){ui->animationStartButton->setText(QString("▶"));
                           ui->animationStartButton->setStyleSheet("QPushButton {color: #32CD32};");});
    connect(ui->fpsSlider, &QSlider::valueChanged,
            model, &SpriteEditorModel::changeAnimationSpeed);

    // Connections for managing edits (undo, redo)
    connect(ui->actionUndo, &QAction::triggered,
            model, &SpriteEditorModel::undo);
    connect(ui->actionRedo, &QAction::triggered,
            model, &SpriteEditorModel::redo);
    connect(&undoShortcut, &QShortcut::activated,
            model, &SpriteEditorModel::undo);
    connect(&redoShortcut, &QShortcut::activated,
            model, &SpriteEditorModel::redo);

    setCanvasBackground(ui->canvasBackgroundLabel);
    setCanvasBackground(ui->previewBackground);

    // Initialize a first frame
    model->createNewFrame();

    // Initialize the preview frame
    QImage scaledCanvas = model->getFrame(model->getCurrentFrameIndex())
                              ->scaledToWidth(ui->previewLabel->width(), Qt::FastTransformation);
    ui->previewLabel->setPixmap(QPixmap::fromImage(scaledCanvas));
}

/**
 * @brief SpriteEditorView::~SpriteEditorView
 * Destructor. Deallocates memory for the ui object, and destructs this View.
 */
SpriteEditorView::~SpriteEditorView()
{
    delete ui;
}


// ===================================================
// ===              FILE MANIPULATION              ===
// ===================================================

/**
 * @brief SpriteEditorView::saveAsClicked
 * Prompts the user to choose a save location. If the save location is valid, attempt to save the file.
 */
void SpriteEditorView::saveAsClicked()
{
    QString fileName = QFileDialog::getSaveFileName(
                this, "Save As", QDir::homePath());
    if (!fileName.isEmpty())
        model->saveFile(fileName + ".ssp");
}

/**
 * @brief SpriteEditorView::openClicked
 * Prompts the user to choose a file. If the file is valid, attempt to open it.
 */
void SpriteEditorView::openClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, "Open", QDir::homePath(), "SSP files (*.ssp)");
    if (!fileName.isEmpty())
        model->openFile(fileName);
}

/**
 * @brief SpriteEditorView::newClicked
 * Prompts the user to choose a canvas size between 1 and 32 pixels. Attempt to
 * create a new file if the user clicks "ok".
 */
void SpriteEditorView::newClicked()
{
    bool ok;
    int newCanvasSize = QInputDialog::getInt(
                this, "Sprite Editor", "Enter a canvas size 1 - 32:", 16, 1, 32, 1, &ok);
    if (ok)
        model->newFile(newCanvasSize);
}


// ===================================================
// ===                   DRAWING                   ===
// ===================================================

/**
 * @brief SpriteView::drawPixel
 * Converts window coordinates to canvas coordinates and sets the respective
 * pixel to be the given QColor. If the window coordinates are not in the bounds
 * of the canvas, then nothing is drawn.
 *
 * @param point -- window coordinate
 * @param color -- rgb color to set the pixel
 */
void SpriteEditorView::drawPixel(QPoint point, QColor newColor)
{
    int currentFrameIndex = model->getCurrentFrameIndex();
    QImage* currentFrame = model->getFrame(currentFrameIndex);

    // If the color being drawn isn't different than the pixel's color, it's cheaper to not draw at all.
    // This also ensures that undoing/redoing won't put no-effect edits in the edit stack.
    QColor oldColor = (currentFrame->pixelColor(point.x(), point.y())).toHsv();

    // Comparison of rgba values is needed here, since Qt assigns meaningless values to the Hue value of HSV colors
    //  when the color being represented is achromatic (a shade of gray, which includes black and white).
    //  Meaningless H values means two HSV colors can be identical but have different H values.
    if (oldColor.rgba() != newColor.rgba())
    {
        model->addToEdit(point, oldColor, newColor);
        currentFrame->setPixel(point.x(), point.y(), newColor.rgba());

        refreshFrame(currentFrameIndex);
    }
}

/**
 * @brief SpriteView::mousePressEvent
 * On a mouse left click, a pixel is drawn on the canvas.
 *
 * @param event -- mouse click event
 */
void SpriteEditorView::mousePressEvent(QMouseEvent *event)
{
    int leftCanvasX = ui->centerFrame->x() + 10;  // +10 to adjust for mouse alignment
    int leftCanvasY = ui->canvasFrame->y() + 40;  // +40 to adjust for mouse alignment
    double canvasWidth = ui->canvasLabel->width();

    // Check if the event is a mouse left button click, and is in the bounds of the canvas.
    // An edit should begin and a pixel color should be set, only under those conditions.
    if (event->button() == Qt::LeftButton && ui->canvasLabel->underMouse())
    {
        model->beginEdit();
        toggleDraw = true;

        QPoint eventLoc = event->position().toPoint();
        int pixelX = (eventLoc.x() - leftCanvasX) / (canvasWidth / model->getCanvasSize());
        int pixelY = (eventLoc.y() - leftCanvasY) / (canvasWidth / model->getCanvasSize());
        drawPixel(QPoint(pixelX, pixelY), model->getCurrentColor());
    }
}

/**
 * @brief SpriteView::mouseMoveEvent
 * On a mouse drag, a pixel is drawn on the canvas if the mouse is also clicked.
 *
 * @param event -- mouse drag event
 */
void SpriteEditorView::mouseMoveEvent(QMouseEvent *event)
{
    int leftCanvasX = ui->centerFrame->x() + 10;  // +10 to adjust for mouse alignment
    int leftCanvasY = ui->canvasFrame->y() + 40;  // +40 to adjust for mouse alignment
    double canvasWidth = ui->canvasLabel->width();

    QPoint eventLoc = event->position().toPoint();

    // Check if the mouse click is a left button click, drawing is currently happening,
    // and is in the bounds of the canvas. A pixel color should be set only under those conditions.
    if (event->buttons() == Qt::LeftButton && toggleDraw &&
            leftCanvasX < eventLoc.x() && eventLoc.x() < (leftCanvasX + canvasWidth) &&
            leftCanvasY < eventLoc.y() && eventLoc.y() < (leftCanvasY + canvasWidth))
    {
        int pixelX = (eventLoc.x() - leftCanvasX) / (canvasWidth / model->getCanvasSize());
        int pixelY = (eventLoc.y() - leftCanvasY) / (canvasWidth / model->getCanvasSize());
        drawPixel(QPoint(pixelX, pixelY), model->getCurrentColor());
    }
}

/**
 * @brief SpriteView::mouseReleaseEvent
 * On a mouse release, all canvas drawing is stopped.
 *
 * @param event -- mouse release event
 */
void SpriteEditorView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && toggleDraw)
    {
        model->endEdit();
        toggleDraw = false;
    }
}


// ===================================================
// ===                DRAWING TOOLS                ===
// ===================================================

/**
 * @brief SpriteEditorView::handlePenPress
 * On a pen button click, the model sets the current tool
 * to pen and the pen/eraser buttons' visual appearances are
 * changed.
 */
void SpriteEditorView::onPenClick()
{
    model->setTool(SpriteEditorModel::PEN);
    ui->penButton->setDisabled(true);
    ui->eraserButton->setDisabled(false);
}

/**
 * @brief SpriteEditorView::handleEraserPress
 * On an eraser button click, the model sets the current tool
 * to eraser and the pen/eraser buttons' visual appearances are
 * changed.
 */
void SpriteEditorView::onEraserClick()
{
    model->setTool(SpriteEditorModel::ERASER);
    ui->penButton->setDisabled(false);
    ui->eraserButton->setDisabled(true);
}


// ===================================================
// ===                PREVIEW FRAME                ===
// ===================================================

/**
 * @brief SpriteEditorView::displayPreviewFrame
 * Displays a frame given from the model in the preview window.
 *
 * @param previewFrame -- the frame to display
 */
void SpriteEditorView::displayPreviewFrame(QImage *previewFrame)
{
    // If the toggle to show true sprite size is checked, don't scale the canvas
    if(ui->sizeToggle->isChecked())
    {
        QImage canvasBackground(model->getCanvasSize(), model->getCanvasSize(), QImage::Format_RGB32);
        canvasBackground.fill(qRgb(255,255,255));
        QImage scaledBackground = canvasBackground.scaledToWidth(ui->previewBackground->width(), Qt::FastTransformation);
        ui->previewBackground->setPixmap(QPixmap::fromImage(scaledBackground));
        ui->previewLabel->setPixmap(QPixmap::fromImage(*previewFrame));
    }
    else
    {
        setCanvasBackground(ui->previewBackground);
        QImage scaledCanvas = previewFrame->scaledToWidth(ui->previewLabel->width(), Qt::FastTransformation);
        ui->previewLabel->setPixmap(QPixmap::fromImage(scaledCanvas));
    }
}

/**
 * @brief SpriteEditorView::resetPreview
 * Resets the preview pane and controls. The FPS slider is reset to 0, the start button
 * is paused, and the first frame is shown in the display preview.
 */
void SpriteEditorView::resetPreview()
{
    ui->fpsSlider->setValue(0);
    ui->animationStartButton->setText(QString("▶"));
    ui->animationStartButton->setStyleSheet("QPushButton {color: #32CD32};");
    displayPreviewFrame(model->getFrame(0));
}


// ===================================================
// ===             FRAME MANIPULATION              ===
// ===================================================

/**
 * @brief SpriteEditorView::setCanvasBackground
 * Sets the background of the canvas to a checkerboard grid.
 */
void SpriteEditorView::setCanvasBackground(QLabel *background)
{
    // Create the default canvas with the default size (in pixels), setting the background to
    // be visible with a white/grey checkerboard pattern.
    QImage canvasBackground(model->getCanvasSize(), model->getCanvasSize(), QImage::Format_RGB32);

    for(int i = 0; i < model->getCanvasSize(); i++)
    {
        for(int j = 0; j < model->getCanvasSize(); j++)
        {
            if((i + j) % 2 == 0)
                canvasBackground.setPixel(i, j, qRgb(230, 230, 230));
            else
                canvasBackground.setPixel(i, j, qRgb(255, 255, 255));
        }
    }

    // Set up the canvas background
    QImage scaledBackground = canvasBackground.scaledToWidth(background->width(), Qt::FastTransformation);
    background->setPixmap(QPixmap::fromImage(scaledBackground));
}

/**
 * @brief SpriteEditorView::setUpNewFrame
 * When called, this helper method will set the current frame to be filled with blank color.
 * Then, it will set the scaled canvas and canvas label for the current frame.
 */
void SpriteEditorView::setUpNewFrame()
{
    int currentFrameIndex = model->getCurrentFrameIndex();

    // Set up the drawing canvas as blank
    model->getFrame(currentFrameIndex)->fill(Qt::transparent);

    // Setup the frame that the user will draw on
    refreshFrame(currentFrameIndex);
}

/**
 * @brief SpriteEditorView::refreshSelectedFrame
 * This is a helper method/slot that takes the current frame and updates the pixmap to show current changes.
 */
void SpriteEditorView::refreshFrame(int frameIndex)
{
    QImage scaledCanvas = model->getFrame(frameIndex)->scaledToWidth(ui->canvasLabel->width(), Qt::FastTransformation);
    QIcon buttonImage(QPixmap::fromImage(scaledCanvas));
    model->getFrameButton(frameIndex)->setIcon(buttonImage);

    // If the frame being updated is the currently selected one,
    // then the drawing canvas needs to be updated as well.
    if (frameIndex == model->getCurrentFrameIndex())
        updateCanvas(frameIndex);
}

/**
 * @brief SpriteEditorView::updateCanvas
 * Redraws the drawing canvas using the frame associated with the given frame index.
 *
 * @param frameIndex -- the index of the frame to display in the drawing canvas
 */
void SpriteEditorView::updateCanvas(int frameIndex)
{
    QImage scaledCanvas = model->getFrame(frameIndex)->scaledToWidth(ui->canvasLabel->width(), Qt::FastTransformation);
    ui->canvasLabel->setPixmap(QPixmap::fromImage(scaledCanvas));
}

/**
 * @brief SpriteEditorView::setUpFrameButton
 * This method is used to initialize a QPushButton image. It takes the current frame and changes it into an image
 * of a QPushButton that represents the frame.
 */
void SpriteEditorView::setUpFrameButton()
{
    QImage scaledCanvas = model->getFrame(model->getCurrentFrameIndex())
                               ->scaledToWidth(ui->canvasLabel->width(), Qt::FastTransformation);
    QIcon buttonImage(QPixmap::fromImage(scaledCanvas));

    QPushButton* currentFrameButton = model->getFrameButton(model->getCurrentFrameIndex());

    ui->scrollLayout->addWidget( currentFrameButton );

    currentFrameButton->setIcon(buttonImage);
    currentFrameButton->setIconSize(QSize(93,93));
    currentFrameButton->setFixedSize(QSize(93,93));
}

/**
 * @brief SpriteEditorView::clearCurrentFrame
 * This slot fills the current frame with empty/clear pixels, effectively clearing it.
 * Then, it updates the pixmap, displaying the changes.
 */
void SpriteEditorView::clearCurrentFrame()
{
    int currentFrameIndex = model->getCurrentFrameIndex();
    model->getFrame(currentFrameIndex)->fill(qRgba(0, 0, 0, 0));

    refreshFrame(currentFrameIndex);
}

/**
 * @brief SpriteEditorView::warnAboutClearFrame
 * Displays the warning about Clear Frame removing edits and takes appropriate action
 * based on the user's response.
 */
void SpriteEditorView::warnAboutClearFrame()
{
    QMessageBox msgbox;
    msgbox.setWindowTitle("WARNING");
    msgbox.setIcon(QMessageBox::Warning);
    msgbox.setText("Warning! This will clear all edit history for this frame.\n\nContinue?");
    msgbox.setStandardButtons(QMessageBox::Yes|QMessageBox::Cancel);
    msgbox.setDefaultButton(QMessageBox::Cancel);

    // If the user selects "Yes", then the frame is cleared.
    if(msgbox.exec() == QMessageBox::Yes)
    {
        clearCurrentFrame();
        model->clearEditsOnCurrentFrame();
    }
}

/**
 * @brief SpriteEditorView::warnAboutDeletion
 * If the animation of the preview frame is running, prevent the user from
 * deleting any frames. Deleting a frame with the animation running easily creates
 * index out of bounds errors.
 */
void SpriteEditorView::warnAboutDeletion()
{
    QMessageBox msgbox;
    msgbox.setWindowTitle("WARNING");
    msgbox.setIcon(QMessageBox::Warning);
    msgbox.setText("Warning! You cannot delete a frame while the animation is running.");
    msgbox.setStandardButtons(QMessageBox::Cancel);
    msgbox.setDefaultButton(QMessageBox::Cancel);
    msgbox.exec();
}
