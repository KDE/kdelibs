/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>

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

#include "kxmlguifactory.h"
#include "kxmlguifactory_p.h"
#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"

#include <assert.h>

#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qwidget.h>
#include <qdatetime.h>
#include <qvariant.h>

#include <kaction.h>
#include <kdebug.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kshortcut.h>
#include <kstandarddirs.h>
#include <kkeydialog.h>

using namespace KXMLGUI;

/*
 * TODO:     - make more use of QValueList instead of QPtrList
 */

class KXMLGUIFactoryPrivate : public BuildState
{
public:
    KXMLGUIFactoryPrivate()
    {
        static const QString &defaultMergingName = KGlobal::staticQString( "<default>" );
        static const QString &actionList = KGlobal::staticQString( "actionlist" );
        static const QString &name = KGlobal::staticQString( "name" );

        m_rootNode = new ContainerNode( 0L, QString::null, 0L );
        m_defaultMergingName = defaultMergingName;
        tagActionList = actionList;
        attrName = name;
    }
    ~KXMLGUIFactoryPrivate()
    {
        delete m_rootNode;
    }

    void pushState()
    {
        m_stateStack.push( *this );
    }

    void popState()
    {
        BuildState::operator=( m_stateStack.pop() );
    }

    ContainerNode *m_rootNode;

    QString m_defaultMergingName;

    /*
     * Contains the container which is searched for in ::container .
     */
    QString m_containerName;

    /*
     * List of all clients
     */
    QList<KXMLGUIClient*> m_clients;

    QString tagActionList;

    QString attrName;

    BuildStateStack m_stateStack;
};

QString KXMLGUIFactory::readConfigFile( const QString &filename, const KInstance *instance )
{
    return readConfigFile( filename, false, instance );
}

QString KXMLGUIFactory::readConfigFile( const QString &filename, bool never_null, const KInstance *_instance )
{
    const KInstance *instance = _instance ? _instance : KGlobal::instance();
    QString xml_file;

    if (!QDir::isRelativePath(filename))
        xml_file = filename;
    else
    {
        xml_file = locate("data", QLatin1String(instance->instanceName() + '/' ) + filename);
        if ( !QFile::exists( xml_file ) )
          xml_file = locate( "data", filename );
    }

    QFile file( xml_file );
    if ( !file.open( QIODevice::ReadOnly ) )
    {
        kdError(240) << "No such XML file " << filename << endl;
        if ( never_null )
            return QLatin1String( "<!DOCTYPE kpartgui>\n<kpartgui name=\"empty\">\n</kpartgui>" );
        else
            return QString::null;
    }

    QByteArray buffer(file.readAll());
    return QString::fromUtf8(buffer.data(), buffer.size());
}

bool KXMLGUIFactory::saveConfigFile( const QDomDocument& doc,
                                     const QString& filename, const KInstance *_instance )
{
    const KInstance *instance = _instance ? _instance : KGlobal::instance();
    QString xml_file(filename);

    if (QDir::isRelativePath(xml_file))
        xml_file = locateLocal("data", QLatin1String( instance->instanceName() + '/' )
                               + filename);

    QFile file( xml_file );
    if ( !file.open( QIODevice::WriteOnly ) )
    {
        kdError(240) << "Could not write to " << filename << endl;
        return false;
    }

    // write out our document
    QTextStream ts(&file);
    ts.setEncoding( QTextStream::UnicodeUTF8 );
    ts << doc;

    file.close();
    return true;
}

QString KXMLGUIFactory::documentToXML( const QDomDocument& doc )
{
    QString str;
    QTextStream ts(&str, QIODevice::WriteOnly);
    ts.setEncoding( QTextStream::UnicodeUTF8 );
    ts << doc;
    return str;
}

QString KXMLGUIFactory::elementToXML( const QDomElement& elem )
{
    QString str;
    QTextStream ts(&str, QIODevice::WriteOnly);
    ts.setEncoding( QTextStream::UnicodeUTF8 );
    ts << elem;
    return str;
}

