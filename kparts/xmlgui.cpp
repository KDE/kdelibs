/* This file is part of the KDE libraries
    Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>

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

#include <kparts/xmlgui.h>

#include <qfile.h>
#include <kdebug.h>
#include <assert.h>

using namespace KParts;

namespace KParts
{
class XMLGUIServantPrivate
{
public:
  XMLGUIServantPrivate()
  {
    m_factory = 0L;
  }

  ~XMLGUIServantPrivate()
  {
  }

  QMap<QString,QByteArray> m_containerStates;
  XMLGUIFactory *m_factory;
};

/**
 * This structure is used to know to which servant certain actions belong. In addition we store
 * a boolean value indicating if the actions have been plugged with a merging index or not.
 * A ContainerClient always belongs to a ContainerNode.
 */
struct XMLGUIContainerClient
{
  XMLGUIServant *m_servant;
  QList<QAction> m_actions;
  bool m_mergedClient;
  QValueList<int> m_separators;
};

template class QList<XMLGUIContainerClient>;
class XMLGUIContainerNode;
template class QList<XMLGUIContainerNode>;

/**
 * Here we store detailed information about a container, its clients (client=a servant having actions
 * plugged into the container), child nodes, naming information (tagname and name attribute) and
 * index information, used to plug in actions at the correct index for correct GUI merging. In addition
 * we store a boolean value indicating whether the container was inserted into the parent container via
 * a merging-index or not. A merging index is used to plug in child actions and child containers at a
 * specified index, in order to merge the GUI correctly.
 */
struct XMLGUIContainerNode
{
  XMLGUIContainerNode( QObject *_container, const QString &_tagName, const QString &_name, XMLGUIContainerNode *_parent = 0L, XMLGUIServant *_servant = 0L, bool _merged = false, int id = -1 );

  XMLGUIContainerNode *parent;
  XMLGUIServant *servant;
  QObject *container;
  int containerId;

  QString tagName;
  QString name;

  QList<XMLGUIContainerClient> clients;
  QList<XMLGUIContainerNode> children;

  int index;
  QMap<QString,int> mergingIndices;

  bool mergedContainer;
};

class XMLGUIFactoryPrivate
{
public:
  XMLGUIFactoryPrivate()
  {
    m_rootNode = new XMLGUIContainerNode( 0L, QString::null, 0L );
    m_defaultMergingName = QString::fromLatin1( "<default>" );
  }
  ~XMLGUIFactoryPrivate()
  {
    delete m_rootNode;
  }

  XMLGUIContainerNode *m_rootNode;
  QString m_servantName;
  QString m_defaultMergingName;

  int m_genId;
};

};

XMLGUIServant::XMLGUIServant()
{
  d = new XMLGUIServantPrivate;
}

XMLGUIServant::~XMLGUIServant()
{
  kDebugArea( 1000, "XMLGUIServant::~XMLGUIServant()");
  //  if ( d->m_factory )
  //   d->m_factory->removeServant( this );

  delete d;
}

void XMLGUIServant::storeContainerStateBuffer( const QString &key, const QByteArray &data )
{
  d->m_containerStates.replace( key, data );
}

QByteArray XMLGUIServant::takeContainerStateBuffer( const QString &key )
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

void XMLGUIServant::setFactory( XMLGUIFactory *factory )
{
  d->m_factory = factory;
}

XMLGUIFactory *XMLGUIServant::factory() const
{
  return d->m_factory;
}

XMLGUIContainerNode::XMLGUIContainerNode( QObject *_container, const QString &_tagName, const QString &_name, XMLGUIContainerNode *_parent, XMLGUIServant *_servant, bool _merged, int id )
{
  container = _container;
  containerId = id;
  parent = _parent;
  servant = _servant;
  tagName = _tagName;
  name = _name;
  children.setAutoDelete( true );
  clients.setAutoDelete( true );
  index = 0;
  mergedContainer = _merged;

  if ( parent )
    parent->children.append( this );
}

QString XMLGUIFactory::readConfigFile( const QString &filename )
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

XMLGUIFactory::XMLGUIFactory( XMLGUIBuilder *builder )
{
  d = new XMLGUIFactoryPrivate;
  m_builder = builder;
  m_servant = 0L;
}

XMLGUIFactory::~XMLGUIFactory()
{
  kDebugArea( 1002, "XMLGUIFactory::~XMLGUIFactory(), calling removeRecursive" );
  m_servant = 0L;
  removeRecursive( d->m_rootNode );
  delete d;
}

void XMLGUIFactory::addServant( XMLGUIServant *servant )
{
  m_servant = servant;

  if ( servant->factory() && servant->factory() != this )
    servant->factory()->removeServant( servant ); //just in case someone does stupid things ;-)

  QDomElement docElement = servant->document().documentElement();

  d->m_rootNode->index = -1;
  d->m_servantName = docElement.attribute( "name" );
  d->m_genId = 0;

  buildRecursive( docElement, d->m_rootNode );

  servant->setFactory( this );

  m_servant = 0L;
  d->m_servantName = QString::null;
}

