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

/*
 * TODO: - cleanup ContainerClient::m_* and Node::*
 *       - improve performance
 *       - remove m_groupName stuff!?
 *       - make all the blah( KXMLGUIContainerNode *node, ... ) methods part of
 *         KXMLGUIContainerNode
 *       - make more use of QValueList instead of QList
 */

/**
 * This structure is used to know to which servant certain actions belong. In addition we store
 * a boolean value indicating if the actions have been plugged with a merging index or not.
 * A ContainerClient always belongs to a ContainerNode.
 */
struct KXMLGUIContainerClient
{
  KXMLGUIClient *client;
  QList<KAction> actions;
  QValueList<int> customElements;
  QString groupName; //is empty if no group client
  QMap<QString, QList<KAction> > actionLists;
  QString mergingName;
};

template class QList<KXMLGUIContainerClient>;
class KXMLGUIContainerNode;
template class QList<KXMLGUIContainerNode>;

struct MergingIndex
{
  int value; // the actual index value, used as index for plug() or createContainer() calls
  QString mergingName;
  QString clientName; // the name of the client that defined this index
};

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
  KXMLGUIContainerNode( QWidget *_container, const QString &_tagName, const QString &_name, KXMLGUIContainerNode *_parent = 0L, KXMLGUIClient *_client = 0L, KXMLGUIBuilder *_builder = 0L, int id = -1, const QString &_mergingName = QString::null, const QString &groupName = QString::null );

  KXMLGUIContainerNode *parent;
  KXMLGUIClient *client;
  KXMLGUIBuilder *builder;
  QStringList builderCustomTags;
  QStringList builderContainerTags;
  QWidget *container;
  int containerId;

  QString tagName;
  QString name;

  QString groupName; //is empty if the container is in no group

  QList<KXMLGUIContainerClient> clients;
  QList<KXMLGUIContainerNode> children;

  int index;
  QValueList<MergingIndex> mergingIndices;

  QString mergingName;

  QValueList<MergingIndex>::Iterator findIndex( const QString &name );
  KXMLGUIContainerNode *findContainerNode( QWidget *container );
  KXMLGUIContainerNode *findContainer( const QString &_name, bool tag );
  KXMLGUIContainerNode *findContainer( const QString &name, const QString &tagName, const QList<QWidget> *excludeList, KXMLGUIClient *currClient );
};

class KXMLGUIFactoryPrivate
{
public:
  KXMLGUIFactoryPrivate()
  {
    static const QString &defaultMergingName = KGlobal::staticQString( "<default>" );
    static const QString &actionList = KGlobal::staticQString( "actionlist" );
    static const QString &name = KGlobal::staticQString( "name" );

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
  QList<KXMLGUIClient> m_clients;

  QString m_actionListName;
  QList<KAction> m_actionList;

  QString tagActionList;

  QString attrName;

  QValueList<MergingIndex>::Iterator m_currentDefaultMergingIt;
  QValueList<MergingIndex>::Iterator m_currentClientMergingIt;

  QStringList m_builderContainerTags;
};

KXMLGUIContainerNode::KXMLGUIContainerNode( QWidget *_container, const QString &_tagName, const QString &_name, KXMLGUIContainerNode *_parent, KXMLGUIClient *_client, KXMLGUIBuilder *_builder, int id, const QString &_mergingName, const QString &_groupName )
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
  mergingName = _mergingName;

  if ( builder )
  {
    builderCustomTags = builder->customTags();
    builderContainerTags = builder->containerTags();
  }

  if ( parent )
    parent->children.append( this );
}

QValueList<MergingIndex>::Iterator KXMLGUIContainerNode::findIndex( const QString &name )
{
  QValueList<MergingIndex>::Iterator it = mergingIndices.begin();
  QValueList<MergingIndex>::Iterator end = mergingIndices.end();
  for (; it != end; ++it )
    if ( (*it).mergingName == name )
      return it;
  return it;
}

KXMLGUIContainerNode *KXMLGUIContainerNode::findContainerNode( QWidget *container )
{
  QListIterator<KXMLGUIContainerNode> it( children );

  for (; it.current(); ++it )
    if ( it.current()->container == container )
      return it.current();

  return 0L;
}

