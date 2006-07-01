/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef K3WIDGETACTION
#define K3WIDGETACTION

#include <kaction.h>

#include <QPointer>

/**
 * An action that automatically embeds a widget into a
 * toolbar.
 *
 * \deprecated This action can only be plugged into one toolbar at
 *             a time.  Instead, create your own subclass of KAction,
 *             and provide a QActionWidgetFactory to create widgets
 *             on request.
 */
class KDEUI_EXPORT K3WidgetAction : public KAction
{
    Q_OBJECT
public:
    /**
     * Create an action that will embed widget into a toolbar
     * when plugged. This action may only be plugged into
     * ONE toolbar.
     */
    KDE_CONSTRUCTOR_DEPRECATED K3WidgetAction( QWidget* widget, const QString& text,
                   KActionCollection* parent, const QString& name );

    /**
     * Create an action that will embed widget into a toolbar
     * when plugged. This action may only be plugged into
     * ONE toolbar.
     */
    KDE_CONSTRUCTOR_DEPRECATED K3WidgetAction( QWidget* widget, const QString& text,
                   const KShortcut& cut,
                   const QObject* receiver, const char* slot,
                   KActionCollection* parent, const QString& name );
    virtual ~K3WidgetAction();

    /**
     * Returns the widget associated with this action.
     */
    KDE_DEPRECATED QWidget* widget() const { return m_widget; }

protected:
    virtual bool event(QEvent* event);

private:
    QPointer<QWidget> m_widget;

    class K3WidgetActionPrivate* const d;
};

#endif
