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

class QAction;
class KXMLGUIFactoryPrivate;

/**
 * @internal
 * Abstract interface for a "GUI builder", used by the GUIFactory
 * This interface is implemented by @ref MainWindow for the case where
 * the toplevel widget is a KTMainWindow. Other implementations may appear
 * in the future (dialogs for instance)
 */
class KXMLGUIBuilder
{
 public:

  KXMLGUIBuilder() {}
  virtual ~KXMLGUIBuilder() {}

  /**
   * Create a container (menubar/menu/toolbar/statusbar/separator/...)
   * from an element in the XML file
   *
   * @param parent The parent for the container
   * @param index The index where the container should be inserted
   *              into the parent container/widget
   * @param element The element from the DOM tree describing the
   *                container (use it to access container specified
   *                attributes or child elements)
   * @param containerStateBuffer A buffer which possibibly contains
   *                             previously saved container state
   *                             information, return via @ref
   *                             removeContainer .
   */
  virtual QWidget *createContainer( QWidget *parent, int index,
          const QDomElement &element, const QByteArray &containerStateBuffer,
          int &id ) = 0;

  /**
   * Remove the given (and previously via @ref createContainer )
   * created container.
   *
   * @return A buffer containing state information about the deleted
   *         container (like the last position of a toolbar container
   *         for example). The buffer is passed again to
   *         @ref createContainer when the same container is about to be
   *         created again later.
   */
  virtual QByteArray removeContainer( QWidget *container, QWidget *parent,
          int id ) = 0;

  virtual int insertSeparator( QWidget *parent, int index ) = 0;
  virtual void removeSeparator( QWidget *parent, int id ) = 0;
};

class KXMLGUIServantPrivate;
class KXMLGUIFactory;

/**
 * @internal
 * Abstract interface for serving actions and xml to the GUI factory
 */
class KXMLGUIServant
{
 public:
  KXMLGUIServant();
  KXMLGUIServant( KXMLGUIServant *parent );
  virtual ~KXMLGUIServant();

  virtual QAction *action( const QDomElement &element ) = 0;

  virtual QDomDocument document() const = 0;

  /**
   * default implementation, storing the given data in an internal
   * map. Called from KKXMLGUIFactory when removing containers which
   * were owned by the servant.
   */
  virtual void storeContainerStateBuffer( const QString &key, const QByteArray &data );
  /**
   * default implementation, returning a previously via
   * @ref storeContainerStateBuffer saved data. Called from
   * KKXMLGUIFactory when creating a new container.
   */
  virtual QByteArray takeContainerStateBuffer( const QString &key );

  void setFactory( KXMLGUIFactory *factory );
  KXMLGUIFactory *factory() const;

  KXMLGUIServant *parentServant() const;

  void insertChildServant( KXMLGUIServant *child );
  void removeChildServant( KXMLGUIServant *child );
  const QList<KXMLGUIServant> *childServants();

  void setServantBuilder( KXMLGUIBuilder *builder );
  KXMLGUIBuilder *servantBuilder() const;

 private:
  KXMLGUIServantPrivate *d;
};

class KXMLGUIContainerNode;
class KXMLGUIContainerClient;

/**
 * @internal
 * The GUI merging engine, core part of KParts.
 * It is able to dynamically add or remove a "servant" without having to rebuild
 * the whole GUI from scratch. Its author says it's not "big thing or new
 * technology" but it really is.
 */
class KXMLGUIFactory
{
 public:
  KXMLGUIFactory( KXMLGUIBuilder *builder );
  ~KXMLGUIFactory();

  // XXX move to somewhere else? (Simon)
  static QString readConfigFile( const QString &filename );

  /**
   * Creates the GUI described by the QDomDocument of the servant,
   * using the servant's actions, and merges it with the previously
   * created GUI.
   */
  void addServant( KXMLGUIServant *servant );

  /**
   * Removes the GUI described by the servant, by unplugging all
   * provided actions and removing all owned containers (and storing
   * container state information in the given servant)
   */
  void removeServant( KXMLGUIServant *servant );

  QWidget *container( const QString &containerName, KXMLGUIServant *servant );

 private:
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

  KXMLGUIServant *m_servant;
  KXMLGUIBuilder *m_builder;

  KXMLGUIFactoryPrivate *d;
};

#endif
