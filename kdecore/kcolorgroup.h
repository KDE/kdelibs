/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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
// $Id$
//
// $Log$
// Revision 1.9  1999/04/18 09:15:04  kulow
// taking out config.h from Header files. I don't know if I haven't noticed
// before, but this is even very dangerous
//
// Revision 1.8  1998/06/16 06:03:20  kalle
// Implemented copy constructors and assignment operators or disabled them
//
// Revision 1.7  1998/01/18 14:38:36  kulow
// reverted the changes, Jacek commited.
// Only the RCS comments were affected, but to keep them consistent, I
// thought, it's better to revert them.
// I checked twice, that only comments are affected ;)
//
// Revision 1.5  1997/10/21 20:44:43  kulow
// removed all NULLs and replaced it with 0L or "".
// There are some left in mediatool, but this is not C++
//
// Revision 1.4  1997/10/16 11:14:29  torben
// Kalle: Copyright headers
// kdoctoolbar removed
//
// Revision 1.3  1997/10/09 11:46:20  kalle
// Assorted patches by Fritz Elfert, Rainer Bawidamann, Bernhard Kuhn and Lars Kneschke
//
// Revision 1.2  1997/10/04 19:42:46  kalle
// new KConfig
//
// Revision 1.1.1.1  1997/04/13 14:42:41  cvsuser
// Source imported
//
// Revision 1.1.1.1  1997/04/09 00:28:07  cvsuser
// Sources imported
//
// Revision 1.1  1996/11/24 12:54:42  kalle
// Initial revision
//
//
// KColorGroup header file - provide consistent colors for applications
//
// (C) 1996 Matthias Kalle Dalheimer <mda@stardivision.de>

#ifndef _KCOLORGROUP_H
#define _KCOLORGROUP_H

class KConfig;
class QString;
#include <qpalette.h>

/// KColorGroup: A collection of color entries for application widgets
/** A KColorGroup provides seven colors for widgets: foreground,
  background, light, mid, dark, text and base - exactly as the
  QColorGroup. KColorGroup adds the possibility to store the
  KColorGroup via the KConfig mechanism. It would be nicer if
  KColorGroup was derived from QColorGroup, but QColorGroup provides
  no write access to its data members except via the constructor.
  */
class KColorGroup
{
private:
  QColorGroup* pQColorGroup;
public:
  /// Construct a KColorGroup with all colors black
  /** Construct a KColorGroup with all colors black */
  KColorGroup();

  /// Construct a KColorGroup with given colors.
  /** Construct a KColorGroup with given colors. */
  KColorGroup( const QColor& rForeground, const QColor& rBackground,
	       const QColor& rLight, const QColor& rMid, const QColor&
	       rDark, const QColor& rText, const QColor& rBase );

  /** Copy-constructor */
  KColorGroup( const KColorGroup& );
  
  /** assignment operator */
  KColorGroup& operator= ( const KColorGroup& );

  /// KColorGroup destructor
  /** KColorGroup destructor */
  ~KColorGroup();

  /// Retrieve the foreground value.
  /** Retrieve the foreground value. */
  const QColor& foreground() const 
  { return pQColorGroup->foreground(); }

  /// Retrieve the background value.
  /** Retrieve the background value. */
  const QColor& background() const 
  { return pQColorGroup->background(); }

  /// Retrieve the light value.
  /** Retrieve the light value. */
  const QColor& light() const 
  { return pQColorGroup->light(); }

  /// Retrieve the mid value.
  /** Retrieve the mid value. */
  const QColor& mid() const 
  { return pQColorGroup->mid(); }

  /// Retrieve the dark value.
  /** Retrieve the dark value. */
  const QColor& dark() const 
  { return pQColorGroup->dark(); }

  /// Retrieve the text value.
  /** Retrieve the text value. */
  const QColor& text() const 
  { return pQColorGroup->text(); }

  /// Retrieve the base value.
  /** Retrieve the base value. */
  const QColor& base() const 
  { return pQColorGroup->base(); }

  /// Compare two KColorGroups.
  /** Compare two KColorGroups. */
  bool operator==( const KColorGroup& rColorGroup ) const
  { return (*pQColorGroup) == (*rColorGroup.pQColorGroup); }

  /// Compare two KColorGroups.
  /** Compare two KColorGroups. */
  bool operator!=( const KColorGroup& rColorGroup ) const
  { return (*pQColorGroup) != (*rColorGroup.pQColorGroup); }

  /// Load color values from a KConfig object.
  /** Load color values from a KConfig object. The group "ColorGroup"
    is used. If pString is not 0, the value of pString is prepended
    to the group name.
    */
  void load( KConfig& rConfig, QString* pString = 0L );

  /// Save color values to a KConfig object.
  /** Save color values to a KConfig object. The group "ColorGroup" is
    used. If pString is not 0, the value of pString is prepended to
    the group name.
    */
  void save( KConfig& rConfig, QString* pString = 0L );
  
  /// Return the QColorGroup object
  /** Return the QColorGroup object (for drawing shades e.g.).
   */
  const QColorGroup *colorGroup() const;
};

#endif
