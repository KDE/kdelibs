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

#include <kxmlgui.h>

#include <assert.h>
#include <qaction.h>
#include <qfile.h>
#include <kdebug.h>

class KXMLGUIServantPrivate
{
public:
  KXMLGUIServantPrivate()
  {
    m_factory = 0L;
    m_parent = 0L;
    m_builder = 0L;
  }

  ~KXMLGUIServantPrivate()
  {
  }

  QMap<QString,QByteArray> m_containerStates;
  KXMLGUIFactory *m_factory;
  KXMLGUIServant *m_parent;
  QList<KXMLGUIServant> m_children;
  KXMLGUIBuilder *m_builder;
};

/**
 * This structure is used to know to which servant certain actions belong. In addition we store
 * a boolean value indicating if the actions have been plugged with a merging index or not.
 * A ContainerClient always belongs to a ContainerNode.
 */
struct KXMLGUIContainerClient
{
  KXMLGUIServant *m_servant;
  QList<QAction> m_actions;
  bool m_mergedClient;
  QValueList<int> m_separators;
  QString m_groupName; //is empty if no group client
};

template class QList<KXMLGUIContainerClient>;
class KXMLGUIContainerNode;
template class QList<KXMLGUIContainerNode>;

/**
 * Here we store detailed information about a container, its clients (client=a servant having actions
 * plugged into the container), child nodes, naming information (tagname and name attribute) and
 * index information, used to plug in actions at the correct index for correct GUI merging. In addition
 * we store a boolean value indicating whether the container was inserted into the parent container via
 * a merging-index or not. A merging index is used to plug in child actions and child containers at a
 * specified index, in order to merge the GUI correctly.
 */
struct KXMLGUIContainerNode
{
  KXMLGUIContainerNode( QWidget *_container, const QString &_tagName, const QString &_name, KXMLGUIContainerNode *_parent = 0L, KXMLGUIServant *_servant = 0L, KXMLGUIBuilder *_builder = 0L, bool _merged = false, int id = -1, const QString &groupName = QString::null );

  KXMLGUIContainerNode *parent;
  KXMLGUIServant *servant;
  KXMLGUIBuilder *builder;
  QWidget *container;
  int containerId;

  QString tagName;
  QString name;

  QString groupName; //is empty if the container is in no group

  QList<KXMLGUIContainerClient> clients;
  QList<KXMLGUIContainerNode> children;

  int index;
  QMap<QString,int> mergingIndices;

  bool mergedContainer;
};

class KXMLGUIFactoryPrivate
{
public:
  KXMLGUIFactoryPrivate()
  {
    m_rootNode = new KXMLGUIContainerNode( 0L, QString::null, 0L );
    m_defaultMergingName = QString::fromLatin1( "<default>" );
    m_servantBuilder = 0L;
  }
  ~KXMLGUIFactoryPrivate()
  {
    delete m_rootNode;
  }

  KXMLGUIContainerNode *m_rootNode;
  QString m_servantName;
  QString m_defaultMergingName;
  QString m_containerName;
  KXMLGUIBuilder *m_servantBuilder;
};

KXMLGUIServant::KXMLGUIServant()
{
  d = new KXMLGUIServantPrivate;
}

KXMLGUIServant::KXMLGUIServant( KXMLGUIServant *parent )
{
  d = new KXMLGUIServantPrivate;
  d->m_parent = parent;
}

KXMLGUIServant::~KXMLGUIServant()
{
  kDebugArea( 1000, "KXMLGUIServant::~KXMLGUIServant()");

  if ( d->m_parent )
    d->m_parent->removeChildServant( this );

  QListIterator<KXMLGUIServant> childIt( d->m_children );
  for (; childIt.current(); ++childIt )
    childIt.current()->d->m_parent = 0L;

  d->m_children.setAutoDelete( true );
  d->m_children.clear();

  delete d;
}

void KXMLGUIServant::storeContainerStateBuffer( const QString &key, const QByteArray &data )
{
  d->m_containerStates.replace( key, data );
}

QByteArray KXMLGUIServant::takeContainerStateBuffer( const QString &key )
{
  QByteArray res;

  QMap<QString,QByteArray>::Iterator it = d->m_containerStates.find( key );
  if ( it != d->m_containerStates.end() )
  {
    res = it.data();
    d->m_containerStates.remove( it );
  }

  return res;
}

void KXMLGUIServant::setFactory( KXMLGUIFactory *factory )
{
  d->m_factory = factory;
}

