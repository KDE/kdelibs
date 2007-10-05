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
#include "kxmlguiclient.h"
#include "kxmlguifactory.h"
#include "kmenu.h"
#include "kaction.h"
#include "kactioncollection.h"
#include <kdialog.h>
#include <kshortcutwidget.h>
#include <klocale.h>
#include <kdebug.h>


#include <QWidget>
#include <QDomDocument>
#include <QDomNode>
#include <kapplication.h>
#include <kmainwindow.h>
#include <ktoolbar.h>
#include <kselectaction.h>


namespace KDEPrivate {

KMenuMenuHandler::KMenuMenuHandler( KXMLGUIBuilder *builder )
  : QObject() , m_builder(builder)
{
  m_toolbarAction = new KSelectAction(i18n("Add to toolbar"), this);
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
    
    KDialog dialog(m_builder->widget());
    KShortcutWidget swidget(&dialog);
    swidget.setShortcut(action->shortcut());
    dialog.setMainWidget(&swidget);
    if(dynamic_cast<KXMLGUIClient*>(m_builder))
    {
        QList<QAction*> checklist;
        KXMLGUIFactory *factory=dynamic_cast<KXMLGUIClient*>(m_builder)->factory();
        foreach(KXMLGUIClient *client, factory->clients())
            checklist+=client->actionCollection()->actions();
        swidget.setCheckActionList(checklist);
    }
    
    if(dialog.exec())
    {
        action->setShortcut(swidget.shortcut(), KAction::ActiveShortcut);
        swidget.applyStealShortcut();
        if(KActionCollection *collection = qobject_cast<KActionCollection*>(qvariant_cast<QObject*>(action->property("_k_ActionCollection"))))
            collection->writeSettings();
    }
}


void KMenuMenuHandler::slotAddToToolBar(int tb)
{
    KMainWindow *window=qobject_cast<KMainWindow*>(m_builder->widget());
    if(!window)
        return;

    KMenu * menu=KMenu::contextMenuFocus();
    if(!menu)
        return;
    QAction *action= qobject_cast<QAction*>(menu->contextMenuFocusAction());
    if(!action)
        return;
    
    QString actionName = action->objectName(); // set by KActionCollection::addAction
    KActionCollection *collection= qobject_cast<KActionCollection*>(qvariant_cast<QObject*>(action->property("_k_ActionCollection")));
    if(!collection)
    {
         kWarning(296) << "Cannot find the action collection for action " << action->objectName();
         return;
    }
    
    KToolBar *toolbar=window->toolBars()[tb];
    toolbar->addAction(action);

    const KXMLGUIClient* client = collection->parentGUIClient ();
    QString xmlFile = client->localXMLFile();
    QDomDocument document;
    document.setContent( KXMLGUIFactory::readConfigFile( client->xmlFile(), client->componentData() ) );
    QDomElement elem = document.documentElement().toElement();
    
    static const QString &tagToolBar = KGlobal::staticQString( "ToolBar" );
    static const QString &attrNoEdit = KGlobal::staticQString( "noEdit" );
    static const QString &attrName     = KGlobal::staticQString( "name" );
    
    QDomElement toolbarElem;
    QDomNode n=elem.firstChild();
    for( ; !n.isNull(); n = n.nextSibling() )
    {
        QDomElement elem = n.toElement();
        if (!elem.isNull() && elem.tagName() == tagToolBar && elem.attribute( attrName ) == toolbar->objectName() )
        {
            if(elem.attribute( attrNoEdit ) == "true")
            {
                kWarning(296) << "The toolbar is not editable";
                return;
            }
            toolbarElem = elem;
            break;
        }
    }
    if(toolbarElem.isNull())
    {
        toolbarElem=document.createElement( tagToolBar );
        toolbarElem.setAttribute( attrName, toolbar->objectName() );
        elem.appendChild( toolbarElem );
    }
    
    KXMLGUIFactory::findActionByName(toolbarElem, actionName, true);
    KXMLGUIFactory::saveConfigFile(document, xmlFile);
    
}



} //END namespace KDEPrivate

#include "kmenumenuhandler_p.moc"
