/* This file is part of the KDE libraries
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

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

#include "kxmlguifactory_p.h"
#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"

#include <qwidget.h>

#include <kglobal.h>
#include <kdebug.h>

#include <assert.h>

using namespace KXMLGUI;

void ActionList::plug( QWidget *container, int index ) const
{
    ActionListIt it( *this );
    for (; it.current(); ++it )
        it.current()->plug( container, index++ );
}

void ActionList::unplug( QWidget *container ) const
{
    ActionListIt it( *this );
    for (; it.current(); ++it )
        it.current()->unplug( container );
}

ContainerNode::ContainerNode( QWidget *_container, const QString &_tagName,
                              const QString &_name, ContainerNode *_parent,
                              KXMLGUIClient *_client, KXMLGUIBuilder *_builder,
                              int id, const QString &_mergingName,
                              const QString &_groupName, const QStringList &customTags,
                              const QStringList &containerTags )
    : parent( _parent ), client( _client ), builder( _builder ), 
      builderCustomTags( customTags ), builderContainerTags( containerTags ), 
      container( _container ), containerId( id ), tagName( _tagName ), name( _name ), 
      groupName( _groupName ), index( 0 ), mergingName( _mergingName )
{
    children.setAutoDelete( true );
    clients.setAutoDelete( true );

    if ( parent )
        parent->children.append( this );
}

void ContainerNode::removeChild( ContainerNode *child )
{
    MergingIndexList::Iterator mergingIt = findIndex( child->mergingName );
    adjustMergingIndices( -1, mergingIt );
    children.removeRef( child );
}

/*
 * Find a merging index with the given name. Used to find an index defined by <Merge name="blah"/>
 * or by a <DefineGroup name="foo" /> tag.
 */
MergingIndexList::Iterator ContainerNode::findIndex( const QString &name )
{
    MergingIndexList::Iterator it( mergingIndices.begin() );
    MergingIndexList::Iterator end( mergingIndices.end() );
    for (; it != end; ++it )
        if ( (*it).mergingName == name )
            return it;
    return it;
}

/*
 * Check if the given container widget is a child of this node and return the node structure
 * if fonud.
 */
ContainerNode *ContainerNode::findContainerNode( QWidget *container )
{
    ContainerNodeListIt it( children );

    for (; it.current(); ++it )
        if ( it.current()->container == container )
            return it.current();

    return 0L;
}

/*
 * Find a container recursively with the given name. Either compares _name with the
 * container's tag name or the value of the container's name attribute. Specified by
 * the tag bool .
 */
ContainerNode *ContainerNode::findContainer( const QString &_name, bool tag )
{
    if ( ( tag && tagName == _name ) ||
         ( !tag && name == _name ) )
        return this;

    ContainerNodeListIt it( children );
    for (; it.current(); ++it )
    {
        ContainerNode *res = it.current()->findContainer( _name, tag );
        if ( res )
            return res;
    }

    return 0;
}

/*
 * Finds a child container node (not recursively) with the given name and tagname. Explicitly
 * leaves out container widgets specified in the exludeList . Also ensures that the containers
 * belongs to currClient.
 */
ContainerNode *ContainerNode::findContainer( const QString &name, const QString &tagName,
                                             const QPtrList<QWidget> *excludeList,
                                             KXMLGUIClient */*currClient*/ )
{
    ContainerNode *res = 0L;
    ContainerNodeListIt nIt( children );

    if ( !name.isEmpty() )
    {
        for (; nIt.current(); ++nIt )
            if ( nIt.current()->name == name && 
                 !excludeList->containsRef( nIt.current()->container ) )
            {
                res = nIt.current();
                break;
            }

        return res;
    }

    if ( !tagName.isEmpty() )
        for (; nIt.current(); ++nIt )
        {
            if ( nIt.current()->tagName == tagName &&
                 !excludeList->containsRef( nIt.current()->container )
                 /* 
                  * It is a bad idea to also compare the client, because
                  * we don't want to do so in situations like these:
                  *
                  * <MenuBar>
                  *   <Menu>
                  *     ...
                  *
                  * other client:
                  * <MenuBar>
                  *   <Menu>
                  *    ...
                  *
                 && nIt.current()->client == currClient )
                 */
                )
            {
                res = nIt.current();
                break;
            }
        }

    return res;
}