KXMLGUIFactory *KXMLGUIServant::factory() const
{
  return d->m_factory;
}

KXMLGUIServant *KXMLGUIServant::parentServant() const
{
  return d->m_parent;
}

void KXMLGUIServant::insertChildServant( KXMLGUIServant *child )
{
  if ( child->parentServant() )
    child->parentServant()->removeChildServant( child );

  d->m_children.append( child );
}

void KXMLGUIServant::removeChildServant( KXMLGUIServant *child )
{
  d->m_children.removeRef( child );
}

const QList<KXMLGUIServant> *KXMLGUIServant::childServants()
{
  return &d->m_children;
}

void KXMLGUIServant::setServantBuilder( KXMLGUIBuilder *builder )
{
  d->m_builder = builder;
}

KXMLGUIBuilder *KXMLGUIServant::servantBuilder() const
{
  return d->m_builder;
}

KXMLGUIContainerNode::KXMLGUIContainerNode( QWidget *_container, const QString &_tagName, const QString &_name, KXMLGUIContainerNode *_parent, KXMLGUIServant *_servant, KXMLGUIBuilder *_builder, bool _merged, int id, const QString &_groupName )
{
  container = _container;
  containerId = id;
  parent = _parent;
  servant = _servant;
  builder = _builder;
  tagName = _tagName;
  name = _name;
  groupName = _groupName;
  children.setAutoDelete( true );
  clients.setAutoDelete( true );
  index = 0;
  mergedContainer = _merged;

  if ( parent )
    parent->children.append( this );
}

QString KXMLGUIFactory::readConfigFile( const QString &filename )
{
  QFile file( filename );
  if ( !file.open( IO_ReadOnly ) )
  {
    kDebugError( 1000, "No such XML file %s", filename.local8Bit().data() );
    return QString::null;
  }

  uint size = file.size();
  char* buffer = new char[ size + 1 ];
  file.readBlock( buffer, size );
  buffer[ size ] = 0;
  file.close();

  QString text = QString::fromUtf8( buffer, size );
  delete[] buffer;

  return text;
}

KXMLGUIFactory::KXMLGUIFactory( KXMLGUIBuilder *builder )
{
  d = new KXMLGUIFactoryPrivate;
  m_builder = builder;
  m_servant = 0L;
}

KXMLGUIFactory::~KXMLGUIFactory()
{
  kDebugArea( 1002, "KXMLGUIFactory::~KXMLGUIFactory(), calling removeRecursive" );
  delete d;
}

void KXMLGUIFactory::addServant( KXMLGUIServant *servant )
{
  m_servant = servant;

  if ( servant->factory() && servant->factory() != this )
    servant->factory()->removeServant( servant ); //just in case someone does stupid things ;-)

  QDomElement docElement = servant->document().documentElement();

  d->m_rootNode->index = -1;
  d->m_servantName = docElement.attribute( "name" );
  d->m_servantBuilder = servant->servantBuilder();

  buildRecursive( docElement, d->m_rootNode );

  servant->setFactory( this );

  m_servant = 0L;
  d->m_servantName = QString::null;
  d->m_servantBuilder = 0L;

  if ( servant->childServants()->count() > 0 )
  {
    const QList<KXMLGUIServant> *children = servant->childServants();
    QListIterator<KXMLGUIServant> childIt( *children );
    for (; childIt.current(); ++childIt )
      addServant( childIt.current() );
  }
}

void KXMLGUIFactory::removeServant( KXMLGUIServant *servant )
{
  if ( servant->factory() && servant->factory() != this )
    return;

  if ( servant->childServants()->count() > 0 )
  {
    const QList<KXMLGUIServant> *children = servant->childServants();
    QListIterator<KXMLGUIServant> childIt( *children );
    for (; childIt.current(); ++childIt )
      removeServant( childIt.current() );
  }

  kDebugArea( 1002, "KXMLGUIFactory::removeServant, calling removeRecursive" );
  m_servant = servant;
  d->m_servantName = servant->document().documentElement().attribute( "name" );
  d->m_servantBuilder = servant->servantBuilder();
  servant->setFactory( 0L );
  removeRecursive( d->m_rootNode );
  m_servant = 0L;
  d->m_servantBuilder = 0L;
  d->m_servantName = QString::null;
}

QWidget *KXMLGUIFactory::container( const QString &containerName, KXMLGUIServant *servant )
{
  d->m_containerName = containerName;
  m_servant = servant;

  QWidget *result = findRecursive( d->m_rootNode );

  m_servant = 0L;
  d->m_containerName = QString::null;

  return result;
}