void XMLGUIFactory::removeServant( XMLGUIServant *servant )
{
  if ( servant->factory() && servant->factory() != this )
    return;

  kDebugArea( 1002, "XMLGUIFactory::removeServant, calling removeRecursive" );
  m_servant = servant;
  d->m_servantName = servant->document().documentElement().attribute( "name" );
  servant->setFactory( 0L );
  removeRecursive( d->m_rootNode );
  m_servant = 0L;
  d->m_servantName = QString::null;
}

void XMLGUIFactory::buildRecursive( const QDomElement &element, XMLGUIContainerNode *parentNode )
{
  static QString tagAction = QString::fromLatin1( "Action" );
  static QString tagMerge = QString::fromLatin1( "Merge" );
  static QString tagSeparator = QString::fromLatin1( "Separator" );
  static QString attrName = QString::fromLatin1( "name" );

  /*
   * This list contains references to all the containers we created on the current level.
   * We use it as "exclude" list, in order to avoid container matches of already created containers having
   * no proper name attribute to distinct them (like with Separator tags).
   */
  QList<QObject> containerList;

  XMLGUIContainerClient *containerClient = 0L;

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
    /*
     * The "Merge" tag specifies that all containers and actions from *other* servants should be
     * inserted/plugged in at the current index, and not at the "end" .
     */
    if ( e.tagName() == tagMerge )
    {
      QString mergingName = e.attribute( attrName );
      if ( mergingName.isEmpty() )
        mergingName = d->m_defaultMergingName;

      QMap<QString,int>::Iterator mergingIt = parentNode->mergingIndices.find( mergingName );
      if ( mergingIt == parentNode->mergingIndices.end() )
        parentNode->mergingIndices.insert( mergingName, parentNode->index );
      else
        mergingIt.data() = parentNode->index;

      ignoreMergingIndex = true;
    }
    else if ( e.tagName() == tagAction || e.tagName() == tagSeparator )
    {
      if ( !parentNode->container || !parentNode->container->isWidgetType() )
        continue;

      QMap<QString,int>::Iterator it;
	
      int idx = parentNode->index;
      bool merge = calcMergingIndex( parentNode, it );
	
      if ( merge && !ignoreMergingIndex )
	idx = it.data();
      else
	it = parentNode->mergingIndices.end();

      if ( !containerClient )
      {
	containerClient = new XMLGUIContainerClient;
	containerClient->m_servant = m_servant;
	containerClient->m_mergedClient = merge;
	parentNode->clients.append( containerClient );
      }

      if ( e.tagName() == tagAction )
      {
        QAction *action = m_servant->action( e );

	if ( !action )
	  continue;
	
        action->plug( (QWidget *)parentNode->container, idx );

	containerClient->m_actions.append( action );
      }
      else
      {
        int id = m_builder->insertSeparator( (QWidget *)parentNode->container, idx );
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
      XMLGUIContainerNode *matchingContainer = findContainer( parentNode, e, containerList );

      if ( matchingContainer )
        /*
	 * Enter the next level, as the container already exists :)
	 */
        buildRecursive( e, matchingContainer );
      else
      {	
        if ( parentNode->container && !parentNode->container->isWidgetType() )
        {
          kDebugWarning( 1000,"cannot create container widget with non-widget as parent!" );
    	  continue;
        }

	QMap<QString,int>::Iterator it;
	
        int idx = parentNode->index;
	bool merge = calcMergingIndex( parentNode, it );

	if ( merge && !ignoreMergingIndex )
	  idx = it.data();
       	else
       	  it = parentNode->mergingIndices.end();
	
        // query the servant for possible container state information (like toolbar positions for example)
	// (the array might be empty in case there's no info available)
	QByteArray stateBuffer = m_servant->takeContainerStateBuffer( e.tagName() + e.attribute( attrName ) );
	
	/*
	 * let the builder create the container (note that a container might also be a QAction (that's why
	 * we use QObject as container type), like with separators for example.
	 */
	
	int id;
	
        QObject *container = m_builder->createContainer( (QWidget *)parentNode->container, idx, e, stateBuffer, id );
	
	// no container? (probably some <text> tag or so ;-)
	if ( !container )
	  continue;
	
	adjustMergingIndices( parentNode, idx, 1, it );
	
	containerList.append( container );
	
        XMLGUIContainerNode *containerNode = new XMLGUIContainerNode( container, e.tagName(), e.attribute( attrName ), parentNode, m_servant, merge, id );
	
        buildRecursive( e, containerNode );
      }
    }
  }

}