ContainerClient *ContainerNode::findChildContainerClient( KXMLGUIClient *currentGUIClient,
                                                          const QString &groupName,
                                                          const MergingIndexList::Iterator &mergingIdx )
{
    if ( !clients.isEmpty() )
    {
        ContainerClientListIt clientIt( clients );

        for (; clientIt.current(); ++clientIt )
            if ( clientIt.current()->client == currentGUIClient )
            {
                if ( groupName.isEmpty() )
                    return clientIt.current();

                if ( groupName == clientIt.current()->groupName )
                    return clientIt.current();
            }
    }

    ContainerClient *client = new ContainerClient;
    client->client = currentGUIClient;
    client->groupName = groupName;

    if ( mergingIdx != mergingIndices.end() )
        client->mergingName = (*mergingIdx).mergingName;

    clients.append( client );

    return client;
}

void ContainerNode::plugActionList( BuildState &state )
{
    MergingIndexList::Iterator mIt( mergingIndices.begin() );
    MergingIndexList::Iterator mEnd( mergingIndices.end() );
    for (; mIt != mEnd; ++mIt )
        plugActionList( state, mIt );

    QPtrListIterator<ContainerNode> childIt( children );
    for (; childIt.current(); ++childIt )
        childIt.current()->plugActionList( state );
}

void ContainerNode::plugActionList( BuildState &state, const MergingIndexList::Iterator &mergingIdxIt )
{
    static const QString &tagActionList = KGlobal::staticQString( "actionlist" );

    MergingIndex mergingIdx = *mergingIdxIt;

    QString k( mergingIdx.mergingName );

    if ( k.find( tagActionList ) == -1 )
        return;

    k = k.mid( tagActionList.length() );

    if ( mergingIdx.clientName != state.clientName )
        return;

    if ( k != state.actionListName )
        return;

    ContainerClient *client = findChildContainerClient( state.guiClient, 
                                                        QString::null, 
                                                        mergingIndices.end() );

    client->actionLists.insert( k, state.actionList );

    state.actionList.plug( container, mergingIdx.value );

    adjustMergingIndices( state.actionList.count(), mergingIdxIt );
}

void ContainerNode::unplugActionList( BuildState &state )
{
    MergingIndexList::Iterator mIt( mergingIndices.begin() );
    MergingIndexList::Iterator mEnd( mergingIndices.end() );
    for (; mIt != mEnd; ++mIt )
        unplugActionList( state, mIt );

    QPtrListIterator<ContainerNode> childIt( children );
    for (; childIt.current(); ++childIt )
        childIt.current()->unplugActionList( state );
}

void ContainerNode::unplugActionList( BuildState &state, const MergingIndexList::Iterator &mergingIdxIt )
{
    static const QString &tagActionList = KGlobal::staticQString( "actionlist" );

    MergingIndex mergingIdx = *mergingIdxIt;

    QString k = mergingIdx.mergingName;

    if ( k.find( tagActionList ) == -1 )
        return;

    k = k.mid( tagActionList.length() );

    if ( mergingIdx.clientName != state.clientName )
        return;

    if ( k != state.actionListName )
        return;

    ContainerClient *client = findChildContainerClient( state.guiClient, 
                                                        QString::null, 
                                                        mergingIndices.end() );

    ActionListMap::Iterator lIt( client->actionLists.find( k ) );
    if ( lIt == client->actionLists.end() )
        return;

    lIt.data().unplug( container );

    adjustMergingIndices( -lIt.data().count(), mergingIdxIt );

    client->actionLists.remove( lIt );
}

