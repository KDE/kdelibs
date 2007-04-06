/* This file is part of the KDE project
   Copyright (C) 2000 Falk Brettschneider <gigafalk@yahoo.com>
             (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __DOCKMAINWINDOW_H
#define __DOCKMAINWINDOW_H

#include <Qt3Support/Q3PtrList>
#include <kaction.h>

#include <k3dockwidget.h>

#include <kparts/part.h>

class QString;

namespace KParts
{

class DockMainWindow3Private;

/**
 * A KPart-aware main window with ability for docking widgets, whose user interface is described in XML.
 *
 * Inherit your main dock-window from this class
 * and don't forget to call setXMLFile() in the inherited constructor.
 *
 * It implements all internal interfaces in the case of a K3DockMainWindow3 as host:
 * the builder and servant interface (for menu merging).
 */
class KDE3SUPPORT_EXPORT DockMainWindow3 : public K3DockMainWindow, virtual public PartBase
{
  Q_OBJECT
 public:
  /**
   * Constructor, same signature as K3DockMainWindow3.
   */
  DockMainWindow3( QWidget* parent = 0L, const char *name = 0L, Qt::WFlags f = 0 );
  /**
   * Destructor.
   */
  virtual ~DockMainWindow3();

protected Q_SLOTS:

  /**
   * Create the GUI (by merging the host's and the active part's)
   *
   * Called on startup and whenever the active part changes
   * For this you need to connect this slot to the
   * KPartManager::activePartChanged() signal
   * @param part The active part (set to 0L if no part).
   */
  void createGUI( KParts::Part * part );

  /**
   * Called when the active part wants to change the statusbar message
   * Reimplement if your dock-mainwindow has a complex statusbar
   * (with several items)
   */
  virtual void slotSetStatusBarText( const QString & );

protected:
    virtual void createShellGUI( bool create = true );

 private:
  DockMainWindow3Private *d;
  };

}

#endif
