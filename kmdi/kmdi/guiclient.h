/* This file is part of the KDE libraries
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2004 Christoph Cullmann <cullmann@kde.org>
   based on ktoolbarhandler.cpp: Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>

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

#ifndef _KMDI_GUICLIENT_H_
#define _KMDI_GUICLIENT_H_

#include <qobject.h>
#include <qguardedptr.h>
#include <kxmlguiclient.h>
#include <kaction.h>

#include <kmdi/global.h>

class KMainWindow;
class KToolBar;

namespace KMDI {
  class MainWindow;
  class ToolViewAccessor;
}

class KDockWidget;

namespace KMDIPrivate {

class GUIClientPrivate;

class GUIClient : public QObject, public KXMLGUIClient
{
  Q_OBJECT

  public:
    GUIClient( KMDI::MainWindow *mdiMainFrm, const char *name = 0 );
    virtual ~GUIClient();

    void addToolView(KMDI::ToolViewAccessor*);

  private slots:
    void clientAdded( KXMLGUIClient *client );
    void setupActions();
    void actionDeleted(QObject*);

  signals:
    void toggleTop();
    void toggleLeft();
    void toggleRight();
    void toggleBottom();

  private:
    GUIClientPrivate *d;

    QGuardedPtr<KMDI::MainWindow> m_mdiMainFrm;
    QPtrList<KAction> m_toolViewActions;
    QPtrList<KAction> m_documentViewActions;

    KActionMenu *m_docMenu;
    KActionMenu *m_toolMenu;

    KActionMenu *m_gotoToolDockMenu;
};

class ToggleToolViewAction:public KToggleAction
{
  Q_OBJECT

  public:
    ToggleToolViewAction ( const QString& text, const KShortcut& cut = KShortcut(),
                           KDockWidget *dw=0,KMDI::MainWindow *mdiMainFrm=0, QObject* parent = 0, const char* name = 0 );

    virtual ~ToggleToolViewAction();

  protected slots:
    void slotToggled(bool);
    void anDWChanged();
    void slotWidgetDestroyed();

  private:
    KDockWidget *m_dw;
    KMDI::MainWindow *m_mdiMainFrm;
};

}

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
