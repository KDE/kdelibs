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

/**
 * Class for handling Palettes.
 *
 * This class makes it easy to handle palettes.
 * A palette is a set of colours. This class can read
 * and write palettes from and to file.
 *
 * This class uses the "GIMP" palette file format.
 *
 * This class is totally unrelated to QPalette.
 *
 * @author Waldo Bastian (bastian@kde.org)
 * @version $Id:  $
 **/
class KPalette {
public:
   /**
    * Query which KDE palettes are installed.
    *
    * @return A list with a palette names.
    */
   static QStringList getPaletteList();

   /**
    * KPalette constructor. Creates a KPalette from a file
    * the filename is derived from the name.
    * @param name The name of palette as returned by @ref #getPaletteList
    **/
   KPalette(const QString &name=QString::null);

   /**
    * KPalette copy constructor.
    **/
   KPalette(const KPalette &);

   /**
    * KPalette destructor.
    **/
   virtual ~KPalette();
   
   /**
    * KPalette assignment operator
    **/
   KPalette& operator=( const KPalette &);

   /**
    * Get the description of the palette.
    **/
   QString description()
   	{ return mDesc; }

   /**   	
    * Set the description of the palette.
    **/
   void setDescription(const QString &desc)
   	{ mDesc = desc; }
   
   /**
    * Get the name of the palette. 
    **/
   QString name()
   	{ return mName; }

   /**
    * Set the name of the palette
    **/
   void setName(const QString &name)
   	{ mName = name; }

   enum Editable { Yes, No, Ask };
  
   /**
    * Returns whether the palette may be edited.
    **/
   Editable editable()
   	{ return mEditable; }

   /**
    * Change whether the palette may be editted.
    **/
   void setEditable(Editable editable)
   	{ mEditable = editable; }
   
   /**
    * Return the number of colors in the palette.
    **/
   int nrColors()
   	{ return (int) mKolorList.count(); }

   /**
    * Find color by index.
    * @return the index-th color of the palette.
    **/
   QColor color(int index);
   
   /**
    * Find index by color.
    * @return the index of the color in the palette or -1 if the
    * color is not found.
    **/
   int findColor(const QColor &color);

   /** 
    * Find colorname by index.
    * @return The name of the index-th color.
    * Note that not all palettes have named the colors.
    **/
   QString colorName(int index);
   
   /**
    * Find colorname by color.
    * @return The name of color according to this palette.
    * Note that not all palettes have named the colors.
    * Note also that each palette can give the same color
    * a different name.
    **/
   QString colorName(const QColor &color) 
   	{ return colorName( findColor(color)); }
   
   /**
    * Add a color.
    * @param newColor The color to add.
    * @param newColorName The name of the color.
    * @return the index of the added color.
    **/
   int addColor(const QColor &newColor, 
                const QString &newColorName = QString::null);

   /**
    * Change a color
    * @param index Index of the color to change
    * @param newColor The new color.
    * @param newColorName The new color name.
    * @return the index of the new color or -1 if the color couldn't
    * be changed.
    **/
   int changeColor(int index, 
                   const QColor &newColor, 
                   const QString &newColorName = QString::null);
 
   /**
    * Change a color
    * @param oldColor The original color
    * @param newColor The new color.
    * @param newColorName The new color name.
    * @return the index of the new color or -1 if the color couldn't
    * be changed.
    **/
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

