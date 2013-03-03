/*
    Copyright (c) 1999, 2000 David Faure <faure@kde.org>
    Copyright (c) 1999, 2000 Simon Hausmann <hausmann@kde.org>

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

#ifndef __parts_h__
#define __parts_h__

#include <kparts/part.h>
#include <kaboutdata.h>

class QTextEdit;
namespace KParts {
class GUIActivateEvent;
}

class Part1 : public KParts::ReadOnlyPart
{
  Q_OBJECT
public:
  Part1( QObject *parent, QWidget * parentWidget );
  virtual ~Part1();

public slots:
 void slotBlah();
 void slotFooBar();

protected:
  virtual bool openFile();

protected:
  QTextEdit * m_edit;
  KAboutData m_componentData;
};

class Part2 : public KParts::Part
{
  Q_OBJECT
public:
  Part2( QObject *parent, QWidget * parentWidget );
  virtual ~Part2();

protected:
  // This is not mandatory - only if you care about setting the
  // part caption when the part is used in a multi-part environment
  // (i.e. in a part manager)
  // There is a default impl for ReadOnlyPart...
  virtual void guiActivateEvent( KParts::GUIActivateEvent * );
  KAboutData m_componentData;
};

#endif
