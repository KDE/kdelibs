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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kxmlguifactory.h"
#include "kxmlguifactory_p.h"
#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"

#include <assert.h>

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

using namespace KXMLGUI;

/*
 * TODO:     - make more use of QValueList instead of QPtrList
 */

class KXMLGUIFactoryPrivate
{
public:
    KXMLGUIFactoryPrivate()
    {
        static const QString &defaultMergingName = KGlobal::staticQString( "<default>" );
        static const QString &actionList = KGlobal::staticQString( "actionlist" );
        static const QString &name = KGlobal::staticQString( "name" );

        m_rootNode = new ContainerNode( 0L, QString::null, 0L );
        m_defaultMergingName = defaultMergingName;
        m_clientBuilder = 0L;
        tagActionList = actionList;
        attrName = name;
    }
    ~KXMLGUIFactoryPrivate()
    {
        delete m_rootNode;
    }

    ContainerNode *m_rootNode;

    QString m_clientName;

    QString m_defaultMergingName;

    /*
     * Contains the container which is searched for in ::container .
     */
    QString m_containerName;

    /*
     * Basically what client->clientBuilder() returns for ::addClient. Cached.
     */
    KXMLGUIBuilder *m_clientBuilder;

    /*
     * List of all clients
     */
    QPtrList<KXMLGUIClient> m_clients;

    /*
     * Contains the name of the actionlist currently plugged/unplugged in ::plugActionList
     * and ::unplugActionList .
     */
    QString m_actionListName;
    /*
     * Similar to m_actionListName.
     */
    ActionList m_actionList;

    QString tagActionList;

    QString attrName;

    /*
     * The current running default merging index, valid only within one level in ::buildRecursive.
     * Faster than calculating it each time it's needed.
     */
    MergingIndexList::Iterator m_currentDefaultMergingIt;
    /*
     * The current client merging index, valid only within one leve in ::buildRecursive.
     * Faster than calling calcMergingIndex all the time.
     */
    MergingIndexList::Iterator m_currentClientMergingIt;

    /*
     * The stringlists the factory's GUIBuilder returns with the corresponding methods.
     */
    QStringList m_builderCustomTags;
    QStringList m_builderContainerTags;

    /*
     * The stringlists the client's GUIBuilder returns with the corresponding methods.
     * Only used within ::addClient . Saved globally here to avoid calling the methods
     * for each ::buildRecursive call.
     */
    QStringList m_clientBuilderCustomTags;
    QStringList m_clientBuilderContainerTags;
};

QString KXMLGUIFactory::readConfigFile( const QString &filename, KInstance *instance )
{
    return readConfigFile( filename, false, instance );
}

QString KXMLGUIFactory::readConfigFile( const QString &filename, bool never_null, KInstance *_instance )
{
    KInstance *instance = _instance ? _instance : KGlobal::instance();
    QString xml_file;

    if (filename[0] == '/')
        xml_file = filename;
    else
    {
        xml_file = locate("data", QString::fromLatin1(instance->instanceName() + '/' ) + filename);
        if ( !QFile::exists( xml_file ) )
          xml_file = locate( "data", filename );
    }

    QFile file( xml_file );
    if ( !file.open( IO_ReadOnly ) )
    {
        kdError(1000) << "No such XML file " << filename << endl;
        if ( never_null )
            return QString::fromLatin1( "<!DOCTYPE kpartgui>\n<kpartgui name=\"empty\">\n</kpartgui>" );
        else
            return QString::null;
    }

    QByteArray buffer(file.readAll());
    return QString::fromUtf8(buffer.data(), buffer.size());
}

