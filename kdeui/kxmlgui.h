/* This file is part of the KDE libraries
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kxmlgui_h__
#define __kxmlgui_h__

#include <qlist.h>
#include <qdom.h>
#include <qmap.h>
#include <qlist.h>

class KAction;
class KXMLGUIFactoryPrivate;
class KXMLGUIClient;
class KXMLGUIBuilder;
class KXMLGUIContainerNode;
class KXMLGUIContainerClient;

/**
 * KXMLGUIFactory, together with @ref KXMLGUIClient objects, can be used to create
 * a GUI of container widgets (like menus, toolbars, etc.) and container items
 * (menu items, toolbar buttons, etc.) from an XML document and action objects.
 *
 * Each @ref KXMLGUIClient represents a part of the GUI, composed from containers and
 * actions. KXMLGUIFactory takes care of building (with the help of a @ref KXMLGUIBuilder)
 * and merging the GUI from an unlimited number of clients.
 *
 * Each client provides XML through a @ref QDomDocument and actions through a
 * @ref KActionCollection . The XML document contains the rules for how to merge the
 * GUI.
 *
 * KXMLGUIFactory processes the DOM tree provided by a client and plugs in the client's actions,
 * according to the XML and the merging rules of previously inserted clients. Container widgets
 * are built via a @ref KXMLGUIBuilder , which has to be provided with the KXMLGUIFactory constructor.
 */
class KXMLGUIFactory
{
 public:
  /**
   * Constructs a KXMLGUIFactory. The provided @p builder @ref KXMLGUIBuilder will be called
   * for creating and removing container widgets, when clients are added/removed from the GUI.
   */
  KXMLGUIFactory( KXMLGUIBuilder *builder );

  /**
   * Destructor
   */
  ~KXMLGUIFactory();

  // XXX move to somewhere else? (Simon)
  static QString readConfigFile( const QString &filename );

  static QString documentToXML( const QDomDocument& doc );
  static QString elementToXML( const QDomElement& elem );

  /**
   * Creates the GUI described by the QDomDocument of the client,
   * using the client's actions, and merges it with the previously
   * created GUI.
   */
  void addClient( KXMLGUIClient *client );

  /**
   * Removes the GUI described by the client, by unplugging all
   * provided actions and removing all owned containers (and storing
   * container state information in the given client)
   */
  void removeClient( KXMLGUIClient *client );

  /**
   * Use this method to get access to a container widget with the name specified with @p containerName
   * and which is owned by the @p client. The container name is specified with a "name" attribute in the
   * XML document.
   *
   * This method may return 0L if no container with the given name exists or is not owned by the client.
   */
  QWidget *container( const QString &containerName, KXMLGUIClient *client );

  /**
   * Use this method to free all memory allocated by the KXMLGUIFactory. This deletes the internal node
   * tree and therefore resets the internal state of the class. Please note that the actual GUI is
   * NOT touched at all, meaning no containers are being deleted nor any actions unplugged. That is 
   * something you have to do on your own. So use this method only if you know what you are doing :-)
   *
   * (also note that this will call @ref KXMLGUIClient::setFactory( 0L ) for all inserted clients)
   */
  void reset();

 private:

  void resetInternal( KXMLGUIContainerNode *node );

  void buildRecursive( const QDomElement &element, KXMLGUIContainerNode *parentNode );
  bool removeRecursive( KXMLGUIContainerNode *node );

  bool calcMergingIndex( KXMLGUIContainerNode *node, const QString &mergingName, QMap<QString,int>::Iterator &it );
  void adjustMergingIndices( KXMLGUIContainerNode *node, int idx, int val, const QMap<QString,int>::Iterator &it );

  void pruneContainers( KXMLGUIContainerNode *node );

  KXMLGUIContainerNode *findContainer( KXMLGUIContainerNode *node, const QDomElement &element, const QList<QWidget> *excludeList );

  KXMLGUIContainerNode *findContainerNode( KXMLGUIContainerNode *parentNode, QWidget *container );

  QWidget *findRecursive( KXMLGUIContainerNode *node );

  QWidget *createContainer( QWidget *parent, int index, const QDomElement &element, const QByteArray &containerStateBuffer, int &id, KXMLGUIBuilder **builder );

  KXMLGUIContainerClient *findClient( KXMLGUIContainerNode *node, const QString &groupName );

  KXMLGUIClient *m_client;
  KXMLGUIBuilder *m_builder;

  KXMLGUIFactoryPrivate *d;
};

#endif