void KXMLGUIFactory::removeDOMComments( QDomNode &node )
{
    QDomNode n = node.firstChild();
    while ( !n.isNull() )
    {
        if ( n.nodeType() == QDomNode::CommentNode )
        {
            QDomNode tmp = n;
            n = n.nextSibling();
            node.removeChild( tmp );
        }
        else
        {
            QDomNode tmp = n;
            n = n.nextSibling();
            removeDOMComments( tmp );
        }
    }
}

KXMLGUIFactory::KXMLGUIFactory( KXMLGUIBuilder *builder, QObject *parent )
    : QObject( parent )
{
    d = new KXMLGUIFactoryPrivate;
    d->builder = builder;
    d->guiClient = 0;
    if ( d->builder )
    {
        d->builderContainerTags = d->builder->containerTags();
        d->builderCustomTags = d->builder->customTags();
    }
}

KXMLGUIFactory::~KXMLGUIFactory()
{
    delete d;
}

void KXMLGUIFactory::addClient( KXMLGUIClient *client )
{
    kdDebug(1002) << "KXMLGUIFactory::addClient( " << client << " )" << endl; // ellis
    static const QString &actionPropElementName = KGlobal::staticQString( "ActionProperties" );

    if ( client->factory() ) {
        if ( client->factory() == this )
            return;
        else
            client->factory()->removeClient( client ); //just in case someone does stupid things ;-)
    }

    d->pushState();

//    QTime dt; dt.start();

    d->guiClient = client;

    // add this client to our client list
    if ( !d->m_clients.contains( client ) )
        d->m_clients.append( client );
    else
        kdDebug(1002) << "XMLGUI client already added " << client << endl;

    // Tell the client that plugging in is process and
    //  let it know what builder widget its mainwindow shortcuts
    //  should be attached to.
    client->beginXMLPlug( d->builder->widget() );

    // try to use the build document for building the client's GUI, as the build document
    // contains the correct container state information (like toolbar positions, sizes, etc.) .
    // if there is non available, then use the "real" document.
    QDomDocument doc = client->xmlguiBuildDocument();
    if ( doc.documentElement().isNull() )
        doc = client->domDocument();

    QDomElement docElement = doc.documentElement();

    d->m_rootNode->index = -1;

    // cache some variables

    d->clientName = docElement.attribute( d->attrName );
    d->clientBuilder = client->clientBuilder();

    if ( d->clientBuilder )
    {
        d->clientBuilderContainerTags = d->clientBuilder->containerTags();
        d->clientBuilderCustomTags = d->clientBuilder->customTags();
    }
    else
    {
        d->clientBuilderContainerTags.clear();
        d->clientBuilderCustomTags.clear();
    }

    // process a possibly existing actionproperties section

    QDomElement actionPropElement = docElement.namedItem( actionPropElementName ).toElement();
    if ( actionPropElement.isNull() )
        actionPropElement = docElement.namedItem( actionPropElementName.toLower() ).toElement();

    if ( !actionPropElement.isNull() )
        applyActionProperties( actionPropElement );

    BuildHelper( *d, d->m_rootNode ).build( docElement );

    // let the client know that we built its GUI.
    client->setFactory( this );

    // call the finalizeGUI method, to fix up the positions of toolbars for example.
    // ### FIXME : obey client builder
    // --- Well, toolbars have a bool "positioned", so it doesn't really matter,
    // if we call positionYourself on all of them each time. (David)
    d->builder->finalizeGUI( d->guiClient );

    // reset some variables, for safety
    d->BuildState::reset();

    client->endXMLPlug();

    d->popState();

    emit clientAdded( client );

    // build child clients
    foreach (KXMLGUIClient *child, client->childClients())
        addClient( child );

//    kdDebug() << "addClient took " << dt.elapsed() << endl;
}

