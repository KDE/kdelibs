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

#include "kxmlgui.h"

#include <qfile.h>
#include <kdebug.h>

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
class XMLGUIContainerNode
{
public:
  XMLGUIContainerNode( QObject *container, const QString &tagName, const QString &name, XMLGUIContainerNode *parent = 0L, XMLGUIServant *servant = 0L, bool merged = false );

  XMLGUIContainerNode *parent() const { return m_parent; }

  void setServant( XMLGUIServant *servant ) { m_servant = servant; }
  XMLGUIServant *servant() const { return m_servant; }

  QObject *container() const { return m_container; }

  QString tagName() const { return m_tagName; }
  QString name() const { return m_name; }

  QList<XMLGUIContainerClient> *clients() { return &m_clients; }

  QList<XMLGUIContainerNode> *children() { return &m_children; }

  int *index() { return &m_index; }

  void setMergingIndex( int val ) { m_mergingIndex = val; }
  int *mergingIndex() { return &m_mergingIndex; }

  bool mergedContainer() const { return m_merged; }

private:
  QObject *m_container;
  QString m_tagName;
  QString m_name;
  QList<XMLGUIContainerClient> m_clients;
  QList<XMLGUIContainerNode> m_children;
  XMLGUIContainerNode *m_parent;
  int m_index;
  int m_mergingIndex;
  XMLGUIServant *m_servant;
  bool m_merged;
};

class XMLGUIFactoryPrivate
{
public:
  XMLGUIFactoryPrivate()
  {
    m_rootNode = new XMLGUIContainerNode( 0L, QString::null, 0L );
  }
  ~XMLGUIFactoryPrivate()
  {
    delete m_rootNode;
  }

  XMLGUIContainerNode *m_rootNode;
};

};

XMLGUIServant::XMLGUIServant()
{
  d = new XMLGUIServantPrivate;
}

XMLGUIServant::~XMLGUIServant()
{
  if ( d->m_factory )
    d->m_factory->removeServant( this );

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

XMLGUIContainerNode::XMLGUIContainerNode( QObject *container, const QString &tagName, const QString &name, XMLGUIContainerNode *parent, XMLGUIServant *servant, bool merge )
{
  m_container = container;
  m_tagName = tagName;
  m_name = name;
  m_parent = parent;
  m_children.setAutoDelete( true );
  m_clients.setAutoDelete( true );
  m_mergingIndex = -1;
  m_index = 0;
  m_servant = servant;
  m_merged = merge;

  if ( m_parent )
    m_parent->children()->append( this );
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

  *d->m_rootNode->index() = -1;

  buildRecursive( docElement, d->m_rootNode );

  servant->setFactory( this );

  m_servant = 0L;
}

void XMLGUIFactory::removeServant( XMLGUIServant *servant )
{
  if ( servant->factory() && servant->factory() != this )
    return;

  m_servant = servant;
  servant->setFactory( 0L );
  removeRecursive( d->m_rootNode );
  m_servant = 0L;
}

void XMLGUIFactory::buildRecursive( const QDomElement &element, XMLGUIContainerNode *parentNode )
{
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
    if ( e.tagName() == "Merge" )
    {
      parentNode->setMergingIndex( *parentNode->index() );
      ignoreMergingIndex = true;
    }
    else if ( e.tagName() == "Action" )
    {
      QAction *action = m_servant->action( e );

      if ( action && parentNode->container() && parentNode->container()->isWidgetType() )
      {
       	int idx;
	bool merged = false;
	
	// ( a merging index with the value -1 means that no Merge tag was specified on the parent level )
       	if ( *parentNode->mergingIndex() != -1 && !ignoreMergingIndex )
	{
       	  idx = (*parentNode->mergingIndex())++;
	  merged = true;
	}
	else
	  idx = (*parentNode->index())++;

	if ( !containerClient )
	{
	  containerClient = new XMLGUIContainerClient;
	  containerClient->m_servant = m_servant;
	  containerClient->m_mergedClient = merged;
	  parentNode->clients()->append( containerClient );
	}
	
        action->plug( (QWidget *)parentNode->container(), idx );

	containerClient->m_actions.append( action );
      }
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
        if ( parentNode->container() && !parentNode->container()->isWidgetType() )
        {
          kDebugInfo( 1000,"cannot create container widget with non-widget as parent!" );
    	  continue;
        }

        int *idx = parentNode->index();
	bool merged = false;

	if ( *parentNode->mergingIndex() != -1 && !ignoreMergingIndex )
	{
	  idx = parentNode->mergingIndex();
	  merged = true;
	}

        // query the servant for possible container state information (like toolbar positions for example)
	// (the array might be empty in case there's no info available)
	QByteArray stateBuffer = m_servant->takeContainerStateBuffer( e.tagName() + e.attribute( "name" ) );
	
	/*
	 * let the builder create the container (note that a container might also be a QAction (that's why
	 * we use QObject as container type), like with separators for example.
	 */
	
        QObject *container = m_builder->createContainer( (QWidget *)parentNode->container(), *idx, e, stateBuffer );
	
	// no container? (probably some <text> tag or so ;-)
	if ( !container )
	  continue;
	
        (*idx)++;
	
	containerList.append( container );
	
        XMLGUIContainerNode *containerNode = new XMLGUIContainerNode( container, e.tagName(), e.attribute( "name" ), parentNode, m_servant, merged );
	
        buildRecursive( e, containerNode );
      }
    }
  }

}