KXMLGUIContainerNode *KXMLGUIContainerNode::findContainer( const QString &_name, bool tag )
{
  if ( ( tag && tagName == _name ) ||
       ( !tag && name == _name ) )
    return this;

  QListIterator<KXMLGUIContainerNode> it( children );
  for (; it.current(); ++it )
  {
    KXMLGUIContainerNode *res = it.current()->findContainer( _name, tag );
    if ( res )
      return res;
  }

  return 0;
}

KXMLGUIContainerNode *KXMLGUIContainerNode::findContainer( const QString &name, const QString &tagName, const QList<QWidget> *excludeList,
                                                           KXMLGUIClient *currClient )
{
  KXMLGUIContainerNode *res = 0L;
  QListIterator<KXMLGUIContainerNode> nIt( children );

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

  if ( !tagName.isEmpty() )
    for (; nIt.current(); ++nIt )
    {
      if ( nIt.current()->tagName == tagName && !excludeList->containsRef( nIt.current()->container ) &&
           nIt.current()->client == currClient )
      {
        res = nIt.current();
        break;
      }
    }

  return res;
}

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
    xml_file = locate("data", QString::fromLatin1(instance->instanceName() + '/' ) + filename);

  QFile file( xml_file );
  if ( !file.open( IO_ReadOnly ) )
  {
    kdError(1000) << "No such XML file " << filename << endl;
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
                                     const QString& filename, KInstance *_instance )
{
  KInstance *instance = _instance ? _instance : KGlobal::instance();
  QString xml_file(filename);

  if (xml_file[0] != '/')
    xml_file = locateLocal("data", QString::fromLatin1( instance->instanceName() + '/' ) + filename);

  QFile file( xml_file );
  if ( !file.open( IO_WriteOnly ) )
  {
    kdError(1000) << "Could not write to " << filename << endl;
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
  if ( m_builder )
      d->m_builderContainerTags = m_builder->containerTags();
}

KXMLGUIFactory::~KXMLGUIFactory()
{
  delete d;
}

void KXMLGUIFactory::addClient( KXMLGUIClient *client )
{
  static const QString &actionPropElementName = KGlobal::staticQString( "ActionProperties" );
  static const QString &tagAction = KGlobal::staticQString( "action" );
  static const QString &attrAccel = KGlobal::staticQString( "accel" );

  m_client = client;

  if ( client->factory() && client->factory() != this )
    client->factory()->removeClient( client ); //just in case someone does stupid things ;-)

  // add this client to our client list
  if ( d->m_clients.containsRef( client ) == 0 )
    d->m_clients.append( client );

  QDomDocument doc = client->xmlguiBuildDocument();
  if ( doc.isNull() )
    doc = client->domDocument();

  QDomElement docElement = doc.documentElement();

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

  m_builder->finalizeGUI( m_client );

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
  d->m_clients.removeRef( client );

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

  QDomDocument doc = client->xmlguiBuildDocument();
  if ( doc.isNull() )
  {
      doc = client->domDocument().cloneNode( true ).toDocument();
      client->setXMLGUIBuildDocument( doc );
  }

  QDomElement tmp = doc.documentElement();
  removeRecursive( tmp, d->m_rootNode );
  m_client = 0L;
  d->m_clientBuilder = 0L;
  d->m_clientName = QString::null;

  emit clientRemoved( client );
}

QList<KXMLGUIClient> KXMLGUIFactory::clients() const
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

  KXMLGUIContainerNode *container = d->m_rootNode->findContainer( containerName, useTagName );

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
  static const QString &tagAction = KGlobal::staticQString( "action" );
  static const QString &tagMerge = KGlobal::staticQString( "merge" );
  static const QString &tagDefineGroup = KGlobal::staticQString( "definegroup" );
  static const QString &attrGroup = KGlobal::staticQString( "group" );

  QStringList customTags = parentNode->builderCustomTags;

  QStringList containerTags = d->m_builderContainerTags;
  if ( parentNode->builder != m_builder )
    containerTags += parentNode->builderContainerTags;

  /*
   * This list contains references to all the containers we created on the current level.
   * We use it as "exclude" list, in order to avoid container matches of already created containers having
   * no proper name attribute to distinct them (like with Separator tags).
   */
  QList<QWidget> containerList;

  KXMLGUIContainerClient *containerClient = 0L;

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
    QString tag = e.tagName().lower();
    QString currName = e.attribute( d->attrName );

    /*
     * The "Merge" tag specifies that all containers and actions from *other* servants should be
     * inserted/plugged in at the current index, and not at the "end" .
     */
    if ( tag == tagMerge || tag == tagDefineGroup || tag == d->tagActionList )
    {
      QString mergingName = currName;
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
      else if ( tag == d->tagActionList )
        mergingName.prepend( d->tagActionList );

      if ( parentNode->findIndex( mergingName ) != parentNode->mergingIndices.end() )
        continue; //do not allow the redefinition of merging indices!

      QValueList<MergingIndex>::Iterator mIt = parentNode->mergingIndices.end();

      MergingIndex newIdx;
      newIdx.value = calcMergingIndex( parentNode, QString::null /* ### allow group for <merge/> ? */ ,
                                       mIt, ignoreDefaultMergingIndex );
      newIdx.mergingName = mergingName;
      newIdx.clientName = d->m_clientName;

      if ( mIt != parentNode->mergingIndices.end() )
        parentNode->mergingIndices.insert( ++mIt, newIdx );
      else
        parentNode->mergingIndices.append( newIdx );

      if ( mergingName == d->m_defaultMergingName )
        ignoreDefaultMergingIndex = true;

      d->m_currentDefaultMergingIt = parentNode->findIndex( d->m_defaultMergingName );
      calcMergingIndex( parentNode, QString::null, d->m_currentClientMergingIt, ignoreDefaultMergingIndex );
    }
    else if ( tag == tagAction || customTags.contains( tag ) )
    {
      if ( !parentNode->container )
        continue;

      QValueList<MergingIndex>::Iterator it = d->m_currentClientMergingIt;

      bool haveGroup = false;
      QString group = e.attribute( attrGroup );
      if ( !group.isEmpty() )
      {
        group.prepend( attrGroup );
        haveGroup = true;
      }

//      int idx = calcMergingIndex( parentNode, group, it, ignoreDefaultMergingIndex );
      int idx;
      if ( haveGroup )
          idx = calcMergingIndex( parentNode, group, it, ignoreDefaultMergingIndex );
      else if ( d->m_currentClientMergingIt == parentNode->mergingIndices.end() )
          idx = parentNode->index;
      else
          idx = (*d->m_currentClientMergingIt).value;


      //      if ( !containerClient || haveGroup )
        containerClient = findClient( parentNode, group, it );

      if ( tag == tagAction )
      {
        KAction *action = m_client->action( e );

        if ( !action )
          continue;

        action->plug( parentNode->container, idx );

        containerClient->actions.append( action );
      }
      else
      {
        assert( parentNode->builder );

        int id = parentNode->builder->createCustomElement( parentNode->container, idx, e );
        if ( id != 0 )
          containerClient->customElements.append( id );
      }

      adjustMergingIndices( parentNode, 1, it );
    }
    else if ( containerTags.contains( tag ) )
    {
      /*
       * No Action or Merge tag? That most likely means that we want to create a new container.
       * But first we have to check if there's already a existing (child) container of the same type in our
       * tree. However we have to ignore just newly created containers!
       */

      KXMLGUIContainerNode *matchingContainer = parentNode->findContainer( currName, tag, &containerList, m_client );

      if ( matchingContainer )
      {
        /*
         * Enter the next level, as the container already exists :)
         */
        buildRecursive( e, matchingContainer );
        d->m_currentDefaultMergingIt = parentNode->findIndex( d->m_defaultMergingName );
        calcMergingIndex( parentNode, QString::null, d->m_currentClientMergingIt, ignoreDefaultMergingIndex );
      }
      else
      {
        QValueList<MergingIndex>::Iterator it = d->m_currentClientMergingIt;

        bool haveGroup = false;
        QString group = e.attribute( attrGroup );
        if ( !group.isEmpty() )
        {
          group.prepend( attrGroup );
          haveGroup = true;
        }

//      int idx = calcMergingIndex( parentNode, group, it, ignoreDefaultMergingIndex );
        //      int idx = d->m_currentClientMergingIndex;
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

        adjustMergingIndices( parentNode, 1, it );

        KXMLGUIContainerNode *containerNode = parentNode->findContainerNode( container );

        if ( !containerNode )
        {
          containerList.append( container );

          QString mergingName;
          if ( it != parentNode->mergingIndices.end() )
            mergingName = (*it).mergingName;

          containerNode = new KXMLGUIContainerNode( container, tag, currName, parentNode, m_client, builder, id, mergingName, group );
        }

        buildRecursive( e, containerNode );
        d->m_currentDefaultMergingIt = parentNode->findIndex( d->m_defaultMergingName );
        calcMergingIndex( parentNode, QString::null, d->m_currentClientMergingIt, ignoreDefaultMergingIndex );
      }
    }
  }
}