void KXMLGUIFactory::buildRecursive( const QDomElement &element, KXMLGUIContainerNode *parentNode )
{
  static QString tagAction = QString::fromLatin1( "action" );
  static QString tagMerge = QString::fromLatin1( "merge" );
  static QString tagSeparator = QString::fromLatin1( "separator" );
  static QString tagDefineGroup = QString::fromLatin1( "definegroup" );
  static QString attrName = QString::fromLatin1( "name" );
  static QString attrGroup = QString::fromLatin1( "group" );

  /*
   * This list contains references to all the containers we created on the current level.
   * We use it as "exclude" list, in order to avoid container matches of already created containers having
   * no proper name attribute to distinct them (like with Separator tags).
   */
  QList<QWidget> containerList;

  KXMLGUIContainerClient *containerClient = 0L;
  
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
  bool ignoreMergingIndex = false;

  QDomElement e = element.firstChild().toElement();
  for (; !e.isNull(); e = e.nextSibling().toElement() )
  {
    QString tag = e.tagName().lower();

    /*
     * The "Merge" tag specifies that all containers and actions from *other* servants should be
     * inserted/plugged in at the current index, and not at the "end" .
     */
    if ( tag == tagMerge || tag == tagDefineGroup )
    {
      QString mergingName = e.attribute( attrName );
      if ( mergingName.isEmpty() )
      {
        if ( tag == tagDefineGroup )
	{
          kDebugError( 1000, "cannot define group without name!");
	  continue;
	}
        mergingName = d->m_defaultMergingName;
      }

      if ( tag == tagDefineGroup )
        mergingName.prepend( attrGroup ); //avoid possible name clashes by prepending "group" to group
                                          //definitions
      
      if ( parentNode->mergingIndices.contains( mergingName ) )
        continue; //do not allow the redefinition of merging indices!
      
      parentNode->mergingIndices.insert( mergingName, parentNode->index );

      ignoreMergingIndex = true;
    }
    else if ( tag == tagAction || tag == tagSeparator )
    {
      if ( !parentNode->container )
        continue;

      QMap<QString,int>::Iterator it;
      
      QString group = e.attribute( attrGroup );
      if ( !group.isEmpty() )
        group.prepend( attrGroup );
      
      int idx = parentNode->index;
      bool merge = calcMergingIndex( parentNode, group, it );
	
      if ( merge && !ignoreMergingIndex )
	idx = it.data();
      else
	it = parentNode->mergingIndices.end();

      containerClient = findClient( parentNode, group );
      
      containerClient->m_mergedClient = merge;
      
      /*      
      if ( !containerClient )
      {
	containerClient = new KXMLGUIContainerClient;
	containerClient->m_servant = m_servant;
	containerClient->m_mergedClient = merge;
	parentNode->clients.append( containerClient );
      }
      */
      
      if ( tag == tagAction )
      {
        QAction *action = m_servant->action( e );

	if ( !action )
	  continue;
	
        action->plug( (QWidget *)parentNode->container, idx );

	containerClient->m_actions.append( action );
      }
      else
      {
        assert( parentNode->builder );
        int id = parentNode->builder->insertSeparator( (QWidget *)parentNode->container, idx );
	containerClient->m_separators.append( id );
      }

      adjustMergingIndices( parentNode, idx, 1, it );
    }
    else
    {
      /*
       * No Action or Merge tag? That most likely means that we want to create a new container.
       * But first we have to check if there's already a existing (child) container of the same type in our
       * tree. However we have to ignore just newly created containers!
       */

      KXMLGUIContainerNode *matchingContainer = findContainer( parentNode, e, &containerList );

      if ( matchingContainer )
        /*
	 * Enter the next level, as the container already exists :)
	 */
        buildRecursive( e, matchingContainer );
      else
      {	
	QMap<QString,int>::Iterator it;

	QString group = e.attribute( attrGroup );
        if ( !group.isEmpty() )
          group.prepend( attrGroup );
	
        int idx = parentNode->index;
	bool merge = calcMergingIndex( parentNode, group, it );

	if ( merge && !ignoreMergingIndex )
	  idx = it.data();
       	else
       	  it = parentNode->mergingIndices.end();
	
        // query the servant for possible container state information (like toolbar positions for example)
	// (the array might be empty in case there's no info available)
	QByteArray stateBuffer = m_servant->takeContainerStateBuffer( e.tagName() + e.attribute( attrName ) );
	
	/*
	 * let the builder create the container
	 */
	
	int id;
	
	KXMLGUIBuilder *builder;
	
        QWidget *container = createContainer( parentNode->container, idx, e, stateBuffer, id, &builder );
	
	// no container? (probably some <text> tag or so ;-)
	if ( !container )
	  continue;
	
	adjustMergingIndices( parentNode, idx, 1, it );
	
	KXMLGUIContainerNode *containerNode = findContainerNode( parentNode, container );
	
	if ( !containerNode )
	{
  	  containerList.append( container );
	
          containerNode = new KXMLGUIContainerNode( container, e.tagName(), e.attribute( attrName ), parentNode, m_servant, builder, merge, id, group );
	}
	
        buildRecursive( e, containerNode );
      }
    }
  }

}

