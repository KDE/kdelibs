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

#include "kxmlgui.h"
#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"

#include <assert.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qwidget.h>

#include <kaction.h>
#include <kdebug.h>
#include <kaccel.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstddirs.h>

/**
 * This structure is used to know to which servant certain actions belong. In addition we store
 * a boolean value indicating if the actions have been plugged with a merging index or not.
 * A ContainerClient always belongs to a ContainerNode.
 */
struct KXMLGUIContainerClient
{
  KXMLGUIClient *m_client;
  QList<KAction> m_actions;
  bool m_mergedClient;
  QValueList<int> m_customElements;
  QString m_groupName; //is empty if no group client
  QMap<QString, QList<KAction> > m_actionLists;
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
  KXMLGUIContainerNode( QWidget *_container, const QString &_tagName, const QString &_name, KXMLGUIContainerNode *_parent = 0L, KXMLGUIClient *_client = 0L, KXMLGUIBuilder *_builder = 0L, bool _merged = false, int id = -1, const QString &groupName = QString::null );

  KXMLGUIContainerNode *parent;
  KXMLGUIClient *client;
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
    static QString defaultMergingName = QString::fromLatin1( "<default>" );
    static QString actionList = QString::fromLatin1( "actionlist" );
    static QString name = QString::fromLatin1( "name" );
    
    m_rootNode = new KXMLGUIContainerNode( 0L, QString::null, 0L );
    m_defaultMergingName = defaultMergingName;
    m_clientBuilder = 0L;
    tagActionList = actionList;
    attrName = name;
  }
  ~KXMLGUIFactoryPrivate()
  {
    delete m_rootNode;
  }

  KXMLGUIContainerNode *m_rootNode;
  QString m_clientName;
  QString m_defaultMergingName;
  QString m_containerName;
  KXMLGUIBuilder *m_clientBuilder;
  QValueList<KXMLGUIClient*> m_clients;

  QString m_actionListName;
  QList<KAction> m_actionList;

  QString tagActionList;

  QString attrName;
};

KXMLGUIContainerNode::KXMLGUIContainerNode( QWidget *_container, const QString &_tagName, const QString &_name, KXMLGUIContainerNode *_parent, KXMLGUIClient *_client, KXMLGUIBuilder *_builder, bool _merged, int id, const QString &_groupName )
{
  container = _container;
  containerId = id;
  parent = _parent;
  client = _client;
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
  return readConfigFile( filename, false );
}

