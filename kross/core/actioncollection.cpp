/***************************************************************************
 * actioncollection.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "actioncollection.h"
#include "action.h"
#include "manager.h"

#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QtCore/QPointer>
#include <QtCore/QIODevice>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtXml/QDomAttr>

#include <kicon.h>

using namespace Kross;

namespace Kross {

    /// \internal d-pointer class.
    class ActionCollection::Private
    {
        public:
            QPointer<ActionCollection> parent;
            QHash< QString, QPointer<ActionCollection> > collections;
            QStringList collectionnames;

            QList< Action* > actionList;
            QHash< QString, Action* > actionMap;

            QString text;
            QString description;
            QString iconname;
            bool enabled;

            Private(ActionCollection* const p) : parent(p) {}
    };

}

ActionCollection::ActionCollection(const QString& name, ActionCollection* parent)
    : QObject(parent)
    , d( new Private(parent) )
{
    setObjectName(name);
    d->text = name;
    d->enabled = true;
    if( d->parent )
        d->parent->registerCollection(this);
}

ActionCollection::~ActionCollection()
{
    if( d->parent )
        d->parent->unregisterCollection(objectName());
    delete d;
}

QString ActionCollection::name() const { return objectName(); }

QString ActionCollection::text() const { return d->text; }
void ActionCollection::setText(const QString& text) { d->text = text; emit updated(); }

QString ActionCollection::description() const { return d->description; }
void ActionCollection::setDescription(const QString& description) { d->description = description; emit updated(); }

QString ActionCollection::iconName() const { return d->iconname; }
void ActionCollection::setIconName(const QString& iconname) { d->iconname = iconname; }
QIcon ActionCollection::icon() const { return KIcon(d->iconname); }

bool ActionCollection::isEnabled() const { return d->enabled; }
void ActionCollection::setEnabled(bool enabled) { d->enabled = enabled; emit updated(); }

ActionCollection* ActionCollection::parentCollection() const
{
    return d->parent;
}

bool ActionCollection::hasCollection(const QString& name) const
{
    return d->collections.contains(name);
}

ActionCollection* ActionCollection::collection(const QString& name) const
{
    return d->collections.contains(name) ? d->collections[name] : QPointer<ActionCollection>(0);
}

QStringList ActionCollection::collections() const
{
    return d->collectionnames;
}

void ActionCollection::registerCollection(ActionCollection* collection)
{
    Q_ASSERT(collection);
    const QString name = collection->objectName();
    //Q_ASSERT( !name.isNull() );
    d->collections.insert(name, collection);
    d->collectionnames.append(name);
    connect(collection, SIGNAL(updated()), this, SIGNAL(updated()));
    //emit updated();
}

void ActionCollection::unregisterCollection(const QString& name)
{
    if( ! d->collections.contains(name) )
        return;
    ActionCollection* collection = d->collections[name];
    d->collectionnames.removeAll(name);
    d->collections.remove(name);
    disconnect(collection, SIGNAL(updated()), this, SIGNAL(updated()));
    //emit updated();
}

QList<Action*> ActionCollection::actions() const
{
    return d->actionList;
}

Action* ActionCollection::action(const QString& name) const
{
    return d->actionMap.contains(name) ? d->actionMap[name] : 0;
}

void ActionCollection::addAction(Action* action)
{
    Q_ASSERT( action && ! action->objectName().isEmpty() );
    addAction(action->objectName(), action);
}

void ActionCollection::addAction(const QString& name, Action* action)
{
    Q_ASSERT( action && ! name.isEmpty() );
    if( d->actionMap.contains(name) )
        d->actionList.removeAll( d->actionMap[name] );
    d->actionMap.insert(name, action);
    d->actionList.append(action);
    connect(action, SIGNAL(updated()), this, SIGNAL(updated()));
    emit updated();
}

void ActionCollection::removeAction(const QString& name)
{
    if( ! d->actionMap.contains(name) )
        return;
    Action* action = d->actionMap[name];
    d->actionList.removeAll(action);
    d->actionMap.remove(name);
    disconnect(action, SIGNAL(updated()), this, SIGNAL(updated()));
    emit updated();
}

void ActionCollection::removeAction(Action* action)
{
    Q_ASSERT( action && ! action->objectName().isEmpty() );
    const QString name = action->objectName();
    if( ! d->actionMap.contains(name) )
        return;
    d->actionList.removeAll(action);
    d->actionMap.remove(name);
    disconnect(action, SIGNAL(updated()), this, SIGNAL(updated()));
    emit updated();
}

/*********************************************************************
 * Unserialize from XML / QIODevice / file / resource to child
 * ActionCollection's and Action's this ActionCollection has.
 */

