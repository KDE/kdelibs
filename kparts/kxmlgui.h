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

#ifndef __kxmlgui_h__
#define __kxmlgui_h__

#include <qobject.h>
#include <qaction.h>
#include <qdom.h>
#include <qmap.h>
#include <qguardedptr.h>

class KPart;
class KXMLGUIFactoryPrivate;

/**
 * This file contains all the "kernel" of KParts.
 * You shouldn't have to look in here unless you want to hack KParts.
 */

/**
 * Abstract interface for a "GUI builder", used by the GUIFactory
 */
class KXMLGUIBuilder
{
 public:

  KXMLGUIBuilder() {}
  virtual ~KXMLGUIBuilder() {}

  /**
   * Create a container (menubar/menu/toolbar/statusbar/...) from an
   * element in the XML file
   * @param parent the parent for the widget
   * @return 0L if you handled the element yourself (like for Separators for
   * example)
   */
  virtual QObject *createContainer( QWidget *parent, int index, const QDomElement &element, const QByteArray &containerStateBuffer ) = 0;

  virtual QByteArray removeContainer( QObject *container, QWidget *parent ) = 0;
};

class KXMLGUIServantPrivate;

/**
 * Abstract interface for serving actions and xml to the GUI factory
 */
class KXMLGUIServant
{
 public:
  KXMLGUIServant();
  virtual ~KXMLGUIServant();

  virtual QAction *action( const QDomElement &element ) = 0;

  virtual QDomDocument document() = 0;

  virtual void storeContainerStateBuffer( const QString &key, const QByteArray &data );
  virtual QByteArray takeContainerStateBuffer( const QString &key );

 private:
  KXMLGUIServantPrivate *d;
};

/**
 * Implementation of the servant interface that serves nothing.
 * Used when no part is active.
 */
class KNullGUIServant : public KXMLGUIServant
{
 public:
  KNullGUIServant() {}

  virtual QAction *action( const QDomElement & ) { return 0L; }
  virtual QDomDocument document() { return QDomDocument(); }
};

class KXMLGUIContainerNode;

class KXMLGUIFactory
{
 public:
  KXMLGUIFactory( KXMLGUIBuilder *builder );
  ~KXMLGUIFactory();

  static QString readConfigFile( const QString &filename );

  void addServant( KXMLGUIServant *servant );

  void removeServant( KXMLGUIServant *servant );

 private:
  void buildRecursive( const QDomElement &element, KXMLGUIContainerNode *parentNode );
  bool removeRecursive( KXMLGUIContainerNode *node );

  static KXMLGUIContainerNode *findContainer( KXMLGUIContainerNode *node, const QDomElement &element, const QList<QObject> &excludeList );

  KXMLGUIServant *m_servant;
  KXMLGUIBuilder *m_builder;

  KXMLGUIFactoryPrivate *d;
};

#endif
