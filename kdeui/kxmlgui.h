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

  QWidget *container( const QString &containerName, KXMLGUIClient *client );

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

  KXMLGUIClient *m_client;
  KXMLGUIBuilder *m_builder;

  KXMLGUIFactoryPrivate *d;
};

#endif