void ContainerNode::adjustMergingIndices( int offset,
                                          const MergingIndexList::Iterator &it )
{
    MergingIndexList::Iterator mergingIt = it;
    MergingIndexList::Iterator mergingEnd = mergingIndices.end();

    for (; mergingIt != mergingEnd; ++mergingIt )
        (*mergingIt).value += offset;

    index += offset;
}

bool ContainerNode::destruct( QDomElement element, BuildState &state )
{
    destructChildren( element, state );

    unplugActions( state );

    // remove all merging indices the client defined
    MergingIndexList::Iterator cmIt = mergingIndices.begin();
    while ( cmIt != mergingIndices.end() )
        if ( (*cmIt).clientName == state.clientName )
            cmIt = mergingIndices.remove( cmIt );
        else
            ++cmIt;

    // ### check for merging index count, too?
    if ( clients.count() == 0 && children.count() == 0 && container &&
         client == state.guiClient )
    {
        QWidget *parentContainer = 0L;

        if ( parent && parent->container )
            parentContainer = parent->container;

        assert( builder );

        builder->removeContainer( container, parentContainer, element, containerId );

        client = 0L;

        return true;
    }

    if ( client == state.guiClient )
        client = 0L;

    return false;

}

void ContainerNode::destructChildren( const QDomElement &element, BuildState &state )
{
    QPtrListIterator<ContainerNode> childIt( children );
    while ( childIt.current() )
    {
        ContainerNode *childNode = childIt.current();

        QDomElement childElement = findElementForChild( element, childNode );

        // destruct returns true in case the container really got deleted
        if ( childNode->destruct( childElement, state ) )
            removeChild( childNode );
        else
            ++childIt;
    }
}

QDomElement ContainerNode::findElementForChild( const QDomElement &baseElement,
                                                ContainerNode *childNode )
{
    static const QString &attrName = KGlobal::staticQString( "name" );

    QDomElement e;
    // ### slow
    for ( e = baseElement.firstChild().toElement(); !e.isNull();
          e = e.nextSibling().toElement() )
        if ( e.tagName() == childNode->tagName &&
             e.attribute( attrName ) == childNode->name )
            return e;

    return QDomElement();
}

void ContainerNode::unplugActions( BuildState &state )
{
    if ( !container )
        return;

    ContainerClientListIt clientIt( clients );

    if ( clients.count() == 1 && clientIt.current()->client == client &&
         client == state.guiClient )
        container->hide(); // this container is going to die, that's for sure.
                           // in this case let's just hide it, which makes the
                           // destruction faster

    while ( clientIt.current() )
        //only unplug the actions of the client we want to remove, as the container might be owned
        //by a different client
        if ( clientIt.current()->client == state.guiClient )
        {
            unplugClient( clientIt.current() );
            clients.removeRef( clientIt.current() );
        }
        else
            ++clientIt;
}

void ContainerNode::unplugClient( ContainerClient *client )
{
    static const QString &tagActionList = KGlobal::staticQString( "actionlist" );

    assert( builder );

    // now quickly remove all custom elements (i.e. separators) and unplug all actions

    QValueList<int>::ConstIterator custIt = client->customElements.begin();
    QValueList<int>::ConstIterator custEnd = client->customElements.end();
    for (; custIt != custEnd; ++custIt )
        builder->removeCustomElement( container, *custIt );

    client->actions.unplug( container );

    // now adjust all merging indices

    MergingIndexList::Iterator mergingIt = findIndex( client->mergingName );

    adjustMergingIndices( - ( client->actions.count()
                          + client->customElements.count() ),
                          mergingIt );

    // unplug all actionslists

    ActionListMap::ConstIterator alIt = client->actionLists.begin();
    ActionListMap::ConstIterator alEnd = client->actionLists.end();
    for (; alIt != alEnd; ++alIt )
    {
        alIt.data().unplug( container );

        // construct the merging index key (i.e. like named merging) , find the
        // corresponding merging index and adjust all indices
        QString mergingKey = alIt.key();
        mergingKey.prepend( tagActionList );

        MergingIndexList::Iterator mIt = findIndex( mergingKey );
        if ( mIt == mergingIndices.end() )
            continue;

        adjustMergingIndices( - alIt.data().count(), mIt );

        // remove the actionlists' merging index
        // ### still needed? we clean up below anyway?
        mergingIndices.remove( mIt );
    }
}

