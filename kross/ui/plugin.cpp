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

struct Object
{
    QPointer<QObject> object;
    ChildrenInterface::Options options;
    Object(QObject* obj, ChildrenInterface::Options opt):object(obj),options(opt){}
};


/// \internal d-pointer class
class ScriptingPlugin::ScriptingPluginPrivate
{
public:
    QString collectionName;
    QString userActionsFile;
    QString referenceActionsDir;
    QHash<QString, Object> objects;

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
    d->collectionName="scripting-plugin";
}

ScriptingPlugin::ScriptingPlugin(const QString& collectionName, const QString& userActionsFile, const QString& referenceActionsDir, QObject* parent)
    : KParts::Plugin(parent)
    , d(new ScriptingPluginPrivate())
{
    d->collectionName=collectionName;
    d->userActionsFile = userActionsFile;
    d->referenceActionsDir = referenceActionsDir;
}

ScriptingPlugin::~ScriptingPlugin()
{
    if (QFile::exists(d->userActionsFile))
        save();

    Kross::ActionCollection* collection=Kross::Manager::self().actionCollection()->collection(d->collectionName);
    if (collection) {
        collection->setParentCollection(0);
        collection->deleteLater();
    }

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
    d->objects.insert(n, Object(object,ChildrenInterface::NoOption));
}

void ScriptingPlugin::addObject(QObject* object, const QString& name, ChildrenInterface::Options options)
{
    QString n = name.isNull() ? object->objectName() : name;
    d->objects.insert(n, Object(object,options));
}

QDomDocument ScriptingPlugin::buildDomDocument(const QDomDocument& document)
{
    Kross::ActionCollection* collection=Kross::Manager::self().actionCollection()->collection(d->collectionName);
    if (!collection) {
        collection=new Kross::ActionCollection(d->collectionName, Kross::Manager::self().actionCollection());
    }

    QStringList allActionFiles = KGlobal::dirs()->findAllResources("appdata", "scripts/"+d->referenceActionsDir+"/*.rc");
    //move userActionsFile to the end so that it updates existing actions and adds new ones.
    int pos=allActionFiles.indexOf(d->userActionsFile);
    if (pos!=-1)
        allActionFiles.append(allActionFiles.takeAt(pos));
    else if (QFile::exists(d->userActionsFile)) //in case d->userActionsFile isn't in the standard local dir
        allActionFiles.append(d->userActionsFile);

    QStringList searchPath=KGlobal::dirs()->findDirs("appdata", "scripts/"+d->referenceActionsDir);
    foreach(const QString &file, allActionFiles) {
        QFile f(file);
        if (!f.open(QIODevice::ReadOnly))
            continue;

        collection->readXml(&f, searchPath+QStringList(QFileInfo(f).absolutePath()));
        f.close();

    }

    QDomDocument doc(document);
    buildDomDocument(doc, collection);

    return doc;
}

void ScriptingPlugin::buildDomDocument(QDomDocument& document,
    Kross::ActionCollection* collection)
{
    QDomElement menuElement = d->menuFromName(collection->name(), document);

    foreach(Kross::Action* action, collection->actions()) {
        QHashIterator<QString, Object> i(d->objects);
        while(i.hasNext()) {
            i.next();
            action->addObject(i.value().object, i.key(), i.value().options);
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
            QDomElement root;
            if(parentCollection) {
                QDomElement parentMenuElement = d->menuFromName(parentCollection->name(), document);
                if(!parentMenuElement.isNull())
                    root=parentMenuElement;
            }
            if (root.isNull())
                root=document.documentElement().firstChildElement("MenuBar");
            root.appendChild(menuElement);
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

void ScriptingPlugin::save()
{
    QFile f(d->userActionsFile);
    if(!f.open(QIODevice::WriteOnly))
        return;

    Kross::ActionCollection* collection=Kross::Manager::self().actionCollection()->collection(d->collectionName);
    bool collectionEmpty = !collection||(collection->actions().empty()&&collection->collections().empty());

    if( !collectionEmpty ) {
        QStringList searchPath=KGlobal::dirs()->findDirs("appdata", "scripts/"+d->referenceActionsDir);
        searchPath.append(QFileInfo(d->userActionsFile).absolutePath());
        if( collection->writeXml(&f, 2, searchPath) ) {
            kDebug() << "Successfully saved file: " << d->userActionsFile;
        }
    }
    else {
        QTextStream out(&f);
        QString xml=
        "<!-- "
        "\n"
        "Collection name attribute represents the name of the menu, e.g., to use menu \"File\" use \"file\" or \"Help\" use \"help\". You can add new menus."
        "\n\n\n"
        "If you type a relative script file beware the this script is located in  $KDEHOME/share/apps/applicationname/"
        "\n\n"
        "The following example adds an action with the text \"Export...\" into the \"File\" menu"
        "\n\n"
        "<KrossScripting>"
        "\n"
        "<collection name=\"file\" text=\"File\" comment=\"File menu\">"
        "\n"
        "<script name=\"export\" text=\"Export...\" comment=\"Export content\" file=\"export.py\" />"
        "\n"
        "</collection>"
        "\n"
        "</KrossScripting>"
        "\n"
        "-->";


        out << xml;
    }
    f.close();
}

void ScriptingPlugin::slotEditScriptActions()
{
    if(!KIO::NetAccess::exists(KUrl(d->userActionsFile), KIO::NetAccess::SourceSide, 0)) {
        KUrl dir = KUrl(d->userActionsFile).directory();
        KIO::NetAccess::mkdir(dir, 0);

        save();
    }

    //TODO very funny! this should use ui/view.h instead --Nick
    KRun::runUrl(KUrl(d->userActionsFile), QString("text/plain"), 0, false);
}

void ScriptingPlugin::slotResetScriptActions()
{
    KIO::NetAccess::del(KUrl(d->userActionsFile), 0);
}

#include "plugin.moc"
