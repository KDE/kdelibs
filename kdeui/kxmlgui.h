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

#include <qobject.h>
#include <qlist.h>
#include <qdom.h>
#include <qmap.h>
#include <qlist.h>
#include <qvaluelist.h>

class KAction;
class KXMLGUIFactoryPrivate;
class KXMLGUIClient;
class KXMLGUIBuilder;
class KXMLGUIContainerNode;
class KXMLGUIContainerClient;
class KInstance;
struct MergingIndex; // ### find better name to avoid possible name clashes

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
class KXMLGUIFactory : public QObject
{
  Q_OBJECT
 public:
  /**
   * Constructs a KXMLGUIFactory. The provided @p builder @ref KXMLGUIBuilder will be called
   * for creating and removing container widgets, when clients are added/removed from the GUI.
   */
  KXMLGUIFactory( KXMLGUIBuilder *builder, QObject *parent = 0, const char *name = 0 );

  /**
   * Destructor
   */
  ~KXMLGUIFactory();

  // XXX move to somewhere else? (Simon)
  static QString readConfigFile( const QString &filename, bool never_null, KInstance *instance = 0 );
  static QString readConfigFile( const QString &filename, KInstance *instance = 0 );
  static bool saveConfigFile( const QDomDocument& doc, const QString& filename,
                              KInstance *instance = 0 );

  static QString documentToXML( const QDomDocument& doc );
  static QString elementToXML( const QDomElement& elem );

  /**
   * Removes all QDomComment objects from the specified node and all its children.
   */
  static void removeDOMComments( QDomNode &node );

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

  void plugActionList( KXMLGUIClient *client, const QString &name, const QList<KAction> &actionList );
  void unplugActionList( KXMLGUIClient *client, const QString &name );

  /**
   * Returns a list of all clients currently added to this factory
   */
  QList<KXMLGUIClient> clients() const;

  /**
   * Use this method to get access to a container widget with the name specified with @p containerName
   * and which is owned by the @p client. The container name is specified with a "name" attribute in the
   * XML document.
   *
   * @param useTagName Specify whether the compare the specified name with the name attribute or
   *        the tag name.
   *
   * This method may return 0L if no container with the given name exists or is not owned by the client.
   */
  QWidget *container( const QString &containerName, KXMLGUIClient *client, bool useTagName = false );

  QList<QWidget> containers( const QString &tagName );

  /**
   * Use this method to free all memory allocated by the KXMLGUIFactory. This deletes the internal node
   * tree and therefore resets the internal state of the class. Please note that the actual GUI is
   * NOT touched at all, meaning no containers are deleted nor any actions unplugged. That is
   * something you have to do on your own. So use this method only if you know what you are doing :-)
   *
   * (also note that this will call @ref KXMLGUIClient::setFactory( 0L ) for all inserted clients)
   */
  void reset();

  /**
   * Use this method to free all memory allocated by the KXMLGUIFactory for a specific container,
   * including all child containers and actions. This deletes the internal node subtree for the
   * specified container. The actual GUI is not touched, no containers are deleted or any actions
   * unplugged. Use this method only if you know what you are doing :-)
   *
   * (also note that this will call @ref KXMLGUIClient::setFactory( 0L ) for all clients of the
   * container)
   */
  void resetContainer( const QString &containerName, bool useTagName = false );

 signals:
  void clientAdded( KXMLGUIClient *client );
  void clientRemoved( KXMLGUIClient *client );

 private:

  void resetInternal( KXMLGUIContainerNode *node );

  void buildRecursive( const QDomElement &element, KXMLGUIContainerNode *parentNode );
  bool removeRecursive( QDomElement &element, KXMLGUIContainerNode *node );

  int calcMergingIndex( KXMLGUIContainerNode *node, const QString &mergingName,
                        QValueList<MergingIndex>::Iterator &it, bool ingoreDefaultMergingIndex );
  void adjustMergingIndices( KXMLGUIContainerNode *node, int offset,
                             const QValueList<MergingIndex>::Iterator &it );

  QWidget *findRecursive( KXMLGUIContainerNode *node, bool tag );

  QList<QWidget> findRecursive( KXMLGUIContainerNode *node, const QString &tagName );

  QWidget *createContainer( QWidget *parent, int index, const QDomElement &element, int &id,
                            KXMLGUIBuilder **builder );

  KXMLGUIContainerClient *findClient( KXMLGUIContainerNode *node, const QString &groupName,
                                      const QValueList<MergingIndex>::Iterator &mIt );

  void plugActionListRecursive( KXMLGUIContainerNode *node );
  void unplugActionListRecursive( KXMLGUIContainerNode *node );

  KXMLGUIClient *m_client;
  KXMLGUIBuilder *m_builder;

  KXMLGUIFactoryPrivate *d;
};

#endif