void ContainerNode::reset()
{
    QPtrListIterator<ContainerNode> childIt( children );
    for (; childIt.current(); ++childIt )
        childIt.current()->reset();

    if ( client )
        client->setFactory( 0L );
}

int ContainerNode::calcMergingIndex( const QString &mergingName, 
                                     MergingIndexList::Iterator &it,
                                     BuildState &state,
                                     bool ignoreDefaultMergingIndex )
{
    MergingIndexList::Iterator mergingIt;

    if ( mergingName.isEmpty() )
        mergingIt = findIndex( state.clientName );
    else
        mergingIt = findIndex( mergingName );

    MergingIndexList::Iterator mergingEnd = mergingIndices.end();
    it = mergingEnd;

    if ( ( mergingIt == mergingEnd && state.currentDefaultMergingIt == mergingEnd ) ||
         ignoreDefaultMergingIndex )
        return index;

    if ( mergingIt != mergingEnd )
        it = mergingIt;
    else
        it = state.currentDefaultMergingIt;

    return (*it).value;
}

BuildHelper::BuildHelper( BuildState &state, ContainerNode *node )
    : containerClient( 0 ), ignoreDefaultMergingIndex( false ), m_state( state ), 
      parentNode( node )
{
    static const QString &defaultMergingName = KGlobal::staticQString( "<default>" );

    // create a list of supported container and custom tags
    customTags = m_state.builderCustomTags;
    containerTags = m_state.builderContainerTags;

    if ( parentNode->builder != m_state.builder )
    {
        customTags += parentNode->builderCustomTags;
        containerTags += parentNode->builderContainerTags;
    }

    if ( m_state.clientBuilder ) {
        customTags = m_state.clientBuilderCustomTags + customTags;
        containerTags = m_state.clientBuilderContainerTags + containerTags;
    }

    m_state.currentDefaultMergingIt = parentNode->findIndex( defaultMergingName );
    parentNode->calcMergingIndex( QString::null, m_state.currentClientMergingIt, 
                                  m_state, /*ignoreDefaultMergingIndex*/ false );
}

void BuildHelper::build( const QDomElement &element )
{
    QDomElement e = element.firstChild().toElement();
    for (; !e.isNull(); e = e.nextSibling().toElement() )
        processElement( e );
}

void BuildHelper::processElement( const QDomElement &e )
{
    // some often used QStrings
    static const QString &tagAction = KGlobal::staticQString( "action" );
    static const QString &tagMerge = KGlobal::staticQString( "merge" );
    static const QString &tagState = KGlobal::staticQString( "state" );
    static const QString &tagDefineGroup = KGlobal::staticQString( "definegroup" );
    static const QString &tagActionList = KGlobal::staticQString( "actionlist" );
    static const QString &attrName = KGlobal::staticQString( "name" );

    QString tag( e.tagName().lower() );
    QString currName( e.attribute( attrName ) );

    bool isActionTag = ( tag == tagAction );

    if ( isActionTag || customTags.findIndex( tag ) != -1 )
        processActionOrCustomElement( e, isActionTag );
    else if ( containerTags.findIndex( tag ) != -1 )
        processContainerElement( e, tag, currName );
    else if ( tag == tagMerge || tag == tagDefineGroup || tag == tagActionList )
        processMergeElement( tag, currName, e );
    else if ( tag == tagState )
        processStateElement( e );
}

