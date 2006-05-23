/* This file is part of the KDE project
   Copyright (C) 2006  Olivier Goffart  <ogoffart@kde.org>

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

#include "kmenumenuhandler_p.h"

#include "kxmlguibuilder.h"
#include "kmenu.h"
#include "kaction.h"
#include "kactioncollection.h"
#include "kshortcutdialog.h"
#include <klocale.h>


#include <QWidget>
#include <kapplication.h>
#include <kmainwindow.h>
#include <ktoolbar.h>
#include <kselectaction.h>

namespace KDEPrivate {

KMenuMenuHandler::KMenuMenuHandler( KXMLGUIBuilder *builder ) 
  : QObject() , m_builder(builder) 
{
  m_toolbarAction=new KSelectAction(i18n("Add to toolbar") , 0 , QString());
  connect(m_toolbarAction , SIGNAL(triggered(int)) , this , SLOT(slotAddToToolBar(int)));
}
  
  

void KMenuMenuHandler::insertKMenu( KMenu *popup )
{
  popup->contextMenu()->addAction( i18n("Configure Shortcut") , this , SLOT( slotSetShortcut() ));
  
  KMainWindow *window=qobject_cast<KMainWindow*>(m_builder->widget());
  if(window)
  {
    popup->contextMenu()->addAction( m_toolbarAction );
    connect(popup, SIGNAL(aboutToShowContextMenu(KMenu*,QAction*,QMenu*)) , this, SLOT(buildToolbarAction()));
  }
}



void KMenuMenuHandler::buildToolbarAction()
{
  KMainWindow *window=qobject_cast<KMainWindow*>(m_builder->widget());
  if(!window)
    return;
  QStringList toolbarlist;
  foreach(KToolBar *b , window->toolBars())
  {
    toolbarlist << (b->windowTitle().isEmpty() ? b->objectName() : b->windowTitle());
  }
  m_toolbarAction->setItems(toolbarlist);
}


void KMenuMenuHandler::slotSetShortcut()
{
  KMenu * menu=KMenu::contextMenuFocus();
  if(!menu)
    return;
  KAction *action= qobject_cast<KAction*>(menu->contextMenuFocusAction());
  if(!action)
    return;

  KShortcutDialog dialog(action->shortcut() , m_builder->widget() );
  dialog.exec();
  action->setShortcut(dialog.shortcut(), KAction::CustomShortcut); 

  if(action->parentCollection())
    action->parentCollection()->writeSettings();
}


void KMenuMenuHandler::slotAddToToolBar(int tb)
{
  KMainWindow *window=qobject_cast<KMainWindow*>(m_builder->widget());
  if(!window)
    return;
  
  KMenu * menu=KMenu::contextMenuFocus();
  if(!menu)
    return;
  KAction *action= qobject_cast<KAction*>(menu->contextMenuFocusAction());
  if(!action)
    return;


  KToolBar *toolbar=window->toolBars()[tb];
  
  
  toolbar->addAction(action);  
  
}

        
        
} //END namespace KDEPrivate

#include "kmenumenuhandler_p.moc"
