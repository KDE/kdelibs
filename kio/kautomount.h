/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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
#ifndef __auto_mount_h__
#define __auto_mount_h__

#include <qobject.h>
#include <qstring.h>

namespace KIO {
class Job;
}

class KAutoMount : public QObject
{
  Q_OBJECT
public:
  KAutoMount( bool _readonly, const QString& _format, const QString& _device, const QString& _mountpoint,
              const QString & _desktopFile, bool _show_filemanager_window = true );
    
protected slots:
  void slotResult( KIO::Job * );
    
protected:
  QString m_strDevice;
  bool m_bShowFilemanagerWindow;
  QString m_desktopFile;
};

class KAutoUnmount : public QObject
{
  Q_OBJECT
public:
  KAutoUnmount( const QString & _mountpoint, const QString & _desktopFile );
    
protected slots:
  void slotResult( KIO::Job * );
private:
  QString m_desktopFile;
};

#endif