void BuildHelper::processActionOrCustomElement( const QDomElement &e, bool isActionTag )
{
    static const QString &attrGroup = KGlobal::staticQString( "group" );

    if ( !parentNode->container )
        return;

    MergingIndexList::Iterator it( m_state.currentClientMergingIt );

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
        idx = parentNode->calcMergingIndex( group, it, m_state, ignoreDefaultMergingIndex );
    else if ( m_state.currentClientMergingIt == parentNode->mergingIndices.end() )
        // if we don't have a current merging index, then we want to append our action
        idx = parentNode->index;
    else
        idx = (*m_state.currentClientMergingIt).value;

    containerClient = parentNode->findChildContainerClient( m_state.guiClient, group, it );

    bool guiElementCreated = false;
    if ( isActionTag )
        guiElementCreated = processActionElement( e, idx );
    else
        guiElementCreated = processCustomElement( e, idx );

    if ( guiElementCreated )
        // adjust any following merging indices and the current running index for the container
        parentNode->adjustMergingIndices( 1, it );
}

bool BuildHelper::processActionElement( const QDomElement &e, int idx )
{
    assert( m_state.guiClient );

    // look up the action and plug it in
    KAction *action = m_state.guiClient->action( e );

    if ( !action )
        return false;

    action->plug( parentNode->container, idx );

    // save a reference to the plugged action, in order to properly unplug it afterwards.
    containerClient->actions.append( action );

    return true;
}

bool BuildHelper::processCustomElement( const QDomElement &e, int idx )
{
    assert( parentNode->builder );

    int id = parentNode->builder->createCustomElement( parentNode->container, idx, e );
    if ( id == 0 )
        return false;

    containerClient->customElements.append( id );
    return true;
}

void BuildHelper::processStateElement( const QDomElement &element )
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
                m_state.guiClient->addStateActionEnabled( stateName, actionName );
            else
                m_state.guiClient->addStateActionDisabled( stateName, actionName );

        }
    }
}

void BuildHelper::processMergeElement( const QString &tag, const QString &name, const QDomElement &e )
{
    static const QString &tagDefineGroup = KGlobal::staticQString( "definegroup" );
    static const QString &tagActionList = KGlobal::staticQString( "actionlist" );
    static const QString &defaultMergingName = KGlobal::staticQString( "<default>" );
    static const QString &attrGroup = KGlobal::staticQString( "group" );

    QString mergingName( name );
    if ( mergingName.isEmpty() )
    {
        if ( tag == tagDefineGroup )
        {
            kdError(1000) << "cannot define group without name!" << endl;
            return;
        }
        if ( tag == tagActionList )
        {
            kdError(1000) << "cannot define actionlist without name!" << endl;
            return;
        }
        mergingName = defaultMergingName;
    }

    if ( tag == tagDefineGroup )
        mergingName.prepend( attrGroup ); //avoid possible name clashes by prepending
                                              // "group" to group definitions
    else if ( tag == tagActionList )
        mergingName.prepend( tagActionList );

    if ( parentNode->findIndex( mergingName ) != parentNode->mergingIndices.end() )
        return; //do not allow the redefinition of merging indices!

    MergingIndexList::Iterator mIt( parentNode->mergingIndices.end() );

    QString group( e.attribute( attrGroup ) );
    if ( !group.isEmpty() ) 
        group.prepend( attrGroup );

    // calculate the index of the new merging index. Usually this does not need any calculation,
    // we just want the last available index (i.e. append) . But in case the <Merge> tag appears
    // "inside" another <Merge> tag from a previously build client, then we have to use the
    // "parent's" index. That's why we call calcMergingIndex here.
    MergingIndex newIdx;
    newIdx.value = parentNode->calcMergingIndex( group, mIt, m_state, ignoreDefaultMergingIndex );
    newIdx.mergingName = mergingName;
    newIdx.clientName = m_state.clientName;

    // if that merging index is "inside" another one, then append it right after the "parent" .
    if ( mIt != parentNode->mergingIndices.end() )
        parentNode->mergingIndices.insert( ++mIt, newIdx );
    else
        parentNode->mergingIndices.append( newIdx );

    if ( mergingName == defaultMergingName )

        ignoreDefaultMergingIndex = true;

    // re-calculate the running default and client merging indices.
    m_state.currentDefaultMergingIt = parentNode->findIndex( defaultMergingName );
    parentNode->calcMergingIndex( QString::null, m_state.currentClientMergingIt,
                                  m_state, ignoreDefaultMergingIndex );
}