void KXMLGUIFactory::removeClient( KXMLGUIClient *client )
{
    kdDebug(1002) << "KXMLGUIFactory::removeClient( " << client << " )" << endl; // ellis

    // don't try to remove the client's GUI if we didn't build it
    if ( !client || client->factory() != this )
        return;

    // remove this client from our client list
    d->m_clients.removeAll( client );

    // remove child clients first (create a copy of the list just in case the
    // original list is modified directly or indirectly in removeClient())
    const QList<KXMLGUIClient*> childClients(client->childClients());
    foreach (KXMLGUIClient *child, childClients)
        removeClient(child);

    kdDebug(1002) << "KXMLGUIFactory::removeServant, calling removeRecursive" << endl;

    d->pushState();

    // cache some variables

    d->guiClient = client;
    d->clientName = client->domDocument().documentElement().attribute( d->attrName );
    d->clientBuilder = client->clientBuilder();

    client->setFactory( 0L );

    // if we don't have a build document for that client, yet, then create one by
    // cloning the original document, so that saving container information in the
    // DOM tree does not touch the original document.
    QDomDocument doc = client->xmlguiBuildDocument();
    if ( doc.documentElement().isNull() )
    {
        doc = client->domDocument().cloneNode( true ).toDocument();
        client->setXMLGUIBuildDocument( doc );
    }

    d->m_rootNode->destruct( doc.documentElement(), *d );

    d->builder->finalizeGUI( d->guiClient ); //JoWenn

    // reset some variables
    d->BuildState::reset();

    // This will destruct the KAccel object built around the given widget.
    client->prepareXMLUnplug( d->builder->widget() );

    d->popState();

    emit clientRemoved( client );
}

const QList<KXMLGUIClient*>& KXMLGUIFactory::clients() const
{
    return d->m_clients;
}

QWidget *KXMLGUIFactory::container( const QString &containerName, KXMLGUIClient *client,
                                    bool useTagName )
{
    d->pushState();
    d->m_containerName = containerName;
    d->guiClient = client;

    QWidget *result = findRecursive( d->m_rootNode, useTagName );

    d->guiClient = 0L;
    d->m_containerName = QString::null;

    d->popState();

    return result;
}

QList<QWidget*> KXMLGUIFactory::containers( const QString &tagName )
{
    return findRecursive( d->m_rootNode, tagName );
}

void KXMLGUIFactory::reset()
{
    d->m_rootNode->reset();

    d->m_rootNode->clearChildren();
}

void KXMLGUIFactory::resetContainer( const QString &containerName, bool useTagName )
{
    if ( containerName.isEmpty() )
        return;

    ContainerNode *container = d->m_rootNode->findContainer( containerName, useTagName );

    if ( !container )
        return;

    ContainerNode *parent = container->parent;
    if ( !parent )
        return;

    //  resetInternal( container );

    parent->removeChild( container );
}

QWidget *KXMLGUIFactory::findRecursive( KXMLGUI::ContainerNode *node, bool tag )
{
    if ( ( ( !tag && node->name == d->m_containerName ) ||
           ( tag && node->tagName == d->m_containerName ) ) &&
         ( !d->guiClient || node->client == d->guiClient ) )
        return node->container;

    foreach (ContainerNode* child, node->children)
    {
        QWidget *cont = findRecursive( child, tag );
        if ( cont )
            return cont;
    }

    return 0L;
}

QList<QWidget*> KXMLGUIFactory::findRecursive( KXMLGUI::ContainerNode *node,
                                                 const QString &tagName )
{
    QList<QWidget*> res;

    if ( node->tagName == tagName.toLower() )
        res.append( node->container );

    foreach (KXMLGUI::ContainerNode* child, node->children)
        res << findRecursive( child, tagName );

    return res;
}

void KXMLGUIFactory::plugActionList( KXMLGUIClient *client, const QString &name,
                                     const QList<KAction*> &actionList )
{
    d->pushState();
    d->guiClient = client;
    d->actionListName = name;
    d->actionList = actionList;
    d->clientName = client->domDocument().documentElement().attribute( d->attrName );

    d->m_rootNode->plugActionList( *d );

    d->BuildState::reset();
    d->popState();
}

