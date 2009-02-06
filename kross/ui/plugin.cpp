/* This file is part of the KDE project
   Copyright (C) 2008 Paulo Moura Guedes <moura@kdewebdev.org>

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

#include "plugin.h"

#include <kaction.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>
#include <kross/core/manager.h>
#include <kross/core/actioncollection.h>
#include <kio/netaccess.h>

#include <QPointer>

using namespace Kross;

/// \internal d-pointer class
class ScriptingPlugin::ScriptingPluginPrivate
{
public:
    QString userActionsFile;
    QHash<QString, QPointer<QObject> > objects;

    QDomElement menuFromName(QString const& name, const QDomDocument& document)
    {
        QDomElement menuBar = document.documentElement().firstChildElement("MenuBar");
        QDomElement menu = menuBar.firstChildElement("Menu");
        for(; !menu.isNull(); menu = menu.nextSiblingElement("Menu")) {
            if(menu.attribute("name") == name) {
                return menu;
            }
        }
        return QDomElement();
    }
};

ScriptingPlugin::ScriptingPlugin(QObject* parent)
    : KParts::Plugin(parent)
    , d(new ScriptingPluginPrivate())
{
    d->userActionsFile = KGlobal::dirs()->locateLocal("appdata", "scripts/scriptactions.rc");
}

ScriptingPlugin::~ScriptingPlugin()
{
    delete d;
}

void ScriptingPlugin::setDOMDocument(const QDomDocument &document, bool merge)
{
    QDomDocument doc = buildDomDocument(document);
    KXMLGUIClient::setDOMDocument(doc, merge);
}

void ScriptingPlugin::addObject(QObject* object, const QString& name)
{
    QString n = name.isNull() ? object->objectName() : name;
    d->objects.insert(n, object);
}

QDomDocument ScriptingPlugin::buildDomDocument(const QDomDocument& document)
{
    QStringList allActionFiles = KGlobal::dirs()->findAllResources("appdata", "scripts/*.rc");

    Kross::Manager::self().setProperty("configfile", d->userActionsFile);
    Kross::Manager::self().setProperty("configfiles", allActionFiles);

    if(KIO::NetAccess::exists(KUrl(d->userActionsFile), KIO::NetAccess::SourceSide, 0)) {
        Kross::Manager::self().actionCollection()->readXmlFile(d->userActionsFile);
    }
    else {
        foreach(const QString &f, allActionFiles) {
            Kross::Manager::self().actionCollection()->readXmlFile(f);
        }
    }

    QDomDocument doc(document);
    buildDomDocument(doc, Kross::Manager::self().actionCollection());

    return doc;
}

void ScriptingPlugin::buildDomDocument(QDomDocument& document,
    Kross::ActionCollection* collection)
{
    QDomElement menuElement = d->menuFromName(collection->name(), document);

    foreach(Kross::Action* action, collection->actions()) {
        QHashIterator<QString, QPointer<QObject> > i(d->objects);
        while(i.hasNext()) {
            i.next();
            action->addObject(i.value(), i.key());
        }

        // Create and append new Menu element if doesn't exist
        if(menuElement.isNull()) {
            menuElement = document.createElement("Menu");
            menuElement.setAttribute("name", collection->name());
            menuElement.setAttribute("noMerge", "0");

            QDomElement textElement = document.createElement("text");
            textElement.appendChild(document.createTextNode(collection->text()));
            menuElement.appendChild(textElement);

            Kross::ActionCollection* parentCollection = collection->parentCollection();
            if(!parentCollection) {
                document.documentElement().firstChildElement("MenuBar").appendChild(menuElement);
            }
            else {
                QDomElement parentMenuElement = d->menuFromName(parentCollection->name(), document);

                if(!parentMenuElement.isNull()) {
                    parentMenuElement.appendChild(menuElement);
                }
                else {
                    document.documentElement().firstChildElement("MenuBar").appendChild(menuElement);
                }
            }
        }

        // Create and append new Action element
        QDomElement newActionElement = document.createElement("Action");
        newActionElement.setAttribute("name", action->name());

        menuElement.appendChild(newActionElement);


        KAction* adaptor=new KAction(action->text(), action);
        connect (adaptor,SIGNAL(triggered()),action,SLOT(trigger()));
        adaptor->setEnabled(action->isEnabled());
        adaptor->setIcon(action->icon());
        actionCollection()->addAction(action->name(), adaptor);
    }

    foreach(const QString &collectionname, collection->collections()) {
        Kross::ActionCollection* c = collection->collection(collectionname);
        if(c->isEnabled()) {
            buildDomDocument(document, c);
        }
    }
}

void ScriptingPlugin::slotEditScriptActions()
{
    if(!KIO::NetAccess::exists(KUrl(d->userActionsFile), KIO::NetAccess::SourceSide, 0)) {
        KUrl dir = KUrl(d->userActionsFile).directory();
        KIO::NetAccess::mkdir(dir, 0);

        QFile f(d->userActionsFile);
        if(f.open(QIODevice::WriteOnly)) {

            bool collectionEmpty = true;
            if(!Kross::Manager::self().actionCollection()->actions().empty()
                || !Kross::Manager::self().actionCollection()->collections().empty()) {
                collectionEmpty = false;
            }

            if( !collectionEmpty ) {
                if( Kross::Manager::self().actionCollection()->writeXml(&f) ) {
                    kDebug() << "Successfully saved file: " << d->userActionsFile;
                }
            }
            else {
                QTextStream out(&f);
                QString xml("<!-- ");
                xml.append("\n");
                xml.append("Collection name attribute represents the name of the menu, e.g., to use menu \"File\" use \"file\" or \"Help\" use \"help\". One can add new menus also.");
                xml.append("\n\n\n");
                xml.append("If you type a relative script file beware the this script is located in  $KDEHOME/share/apps/applicationname/");
                xml.append("\n\n");
                xml.append("The following example add an action with the text \"Export...\" into the \"File\" menu");
                xml.append("\n");
                xml.append("-->");
                xml.append("\n\n");
                xml.append("<KrossScripting>");
                xml.append("\n");
                xml.append("<collection name=\"file\" text=\"File\" comment=\"File menu\">");
                xml.append("\n");
                xml.append("<script name=\"export\" text=\"Export...\" comment=\"Export content\" file=\"export.py\" />");
                xml.append("\n");
                xml.append("</collection>");
                xml.append("\n");
                xml.append("</KrossScripting>");;

                out << xml;
            }
        }
        f.close();
    }

    KRun::runUrl(KUrl(d->userActionsFile), QString("text/plain"), 0, false);
}

void ScriptingPlugin::slotResetScriptActions()
{
    KIO::NetAccess::del(KUrl(d->userActionsFile), 0);
}

#include "plugin.moc"