void BuildHelper::processContainerElement( const QDomElement &e, const QString &tag,
                                           const QString &name )
{
    static const QString &attrGroup = KGlobal::staticQString( "group" );
    static const QString &defaultMergingName = KGlobal::staticQString( "<default>" );

    ContainerNode *containerNode = parentNode->findContainer( name, tag,
                                                              &containerList,
                                                              m_state.guiClient );

    if ( !containerNode )
    {
        MergingIndexList::Iterator it( m_state.currentClientMergingIt );

        bool haveGroup = false;
        QString group( e.attribute( attrGroup ) );
        if ( !group.isEmpty() )
        {
            group.prepend( attrGroup );
            haveGroup = true;
        }

        int idx;
        if ( haveGroup )
            idx = parentNode->calcMergingIndex( group, it, m_state, ignoreDefaultMergingIndex );
        else if ( m_state.currentClientMergingIt == parentNode->mergingIndices.end() )
            idx = parentNode->index;
        else
            idx = (*m_state.currentClientMergingIt).value;

        int id;

        KXMLGUIBuilder *builder;

        QWidget *container = createContainer( parentNode->container, idx, e, id, &builder );

        // no container? (probably some <text> tag or so ;-)
        if ( !container )
            return;

        parentNode->adjustMergingIndices( 1, it );

        assert( parentNode->findContainerNode( container ) == 0 );
        
        containerList.append( container );

        QString mergingName;
        if ( it != parentNode->mergingIndices.end() )
            mergingName = (*it).mergingName;

        QStringList cusTags = m_state.builderCustomTags;
        QStringList conTags = m_state.builderContainerTags;
        if ( builder != m_state.builder )
        {
            cusTags = m_state.clientBuilderCustomTags;
            conTags = m_state.clientBuilderContainerTags;
        }

        containerNode = new ContainerNode( container, tag, name, parentNode,
                                           m_state.guiClient, builder, id, 
                                           mergingName, group, cusTags, conTags );
    }

    BuildHelper( m_state, containerNode ).build( e );

    // and re-calculate running values, for better performance
    m_state.currentDefaultMergingIt = parentNode->findIndex( defaultMergingName );
    parentNode->calcMergingIndex( QString::null, m_state.currentClientMergingIt,
                                  m_state, ignoreDefaultMergingIndex );
}

QWidget *BuildHelper::createContainer( QWidget *parent, int index, 
                                       const QDomElement &element, int &id, 
                                       KXMLGUIBuilder **builder )
{
    QWidget *res = 0L;

    if ( m_state.clientBuilder )
    {
        res = m_state.clientBuilder->createContainer( parent, index, element, id );

        if ( res )
        {
            *builder = m_state.clientBuilder;
            return res;
        }
    }

    KInstance *oldInstance = m_state.builder->builderInstance();
    KXMLGUIClient *oldClient = m_state.builder->builderClient();

    m_state.builder->setBuilderClient( m_state.guiClient );

    res = m_state.builder->createContainer( parent, index, element, id );

    m_state.builder->setBuilderInstance( oldInstance );
    m_state.builder->setBuilderClient( oldClient );

    if ( res )
        *builder = m_state.builder;

    return res;
}

void BuildState::reset()
{
    clientName = QString::null;
    actionListName = QString::null;
    actionList.clear();
    guiClient = 0;
    clientBuilder = 0;

    currentDefaultMergingIt = currentClientMergingIt = MergingIndexList::Iterator();
}

/* vim: et sw=4
 */
