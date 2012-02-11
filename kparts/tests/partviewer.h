/*
    Copyright (c) 2000 David Faure <faure@kde.org>
    Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef PARTVIEWER_H
#define PARTVIEWER_H

#include <kparts/mainwindow.h>

class PartViewer : public KParts::MainWindow
{
  Q_OBJECT
public:
  PartViewer();
  virtual ~PartViewer();

  void openUrl( const QUrl & url );

protected Q_SLOTS:
  void slotFileOpen();

private:
  KParts::ReadOnlyPart *m_part;
};

#endif
