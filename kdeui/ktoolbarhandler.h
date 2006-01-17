/* This file is part of the KDE libraries
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>

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

#ifndef KBARHANDLER_H
#define KBARHANDLER_H

#include <QLinkedList>
#include <qobject.h>
#include <qpointer.h>
#include <kxmlguiclient.h>

class KMainWindow;
class KToolBar;

namespace KDEPrivate
{

/// @since 3.1
class ToolBarHandler : public QObject,
                       public KXMLGUIClient
{
    Q_OBJECT
public:
    ToolBarHandler( KMainWindow *mainWindow );
    ToolBarHandler( KMainWindow *mainWindow, QObject *parent );
    virtual ~ToolBarHandler();

    KAction *toolBarMenuAction();

public Q_SLOTS:
    void setupActions();

private Q_SLOTS:
    void clientAdded( KXMLGUIClient *client );

private:
    void init( KMainWindow *mainWindow );
    void connectToActionContainers();
    void connectToActionContainer( KAction *action );
    void connectToActionContainer( QWidget *container );

    struct Data;
    Data *d;

    QPointer<KMainWindow> m_mainWindow;
    QList<KAction*> m_actions;
    QLinkedList<KToolBar*> m_toolBars;
};

} // namespace KDEPrivate

#endif // KBARHANDLER_H

/* vim: et sw=4 ts=4
 */
