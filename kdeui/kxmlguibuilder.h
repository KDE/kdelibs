/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
                      David Faure <faure@kde.org>

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
#ifndef __kxmlguibuilder_h__
#define __kxmlguibuilder_h__

#include <qdom.h>
#include <qstringlist.h>

class KXMLGUIBuilderPrivate;
class QWidget;
class KInstance;
class KXMLGUIClient;

/**
 * Abstract interface for a "GUI builder", used by the GUIFactory
 * This interface is implemented by @ref KMainWindow for the case where
 * the toplevel widget is a KMainWindow. Other implementations may appear
 * in the future (dialogs for instance)
 */
class KXMLGUIBuilder
{
 public:

  KXMLGUIBuilder( QWidget *widget );
  virtual ~KXMLGUIBuilder();

  /* @internal */
  KXMLGUIClient *builderClient() const;
  /* @internal */
  void setBuilderClient( KXMLGUIClient *client );
  /* @internal */
  KInstance *builderInstance() const;
  /* @internal */
  void setBuilderInstance( KInstance *instance );
  /* @internal */
  QWidget *widget();

  virtual QStringList containerTags() const;

  /**
   * Creates a container (menubar/menu/toolbar/statusbar/separator/...)
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
          const QDomElement &element, int &id );

  /**
   * Removes the given (and previously via @ref createContainer )
   * created container.
   *
   * @return A buffer containing state information about the deleted
   *         container (like the last position of a toolbar container
   *         for example). The buffer is passed again to
   *         @ref createContainer when the same container is about to be
   *         created again later.
   */
  virtual void removeContainer( QWidget *container, QWidget *parent,
				QDomElement &element, int id );

  virtual QStringList customTags() const;

  virtual int createCustomElement( QWidget *parent, int index, const QDomElement &element );

  virtual void removeCustomElement( QWidget *parent, int id );

  virtual void finalizeGUI( KXMLGUIClient *client );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KXMLGUIBuilderPrivate *d;
};

#endif