bool KXMLGUIFactory::saveConfigFile( const QDomDocument& doc,
                                     const QString& filename, KInstance *_instance )
{
    KInstance *instance = _instance ? _instance : KGlobal::instance();
    QString xml_file(filename);

    if (xml_file[0] != '/')
        xml_file = locateLocal("data", QString::fromLatin1( instance->instanceName() + '/' )
                               + filename);

    QFile file( xml_file );
    if ( !file.open( IO_WriteOnly ) )
    {
        kdError(1000) << "Could not write to " << filename << endl;
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
    QTextStream ts(&str, IO_WriteOnly);
    ts.setEncoding( QTextStream::UnicodeUTF8 );
    ts << doc;
    return str;
}

QString KXMLGUIFactory::elementToXML( const QDomElement& elem )
{
    QString str;
    QTextStream ts(&str, IO_WriteOnly);
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

KXMLGUIFactory::KXMLGUIFactory( KXMLGUIBuilder *builder, QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new KXMLGUIFactoryPrivate;
    m_builder = builder;
    m_client = 0L;
    if ( m_builder )
    {
        d->m_builderContainerTags = m_builder->containerTags();
        d->m_builderCustomTags = m_builder->customTags();
    }
}

KXMLGUIFactory::~KXMLGUIFactory()
{
    delete d;
}

void KXMLGUIFactory::addClient( KXMLGUIClient *client )
{
    static const QString &actionPropElementName = KGlobal::staticQString( "ActionProperties" );

//    QTime dt; dt.start();

    m_client = client;

    if ( client->factory() && client->factory() != this )
        client->factory()->removeClient( client ); //just in case someone does stupid things ;-)

    // add this client to our client list
    if ( d->m_clients.containsRef( client ) == 0 )
        d->m_clients.append( client );

    // try to use the build document for building the client's GUI, as the build document
    // contains the correct container state information (like toolbar positions, sizes, etc.) .
    // if there is non available, then use the "real" document.
    QDomDocument doc = client->xmlguiBuildDocument();
    if ( doc.documentElement().isNull() )
        doc = client->domDocument();

    QDomElement docElement = doc.documentElement();

    d->m_rootNode->index = -1;

    // cache some variables

    d->m_clientName = docElement.attribute( d->attrName );
    d->m_clientBuilder = client->clientBuilder();

    if ( d->m_clientBuilder )
    {
        d->m_clientBuilderContainerTags = d->m_clientBuilder->containerTags();
        d->m_clientBuilderCustomTags = d->m_clientBuilder->customTags();
    }
    else
    {
        d->m_clientBuilderContainerTags = QStringList();
        d->m_clientBuilderCustomTags = QStringList();
    }

    // process a possibly existing actionproperties section

    QDomElement actionPropElement = docElement.namedItem( actionPropElementName ).toElement();
    if ( actionPropElement.isNull() )
        actionPropElement = docElement.namedItem( actionPropElementName.lower() ).toElement();

    if ( !actionPropElement.isNull() )
        applyActionProperties( actionPropElement );

    buildRecursive( docElement, d->m_rootNode );

    // let the client know that we built its GUI.
    client->setFactory( this );

    // call the finalizeGUI method, to fix up the positions of toolbars for example.
    // ### FIXME : obey client builder
    // --- Well, toolbars have a bool "positioned", so it doesn't really matter,
    // if we call positionYourself on all of them each time. (David)
    m_builder->finalizeGUI( m_client );

    // reset some variables, for safety
    m_client = 0L;
    d->m_clientName = QString::null;
    d->m_clientBuilder = 0L;

    emit clientAdded( client );

    // build child clients
    if ( client->childClients()->count() > 0 )
    {
        const QPtrList<KXMLGUIClient> *children = client->childClients();
        QPtrListIterator<KXMLGUIClient> childIt( *children );
        for (; childIt.current(); ++childIt )
            addClient( childIt.current() );
    }

//    kdDebug() << "addClient took " << dt.elapsed() << endl;
}

void KXMLGUIFactory::removeClient( KXMLGUIClient *client )
{
    // don't try to remove the client's GUI if we didn't build it
    if ( client->factory() && client->factory() != this )
        return;

    // remove this client from our client list
    d->m_clients.removeRef( client );

    // remove child clients first
    if ( client->childClients()->count() > 0 )
    {
        const QPtrList<KXMLGUIClient> *children = client->childClients();
        QPtrListIterator<KXMLGUIClient> childIt( *children );
        for (; childIt.current(); ++childIt )
            removeClient( childIt.current() );
    }

    kdDebug(1002) << "KXMLGUIFactory::removeServant, calling removeRecursive" << endl;

    // cache some variables

    m_client = client;
    d->m_clientName = client->domDocument().documentElement().attribute( d->attrName );
    d->m_clientBuilder = client->clientBuilder();

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

    QDomElement tmp = doc.documentElement();
    removeRecursive( tmp, d->m_rootNode );

    // reset some variables
    m_client = 0L;
    d->m_clientBuilder = 0L;
    d->m_clientName = QString::null;

    emit clientRemoved( client );
}

QPtrList<KXMLGUIClient> KXMLGUIFactory::clients() const
{
    return d->m_clients;
}

QWidget *KXMLGUIFactory::container( const QString &containerName, KXMLGUIClient *client,
                                    bool useTagName )
{
    d->m_containerName = containerName;
    m_client = client;

    QWidget *result = findRecursive( d->m_rootNode, useTagName );

    m_client = 0L;
    d->m_containerName = QString::null;

    return result;
}

QPtrList<QWidget> KXMLGUIFactory::containers( const QString &tagName )
{
    return findRecursive( d->m_rootNode, tagName );
}

void KXMLGUIFactory::reset()
{
    resetInternal( d->m_rootNode );

    d->m_rootNode->children.clear();
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

    parent->children.removeRef( container );
}

void KXMLGUIFactory::resetInternal( KXMLGUI::ContainerNode *node )
{
    QPtrListIterator<ContainerNode> childIt( node->children );
    for (; childIt.current(); ++childIt )
        resetInternal( childIt.current() );

    if ( node->client )
        node->client->setFactory( 0L );
}

void KXMLGUIFactory::buildRecursive( const QDomElement &element, 
                                     KXMLGUI::ContainerNode *parentNode )
{
    // some often used QStrings
    static const QString &tagAction = KGlobal::staticQString( "action" );
    static const QString &tagMerge = KGlobal::staticQString( "merge" );
    static const QString &tagState = KGlobal::staticQString( "state" );
    static const QString &tagDefineGroup = KGlobal::staticQString( "definegroup" );
    static const QString &attrGroup = KGlobal::staticQString( "group" );

    // create a list of supported container and custom tags
    QStringList customTags = d->m_builderCustomTags;
    QStringList containerTags = d->m_builderContainerTags;

    if ( parentNode->builder != m_builder )
    {
        customTags += parentNode->builderCustomTags;
        containerTags += parentNode->builderContainerTags;
    }

    /*
     * This list contains references to all the containers we created on the current level.
     * We use it as "exclude" list, in order to avoid container matches of already created
     * containers having no proper name attribute to distinct them.
     * ### still needed?
     */
    QPtrList<QWidget> containerList;

    ContainerClient *containerClient = 0L;

    d->m_currentDefaultMergingIt = parentNode->findIndex( d->m_defaultMergingName );
    calcMergingIndex( parentNode, QString::null, d->m_currentClientMergingIt, false );

    /*
     * When we encounter the "Merge" tag, then have to make sure to ingore it for the actions on the
     * current level. Example:
     * ..
     *  <Action blah/>
     *  <Merge/>
     *  <Action foo/>
     *  <Action gah/>
     *
     *  When plugging in the second action (foo), then we must *not* use (increase) the merging
     *  index but the normal one instead.
     */
    bool ignoreDefaultMergingIndex = false;

    QDomElement e = element.firstChild().toElement();
    for (; !e.isNull(); e = e.nextSibling().toElement() )
    {
        QString tag( e.tagName().lower() );
        QString currName( e.attribute( d->attrName ) );

        bool isActionTag = false;

        if ( ( isActionTag = ( tag == tagAction ) ) ||
             customTags.findIndex( tag ) != -1 )
        {
            if ( !parentNode->container )
                continue;

            MergingIndexList::Iterator it( d->m_currentClientMergingIt );

            bool haveGroup = false;
            QString group( e.attribute( attrGroup ) );
            if ( !group.isEmpty() )
            {
                group.prepend( attrGroup );
                haveGroup = true;
            }

            int idx;
            if ( haveGroup ) // if we have a group attribute, then we cannot use our nicely
                             // cached running merging index values.
                idx = calcMergingIndex( parentNode, group, it, ignoreDefaultMergingIndex );
            else if ( d->m_currentClientMergingIt == parentNode->mergingIndices.end() )
                // if we don't have a current merging index, then we want to append our action
                idx = parentNode->index;
            else
                idx = (*d->m_currentClientMergingIt).value;


            containerClient = parentNode->findChildContainerClient( m_client, group, it );

            if ( isActionTag )
            {
                // look up the action and plug it in
                KAction *action = m_client->action( e );

                if ( !action )
                    continue;

                action->plug( parentNode->container, idx );

                // save a reference to the plugged action, in order to properly unplug it afterwards.
                containerClient->actions.append( action );
            }
            else
            {
                assert( parentNode->builder );

                int id = parentNode->builder->createCustomElement( parentNode->container, idx, e );
                if ( id != 0 )
                    containerClient->customElements.append( id );
            }

            // adjust any following merging indices and the current running index for the container
            parentNode->adjustMergingIndices( 1, it );
        }
        /*
         * The "Merge" tag specifies that all containers and actions from *other* clients should be
         * inserted/plugged in at the current index, and not appended.
         * Same semantics apply for <DefineGroup> and <ActionList> . We simply want to save the
         * index of where the tags appear, in order to "substitute" them with other clients actions,
         * actionlists, containers, custom elements or even merging indices.
         */
        else if ( tag == tagMerge || tag == tagDefineGroup || tag == d->tagActionList )
        {
            QString mergingName( currName );
            if ( mergingName.isEmpty() )
            {
                if ( tag == tagDefineGroup )
                {
                    kdError(1000) << "cannot define group without name!" << endl;
                    continue;
                }
                if ( tag == d->tagActionList )
                {
                    kdError(1000) << "cannot define actionlist without name!" << endl;
                    continue;
                }
                mergingName = d->m_defaultMergingName;
            }

            if ( tag == tagDefineGroup )
                mergingName.prepend( attrGroup ); //avoid possible name clashes by prepending
                                                  // "group" to group definitions
            else if ( tag == d->tagActionList )
                mergingName.prepend( d->tagActionList );

            if ( parentNode->findIndex( mergingName ) != parentNode->mergingIndices.end() )
                continue; //do not allow the redefinition of merging indices!

            MergingIndexList::Iterator mIt( parentNode->mergingIndices.end() );

            // calculate the index of the new merging index. Usually this does not need any calculation,
            // we just want the last available index (i.e. append) . But in case the <Merge> tag appears
            // "inside" another <Merge> tag from a previously build client, then we have to use the
            // "parent's" index. That's why we call calcMergingIndex here.
            MergingIndex newIdx;
            newIdx.value = calcMergingIndex( parentNode,
                                             QString::null /* ### allow group for <merge/> ? */ ,
                                             mIt, ignoreDefaultMergingIndex );
            newIdx.mergingName = mergingName;
            newIdx.clientName = d->m_clientName;

            // if that merging index is "inside" another one, then append it right after the "parent" .
            if ( mIt != parentNode->mergingIndices.end() )
                parentNode->mergingIndices.insert( ++mIt, newIdx );
            else
                parentNode->mergingIndices.append( newIdx );

            if ( mergingName == d->m_defaultMergingName )
                ignoreDefaultMergingIndex = true;

            // re-calculate the running default and client merging indices.
            d->m_currentDefaultMergingIt = parentNode->findIndex( d->m_defaultMergingName );
            calcMergingIndex( parentNode, QString::null, d->m_currentClientMergingIt,
                              ignoreDefaultMergingIndex );
        }
        else if ( tag == tagState )
        {
          processStateElement( e );
        }
        else if ( containerTags.findIndex( tag ) != -1 )
        {
            /*
             * No Action or Merge tag? That most likely means that we want to create a new container.
             * But first we have to check if there's already a existing (child) container of the same
             * type in our tree. However we have to ignore just newly created containers!
             */

            ContainerNode *matchingContainer = parentNode->findContainer( currName, tag,
                                                                          &containerList,
                                                                          m_client );

            if ( matchingContainer )
            {
                /*
                 * Enter the next level, as the container already exists :)
                 */
                buildRecursive( e, matchingContainer );
                // re-calculate current default merging indices and client merging indices,
                // as they have changed in the recursive invocation.
                d->m_currentDefaultMergingIt = parentNode->findIndex( d->m_defaultMergingName );
                calcMergingIndex( parentNode, QString::null, d->m_currentClientMergingIt,
                                  ignoreDefaultMergingIndex );
            }
            else
            {
                MergingIndexList::Iterator it( d->m_currentClientMergingIt );

                bool haveGroup = false;
                QString group( e.attribute( attrGroup ) );
                if ( !group.isEmpty() )
                {
                    group.prepend( attrGroup );
                    haveGroup = true;
                }

                int idx;
                if ( haveGroup )
                    idx = calcMergingIndex( parentNode, group, it, ignoreDefaultMergingIndex );
                else if ( d->m_currentClientMergingIt == parentNode->mergingIndices.end() )
                    idx = parentNode->index;
                else
                    idx = (*d->m_currentClientMergingIt).value;

                /*
                 * let the builder create the container
                 */

                int id;

                KXMLGUIBuilder *builder;

                QWidget *container = createContainer( parentNode->container, idx, e, id, &builder );

                // no container? (probably some <text> tag or so ;-)
                if ( !container )
                    continue;

                parentNode->adjustMergingIndices( 1, it );

                ContainerNode *containerNode = parentNode->findContainerNode( container );

                if ( !containerNode ) // this should be true all times
                {
                    containerList.append( container );

                    QString mergingName;
                    if ( it != parentNode->mergingIndices.end() )
                        mergingName = (*it).mergingName;

                    QStringList cusTags = d->m_builderCustomTags;
                    QStringList conTags = d->m_builderContainerTags;
                    if ( builder != m_builder )
                    {
                        cusTags = d->m_clientBuilderCustomTags;
                        conTags = d->m_clientBuilderContainerTags;
                    }

                    containerNode = new ContainerNode( container, tag, currName, parentNode,
                                                       m_client, builder, id, mergingName, group,
                                                       cusTags, conTags );
                }

                buildRecursive( e, containerNode );

                // and re-calculate running values, for better performance
                d->m_currentDefaultMergingIt = parentNode->findIndex( d->m_defaultMergingName );
                calcMergingIndex( parentNode, QString::null, d->m_currentClientMergingIt,
                                  ignoreDefaultMergingIndex );
            }
        }
    }
}

bool KXMLGUIFactory::removeRecursive( QDomElement &element, KXMLGUI::ContainerNode *node )
{
    // process all child containers first
    QPtrListIterator<ContainerNode> childIt( node->children );
    while ( childIt.current() )
    {
        ContainerNode *childNode = childIt.current();

        // find the corresponding element in the DOM tree (which contains the saved container state)
        QDomElement child, e;
        // ### slow
        for ( e = element.firstChild().toElement(); !e.isNull();
              e = e.nextSibling().toElement() )
        {
            if ( e.tagName() == childNode->tagName &&
                 e.attribute( d->attrName ) == childNode->name )
            {
                child = e;
                break;
            }
        }

        // removeRecursive returns true in case the container really got deleted
        if ( removeRecursive( child, childNode ) )
            node->children.removeRef( childNode );
        else
            ++childIt;
    }

    MergingIndexList::Iterator mergingIt = node->mergingIndices.end();

    ContainerClientListIt clientIt( node->clients );

    if ( node->container )
    {
        if ( node->clients.count() == 1 && clientIt.current()->client == node->client &&
             node->client == m_client )
            node->container->hide(); // this container is going to die, that's for sure.
                                     // in this case let's just hide it, which makes the
                                     // destruction faster

        while ( clientIt.current() )
            //only unplug the actions of the client we want to remove, as the container might be owned
            //by a different client
            if ( clientIt.current()->client == m_client )
            {
                assert( node->builder );

                // now quickly remove all custom elements (i.e. separators) and unplug all actions

                QValueList<int>::ConstIterator custIt = clientIt.current()->customElements.begin();
                QValueList<int>::ConstIterator custEnd = clientIt.current()->customElements.end();
                for (; custIt != custEnd; ++custIt )
                    node->builder->removeCustomElement( node->container, *custIt );

                QPtrListIterator<KAction> actionIt( clientIt.current()->actions );
                for (; actionIt.current(); ++actionIt )
                    actionIt.current()->unplug( node->container );

                // now adjust all merging indices

                mergingIt = node->findIndex( clientIt.current()->mergingName );

                node->adjustMergingIndices( - ( clientIt.current()->actions.count()
                                            + clientIt.current()->customElements.count() ),
                                            mergingIt );

                // unplug all actionslists

                ActionListMap::ConstIterator alIt = clientIt.current()->actionLists.begin();
                ActionListMap::ConstIterator alEnd = clientIt.current()->actionLists.end();
                for (; alIt != alEnd; ++alIt )
                {
                    actionIt = ActionListIt( alIt.data() );
                    for (; actionIt.current(); ++actionIt )
                        actionIt.current()->unplug( node->container );

                    // construct the merging index key (i.e. like named merging) , find the
                    // corresponding merging index and adjust all indices
                    QString mergingKey = alIt.key();
                    mergingKey.prepend( d->tagActionList );

                    MergingIndexList::Iterator mIt = node->findIndex( mergingKey );
                    if ( mIt == node->mergingIndices.end() )
                        continue;

                    node->adjustMergingIndices( - alIt.data().count(), mIt );

                    // remove the actionlists' merging index
                    // ### still needed? we clean up below anyway?
                    node->mergingIndices.remove( mIt );
                }

                node->clients.removeRef( clientIt.current() );
            }
            else
                ++clientIt;
    }

    mergingIt = node->mergingIndices.end();

    // remove all merging indices the client defined
    MergingIndexList::Iterator cmIt = node->mergingIndices.begin();
    while ( cmIt != node->mergingIndices.end() )
        if ( (*cmIt).clientName == d->m_clientName )
            cmIt = node->mergingIndices.remove( cmIt );
        else
            ++cmIt;

    // ### check for merging index count, too?
    if ( node->clients.count() == 0 && node->children.count() == 0 && node->container &&
         node->client == m_client )
    {
        //if at this point the container still contains actions from other clients, then something
        //is wrong with the design of your xml documents ;-) . Anyway, the container was owned by
        //the client, and that client is to be removed, so it's not our problem with other client's
        // actions ;-) .
        //(they will detect the removal through the destroyed() signal anyway :)

        QWidget *parentContainer = 0L;

        // check if we have a parent container and adjust the parent's merging indices
        if ( node->parent && node->parent->container )
        {
            parentContainer = node->parent->container;

            ContainerNode *p = node->parent;

            mergingIt = p->findIndex( node->mergingName );

            p->adjustMergingIndices( -1, mergingIt );
        }

        if ( node == d->m_rootNode ) kdDebug(1002) << "root node !" << endl;
        if ( !node->container ) kdDebug(1002) << "no container !" << endl;

        assert( node->builder );

        //remove/kill the container and give the builder a chance to store abitrary state information of
        //the container in the DOM element. This information will be re-used for the creation of the
        //same container in case we add the same client again later.
        node->builder->removeContainer( node->container, parentContainer, element, node->containerId );

        node->client = 0L;

        //indicate the caller that we successfully killed ourselves ;-) and want to be removed from the
        //parent's child list.
        return true;
    }

    if ( node->client == m_client )
        node->client = 0L;

    return false;
}

/*
 * Calculate a merging index. In fact it's more like "find a good merging index" .
 * Returns the actual index value (for the plug() call for example) and also return
 * the corresponding MergingIndex in the it argument.
 */
int KXMLGUIFactory::calcMergingIndex( KXMLGUI::ContainerNode *node, 
                                      const QString &mergingName,
                                      QValueList<KXMLGUI::MergingIndex>::Iterator &it,
                                      bool ignoreDefaultMergingIndex )
{
    MergingIndexList::Iterator mergingIt;

    // if we are not looking for a special merging name (like a group or an actionlist name) ,
    // then use the client's name, to get the match between <Merge name="blah" /> and the client name.
    if ( mergingName.isEmpty() )
        mergingIt = node->findIndex( d->m_clientName );
    else
        mergingIt = node->findIndex( mergingName );

    MergingIndexList::Iterator mergingEnd = node->mergingIndices.end();
    it = mergingEnd;

    if ( ( mergingIt == mergingEnd && d->m_currentDefaultMergingIt == mergingEnd ) ||
         ignoreDefaultMergingIndex )
        return node->index;

    if ( mergingIt != mergingEnd )
        it = mergingIt;
    else
        it = d->m_currentDefaultMergingIt;

    return (*it).value;
}

QWidget *KXMLGUIFactory::findRecursive( KXMLGUI::ContainerNode *node, bool tag )
{
    if ( ( ( !tag && node->name == d->m_containerName ) ||
           ( tag && node->tagName == d->m_containerName ) ) &&
         ( !m_client || node->client == m_client ) )
        return node->container;

    QPtrListIterator<ContainerNode> it( node->children );
    for (; it.current(); ++it )
    {
        QWidget *cont = findRecursive( it.current(), tag );
        if ( cont )
            return cont;
    }

    return 0L;
}

QPtrList<QWidget> KXMLGUIFactory::findRecursive( KXMLGUI::ContainerNode *node, 
                                                 const QString &tagName )
{
    QPtrList<QWidget> res;

    if ( node->tagName == tagName.lower() )
        res.append( node->container );

    QPtrListIterator<KXMLGUI::ContainerNode> it( node->children );
    for (; it.current(); ++it )
    {
        QPtrList<QWidget> lst = findRecursive( it.current(), tagName );
        QPtrListIterator<QWidget> wit( lst );
        for (; wit.current(); ++wit )
            res.append( wit.current() );
    }

    return res;
}

QWidget *KXMLGUIFactory::createContainer( QWidget *parent, int index, const QDomElement &element,
                                          int &id, KXMLGUIBuilder **builder )
{
    QWidget *res = 0L;

    if ( d->m_clientBuilder )
    {
        res = d->m_clientBuilder->createContainer( parent, index, element, id );

        if ( res )
        {
            *builder = d->m_clientBuilder;
            return res;
        }
    }

    KInstance *oldInstance = m_builder->builderInstance();
    KXMLGUIClient *oldClient = m_builder->builderClient();

    m_builder->setBuilderClient( m_client );

    res = m_builder->createContainer( parent, index, element, id );

    m_builder->setBuilderInstance( oldInstance );
    m_builder->setBuilderClient( oldClient );

    if ( res )
        *builder = m_builder;

    return res;
}

void KXMLGUIFactory::plugActionList( KXMLGUIClient *client, const QString &name,
                                     const QPtrList<KAction> &actionList )
{
    m_client = client;
    d->m_actionListName = name;
    d->m_actionList = actionList;
    d->m_clientName = client->domDocument().documentElement().attribute( d->attrName );

    plugActionListRecursive( d->m_rootNode );

    m_client = 0;
    d->m_actionListName = QString::null;
    d->m_actionList = QPtrList<KAction>();
    d->m_clientName = QString::null;
}

void KXMLGUIFactory::unplugActionList( KXMLGUIClient *client, const QString &name )
{
    m_client = client;
    d->m_actionListName = name;
    d->m_clientName = client->domDocument().documentElement().attribute( d->attrName );

    unplugActionListRecursive( d->m_rootNode );

    m_client = 0;
    d->m_actionListName = QString::null;
    d->m_clientName = QString::null;
}

void KXMLGUIFactory::plugActionListRecursive( KXMLGUI::ContainerNode *node )
{
    MergingIndexList::Iterator mIt( node->mergingIndices.begin() );
    MergingIndexList::Iterator mEnd( node->mergingIndices.end() );
    for (; mIt != mEnd; ++mIt )
    {
        QString k( (*mIt).mergingName );

        if ( k.find( d->tagActionList ) == -1 )
            continue;

        k = k.mid( d->tagActionList.length() );

        if ( (*mIt).clientName != d->m_clientName )
            continue;

        if ( k != d->m_actionListName )
            continue;

        int idx = (*mIt).value;

        ContainerClient *client = node->findChildContainerClient( m_client, QString::null, node->mergingIndices.end() );

        client->actionLists.insert( k, d->m_actionList );

        ActionListIt aIt( d->m_actionList );
        for (; aIt.current(); ++aIt )
            aIt.current()->plug( node->container, idx++ );

        node->adjustMergingIndices( d->m_actionList.count(), mIt );
    }

    QPtrListIterator<ContainerNode> childIt( node->children );
    for (; childIt.current(); ++childIt )
        plugActionListRecursive( childIt.current() );
}

void KXMLGUIFactory::unplugActionListRecursive( KXMLGUI::ContainerNode *node )
{
    MergingIndexList::Iterator mIt( node->mergingIndices.begin() );
    MergingIndexList::Iterator mEnd( node->mergingIndices.end() );
    for (; mIt != mEnd; ++mIt )
    {
        QString k = (*mIt).mergingName;

        if ( k.find( d->tagActionList ) == -1 )
            continue;

        k = k.mid( d->tagActionList.length() );

        if ( (*mIt).clientName != d->m_clientName )
            continue;

        if ( k != d->m_actionListName )
            continue;

        ContainerClient *client = node->findChildContainerClient( m_client, QString::null, node->mergingIndices.end() );

        ActionListMap::Iterator lIt( client->actionLists.find( k ) );
        if ( lIt == client->actionLists.end() )
            continue;

        ActionListIt aIt( lIt.data() );
        for (; aIt.current(); ++aIt )
            aIt.current()->unplug( node->container );

        node->adjustMergingIndices( -lIt.data().count(), mIt );

        client->actionLists.remove( lIt );
    }


    QPtrListIterator<ContainerNode> childIt( node->children );
    for (; childIt.current(); ++childIt )
        unplugActionListRecursive( childIt.current() );
}

void KXMLGUIFactory::processStateElement( const QDomElement &element )
{
    QString stateName = element.attribute( "name" );

    if ( !stateName || !stateName.length() ) return;

    QDomElement e = element.firstChild().toElement();

    for (; !e.isNull(); e = e.nextSibling().toElement() ) {
        QString tagName = e.tagName().lower();

        if ( tagName != "enable" && tagName != "disable" )
            continue;
    
        bool processingActionsToEnable = (tagName == "enable");

        // process action names
        QDomElement actionEl = e.firstChild().toElement();

        for (; !actionEl.isNull(); actionEl = actionEl.nextSibling().toElement() ) {
            if ( actionEl.tagName().lower() != "action" ) continue;

            QString actionName = actionEl.attribute( "name" );
            if ( !actionName || !actionName.length() ) return;

            if ( processingActionsToEnable )
                m_client->addStateActionEnabled( stateName, actionName );
            else
                m_client->addStateActionDisabled( stateName, actionName );

        }

    }
}

void KXMLGUIFactory::applyActionProperties( const QDomElement &actionPropElement )
{
    static const QString &tagAction = KGlobal::staticQString( "action" );

    QDomElement e = actionPropElement.firstChild().toElement();
    for (; !e.isNull(); e = e.nextSibling().toElement() )
    {
        if ( e.tagName().lower() != tagAction )
            continue;

        KAction *action = m_client->action( e );
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

    //don't let someone change the name of the action! (Simon)
    if ( attribute.name() == d->attrName )
        return;

    QVariant propertyValue;

    QVariant::Type propertyType = action->property( attribute.name().latin1() ).type();

    // If the attribute is a depricated "accel", change to "shortcut".
    if ( attrName.lower() == "accel" )
        attrName = attrShortcut;

    if ( propertyType == QVariant::Int )
        propertyValue = QVariant( attribute.value().toInt() );
    else if ( propertyType == QVariant::UInt )
        propertyValue = QVariant( attribute.value().toUInt() );
    else
        propertyValue = QVariant( attribute.value() );

    action->setProperty( attrName.latin1() /* ???????? */, propertyValue );
}

#include "kxmlguifactory.moc"

/* vim: et sw=4
 */
