/* This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 
    2 of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//-----------------------------------------------------------------------------
// KDE color palette.

#ifndef __KPALETTE_H__
#define __KPALETTE_H__

#include <qcolor.h>
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

class KPalette {
public:
   static QStringList getPaletteList();

   KPalette(const QString &name=QString::null);
   KPalette(const KPalette &);
   virtual ~KPalette();
   
   KPalette& operator=( const KPalette &);

   QString description()
   	{ return mDesc; }
   void setDescription(const QString &desc)
   	{ mDesc = desc; }
   
   QString name()
   	{ return mName; }
   void setName(const QString &name)
   	{ mName = name; }

   enum Editable { Yes, No, Ask };
   Editable editable()
   	{ return mEditable; }
   void setEditable(Editable editable)
   	{ mEditable = editable; }
   
   int nrColors()
   	{ return (int) mKolorList.count(); }

   QColor color(int index);
   int findColor(const QColor &color);

   QString colorName(int index);
   QString colorName(const QColor &color) 
   	{ return colorName( findColor(color)); }
   
   int addColor(const QColor &newColor, 
                const QString &newColorName = QString::null);
   int changeColor(int index, 
                   const QColor &newColor, 
                   const QString &newColorName = QString::null);
   int changeColor(const QColor &oldColor, 
                   const QColor &newColor, 
                   const QString &newColorName = QString::null)
   	{ return changeColor( findColor(oldColor), newColor, newColorName); }

private:   
   typedef struct { QColor color; QString name; } kolor;
   QList<kolor> mKolorList;
   
   QString mName;
   QString mDesc;
   Editable mEditable;
};


#endif		// __KCOLORDLG_H__

