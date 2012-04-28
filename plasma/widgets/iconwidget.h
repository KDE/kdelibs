/*
*   Copyright (C) 2007 by Siraj Razick <siraj@kde.org>
*   Copyright (C) 2007 by Riccardo Iaconelli <riccardo@kde.org>
*   Copyright (C) 2007 by Matt Broadstone <mbroadst@gmail.com>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef PLASMA_ICONWIDGET_H
#define PLASMA_ICONWIDGET_H

#include <QtCore/QObject>
#include <QtCore/QWeakPointer>
#include <QGraphicsTextItem>
#include <QIcon>
#include <QGraphicsWidget>

#include <plasma/dataengine.h>
#include <plasma/animator.h>
#include <plasma/plasma_export.h>

class QAction;
class QPropertyAnimation;

/**
 * @class IconWidget plasma/widgets/iconwidget.h <Plasma/Widgets/IconWidget>
 *
 * @short Provides a generic icon.
 *
 * An icon, in this sense, is not restricted to just an image, but can also
 * contain text. Currently, the IconWidget class is primarily used for desktop items,
 * but is designed to be used anywhere an icon is needed in an applet.
 *
 * @author Siraj Razick <siraj@kde.org>
 * @author Matt Broadstone <mbroadst@gmail.com>
 */
namespace Plasma
{

class IconWidgetPrivate;

class PLASMA_EXPORT IconWidget : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString infoText READ infoText WRITE setInfoText)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(QColor textBackgroundColor READ textBackgroundColor WRITE setTextBackgroundColor)
    Q_PROPERTY(QSizeF iconSize READ iconSize)
    Q_PROPERTY(QString svg READ svg WRITE setSvg)
    Q_PROPERTY(bool drawBackground READ drawBackground WRITE setDrawBackground)
    Q_PROPERTY(QAction *action READ action WRITE setAction)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(int numDisplayLines READ numDisplayLines WRITE setNumDisplayLines)
    Q_PROPERTY(QSizeF preferredIconSize READ preferredIconSize WRITE setPreferredIconSize)
    Q_PROPERTY(QSizeF minimumIconSize READ minimumIconSize WRITE setMinimumIconSize)
    Q_PROPERTY(QSizeF maximumIconSize READ maximumIconSize WRITE setMaximumIconSize)