void KXMLGUIFactory::unplugActionList( KXMLGUIClient *client, const QString &name )
{
    d->pushState();
    d->guiClient = client;
    d->actionListName = name;
    d->clientName = client->domDocument().documentElement().attribute( d->attrName );

    d->m_rootNode->unplugActionList( *d );

    d->BuildState::reset();
    d->popState();
}

void KXMLGUIFactory::applyActionProperties( const QDomElement &actionPropElement )
{
    static const QString &tagAction = KGlobal::staticQString( "action" );

    for (QDomNode n = actionPropElement.firstChild();
         !n.isNull(); n = n.nextSibling() )
    {
        QDomElement e = n.toElement();
        if ( e.tagName().toLower() != tagAction )
            continue;

        KAction *action = d->guiClient->action( e );
        if ( !action )
            continue;

        configureAction( action, e.attributes() );
    }
}

void KXMLGUIFactory::configureAction( KAction *action, const QDomNamedNodeMap &attributes )
{
    for ( uint i = 0; i < attributes.length(); i++ )
    {
        QDomAttr attr = attributes.item( i ).toAttr();
        if ( attr.isNull() )
            continue;

        configureAction( action, attr );
    }
}

void KXMLGUIFactory::configureAction( KAction *action, const QDomAttr &attribute )
{
    static const QString &attrShortcut = KGlobal::staticQString( "shortcut" );

    QString attrName = attribute.name();
    // If the attribute is a deprecated "accel", change to "shortcut".
    if ( attrName.toLower() == QLatin1String("accel") )
        attrName = attrShortcut;

    // No need to re-set name, particularly since it's "objectName" in Qt4
    if ( attrName.toLower() == QLatin1String("name") )
        return;

    QVariant propertyValue;

    QVariant::Type propertyType = action->property( attrName.toLatin1() ).type();

    if ( propertyType == QVariant::Int )
        propertyValue = QVariant( attribute.value().toInt() );
    else if ( propertyType == QVariant::UInt )
        propertyValue = QVariant( attribute.value().toUInt() );
    else
        propertyValue = QVariant( attribute.value() );

    action->setProperty( attrName.toLatin1(), propertyValue );
}


int KXMLGUIFactory::configureShortcuts(bool bAllowLetterShortcuts , bool bSaveSettings )
{
	KKeyDialog dlg( bAllowLetterShortcuts, dynamic_cast<QWidget*>(parent()) );
	foreach (KXMLGUIClient *client, d->m_clients)
	{
		if(!client->xmlFile().isEmpty())
			dlg.insert( client->actionCollection() );
	}
	return dlg.configure(bSaveSettings);
}

QDomElement KXMLGUIFactory::actionPropertiesElement( QDomDocument& doc )
{
	const QString tagActionProp = QLatin1String("ActionProperties");
	// first, lets see if we have existing properties
	QDomElement elem;
	QDomNode it = doc.documentElement().firstChild();
	for( ; !it.isNull(); it = it.nextSibling() ) {
		QDomElement e = it.toElement();
		if( e.tagName() == tagActionProp ) {
			elem = e;
			break;
		}
	}

	// if there was none, create one
	if( elem.isNull() ) {
		elem = doc.createElement( tagActionProp );
		doc.documentElement().appendChild( elem );
	}
	return elem;
}

QDomElement KXMLGUIFactory::findActionByName( QDomElement& elem, const QString& sName, bool create )
{
        static const QString& attrName = KGlobal::staticQString( "name" );
	static const QString& tagAction = KGlobal::staticQString( "Action" );
	for( QDomNode it = elem.firstChild(); !it.isNull(); it = it.nextSibling() ) {
		QDomElement e = it.toElement();
		if( e.attribute( attrName ) == sName )
			return e;
	}

	if( create ) {
		QDomElement act_elem = elem.ownerDocument().createElement( tagAction );
		act_elem.setAttribute( attrName, sName );
                elem.appendChild( act_elem );
                return act_elem;
	}
        return QDomElement();
}

void KXMLGUIFactory::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kxmlguifactory.moc"

/* vim: et sw=4
 */
