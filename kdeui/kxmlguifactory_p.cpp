
#include "kxmlguifactory_p.h"
#include "kxmlguifactory.h"
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
                                             KXMLGUIClient *currClient )
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
                 !excludeList->containsRef( nIt.current()->container ) &&
                 nIt.current()->client == currClient )
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

void ContainerNode::reset()
{
    QPtrListIterator<ContainerNode> childIt( children );
    for (; childIt.current(); ++childIt )
        childIt.current()->reset();

    if ( client )
        client->setFactory( 0L );
}

BuildHelper::BuildHelper( BuildState &state, const QStringList &_customTags,
                          const QStringList &_containerTags, ContainerNode *node, 
                          KXMLGUIFactory *_factory )
    : customTags( _customTags ), containerTags( _containerTags ), containerClient( 0 ),
      ignoreDefaultMergingIndex( false ), m_state( state ), m_factory( _factory ), 
      parentNode( node )
{
    static const QString &defaultMergingName = KGlobal::staticQString( "<default>" );

    m_state.currentDefaultMergingIt = parentNode->findIndex( defaultMergingName );
    m_factory->calcMergingIndex( parentNode, QString::null, m_state.currentClientMergingIt, false );
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

    bool isActionTag = false;

    if ( ( isActionTag = ( tag == tagAction ) ) ||
           customTags.findIndex( tag ) != -1 )
        processActionOrCustomElement( e, isActionTag );
    /*
     * The "Merge" tag specifies that all containers and actions from *other* clients should be
     * inserted/plugged in at the current index, and not appended.
     * Same semantics apply for <DefineGroup> and <ActionList> . We simply want to save the
     * index of where the tags appear, in order to "substitute" them with other clients actions,
     * actionlists, containers, custom elements or even merging indices.
     */
    else if ( tag == tagMerge || tag == tagDefineGroup || tag == tagActionList )
        processMergeElement( tag, currName );
    else if ( tag == tagState )
        processStateElement( e );
    else if ( containerTags.findIndex( tag ) != -1 )
        processContainerElement( e, tag, currName );
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
        idx = m_factory->calcMergingIndex( parentNode, group, it, ignoreDefaultMergingIndex );
    else if ( m_state.currentClientMergingIt == parentNode->mergingIndices.end() )
        // if we don't have a current merging index, then we want to append our action
        idx = parentNode->index;
    else
        idx = (*m_state.currentClientMergingIt).value;

    containerClient = parentNode->findChildContainerClient( m_state.guiClient, group, it );

    if ( isActionTag )
        processActionElement( e, idx );
    else
        processCustomElement( e, idx );

    // adjust any following merging indices and the current running index for the container
    parentNode->adjustMergingIndices( 1, it );
}

void BuildHelper::processActionElement( const QDomElement &e, int idx )
{
    // look up the action and plug it in
    KAction *action = m_state.guiClient->action( e );

    if ( !action )
        return;

    action->plug( parentNode->container, idx );

    // save a reference to the plugged action, in order to properly unplug it afterwards.
    containerClient->actions.append( action );
}

void BuildHelper::processCustomElement( const QDomElement &e, int idx )
{
    assert( parentNode->builder );

    int id = parentNode->builder->createCustomElement( parentNode->container, idx, e );
    if ( id != 0 )
        containerClient->customElements.append( id );
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

void BuildHelper::processMergeElement( const QString &tag, const QString &name )
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

    // calculate the index of the new merging index. Usually this does not need any calculation,
    // we just want the last available index (i.e. append) . But in case the <Merge> tag appears
    // "inside" another <Merge> tag from a previously build client, then we have to use the
    // "parent's" index. That's why we call calcMergingIndex here.
    MergingIndex newIdx;
    newIdx.value = m_factory->calcMergingIndex( parentNode,
                                                QString::null /* ### allow group for <merge/> ? */ ,
                                                mIt, ignoreDefaultMergingIndex );
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
    m_factory->calcMergingIndex( parentNode, QString::null, m_state.currentClientMergingIt,
                                 ignoreDefaultMergingIndex );
}

void BuildHelper::processContainerElement( const QDomElement &e, const QString &tag,
                                           const QString &name )
{
    static const QString &attrGroup = KGlobal::staticQString( "group" );
    static const QString &defaultMergingName = KGlobal::staticQString( "<default>" );

    ContainerNode *matchingContainer = parentNode->findContainer( name, tag,
                                                                  &containerList,
                                                                  m_state.guiClient );

    if ( matchingContainer )
    {
        /*
         * Enter the next level, as the container already exists :)
         */
        m_factory->buildRecursive( e, matchingContainer );
        // re-calculate current default merging indices and client merging indices,
        // as they have changed in the recursive invocation.
        m_state.currentDefaultMergingIt = parentNode->findIndex( defaultMergingName );
        m_factory->calcMergingIndex( parentNode, QString::null, m_state.currentClientMergingIt,
                                     ignoreDefaultMergingIndex );
    }
    else
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
            idx = m_factory->calcMergingIndex( parentNode, group, it, ignoreDefaultMergingIndex );
            else if ( m_state.currentClientMergingIt == parentNode->mergingIndices.end() )
                idx = parentNode->index;
        else
            idx = (*m_state.currentClientMergingIt).value;

        /*
         * let the builder create the container
         */

        int id;

        KXMLGUIBuilder *builder;

        QWidget *container = m_factory->createContainer( parentNode->container, idx, e, id, &builder );

        // no container? (probably some <text> tag or so ;-)
        if ( !container )
            return;

        parentNode->adjustMergingIndices( 1, it );

        ContainerNode *containerNode = parentNode->findContainerNode( container );

        if ( !containerNode ) // this should be true all times
        {
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

        m_factory->buildRecursive( e, containerNode );

        // and re-calculate running values, for better performance
        m_state.currentDefaultMergingIt = parentNode->findIndex( defaultMergingName );
        m_factory->calcMergingIndex( parentNode, QString::null, m_state.currentClientMergingIt,
                                     ignoreDefaultMergingIndex );
    }
}

void BuildState::reset()
{
    clientName = QString::null;
    actionListName = QString::null;
    actionList.clear();
    guiClient = 0;

    currentDefaultMergingIt = currentClientMergingIt = MergingIndexList::Iterator();
}

/* vim: et sw=4
 */
