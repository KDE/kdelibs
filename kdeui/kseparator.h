/*
 *   Copyright (C) 1997  Michael Roth <mroth@wirlweb.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __KSEPARATOR_H__
#define __KSEPARATOR_H__

#include <qframe.h>

/// Horizontal and vertical lines
/**
'kseparator' is a small class to provide a identically look of horizontal or
vertical lines in all KDE applications.
*/
class KSeparator : public QFrame
{
   public:
      KSeparator(QWidget* parent=0, const char* name=0, WFlags f=0);
      KSeparator(int orientation, QWidget* parent=0, const char* name=0, WFlags f=0);
      
      int orientation() const;
      void setOrientation(int);
      
      virtual QSize sizeHint() const;
};


#endif // __KSEPARATOR_H__
