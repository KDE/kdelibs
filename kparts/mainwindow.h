/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __MAINWINDOW_H
#define __MAINWINDOW_H

#include <qptrlist.h>
#include <kaction.h>

#include <kmainwindow.h>

#include <kparts/part.h>

class QString;

namespace KParts
{

class MainWindowPrivate;

/**
 * A KPart-aware main window, whose user interface is described in XML.
 *
 * Inherit your main window from this class
 * and don't forget to call @ref setXMLFile() in the inherited constructor.
 *
 * It implements all internal interfaces in the case of a
 * @ref KMainWindow as host: the builder and servant interface (for menu
 * merging).
 */
class MainWindow : public KMainWindow, virtual public PartBase
{
  Q_OBJECT
 public:
  /**
   * Constructor, same signature as @ref KMainWindow.
   */
  MainWindow( QWidget* parent,  const char *name = 0L, WFlags f = WType_TopLevel | WDestructiveClose );
  /**
   * Compatibility Constructor, same signature as @ref KMainWindow.
   */
  MainWindow( const char *name = 0L, WFlags f = WDestructiveClose );
  /**
   * Destructor.
   */
  virtual ~MainWindow();

protected slots:

  /**
   * Create the GUI (by merging the host's and the active part's)
   * You _must_ call this in order to see any GUI being created.
   *
   * In a main window with multiple parts being shown (e.g. as in Konqueror)
   * you need to connect this slot to the
   * @ref KPartManager::activePartChanged() signal
   *
   * @param part The active part (set to 0L if no part).
   */
  void createGUI( KParts::Part * part );

  /**
   * Called when the active part wants to change the statusbar message
   * Reimplement if your mainwindow has a complex statusbar
   * (with several items)
   */
  virtual void slotSetStatusBarText( const QString & );

protected:
    virtual void createShellGUI( bool create = true );

private:
  MainWindowPrivate *d;
};

};

#endif
