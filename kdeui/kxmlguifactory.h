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

#ifndef __kxmlguifactory_h__
#define __kxmlguifactory_h__

#include <qobject.h>
#include <qptrlist.h>
#include <qdom.h>
#include <qvaluelist.h>

class KAction;
class KXMLGUIFactoryPrivate;
class KXMLGUIClient;
class KXMLGUIBuilder;
class KInstance;

namespace KXMLGUI
{
struct MergingIndex;
class ContainerNode;
class ContainerClient;
class BuildHelper;
}

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
  friend class KXMLGUI::BuildHelper;
  Q_OBJECT
 public:
  /**
   * Constructs a KXMLGUIFactory. The provided @p builder @ref KXMLGUIBuilder will be called
   * for creating and removing container widgets, when clients are added/removed from the GUI.
   *
   * Note that the ownership of the given KXMLGUIBuilder object won't be transferred to this
   * KXMLGUIFactory, so you have to take care of deleting it properly.
   */
  KXMLGUIFactory( KXMLGUIBuilder *builder, QObject *parent = 0, const char *name = 0 );

  /**
   * Destructor
   */
  ~KXMLGUIFactory();

  // XXX move to somewhere else? (Simon)
  static QString readConfigFile( const QString &filename, bool never_null, const KInstance *instance = 0 );
  static QString readConfigFile( const QString &filename, const KInstance *instance = 0 );
  static bool saveConfigFile( const QDomDocument& doc, const QString& filename,
                              const KInstance *instance = 0 );

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
   * This also means that the order in which clients are added to the factory
   * is relevant; assuming that your application supports plugins, you should
   * first add your application to the factory and then the plugin, so that the
   * plugin's UI is merged into the UI of your application, and not the other
   * way round.
   */
  void addClient( KXMLGUIClient *client );

  /**
   * Removes the GUI described by the client, by unplugging all
   * provided actions and removing all owned containers (and storing
   * container state information in the given client)
   */
  void removeClient( KXMLGUIClient *client );

  void plugActionList( KXMLGUIClient *client, const QString &name, const QPtrList<KAction> &actionList );
  void unplugActionList( KXMLGUIClient *client, const QString &name );

  /**
   * Returns a list of all clients currently added to this factory
   */
  QPtrList<KXMLGUIClient> clients() const;

  /**
   * Use this method to get access to a container widget with the name specified with @p containerName
   * and which is owned by the @p client. The container name is specified with a "name" attribute in the
   * XML document.
   *
   * This function is particularly useful for getting hold of a popupmenu defined in an XMLUI file.
   * For instance:
   * QPopupMenu *popup = static_cast<QPopupMenu*>(factory()->container("my_popup",this))
   * where "my_popup" is the name of the menu in the XMLUI file, and
   * "this" is XMLGUIClient which owns the popupmenu (e.g. the mainwindow, or the part, or the plugin...)
   *
   * @param useTagName Specifies whether to compare the specified name with the name attribute or
   *        the tag name.
   *
   * This method may return 0L if no container with the given name exists or is not owned by the client.
   */
  QWidget *container( const QString &containerName, KXMLGUIClient *client, bool useTagName = false );

  QPtrList<QWidget> containers( const QString &tagName );

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

  QWidget *findRecursive( KXMLGUI::ContainerNode *node, bool tag );

  QPtrList<QWidget> findRecursive( KXMLGUI::ContainerNode *node, const QString &tagName );

  void applyActionProperties( const QDomElement &element );
  void configureAction( KAction *action, const QDomNamedNodeMap &attributes );
  void configureAction( KAction *action, const QDomAttr &attribute );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KXMLGUIFactoryPrivate *d;
};

#endif
/* vim: et sw=4
 */