bool ActionCollection::readXml(const QDomElement& element, const QDir& directory)
{
    #ifdef KROSS_ACTIONCOLLECTION_DEBUG
        krossdebug( QString("ActionCollection::readXml tagName=\"%1\"").arg(element.tagName()) );
    #endif

    blockSignals(true); // block updated() signals and emit it only once if everything is done
    bool ok = true;
    QDomNodeList list = element.childNodes();
    const int size = list.size();
    for(int i = 0; i < size; ++i) {
        QDomElement elem = list.item(i).toElement();
        if( elem.isNull() ) continue;

        #ifdef KROSS_ACTIONCOLLECTION_DEBUG
            krossdebug( QString("  ActionCollection::readXml child=%1 tagName=\"%2\"").arg(i).arg(elem.tagName()) );
        #endif

        if( elem.tagName() == "collection") {
            const QString name = elem.attribute("name");
            const QString text = elem.attribute("text");
            const QString description = elem.attribute("comment");
            const QString iconname = elem.attribute("icon");
            bool enabled = QVariant(elem.attribute("enabled","true")).toBool();
            ActionCollection* c = d->collections.contains(name) ? d->collections[name] : QPointer<ActionCollection>(0);
            if( ! c )
                c = new ActionCollection(name, this);
            c->setText( text.isEmpty() ? name : text );
            c->setDescription( description.isEmpty() ? c->text() : description );
            c->setIconName( iconname );

            if( ! enabled )
                c->setEnabled(false);
            if( ! c->readXml(elem, directory) )
                ok = false;
        }
        else if( elem.tagName() == "script") {
            QString name = elem.attribute("name");
            Action* a = dynamic_cast< Action* >( action(name) );
            if( a ) {
                #ifdef KROSS_ACTIONCOLLECTION_DEBUG
                    krossdebug( QString("  ActionCollection::readXml Updating Action \"%1\"").arg(a->objectName()) );
                #endif
            }
            else {
                #ifdef KROSS_ACTIONCOLLECTION_DEBUG
                    krossdebug( QString("  ActionCollection::readXml Creating Action \"%1\"").arg(name) );
                #endif

                a = new Action(this, name, directory);
                addAction(name, a);
                connect(a, SIGNAL( started(Kross::Action*) ), &Manager::self(), SIGNAL( started(Kross::Action*)) );
                connect(a, SIGNAL( finished(Kross::Action*) ), &Manager::self(), SIGNAL( finished(Kross::Action*) ));
            }
            a->fromDomElement(elem);
        }
        //else if( ! fromXml(elem) ) ok = false;
    }

    blockSignals(false); // unblock signals
    emit updated();
    return ok;
}

bool ActionCollection::readXml(QIODevice* device, const QDir& directory)
{
    QString errMsg;
    int errLine, errCol;
    QDomDocument document;
    bool ok = document.setContent(device, false, &errMsg, &errLine, &errCol);
    if( ! ok ) {
        #ifdef KROSS_ACTIONCOLLECTION_DEBUG
            krosswarning( QString("ActionCollection::readXml Error at line %1 in col %2: %3").arg(errLine).arg(errCol).arg(errMsg) );
        #endif
        return false;
    }
    return readXml(document.documentElement(), directory);
}

bool ActionCollection::readXmlFile(const QString& file)
{
    #ifdef KROSS_ACTIONCOLLECTION_DEBUG
        krossdebug( QString("ActionCollection::readXmlFile file=\"%1\"").arg(file) );
    #endif

    QFile f(file);
    if( ! f.open(QIODevice::ReadOnly) ) {
        #ifdef KROSS_ACTIONCOLLECTION_DEBUG
            krosswarning( QString("ActionCollection::readXmlFile reading file \"%1\" failed.").arg(file) );
        #endif
        return false;
    }
    bool ok = readXml(&f, QFileInfo(file).dir());
    f.close();

    #ifdef KROSS_ACTIONCOLLECTION_DEBUG
        if( ! ok )
            krosswarning( QString("ActionCollection::readXmlFile parsing XML content of file \"%1\" failed.").arg(file) );
    #endif
    return ok;
}

/*********************************************************************
 * Serialize from child ActionCollection's and Action's this
 * ActionCollection has to XML / QIODevice / file / resource.
 */

QDomElement ActionCollection::writeXml()
{
    #ifdef KROSS_ACTIONCOLLECTION_DEBUG
        krossdebug( QString("ActionCollection::writeXml collection.objectName=\"%1\"").arg(objectName()) );
    #endif

    QDomDocument document;
    QDomElement element = document.createElement("collection");
    if( ! objectName().isNull() )
        element.setAttribute("name", objectName());
    if( ! text().isNull() && text() != objectName() )
        element.setAttribute("text", text());
    if( ! d->description.isNull() )
        element.setAttribute("comment", d->description);
    if( ! d->iconname.isNull() )
        element.setAttribute("icon", d->iconname);
    if( ! d->enabled )
        element.setAttribute("enabled", d->enabled);

    foreach(Action* a, actions()) {
        Q_ASSERT(a);
        #ifdef KROSS_ACTIONCOLLECTION_DEBUG
            krossdebug( QString("  ActionCollection::writeXml action.objectName=\"%1\" action.file=\"%2\"").arg(a->objectName()).arg(a->file()) );
        #endif
        QDomElement e = a->toDomElement();
        if( ! e.isNull() )
            element.appendChild(e);
    }

    foreach(const QString &name, d->collectionnames) {
        ActionCollection* c = d->collections[name];
        if( ! c ) continue;
        QDomElement e = c->writeXml();
        if( ! e.isNull() )
            element.appendChild(e);
    }

    return element;
}

bool ActionCollection::writeXml(QIODevice* device, int indent)
{
    QDomDocument document;
    QDomElement root = document.createElement("KrossScripting");

    foreach(Action* a, actions()) {
        QDomElement e = a->toDomElement();
        if( ! e.isNull() )
            root.appendChild(e);
    }

    foreach(const QString &name, d->collectionnames) {
        ActionCollection* c = d->collections[name];
        if( ! c ) continue;
        QDomElement e = c->writeXml();
        if( ! e.isNull() )
            root.appendChild(e);
    }

    document.appendChild(root);
    return device->write( document.toByteArray(indent) ) != -1;
}

#include "actioncollection.moc"