bool XMLGUIFactory::removeRecursive( XMLGUIContainerNode *node )
{
  QListIterator<XMLGUIContainerNode> childIt( node->children );
  while ( childIt.current() )
    // removeRecursive returns true in case the container really got deleted
    if ( removeRecursive( childIt.current() ) )
      node->children.removeRef( childIt.current() );
    else
      ++childIt;

  QMap<QString,int>::Iterator mergingIt;

  calcMergingIndex( node, mergingIt );

  QListIterator<XMLGUIContainerClient> clientIt( node->clients );

  if ( node->container && node->container->isWidgetType() )
    while ( clientIt.current() )
      //only unplug the actions of the servant we want to remove, as the container might be owned
      //by a different servant
      if ( clientIt.current()->m_servant == m_servant )
      {
        QValueList<int>::ConstIterator sepIt = clientIt.current()->m_separators.begin();
	QValueList<int>::ConstIterator sepEnd = clientIt.current()->m_separators.end();
	for (; sepIt != sepEnd; ++sepIt )
	  m_builder->removeSeparator( (QWidget *)node->container, *sepIt );

        QListIterator<QAction> actionIt( clientIt.current()->m_actions );
        for (; actionIt.current(); ++actionIt )
        {
          kDebugInfo( 1002, "unplugging %s from %s", actionIt.current()->name(), (QWidget *)node->container->name() );
          actionIt.current()->unplug( (QWidget *)node->container );
        }
	
	//now we have to adjust/correct the index. We do it the fast'n'dirty way by just substracting
	//the amount of actions we just unplugged ;-)
	int idx = node->index;

	if ( clientIt.current()->m_mergedClient )
	  idx = mergingIt.data();
	else
	  mergingIt = node->mergingIndices.end();
	
	adjustMergingIndices( node, idx, - ( clientIt.current()->m_actions.count() + clientIt.current()->m_separators.count() ), mergingIt );
	
	node->clients.removeRef( clientIt.current() );
	
	//hmmm, actually we should be able to exit the loop here, because one servant can only have one client
	//per container
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

      XMLGUIContainerNode *p = node->parent;

      calcMergingIndex( p, mergingIt );

      int idx = p->index;

      if ( node->mergedContainer )
        idx = mergingIt.data();
      else
        mergingIt = p->mergingIndices.end();

      adjustMergingIndices( p, idx, -1, mergingIt );
    }

    if ( node == d->m_rootNode ) kDebugInfo( 1002, "root node !" );
    if ( !node->container ) kDebugInfo( 1002, "no container !" );
    kDebugInfo( 1002, "remove/kill stuff : node is %s, container is %s (%s), parent container is %s", node->name.ascii(), node->container->name(), node->container->className(), parentContainer ? parentContainer->name() : 0L );
    //remove/kill the container and give the builder a chance to store abitrary state information of
    //the container in a QByteArray. This information will be re-used for the creation of the same
    //container in case we add the same servant again later.
    QByteArray containerStateBuffer = m_builder->removeContainer( node->container, parentContainer, node->containerId );

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

bool XMLGUIFactory::calcMergingIndex( XMLGUIContainerNode *node, QMap<QString,int>::Iterator &it )
{
  QMap<QString,int>::Iterator defaultMergingIt = node->mergingIndices.find( d->m_defaultMergingName );
  QMap<QString,int>::Iterator mergingIt = node->mergingIndices.find( d->m_servantName );
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

void XMLGUIFactory::adjustMergingIndices( XMLGUIContainerNode *node, int idx, int val, const QMap<QString,int>::Iterator &it )
{
  QMap<QString,int>::Iterator mergingIt = node->mergingIndices.begin();
  QMap<QString,int>::Iterator mergingEnd = node->mergingIndices.end();
  for (; mergingIt != mergingEnd; ++mergingIt )
    if ( mergingIt.data() > idx || mergingIt == it )
      mergingIt.data() += val;
	
  node->index += val;
}

XMLGUIContainerNode *XMLGUIFactory::findContainer( XMLGUIContainerNode *node, const QDomElement &element, const QList<QObject> &excludeList )
{
  XMLGUIContainerNode *res = 0L;
  QListIterator<XMLGUIContainerNode> nIt( node->children );

  QString name = element.attribute( "name" );

  if ( !name.isEmpty() )
  {
    for (; nIt.current(); ++nIt )
     if ( nIt.current()->name == name && !excludeList.containsRef( nIt.current()->container ) )
     {
       res = nIt.current();
       break;
     }

    return res;
  }

  name = element.tagName();

  if ( !name.isEmpty() )
    for (; nIt.current(); ++nIt )
      if ( nIt.current()->tagName == name && !excludeList.containsRef( nIt.current()->container ) )
      {
        res = nIt.current();
	break;
      }

  return res;
}
