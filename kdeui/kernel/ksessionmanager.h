/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (c) 1998, 1999 KDE Team

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

#ifndef KSESSIONMANAGER_H
#define KSESSIONMANAGER_H

#include <kdeui_export.h>

#include <QtCore/QList>
#include <QtGui/QSessionManager>

/**
   Provides highlevel access to session management on a per-object
   base.

   KSessionManager makes it possible to provide implementations for
 QApplication::commitData() and QApplication::saveState(), without
   subclassing KApplication. KMainWindow internally makes use of this.

   You don't need to do anything with this class when using
   KMainWindow. Instead, use KMainWindow::saveProperties(),
 KMainWindow::readProperties(), KMainWindow::queryClose(),
 KMainWindow::queryExit() and friends.

  @short Highlevel access to session management.
  @author Matthias Ettrich <ettrich@kde.org>
 */
class KDEUI_EXPORT KSessionManager
{
public:
  KSessionManager();
  virtual ~KSessionManager();

    /**
       See QApplication::saveState() for documentation.

       This function is just a convenience version to avoid subclassing KApplication.

       Return true to indicate a successful state save or false to
       indicate a problem and to halt the shutdown process (will
       implicitly call sm.cancel() ).
     */
  virtual bool saveState( QSessionManager& sm );
    /**
       See QApplication::commitData() for documentation.

       This function is just a convenience version to avoid subclassing KApplication.

       Return true to indicate a successful commit of data or false to
       indicate a problem and to halt the shutdown process (will
       implicitly call sm.cancel() ).
     */
  virtual bool commitData( QSessionManager& sm );

  static QList<KSessionManager*>& sessionClients();

protected:
  /** Virtual hook, used to add new "virtual" functions while maintaining
      binary compatibility. Unused in this class.
  */
  virtual void virtual_hook( int id, void* data );

private:
  class Private;
  Private* const d;

  Q_DISABLE_COPY(KSessionManager)
};

#endif // KSESSIONMANAGER_H

