
#include "kxmlguifactory_p.h"

using namespace KXMLGUI;

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
QValueList<MergingIndex>::Iterator ContainerNode::findIndex( const QString &name )
{
    QValueList<MergingIndex>::Iterator it( mergingIndices.begin() );
    QValueList<MergingIndex>::Iterator end( mergingIndices.end() );
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
    QPtrListIterator<ContainerNode> it( children );

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

    QPtrListIterator<ContainerNode> it( children );
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
    QPtrListIterator<ContainerNode> nIt( children );

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

void ContainerNode::adjustMergingIndices( int offset,
                                          const QValueList<MergingIndex>::Iterator &it )
{
    QValueList<MergingIndex>::Iterator mergingIt = it;
    QValueList<MergingIndex>::Iterator mergingEnd = mergingIndices.end();

    for (; mergingIt != mergingEnd; ++mergingIt )
        (*mergingIt).value += offset;

    index += offset;
}

/* vim: et sw=4
 */