bool KXMLGUIFactory::removeRecursive( KXMLGUIContainerNode *node )
{
  QListIterator<KXMLGUIContainerNode> childIt( node->children );
  while ( childIt.current() )
    // removeRecursive returns true in case the container really got deleted
    if ( removeRecursive( childIt.current() ) )
      node->children.removeRef( childIt.current() );
    else
      ++childIt;

  QMap<QString,int>::Iterator mergingIt = node->mergingIndices.end();

  QListIterator<KXMLGUIContainerClient> clientIt( node->clients );

  if ( node->container )
    while ( clientIt.current() )
      //only unplug the actions of the servant we want to remove, as the container might be owned
      //by a different servant
      if ( clientIt.current()->m_servant == m_servant )
      {
        assert( node->builder );
	
        QValueList<int>::ConstIterator sepIt = clientIt.current()->m_separators.begin();
	QValueList<int>::ConstIterator sepEnd = clientIt.current()->m_separators.end();
	for (; sepIt != sepEnd; ++sepIt )
	  node->builder->removeSeparator( (QWidget *)node->container, *sepIt );

        QListIterator<QAction> actionIt( clientIt.current()->m_actions );
        for (; actionIt.current(); ++actionIt )
        {
          kDebugInfo( 1002, "unplugging %s from %s", actionIt.current()->name(), (QWidget *)node->container->name() );
          actionIt.current()->unplug( (QWidget *)node->container );
        }
	
        calcMergingIndex( node, clientIt.current()->m_groupName, mergingIt );
	
	int idx = node->index;

	if ( clientIt.current()->m_mergedClient )
	  idx = mergingIt.data();
	else
	  mergingIt = node->mergingIndices.end();
	
	adjustMergingIndices( node, idx, - ( clientIt.current()->m_actions.count() + clientIt.current()->m_separators.count() ), mergingIt );
	
	node->clients.removeRef( clientIt.current() );
      }
      else
        ++clientIt;

  if ( node->clients.count() == 0 && node->children.count() == 0 && node->container &&
       node->servant == m_servant )
  {
    //if at this point the container still contains actions from other servants, then something is wrong
    //with the design of your xml documents ;-) . Anyway, the container was owned by the servant, and that
    //servant is to be removed, so it's not our problem with other servant's actions ;-) .
    //(they will detect the removal through the destroyed() signal anyway :)

    QWidget *parentContainer = 0L;

    if ( node->parent && node->parent->container )
    {
      parentContainer = (QWidget *)node->parent->container;

      KXMLGUIContainerNode *p = node->parent;

      calcMergingIndex( p, node->groupName, mergingIt );

      int idx = p->index;

      if ( node->mergedContainer )
        idx = mergingIt.data();
      else
        mergingIt = p->mergingIndices.end();

      adjustMergingIndices( p, idx, -1, mergingIt );
    }

    if ( node == d->m_rootNode ) kDebugInfo( 1002, "root node !" );
    if ( !node->container ) kDebugInfo( 1002, "no container !" );

    assert( node->builder );

    kDebugInfo( 1002, "remove/kill stuff : node is %s, container is %s (%s), parent container is %s", node->name.ascii(), node->container->name(), node->container->className(), parentContainer ? parentContainer->name() : 0L );
    //remove/kill the container and give the builder a chance to store abitrary state information of
    //the container in a QByteArray. This information will be re-used for the creation of the same
    //container in case we add the same servant again later.
    QByteArray containerStateBuffer = node->builder->removeContainer( node->container, parentContainer, node->containerId );

    if ( containerStateBuffer.size() > 0 )
      m_servant->storeContainerStateBuffer( node->tagName + node->name, containerStateBuffer );

    node->servant = 0L;

    //indicate the caller that we successfully killed ourselves ;-) and want to be removed from the
    //parent's child list.
    return true;
  }

  if ( node->servant == m_servant )
    node->servant = 0L;

  return false;
}

