/*
 * Team             |   Chandler
 * Members          |   Braden Fiedel, Caden Erickson, Connor Blood, Josie Fiedel
 * Class            |   CS 3505
 * Project          |   A7: Sprite Editor Implementation
 * Last modified    |   April 3, 2023
 * Style Reviewer   |   Connor Blood
 *
 * This file contains the implementation of the class definition located in colorpicker.h.
 */

#include "colorpicker.h"
#include "ui_colorpicker.h"


/**
 * @brief ColorPicker::ColorPicker
 * Constructor. Creates a new ColorPicker object and
 * sets up the object's UI.
 *
 * @param parent
 */
ColorPicker::ColorPicker(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorPicker)
{
    ui->setupUi(this);

    // Connections for a change in the hue slider value:
    connect(ui->gradientHueSlider, &QSlider::valueChanged,
            this, &ColorPicker::changeGradientHue);
    connect(ui->gradientHueSlider, &QSlider::sliderReleased,
            this, [this](){updateCurrentColor(currPt);});

    // Connections for clickable buttons:
    connect(ui->color1, &QPushButton::clicked,
            this, [this](){setRecentColorSelection(ui->color1);});
    connect(ui->color2, &QPushButton::clicked,
            this, [this](){setRecentColorSelection(ui->color2);});
    connect(ui->color3, &QPushButton::clicked,
            this, [this](){setRecentColorSelection(ui->color3);});
    connect(ui->color4, &QPushButton::clicked,
            this, [this](){setRecentColorSelection(ui->color4);});
    connect(ui->color5, &QPushButton::clicked,
            this, [this](){setRecentColorSelection(ui->color5);});

    // The current clicked point is set to the black part of the canvas.
    // The offset of x+2 and y-3 improves the selection's visual appearance.
    currPt = QPoint(ui->gradientBackground->x() + 2,
                    ui->gradientBackground->y() + ui->gradientBackground->width() - 3);

    // The recent colors list starts off with the default black.
    drawRecentColors(QList<QColor>{getColorFromCurrentPoint()});
}

/**
 * @brief ColorPicker::~ColorPicker
 * Destructor. Deallocates memory for the ui object, and destructs this View.
 */
ColorPicker::~ColorPicker()
{
    delete ui;
}

/**
 * @brief ColorPicker::isInGradientBounds
 * Checks if the given point is within bounds of the gradient square.
 *
 * @param point -- current point to check
 * @return true if within bounds, false otherwise.
 */
bool ColorPicker::isInGradientBounds(QPoint point)
{
    int leftPaletteX = ui->gradientBackground->x() + ui->colorPicker->x();
    int leftPaletteY = ui->gradientBackground->y() + ui->colorPicker->y();
    int paletteWidth = ui->gradientBackground->width();

    return leftPaletteX < point.x() && point.x() < (leftPaletteX + paletteWidth) &&
                leftPaletteY < point.y() && point.y() < (leftPaletteY + paletteWidth);
}

/**
 * @brief ColorPicker::mousePressEvent
 * On a mouse click, if the point is within the bounds of the gradient square, then
 * the color is updated with the corresponding color at the currently-selected point.
 *
 * @param event -- a mouse click
 */
void ColorPicker::mousePressEvent(QMouseEvent *event)
{
    QPoint clickPt = event->position().toPoint();
    if(event->button() == Qt::LeftButton && isInGradientBounds(clickPt) && clickPt != currPt)
        updateCurrentColor(clickPt);
}

/**
 * @brief ColorPicker::mouseMoveEvent
 * On a mouse move, if the point is within the bounds of the gradient square, then
 * the currently-selected point is updated to be this point.
 *
 * @param event -- a mouse drag
 */
void ColorPicker::mouseMoveEvent(QMouseEvent *event)
{
    QPoint movePt = event->position().toPoint();
    if(event->buttons() == Qt::LeftButton && isInGradientBounds(movePt) && movePt != currPt)
        currPt = movePt;
}

/**
 * @brief ColorPicker::mouseReleaseEvent
 * On a mouse release, the current color is updated with the corresponding color
 * at the currently-selected point.
 *
 * @param event -- a mouse release
 */
void ColorPicker::mouseReleaseEvent(QMouseEvent *)
{
    updateCurrentColor(currPt);
}

/**
 * @brief ColorPicker::updateCurrentColor
 * Updates the current HSV color given by the currently-selected point. The
 * model is notified of this change.
 *
 * @param point -- point in which the color is calculated from
 */
void ColorPicker::updateCurrentColor(QPoint point)
{
    currPt = point;
    emit updateNewColor(getColorFromCurrentPoint());
}

/**
 * @brief ColorPicker::getColorFromCurrentPoint
 * Updates the current color depending on the currently-selected point. When
 * determining the HSV color form, the hue comes from the gradient slider
 * position, the saturation comes from the x-location with respect to the gradient
 * square, and the value comes from the y-location with respect to the gradient square.
 *
 * @return selectedColor -- the current color at the currently-selected point.
 */
QColor ColorPicker::getColorFromCurrentPoint()
{
    // Scales the color values to be evenly-distributed across the gradient square.
    double colorScale = MAX_COLOR_VAL / double(ui->gradientBackground->width());

    QColor selectedColor;
    selectedColor.setHsv(MAX_COLOR_HUE - ui->gradientHueSlider->sliderPosition(),
                        (currPt.x() - ui->gradientBackground->x()) * colorScale,
                        MAX_COLOR_VAL - ((currPt.y() - ui->gradientBackground->y()) * colorScale));

    return selectedColor;
}