bool KXMLGUIFactory::removeRecursive( QDomElement &element, KXMLGUIContainerNode *node )
{
  QDomNodeList childElements = element.childNodes();
  uint childElementCount = childElements.count();

  QListIterator<KXMLGUIContainerNode> childIt( node->children );
  while ( childIt.current() )
  {
    KXMLGUIContainerNode *childNode = childIt.current();
    QDomElement child;
    // ### slow
    uint i = 0;
    for (; i < childElementCount; ++i )
    {
      QDomElement e = childElements.item( i ).toElement();
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

  QValueList<MergingIndex>::Iterator mergingIt = node->mergingIndices.end();

  QListIterator<KXMLGUIContainerClient> clientIt( node->clients );

  if ( node->container )
  {
    if ( node->clients.count() == 1 && clientIt.current()->client == node->client && node->client == m_client )
      node->container->hide(); // this container is going to die, that's for sure. in this case let's just hide it, which makes the
                               // destruction faster

    while ( clientIt.current() )
      //only unplug the actions of the client we want to remove, as the container might be owned
      //by a different servant
      if ( clientIt.current()->client == m_client )
      {
        assert( node->builder );

        QValueList<int>::ConstIterator custIt = clientIt.current()->customElements.begin();
        QValueList<int>::ConstIterator custEnd = clientIt.current()->customElements.end();
        for (; custIt != custEnd; ++custIt )
          node->builder->removeCustomElement( node->container, *custIt );

        QListIterator<KAction> actionIt( clientIt.current()->actions );
        for (; actionIt.current(); ++actionIt )
        {
          kdDebug(1002) << "unplugging " << actionIt.current()->name() << " from " << node->container->name() << endl;
          actionIt.current()->unplug( node->container );
        }

        mergingIt = node->findIndex( clientIt.current()->mergingName );

        adjustMergingIndices( node, - ( clientIt.current()->actions.count()
                                    + clientIt.current()->customElements.count() ), mergingIt );

        QMap<QString, QList<KAction> >::ConstIterator alIt = clientIt.current()->actionLists.begin();
        QMap<QString, QList<KAction> >::ConstIterator alEnd = clientIt.current()->actionLists.end();
        for (; alIt != alEnd; ++alIt )
        {
          actionIt = QListIterator<KAction>( alIt.data() );
          for (; actionIt.current(); ++actionIt )
            actionIt.current()->unplug( node->container );

          QString mergingKey = alIt.key();
          mergingKey.prepend( d->tagActionList );

          QValueList<MergingIndex>::Iterator mIt = node->findIndex( mergingKey );
          if ( mIt == node->mergingIndices.end() )
            continue;

          adjustMergingIndices( node, - alIt.data().count(), mIt );

          node->mergingIndices.remove( mIt );
        }

        node->clients.removeRef( clientIt.current() );
      }
      else
        ++clientIt;
  }

  mergingIt = node->mergingIndices.end();

  QValueList<MergingIndex>::Iterator cmIt = node->mergingIndices.begin();
  while ( cmIt != node->mergingIndices.end() )
    if ( (*cmIt).clientName == d->m_clientName )
      cmIt = node->mergingIndices.remove( cmIt );
    else
      ++cmIt;

  // ### check for merging index count, too?
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

      mergingIt = p->findIndex( node->mergingName );

      adjustMergingIndices( p, -1, mergingIt );
    }

    if ( node == d->m_rootNode ) kdDebug(1002) << "root node !" << endl;
    if ( !node->container ) kdDebug(1002) << "no container !" << endl;

    assert( node->builder );

    kdDebug(1002) << "remove/kill stuff : node is " << node->name << ", container is " << node->container->name() << " (" << node->container->className() << "), parent container is " << ( parentContainer ? parentContainer->name() : 0L ) << endl;
    //remove/kill the container and give the builder a chance to store abitrary state information of
    //the container in a QByteArray. This information will be re-used for the creation of the same
    //container in case we add the same client again later.
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

int KXMLGUIFactory::calcMergingIndex( KXMLGUIContainerNode *node, const QString &mergingName, QValueList<MergingIndex>::Iterator &it, bool ignoreDefaultMergingIndex )
{
//  QValueList<MergingIndex>::Iterator currentDefaultMergingIt = node->findIndex( d->m_defaultMergingName );
  QValueList<MergingIndex>::Iterator mergingIt;

  if ( !mergingName.isEmpty() )
    mergingIt = node->findIndex( mergingName );
  else
    mergingIt = node->findIndex( d->m_clientName );

  QValueList<MergingIndex>::Iterator mergingEnd = node->mergingIndices.end();
  it = mergingEnd;

  if ( ( mergingIt == mergingEnd && d->m_currentDefaultMergingIt == mergingEnd ) || ignoreDefaultMergingIndex )
    return node->index;

  if ( mergingIt != mergingEnd )
    it = mergingIt;
  else
    it = d->m_currentDefaultMergingIt;

  return (*it).value;
}

void KXMLGUIFactory::adjustMergingIndices( KXMLGUIContainerNode *node, int offset, const QValueList<MergingIndex>::Iterator &it )
{
  QValueList<MergingIndex>::Iterator mergingIt = it;
  QValueList<MergingIndex>::Iterator mergingEnd = node->mergingIndices.end();

  for (; mergingIt != mergingEnd; ++mergingIt )
    (*mergingIt).value += offset;

  node->index += offset;
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

QWidget *KXMLGUIFactory::createContainer( QWidget *parent, int index, const QDomElement &element, int &id, KXMLGUIBuilder **builder )
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

KXMLGUIContainerClient *KXMLGUIFactory::findClient( KXMLGUIContainerNode *node, const QString &groupName,
                                                    const QValueList<MergingIndex>::Iterator &mIt )
{
  QListIterator<KXMLGUIContainerClient> clientIt( node->clients );

  for (; clientIt.current(); ++clientIt )
    if ( clientIt.current()->client == m_client )
    {
      if ( groupName.isEmpty() )
        return clientIt.current();

      if ( groupName == clientIt.current()->groupName )
        return clientIt.current();
    }

  KXMLGUIContainerClient *client = new KXMLGUIContainerClient;
  client->client = m_client;
  client->groupName = groupName;

  if ( mIt != node->mergingIndices.end() )
    client->mergingName = (*mIt).mergingName;

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
  QValueList<MergingIndex>::Iterator mIt = node->mergingIndices.begin();
  QValueList<MergingIndex>::Iterator mEnd = node->mergingIndices.end();
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

    int idx = (*mIt).value;

    KXMLGUIContainerClient *client = findClient( node, QString::null, node->mergingIndices.end() );

    client->actionLists.insert( k, d->m_actionList );

    QListIterator<KAction> aIt( d->m_actionList );
    for (; aIt.current(); ++aIt )
      aIt.current()->plug( node->container, idx++ );

    adjustMergingIndices( node, d->m_actionList.count(), mIt );
  }

  QListIterator<KXMLGUIContainerNode> childIt( node->children );
  for (; childIt.current(); ++childIt )
    plugActionListRecursive( childIt.current() );
}

void KXMLGUIFactory::unplugActionListRecursive( KXMLGUIContainerNode *node )
{
  QValueList<MergingIndex>::Iterator mIt = node->mergingIndices.begin();
  QValueList<MergingIndex>::Iterator mEnd = node->mergingIndices.end();
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

    KXMLGUIContainerClient *client = findClient( node, QString::null, node->mergingIndices.end() );

    QMap<QString, QList<KAction> >::Iterator lIt = client->actionLists.find( k );
    if ( lIt == client->actionLists.end() )
      continue;

    QListIterator<KAction> aIt( lIt.data() );
    for (; aIt.current(); ++aIt )
      aIt.current()->unplug( node->container );

    adjustMergingIndices( node, -lIt.data().count(), mIt );

    client->actionLists.remove( lIt );
  }


  QListIterator<KXMLGUIContainerNode> childIt( node->children );
  for (; childIt.current(); ++childIt )
    unplugActionListRecursive( childIt.current() );
}

#include "kxmlgui.moc"