bool KXMLGUIFactory::calcMergingIndex( KXMLGUIContainerNode *node, const QString &mergingName, QMap<QString,int>::Iterator &it )
{
  QMap<QString,int>::Iterator defaultMergingIt = node->mergingIndices.find( d->m_defaultMergingName );
  QMap<QString,int>::Iterator mergingIt;
  
  if ( !mergingName.isEmpty() )
    mergingIt = node->mergingIndices.find( mergingName );
  else
    mergingIt = node->mergingIndices.find( d->m_servantName );
  
  QMap<QString,int>::Iterator mergingEnd = node->mergingIndices.end();
  it = mergingEnd;

  if ( mergingIt == mergingEnd && defaultMergingIt == mergingEnd )
    return false;

  if ( mergingIt != mergingEnd )
    it = mergingIt;
  else
    it = defaultMergingIt;

  return true;
}

void KXMLGUIFactory::adjustMergingIndices( KXMLGUIContainerNode *node, int idx, int val, const QMap<QString,int>::Iterator &it )
{
  QMap<QString,int>::Iterator mergingIt = node->mergingIndices.begin();
  QMap<QString,int>::Iterator mergingEnd = node->mergingIndices.end();
  for (; mergingIt != mergingEnd; ++mergingIt )
    if ( mergingIt.data() > idx || mergingIt == it )
      mergingIt.data() += val;
	
  node->index += val;
}

KXMLGUIContainerNode *KXMLGUIFactory::findContainer( KXMLGUIContainerNode *node, const QDomElement &element, const QList<QWidget> *excludeList )
{
  KXMLGUIContainerNode *res = 0L;
  QListIterator<KXMLGUIContainerNode> nIt( node->children );

  QString name = element.attribute( "name" );

  if ( !name.isEmpty() )
  {
    for (; nIt.current(); ++nIt )
     if ( nIt.current()->name == name && !excludeList->containsRef( nIt.current()->container ) )
     {
       res = nIt.current();
       break;
     }

    return res;
  }

  name = element.tagName();

  if ( !name.isEmpty() )
    for (; nIt.current(); ++nIt )
    {
      if ( nIt.current()->tagName == name && !excludeList->containsRef( nIt.current()->container ) &&
	   nIt.current()->servant == m_servant )
      {
        res = nIt.current();
	break;
      }
    }

  return res;
}

KXMLGUIContainerNode *KXMLGUIFactory::findContainerNode( KXMLGUIContainerNode *parentNode, QWidget *container )
{
  QListIterator<KXMLGUIContainerNode> it( parentNode->children );

  for (; it.current(); ++it )
    if ( it.current()->container == container )
      return it.current();

  return 0L;
}

QWidget *KXMLGUIFactory::findRecursive( KXMLGUIContainerNode *node )
{
  if ( node->name == d->m_containerName && node->servant == m_servant )
    return node->container;

  QListIterator<KXMLGUIContainerNode> it( node->children );
  for (; it.current(); ++it )
  {
    QWidget *cont = findRecursive( it.current() );
    if ( cont )
      return cont;
  }

  return 0L;
}

QWidget *KXMLGUIFactory::createContainer( QWidget *parent, int index, const QDomElement &element, const QByteArray &containerStateBuffer, int &id, KXMLGUIBuilder **builder )
{
  QWidget *res = 0L;

  if ( d->m_servantBuilder )
  {
    res = d->m_servantBuilder->createContainer( parent, index, element, containerStateBuffer, id );

    if ( res )
    {
      *builder = d->m_servantBuilder;
      return res;
    }
  }

  res = m_builder->createContainer( parent, index, element, containerStateBuffer, id );

  if ( res )
    *builder = m_builder;

  return res;
}

KXMLGUIContainerClient *KXMLGUIFactory::findClient( KXMLGUIContainerNode *node, const QString &groupName )
{
  QListIterator<KXMLGUIContainerClient> clientIt( node->clients );
  
  for (; clientIt.current(); ++clientIt )
    if ( clientIt.current()->m_servant == m_servant )
    {
      if ( groupName.isEmpty() )
        return clientIt.current();
      
      if ( groupName == clientIt.current()->m_groupName )
        return clientIt.current();
    }
  
  KXMLGUIContainerClient *client = new KXMLGUIContainerClient;
  client->m_servant = m_servant;
  client->m_groupName = groupName;
  
  node->clients.append( client );
  
  return client;
} 
