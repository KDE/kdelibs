/*
 *   Copyright © 2008 Fredrik Höglund <fredrik@kde.org>
 *   Copyright © 2008 Marco Martin <notmart@gmail.com>
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

#ifndef PLASMA_SCROLLBAR_H
#define PLASMA_SCROLLBAR_H

#include <QScrollBar>
#include <QGraphicsProxyWidget>

#include <plasma/plasma_export.h>

namespace Plasma
{

class ScrollBarPrivate;

/**
 * @class ScrollBar plasma/widgets/scrollbar.h <Plasma/Widgets/ScrollBar>
 *
 * @short Provides a plasma-themed QScrollBar.
 */
class PLASMA_EXPORT ScrollBar : public QGraphicsProxyWidget
{
    Q_OBJECT

    Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(int pageStep READ pageStep WRITE setPageStep)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet)
    Q_PROPERTY(QScrollBar *nativeWidget READ nativeWidget)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)

public:
    /**
     * Creates a scrollbar; the default orientation is vertical
     */
    explicit ScrollBar(QGraphicsWidget *parent=0);

    ~ScrollBar();

    /**
     * Sets the scrollbar minimum and maximum values
     * @param min minimum value
     * @param max maximum value
     */
    void setRange(int min, int max);

    /**
     * Sets the amount of the single step
     * i.e how much the slider will move when the user press an arrow button
     * @param val
     */
    void setSingleStep(int val);

    /**
     * @return the amount of the single step
     */
    int singleStep();

    /**
     * Sets the amount the slider will scroll when the user press page up or page down
     * @param val
     */
    void setPageStep(int val);

    /**
     * @return the amount of the page step
     */
    int pageStep();

    /**
     * @return the current scrollbar value
     */
    int value() const;

    /**
     * @return the minimum value bound of this ScrollBar
     */
    int minimum() const;

    /**
     * @return the maximum value bound of this ScrollBar
     */
    int maximum() const;

    /**
     * @param the minimum value bound of this ScrollBar
     * @since 4.6
     */
    void setMinimum(const int min) const;

    /**
     * @param the maximum value bound of this ScrollBar
     * @since 4.6
     */
    void setMaximum(const int max) const;

    /**
     * Sets the stylesheet used to control the visual display of this ScrollBar
     *
     * @param stylesheet a CSS string
     */
    void setStyleSheet(const QString &stylesheet);

    /**
     * @return the stylesheet currently used with this widget
     */
    QString styleSheet();

    /**
     * @return the native widget wrapped by this ScrollBar
     */
    QScrollBar *nativeWidget() const;

    /**
     * @return the orientation of the scrollbar
     * @since 4.4
     */
    Qt::Orientation orientation() const;

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

public Q_SLOTS:
    /**
     * Sets the current value for the ScrollBar
     * @param value must be minimum() <= value <= maximum()
     */
    void setValue(int val);

    /**
     * Sets the orientation of the ScrollBar.
     */
    void setOrientation(Qt::Orientation orientation);

Q_SIGNALS:
    /**
     * Emitted when the value of the slider changes
     */
    void valueChanged(int value);

    /**
     * Emitted when the slider has been moved by the user
     * @since 4.6
     */
    void sliderMoved(int value);

private:
    ScrollBarPrivate * const d;
};

}

#endif