bool XMLGUIFactory::removeRecursive( XMLGUIContainerNode *node )
{

  QListIterator<XMLGUIContainerNode> childIt( *node->children() );
  while ( childIt.current() )
    // removeRecursive returns true in case the container really got deleted
    if ( removeRecursive( childIt.current() ) )
      node->children()->removeRef( childIt.current() );
    else
      ++childIt;

  QListIterator<XMLGUIContainerClient> clientIt( *node->clients() );

  if ( node->container() && node->container()->isWidgetType() )
    while ( clientIt.current() )
      //only unplug the actions of the servant we want to remove, as the container might be owned
      //by a different servant
      if ( clientIt.current()->m_servant == m_servant )
      {
        QListIterator<QAction> actionIt( clientIt.current()->m_actions );
        for (; actionIt.current(); ++actionIt )
          actionIt.current()->unplug( (QWidget *)node->container() );
	
	//now we have to adjust/correct the index. We do it the fast'n'dirty way by just substracting
	//the amount of actions we just unplugged ;-)
	int *idx = node->index();
	
	if ( clientIt.current()->m_mergedClient )
	  idx = node->mergingIndex();
	
	(*idx) -= clientIt.current()->m_actions.count();
	
	node->clients()->removeRef( clientIt.current() );
      }
      else
        ++clientIt;

  if ( node->clients()->count() == 0 && node->children()->count() == 0 && node->container() &&
       node->servant() == m_servant )
  {
    //if at this point the container still contains actions from other servants, then something is wrong
    //with the design of your xml documents ;-) . Anyway, the container was owned by the servant, and that
    //servant is to be removed, so it's not our problem with other servant's actions ;-) .
    //(they will detect the removal through the destroyed() signal anyway :)

    QWidget *parentContainer = 0L;

    if ( node->parent() && node->parent()->container() )
    {
      parentContainer = (QWidget *)node->parent()->container();

      XMLGUIContainerNode *p = node->parent();
      if ( node->mergedContainer() )
        (*p->mergingIndex())--;
      else
        (*p->index())--;
    }

    //remove/kill the container and give the builder a chance to store abitrary state information of
    //the container in a QByteArray. This information will be re-used for the creation of the same
    //container in case we add the same servant again later.
    QByteArray containerStateBuffer = m_builder->removeContainer( node->container(), parentContainer );

    if ( containerStateBuffer.size() > 0 )
      m_servant->storeContainerStateBuffer( node->tagName() + node->name(), containerStateBuffer );

    node->setServant( 0L );

    //indicate the caller that we successfully killed ourselves ;-) and want to be removed from the
    //parent's child list.
    return true;
  }

  if ( node->servant() == m_servant )
    node->setServant( 0L );

  return false;
}

XMLGUIContainerNode *XMLGUIFactory::findContainer( XMLGUIContainerNode *node, const QDomElement &element, const QList<QObject> &excludeList )
{
  XMLGUIContainerNode *res = 0L;
  QListIterator<XMLGUIContainerNode> nIt( *node->children() );

  QString name = element.attribute( "name" );

  if ( !name.isEmpty() )
  {
    for (; nIt.current(); ++nIt )
     if ( nIt.current()->name() == name && !excludeList.containsRef( nIt.current()->container() ) )
     {
       res = nIt.current();
       break;
     }

    return res;
  }

  name = element.tagName();

  if ( !name.isEmpty() )
    for (; nIt.current(); ++nIt )
      if ( nIt.current()->tagName() == name && !excludeList.containsRef( nIt.current()->container() ) )
      {
        res = nIt.current();
	break;
      }

  return res;
}