/**
 * @brief ColorPicker::changeGradientHue
 * As the slider is moved, the hue is changed, which corresponds to
 * changing the hue of the color gradient square and updating the
 * gradient's stylesheets.
 *
 * @param value -- the color hue in range of 0 - 359, inclusive.
 */
void ColorPicker::changeGradientHue(int value)
{
    QColor color;
    color.setHsv(MAX_COLOR_HUE - value, MAX_COLOR_VAL, MAX_COLOR_VAL);

    QString colorHSV = QString("hsv(%1, %2, %3), ").arg(color.hue()).arg(color.saturation()).arg(color.value());
    QString colorStyle = QString("background-color: qlineargradient(spread:pad, x1:1, y1:1, x2:0, y2:0, "
                                 "stop:0 %1 %2 %3 %4").arg(colorHSV, "stop:0.45", colorHSV, "stop:1 hsv(0, 0, 255));");
    ui->gradientBackground->setStyleSheet(colorStyle);
}

/**
 * @brief ColorPicker::drawRecentColors
 * Changes the background colors of the "recent colors" buttons by stylesheets to
 * represent which colors on the color gradient have been selected most recently.
 *
 * @param recentColors -- list of recently-selected colors.
 */
void ColorPicker::drawRecentColors(QList<QColor> recentColors)
{
    // Store all "recent colors" buttons in a list for easy access.
    QList<QPushButton*> buttons = ui->recentColors->findChildren<QPushButton*>();
    for(int i = 0; i < buttons.count(); i++)
    {
        // While there exist colors in the recent colors list:
        if(recentColors.count() > i)
        {
            // The color is taken starting from the end of the list (most recent).
            QColor color = recentColors.at(recentColors.count() - (i + 1));
            QString colorHSV = QString("hsv(%1, %2, %3)").arg(color.hue()).arg(color.saturation()).arg(color.value());
            QString buttonStyle = QString("background-color: %1; border-width: 2; border-radius: 5; "
                    "border-style: solid; border-color: rgb(0, 0, 0); padding: -2px; ").arg(colorHSV);
            buttons.at(i)->setStyleSheet(buttonStyle);
        }
        // Otherwise, set the remaining buttons to be transparent.
        else
        {
            QString buttonStyle = QString("background-color: transparent; border-width: 2; border-radius: 5; "
                    "border-style: solid; border-color: rgb(0, 0, 0); padding: -2px; ");
            buttons.at(i)->setStyleSheet(buttonStyle);
        }
    }
}

/**
 * @brief ColorPicker::setRecentColorSelection
 * When a "recent colors" button is clicked, the color gradient and hue slider
 * are updated to match this button's background HSV color.
 *
 * @param button -- the clicked button
 */
void ColorPicker::setRecentColorSelection(QPushButton *button)
{
    // Update the gradient slider to match the color's hue.
    QColor selectedColor = button->palette().color(button->backgroundRole());
    ui->gradientHueSlider->setValue(MAX_COLOR_HUE - selectedColor.hue());

    // Update the currently-selected point depending on the color's saturation and value components.
    // "colorScale" scales the color values to be evenly-distributed across the gradient square.
    double colorScale = MAX_COLOR_VAL / double(ui->gradientBackground->width());
    currPt.setX((selectedColor.saturation() / colorScale) + ui->gradientBackground->x());
    currPt.setY(((MAX_COLOR_VAL - selectedColor.value()) / colorScale) + ui->gradientBackground->y());

    emit updateNewColor(selectedColor);
}

/**
 * @brief ColorPicker::resetColorPalette
 * Resets the color palette to its default state. (i.e. the gradient slider is reset,
 * the selection circle moves back to the bottom left, the color hue changes to red,
 * and the recent colors are replaced with a single black color swatch.)
 */
void ColorPicker::resetColorPalette()
{
    // The currently-selected point is set to the default position.
    currPt = QPoint(ui->gradientBackground->x() + 2,
                       ui->gradientBackground->y() + ui->gradientBackground->width() - 3);

    QColor currColor = getColorFromCurrentPoint();
    drawRecentColors(QList<QColor>{currColor});
    ui->gradientHueSlider->setValue(0);

    emit updateNewColor(currColor);
}

/**
 * @brief ColorPicker::paintEvent
 * Draws a circle on the gradient to represent the currently-selected color.
 */
void ColorPicker::paintEvent(QPaintEvent *)
{
    // Set up the transparent layer for painting.
    QPixmap drawLayer(ui->selectionLayer->width(), ui->selectionLayer->width());
    drawLayer.fill(Qt::transparent);
    QPainter painter(&drawLayer);

    // A black circle is drawn at the currently selected location.
    QPen pen(Qt::black);
    pen.setWidth(2);
    painter.setPen(pen);
    int radius = 6;
    painter.drawEllipse(QPointF(currPt.x(), currPt.y()), radius, radius);

    // An inner white circle is drawn at the currently selected location.
    pen.setColor(Qt::white);
    painter.setPen(pen);
    painter.drawEllipse(QPointF(currPt.x(), currPt.y()), radius - 2, radius - 2);

    ui->selectionLayer->setPixmap(drawLayer);
}