public:
    /**
    * Creates a new Plasma::IconWidget.
    * @param parent the QGraphicsItem this icon is parented to.
    */
    explicit IconWidget(QGraphicsItem *parent = 0);

    /**
    * Convenience constructor to create a Plasma::IconWidget with text.
    * @param text the text that will be displayed with this icon.
    * @param parent the QGraphicsItem this icon is parented to.
    */
    explicit IconWidget(const QString &text, QGraphicsItem *parent = 0);

    /**
    * Creates a new Plasma::IconWidget with text and an icon.
    * @param icon the icon that will be displayed with this icon.
    * @param text the text that will be displayed with this icon.
    * @param parent The QGraphicsItem this icon is parented to.
    */
    IconWidget(const QIcon &icon, const QString &text, QGraphicsItem *parent = 0);

    /**
    * Destroys this Plasma::IconWidget.
    */
    virtual ~IconWidget();

    /**
    * Returns the text associated with this icon.
    */
    QString text() const;

    /**
    * Sets the text associated with this icon.
    * @param text the text to associate with this icon.
    */
    void setText(const QString &text);

    /**
    * Convenience method to set the svg image to use when given the filepath and name of svg.
    * @param svgFilePath the svg filepath including name of the svg.
    * @param svgIconElement the svg element to use when displaying the svg. Defaults to all of them.
    */
    void setSvg(const QString &svgFilePath, const QString &svgIconElement = QString());

    /**
     * @return the path to the svg file set, if any
     */
    QString svg() const;

    /**
    * Returns the meta text associated with this icon.
    */
    QString infoText() const;

    /**
    * Sets the additional information to be displayed by
    * this icon.
    * @param text additional meta text associated with this icon.
    */
    void setInfoText(const QString &text);

    /**
    * @return the icon associated with this icon.
    */
    QIcon icon() const;

    /**
    * Sets the graphical icon for this Plasma::IconWidget.
    * @param icon the KDE::icon to associate with this icon.
    */
    void setIcon(const QIcon &icon);

    /**
     * @return the color to use behind the text of the icon
     * @since 4.3
     */
    QColor textBackgroundColor() const;

    /**
     * Sets the color to use behind the text of the icon
     * @param color the color, or QColor() to reset it to no background color
     * @since 4.3
     */
    void setTextBackgroundColor(const QColor &color);

    /**
    * Convenience method to set the icon of this Plasma::IconWidget
    * using a QString path to the icon.
    * @param icon the path to the icon to associate with this Plasma::IconWidget.
    */
    Q_INVOKABLE void setIcon(const QString &icon);

    /**
    * @return the size of this Plasma::IconWidget's graphical icon.
    */
    QSizeF iconSize() const;

    /**
     * Set the size you prefer the icon will be when positioned in a layout.
     * @param preferred icon size, pass an invalid size to unset this value
     *
     * @since 4.5
     */
    void setPreferredIconSize(const QSizeF &size);

    /**
     * @return The size you prefer the icon will be when positioned in a layout.
     *         The default is QSizeF(-1, -1); an invalid size means the icon
     *         will attempt to be at its default  and "optimal" size
     *
     * @since 4.5
     */
    QSizeF preferredIconSize() const;

    /**
     * Set the size that should be the minimum beyond the icon shouldn't scale when
     * the icon will be when positioned in a layout.
     * @param preferred icon size, pass an invalid size to unset this value
     *
     * @since 4.5
     */
    void setMinimumIconSize(const QSizeF &size);

    /**
     * @return The size that should be the minimum beyond the icon shouldn't scale when
     *         the icon will be when positioned in a layout.
     *         The default is QSizeF(-1, -1); an invalid size means the icon
     *         will attempt to be at its default  and "optimal" size
     *
     * @since 4.5
     */
    QSizeF minimumIconSize() const;

    /**
     * Set the size that should be the maximum beyond the icon shouldn't scale when
     * the icon will be when positioned in a layout.
     * @param preferred icon size, pass an invalid size to unset this value
     *
     * @since 4.5
     */
    void setMaximumIconSize(const QSizeF &size);

    /**
     * @return The size that should be the maximum beyond the icon shouldn't scale when
     *         the icon will be when positioned in a layout.
     *         The default is QSizeF(-1, -1); an invalid size means the icon
     *         will attempt to be at its default  and "optimal" size
     *
     * @since 4.5
     */
    QSizeF maximumIconSize() const;

    /**
    * Plasma::IconWidget allows the user to specify a number of actions
    * (currently four) to be displayed around the widget. This method
    * allows for a created QAction to be added to the Plasma::IconWidget.
    * @param action the QAction to associate with this icon.
    */
    void addIconAction(QAction *action);

    /**
     * Removes a previously set iconAction. The action will be removed from the widget
     * but will not be deleted.
     *
     * @param the QAction to be removed, if 0 all actions will be removed
     */
    void removeIconAction(QAction *action);

    /**
     * Associate an action with this IconWidget
     * this makes the IconWidget follow the state of the action, using its icon, text, etc.
     * when the IconWidget is clicked, it will also trigger the action.
     * Unlike addIconAction, there can be only one associated action.
     */
    void setAction(QAction *action);

    /**
     * @return the currently associated action, if any.
     */
    QAction *action() const;

    /**
    * let set the orientation of the icon
    * Qt::Vertical: text under the icon
    * Qt::Horizontal text at a side of the icon depending
    * by the direction of the language
    * @param orientation the orientation we want
    */
    void setOrientation(Qt::Orientation orientation);

    /**
     * @return the orientation of the icon
     */
    Qt::Orientation orientation() const;

    /**
    * inverts the layout of the icons if the orientation is horizontal,
    * normally we get icon on the left with left-to-right languages
    * @param invert if we want to invert the layout of icons
    */
    void invertLayout(bool invert);

    /**
    * @return if the layout of the icons should appear inverted or not
    */
    bool invertedLayout() const;

    /**
    * @return optimal size given a size for the icon
    * @param  iconWidth desired width of the icon
    */
    Q_INVOKABLE QSizeF sizeFromIconSize(const qreal iconWidth) const;

    /**
    * @return the number of lines allowed to display
    */
    int numDisplayLines();

    /**
    * @param numLines the number of lines to show in the display.
    */
    void setNumDisplayLines(int numLines);

    /**
     * Sets whether or not to draw a background area for the icon
     *
     * @param draw true if a background should be drawn or not
     */
    void setDrawBackground(bool draw);

    /**
     * @return true if a background area is to be drawn for the icon
     */
    bool drawBackground() const;

    /**
     * reimplemented from QGraphicsItem
     */
    QPainterPath shape() const;

public Q_SLOTS:
    /**
    * Sets the appearance of the icon to pressed or restores the appearance
    * to normal. This does not simulate a mouse button press.
    * @param pressed whether to appear as pressed (true) or as normal (false)
    */
    void setPressed(bool pressed = true);

    /**
    * Shortcut for setPressed(false)
    */
    void setUnpressed();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

Q_SIGNALS:
    /**
    * Indicates when the icon has been pressed.
    */
    void pressed(bool down);

    /**
    * Indicates when the icon has been clicked.
    */
    void clicked();

    /**
    * Indicates when the icon has been double-clicked
    */
    void doubleClicked();

    /**
    * Indicates when the icon has been activated following the single
    * or doubleclick settings
    */
    void activated();

    /**
     * Indicates that something about the icon may have changed (image, text, etc)
     * only actually works for icons associated with an action
     */
    void changed();

protected:
    bool isDown();
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
    void changeEvent(QEvent *event);

public:
    /**
    * @internal
    **/
    void drawActionButtonBase(QPainter *painter, const QSize &size, int element);

private:
    Q_PRIVATE_SLOT(d, void syncToAction())
    Q_PRIVATE_SLOT(d, void clearAction())
    Q_PRIVATE_SLOT(d, void svgChanged())
    Q_PRIVATE_SLOT(d, void actionDestroyed(QObject *obj))
    Q_PRIVATE_SLOT(d, void hoverAnimationFinished())
    Q_PRIVATE_SLOT(d, void colorConfigChanged())
    Q_PRIVATE_SLOT(d, void iconConfigChanged())
    Q_PRIVATE_SLOT(d, void setPalette())

    IconWidgetPrivate * const d;
    friend class IconWidgetPrivate;
    friend class PopupAppletPrivate;
};

} // namespace Plasma

#endif
