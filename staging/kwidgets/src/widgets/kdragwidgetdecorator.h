/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDRAGWIDGETDECORATOR_H
#define KDRAGWIDGETDECORATOR_H

#include <QObject>

#include <kwidgets_export.h>

class QDrag;
class KDragWidgetDecoratorPrivate;


/**
 * @brief A decorator which adds drag-support to widgets
 *
 * This is a decorator using an event filter to implement drag-support
 * in widgets.
 * You must override the virtual method dragObject() to specify the
 * QDrag to be used.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KWIDGETS_EXPORT KDragWidgetDecorator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isDragEnabled READ isDragEnabled WRITE setDragEnabled)

public:

    /**
     * Default constructor.
     */
    explicit KDragWidgetDecorator(QWidget *parent = 0);

    /**
     * Destructs the decorator.
     */
    ~KDragWidgetDecorator();

    /**
     * Enables/disables drag-support. Default is enabled.
     */
    void setDragEnabled(bool enable);

    /**
     * @returns if drag support is enabled or not.
     */
    bool isDragEnabled() const;

protected:
    /**
     * @return the widget this decorator is attached to
     */
    QWidget *decoratedWidget() const;

    /**
     * Reimplement this and return the QDrag object that should be used
     * for the drag. Remember to give it "decoratedWidget()" as parent.
     *
     * Default implementation returns 0, so that no drag is initiated.
     */
    virtual QDrag *dragObject();

    /**
     * Reimplemented to add drag-support
     */
    virtual bool eventFilter(QObject *watched, QEvent *event);

    /**
     * Starts a drag (Copy by default) using dragObject()
     */
    virtual void startDrag();

private:
    KDragWidgetDecoratorPrivate * const d;
};

#endif // KDRAGWIDGETDECORATOR_H