QString KXMLGUIFactory::readConfigFile( const QString &filename, bool never_null )
{
  QString xml_file;

  if (filename[0] == '/')
    xml_file = filename;
  else
    xml_file = locate("data", QString::fromLatin1(KGlobal::instance()->instanceName() + '/' ) + filename);

  QFile file( xml_file );
  if ( !file.open( IO_ReadOnly ) )
  {
    kdError(1000) << "No such XML file " << filename.local8Bit().data() << endl;
    if ( never_null )
      return QString::fromLatin1( "<!DOCTYPE kpartgui>\n<kpartgui name=\"empty\">\n</kpartgui>" );
    else
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

bool KXMLGUIFactory::saveConfigFile( const QDomDocument& doc,
                                     const QString& filename)
{
  QString xml_file(filename);

  if (xml_file[0] != '/')
    xml_file = locateLocal("data", QString::fromLatin1( KGlobal::instance()->instanceName() + '/' ) + filename);

  QFile file( xml_file );
  if ( !file.open( IO_WriteOnly ) )
  {
    kdError(1000) << "Could not write to " << filename.local8Bit().data() << endl;
    return false;
  }

  // write out our document
  QTextStream ts(&file);
  ts << doc;

  file.close();
  return true;
}

QString KXMLGUIFactory::documentToXML( const QDomDocument& doc )
{
  QString str;
  QTextStream ts(&str, IO_WriteOnly);
  ts << doc;
  return str;
}

QString KXMLGUIFactory::elementToXML( const QDomElement& elem )
{
  QString str;
  QTextStream ts(&str, IO_WriteOnly);
  ts << elem;
  return str;
}

KXMLGUIFactory::KXMLGUIFactory( KXMLGUIBuilder *builder, QObject *parent, const char *name )
: QObject( parent, name )
{
  d = new KXMLGUIFactoryPrivate;
  m_builder = builder;
  m_client = 0L;
}

KXMLGUIFactory::~KXMLGUIFactory()
{
  delete d;
}

void KXMLGUIFactory::addClient( KXMLGUIClient *client )
{
  static QString actionPropElementName = QString::fromLatin1( "ActionProperties" );
  static QString tagAction = QString::fromLatin1( "action" );
  static QString attrAccel = QString::fromLatin1( "accel" );

  m_client = client;

  if ( client->factory() && client->factory() != this )
    client->factory()->removeClient( client ); //just in case someone does stupid things ;-)

  // add this client to our client list
  if ( d->m_clients.contains( client ) == 0 )
    d->m_clients.append( client );

  QDomElement docElement = client->domDocument().documentElement();

  d->m_rootNode->index = -1;
  d->m_clientName = docElement.attribute( d->attrName );
  d->m_clientBuilder = client->clientBuilder();

  QDomElement actionPropElement = docElement.namedItem( actionPropElementName ).toElement();
  if ( actionPropElement.isNull() )
    actionPropElement = docElement.namedItem( actionPropElementName.lower() ).toElement();

  if ( !actionPropElement.isNull() )
  {
    QDomElement e = actionPropElement.firstChild().toElement();
    for (; !e.isNull(); e = e.nextSibling().toElement() )
    {
      if ( e.tagName().lower() != tagAction )
        continue;

      KAction *action = m_client->action( e );
      if ( !action )
        continue;

      QDomNamedNodeMap attributes = e.attributes();
      for ( uint i = 0; i < attributes.length(); i++ )
      {
        QDomAttr attr = attributes.item( i ).toAttr();
	if ( attr.isNull() )
	  continue;
	
	//don't let someone change the name of the action! (Simon)
	if ( attr.name() == d->attrName )
	  continue;

	QVariant propertyValue;
	
	QVariant::Type propertyType = action->property( attr.name().latin1() ).type();
	
	// readable accels please ;-)
	if ( attr.name().lower() == attrAccel )
  	  propertyValue = QVariant( KAccel::stringToKey( attr.value() ) );
	else if ( propertyType == QVariant::Int )
	  propertyValue = QVariant( attr.value().toInt() );
	else if ( propertyType == QVariant::UInt )
  	  propertyValue = QVariant( attr.value().toUInt() );
	else
	  propertyValue = QVariant( attr.value() );
	
        action->setProperty( attr.name().latin1() /* ???????? */, propertyValue );
      }
    }
  }

  buildRecursive( docElement, d->m_rootNode );

  client->setFactory( this );

  m_client = 0L;
  d->m_clientName = QString::null;
  d->m_clientBuilder = 0L;

  emit clientAdded( client );

  if ( client->childClients()->count() > 0 )
  {
    const QList<KXMLGUIClient> *children = client->childClients();
    QListIterator<KXMLGUIClient> childIt( *children );
    for (; childIt.current(); ++childIt )
      addClient( childIt.current() );
  }
}

void KXMLGUIFactory::removeClient( KXMLGUIClient *client )
{
  if ( client->factory() && client->factory() != this )
    return;

  // remove this client from our client list
  d->m_clients.remove( client );

  if ( client->childClients()->count() > 0 )
  {
    const QList<KXMLGUIClient> *children = client->childClients();
    QListIterator<KXMLGUIClient> childIt( *children );
    for (; childIt.current(); ++childIt )
      removeClient( childIt.current() );
  }

  kdDebug(1002) << "KXMLGUIFactory::removeServant, calling removeRecursive" << endl;
  m_client = client;
  d->m_clientName = client->domDocument().documentElement().attribute( d->attrName );
  d->m_clientBuilder = client->clientBuilder();
  client->setFactory( 0L );
  removeRecursive( d->m_rootNode );
  m_client = 0L;
  d->m_clientBuilder = 0L;
  d->m_clientName = QString::null;

  emit clientRemoved( client );
}

QValueList<KXMLGUIClient*> KXMLGUIFactory::clients() const
{
  return d->m_clients;
}

QWidget *KXMLGUIFactory::container( const QString &containerName, KXMLGUIClient *client )
{
  return container( containerName, client, false );
}

QWidget *KXMLGUIFactory::container( const QString &containerName, KXMLGUIClient *client, bool useTagName )
{
  d->m_containerName = containerName;
  m_client = client;

  QWidget *result = findRecursive( d->m_rootNode, useTagName );

  m_client = 0L;
  d->m_containerName = QString::null;

  return result;
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

  KXMLGUIContainerNode *container = findContainer( d->m_rootNode, containerName, useTagName );

  if ( !container )
    return;

  KXMLGUIContainerNode *parent = container->parent;
  if ( !parent )
    return;

  //  resetInternal( container );

  parent->children.removeRef( container );
}

void KXMLGUIFactory::resetInternal( KXMLGUIContainerNode *node )
{
  QListIterator<KXMLGUIContainerNode> childIt( node->children );
  for (; childIt.current(); ++childIt )
    resetInternal( childIt.current() );

  if ( node->client )
    node->client->setFactory( 0L );
}

void KXMLGUIFactory::buildRecursive( const QDomElement &element, KXMLGUIContainerNode *parentNode )
{
  static QString tagAction = QString::fromLatin1( "action" );
  static QString tagMerge = QString::fromLatin1( "merge" );
  static QString tagDefineGroup = QString::fromLatin1( "definegroup" );
  static QString attrGroup = QString::fromLatin1( "group" );

  QStringList customTags;
  if ( parentNode->builder )
    customTags = parentNode->builder->customTags();

  QStringList containerTags = m_builder->containerTags();
  if ( parentNode->builder && parentNode->builder != m_builder )
    containerTags += parentNode->builder->containerTags();

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
    if ( tag == tagMerge || tag == tagDefineGroup || tag == d->tagActionList )
    {
      QString mergingName = e.attribute( d->attrName );
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
        mergingName.prepend( attrGroup ); //avoid possible name clashes by prepending "group" to group
                                          //definitions

      if ( tag == d->tagActionList )
      {
        mergingName.prepend( d->m_clientName );
        mergingName.prepend( d->tagActionList );
      }

      if ( parentNode->mergingIndices.contains( mergingName ) )
        continue; //do not allow the redefinition of merging indices!

      parentNode->mergingIndices.insert( mergingName, parentNode->index );

      ignoreMergingIndex = true;
    }
    else if ( tag == tagAction || customTags.contains( tag ) )
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

      if ( tag == tagAction )
      {
        KAction *action = m_client->action( e );

	if ( !action )
	  continue;
	
        action->plug( parentNode->container, idx );

	containerClient->m_actions.append( action );
      }
      else
      {
        assert( parentNode->builder );
	
	int id = parentNode->builder->createCustomElement( parentNode->container, idx, e );
	if ( id != 0 )
  	  containerClient->m_customElements.append( id );
      }

      adjustMergingIndices( parentNode, idx, 1, it );
    }
    else if ( containerTags.contains( tag ) )
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
	
        // query the client for possible container state information (like toolbar positions for example)
	// (the array might be empty in case there's no info available)
	QByteArray stateBuffer = m_client->takeContainerStateBuffer( e.tagName() + e.attribute( d->attrName ) );
	
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
	
          containerNode = new KXMLGUIContainerNode( container, e.tagName(), e.attribute( d->attrName ), parentNode, m_client, builder, merge, id, group );
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
  {
    if ( node->clients.count() == 1 && clientIt.current()->m_client == node->client && node->client == m_client )
      node->container->hide(); // this container is going to die, that's for sure. in this case let's just hide it, which makes the
                               // destruction faster

    while ( clientIt.current() )
      //only unplug the actions of the client we want to remove, as the container might be owned
      //by a different servant
      if ( clientIt.current()->m_client == m_client )
      {
        assert( node->builder );
	
        QValueList<int>::ConstIterator custIt = clientIt.current()->m_customElements.begin();
	QValueList<int>::ConstIterator custEnd = clientIt.current()->m_customElements.end();
	for (; custIt != custEnd; ++custIt )
	  node->builder->removeCustomElement( node->container, *custIt );

        QListIterator<KAction> actionIt( clientIt.current()->m_actions );
        for (; actionIt.current(); ++actionIt )
        {
          kdDebug(1002) << "unplugging " << actionIt.current()->name() << " from " << node->container->name() << endl;
          actionIt.current()->unplug( node->container );
        }
	
        calcMergingIndex( node, clientIt.current()->m_groupName, mergingIt );
	
	int idx = node->index;

	if ( clientIt.current()->m_mergedClient )
	  idx = mergingIt.data();
	else
	  mergingIt = node->mergingIndices.end();

	adjustMergingIndices( node, idx, - ( clientIt.current()->m_actions.count()
					     + clientIt.current()->m_customElements.count() ), mergingIt );
	
	QMap<QString, QList<KAction> >::ConstIterator alIt = clientIt.current()->m_actionLists.begin();
	QMap<QString, QList<KAction> >::ConstIterator alEnd = clientIt.current()->m_actionLists.end();
	for (; alIt != alEnd; ++alIt )
	{
	  actionIt = QListIterator<KAction>( alIt.data() );
	  for (; actionIt.current(); ++actionIt )
	    actionIt.current()->unplug( node->container );
	
	  QString mergingKey = alIt.key();
	  mergingKey.prepend( d->m_clientName );
	  mergingKey.prepend( d->tagActionList );
	
	  QMap<QString, int>::Iterator mIt = node->mergingIndices.find( mergingKey );
	  if ( mIt == node->mergingIndices.end() )
	    continue;
	
	  adjustMergingIndices( node, mIt.data(), - alIt.data().count(), mIt );
	
	  node->mergingIndices.remove( mIt );
	}
	
	node->clients.removeRef( clientIt.current() );
      }
      else
        ++clientIt;
  }

  if ( node->clients.count() == 0 && node->children.count() == 0 && node->container &&
       node->client == m_client )
  {
    //if at this point the container still contains actions from other servants, then something is wrong
    //with the design of your xml documents ;-) . Anyway, the container was owned by the servant, and that
    //servant is to be removed, so it's not our problem with other client's actions ;-) .
    //(they will detect the removal through the destroyed() signal anyway :)

    QWidget *parentContainer = 0L;

    if ( node->parent && node->parent->container )
    {
      parentContainer = node->parent->container;

      KXMLGUIContainerNode *p = node->parent;

      calcMergingIndex( p, node->groupName, mergingIt );

      int idx = p->index;

      if ( node->mergedContainer )
        idx = mergingIt.data();
      else
        mergingIt = p->mergingIndices.end();

      adjustMergingIndices( p, idx, -1, mergingIt );
    }

    if ( node == d->m_rootNode ) kdDebug(1002) << "root node !" << endl;
    if ( !node->container ) kdDebug(1002) << "no container !" << endl;

    assert( node->builder );

    kdDebug(1002) << "remove/kill stuff : node is " << node->name << ", container is " << node->container->name() << " (" << node->container->className() << "), parent container is " << ( parentContainer ? parentContainer->name() : 0L ) << endl;
    //remove/kill the container and give the builder a chance to store abitrary state information of
    //the container in a QByteArray. This information will be re-used for the creation of the same
    //container in case we add the same client again later.
    QByteArray containerStateBuffer = node->builder->removeContainer( node->container, parentContainer, node->containerId );

    if ( containerStateBuffer.size() > 0 && !node->name.isEmpty() )
      m_client->storeContainerStateBuffer( node->tagName + node->name, containerStateBuffer );

    node->client = 0L;

    //indicate the caller that we successfully killed ourselves ;-) and want to be removed from the
    //parent's child list.
    return true;
  }

  if ( node->client == m_client )
    node->client = 0L;

  return false;
}

bool KXMLGUIFactory::calcMergingIndex( KXMLGUIContainerNode *node, const QString &mergingName, QMap<QString,int>::Iterator &it )
{
  QMap<QString,int>::Iterator defaultMergingIt = node->mergingIndices.find( d->m_defaultMergingName );
  QMap<QString,int>::Iterator mergingIt;

  if ( !mergingName.isEmpty() )
    mergingIt = node->mergingIndices.find( mergingName );
  else
    mergingIt = node->mergingIndices.find( d->m_clientName );

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

  QString name = element.attribute( d->attrName );

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
	   nIt.current()->client == m_client )
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

KXMLGUIContainerNode *KXMLGUIFactory::findContainer( KXMLGUIContainerNode *node, const QString &name, bool tag )
{
  if ( ( tag && node->tagName == name ) ||
       ( !tag && node->name == name ) )
    return node;

  QListIterator<KXMLGUIContainerNode> it( node->children );
  for (; it.current(); ++it )
  {
    KXMLGUIContainerNode *res = findContainer( it.current(), name, tag );
    if ( res )
      return res;
  }

  return 0;
}

QWidget *KXMLGUIFactory::findRecursive( KXMLGUIContainerNode *node, bool tag )
{
  if ( ( ( !tag && node->name == d->m_containerName ) ||
         ( tag && node->tagName == d->m_containerName ) ) &&
       ( !m_client || node->client == m_client ) )
    return node->container;

  QListIterator<KXMLGUIContainerNode> it( node->children );
  for (; it.current(); ++it )
  {
    QWidget *cont = findRecursive( it.current(), tag );
    if ( cont )
      return cont;
  }

  return 0L;
}

QWidget *KXMLGUIFactory::createContainer( QWidget *parent, int index, const QDomElement &element, const QByteArray &containerStateBuffer, int &id, KXMLGUIBuilder **builder )
{
  QWidget *res = 0L;

  if ( d->m_clientBuilder )
  {
    res = d->m_clientBuilder->createContainer( parent, index, element, containerStateBuffer, id );

    if ( res )
    {
      *builder = d->m_clientBuilder;
      return res;
    }
  }

  KInstance *old = m_builder->builderInstance();

  KInstance *clientInst = m_client->instance();
  if ( clientInst )
    m_builder->setBuilderInstance( clientInst );

  res = m_builder->createContainer( parent, index, element, containerStateBuffer, id );

  m_builder->setBuilderInstance( old );

  if ( res )
    *builder = m_builder;

  return res;
}

KXMLGUIContainerClient *KXMLGUIFactory::findClient( KXMLGUIContainerNode *node, const QString &groupName )
{
  QListIterator<KXMLGUIContainerClient> clientIt( node->clients );

  for (; clientIt.current(); ++clientIt )
    if ( clientIt.current()->m_client == m_client )
    {
      if ( groupName.isEmpty() )
        return clientIt.current();

      if ( groupName == clientIt.current()->m_groupName )
        return clientIt.current();
    }

  KXMLGUIContainerClient *client = new KXMLGUIContainerClient;
  client->m_client = m_client;
  client->m_groupName = groupName;

  node->clients.append( client );

  return client;
}

void KXMLGUIFactory::plugActionList( KXMLGUIClient *client, const QString &name, const QList<KAction> &actionList )
{
  m_client = client;
  d->m_actionListName = name;
  d->m_actionList = actionList;
  d->m_clientName = client->domDocument().documentElement().attribute( d->attrName );

  plugActionListRecursive( d->m_rootNode );

  m_client = 0;
  d->m_actionListName = QString::null;
  d->m_actionList = QList<KAction>();
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

void KXMLGUIFactory::plugActionListRecursive( KXMLGUIContainerNode *node )
{
  QMap<QString,int>::Iterator mIt = node->mergingIndices.begin();
  QMap<QString,int>::Iterator mEnd = node->mergingIndices.end();
  for (; mIt != mEnd; ++mIt )
  {
    QString k = mIt.key();

    if ( k.find( d->tagActionList ) == -1 )
      continue;

    k = k.mid( d->tagActionList.length() );

    if ( k.find( d->m_clientName ) == -1 )
      continue;

    k = k.mid( d->m_clientName.length() );

    if ( k != d->m_actionListName )
      continue;

    int idx = mIt.data();

    KXMLGUIContainerClient *client = findClient( node, QString::null );

    client->m_actionLists.insert( k, d->m_actionList );

    QListIterator<KAction> aIt( d->m_actionList );
    for (; aIt.current(); ++aIt )
      aIt.current()->plug( node->container, idx++ );

    adjustMergingIndices( node, mIt.data(), d->m_actionList.count(), mIt );
  }

  QListIterator<KXMLGUIContainerNode> childIt( node->children );
  for (; childIt.current(); ++childIt )
    plugActionListRecursive( childIt.current() );
}

void KXMLGUIFactory::unplugActionListRecursive( KXMLGUIContainerNode *node )
{
  QMap<QString,int>::Iterator mIt = node->mergingIndices.begin();
  QMap<QString,int>::Iterator mEnd = node->mergingIndices.end();
  for (; mIt != mEnd; ++mIt )
  {
    QString k = mIt.key();

    if ( k.find( d->tagActionList ) == -1 )
      continue;

    k = k.mid( d->tagActionList.length() );

    if ( k.find( d->m_clientName ) == -1 )
      continue;

    k = k.mid( d->m_clientName.length() );

    if ( k != d->m_actionListName )
      continue;

    KXMLGUIContainerClient *client = findClient( node, QString::null );

    QMap<QString, QList<KAction> >::Iterator lIt = client->m_actionLists.find( k );
    if ( lIt == client->m_actionLists.end() )
      continue;

    QListIterator<KAction> aIt( lIt.data() );
    for (; aIt.current(); ++aIt )
      aIt.current()->unplug( node->container );

    adjustMergingIndices( node, mIt.data(), -lIt.data().count(), mIt );

    client->m_actionLists.remove( lIt );
  }


  QListIterator<KXMLGUIContainerNode> childIt( node->children );
  for (; childIt.current(); ++childIt )
    unplugActionListRecursive( childIt.current() );
}

#include "kxmlgui.moc"
