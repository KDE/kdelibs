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
// KColorGroup header file - provide consistent colors for applications
//
// (C) 1996 Matthias Kalle Dalheimer <mda@stardivision.de>

#ifndef _KCOLORGROUP_H
#define _KCOLORGROUP_H

class KConfig;
class QString;
class KColorGroupPrivate;

#include <qpalette.h>

/** 
 * A KColorGroup provides seven colors for widgets: foreground,
 * background, light, mid, dark, text and base - exactly as the
 * QColorGroup. KColorGroup adds the possibility to store the KColorGroup
 * via the KConfig mechanism. It would be nicer if KColorGroup was
 * derived from QColorGroup, but QColorGroup provides no write access
 * to its data members except via the constructor.
 * 
 * @see QColorGroup
 * @short Similar to @ref QColorGroup, with color write accessors.
 * @author Kalle Dalheimer <kalle@kde.org>
 * @version $Id$
 */
class KColorGroup
{
private:
  QColorGroup* pQColorGroup;
  KColorGroupPrivate *d;

public:
  /** Construct a KColorGroup with all colors black */
  KColorGroup();

  /// Construct a KColorGroup with given colors.
  KColorGroup( const QColor& rForeground, const QColor& rBackground,
	       const QColor& rLight, const QColor& rMid, const QColor&
	       rDark, const QColor& rText, const QColor& rBase );

  /** Copy-constructor */
  KColorGroup( const KColorGroup& );
  
  /** assignment operator */
  KColorGroup& operator= ( const KColorGroup& );

  /** KColorGroup destructor */
  ~KColorGroup();

  /** Retrieve the foreground value. */
  const QColor& foreground() const 
  { return pQColorGroup->foreground(); }

  /** Retrieve the background value. */
  const QColor& background() const 
  { return pQColorGroup->background(); }

  /** Retrieve the light value. */
  const QColor& light() const 
  { return pQColorGroup->light(); }

  /** Retrieve the mid value. */
  const QColor& mid() const 
  { return pQColorGroup->mid(); }

  /** Retrieve the dark value. */
  const QColor& dark() const 
  { return pQColorGroup->dark(); }

  /** Retrieve the text value. */
  const QColor& text() const 
  { return pQColorGroup->text(); }

  /** Retrieve the base value. */
  const QColor& base() const 
  { return pQColorGroup->base(); }

  /** Compare two KColorGroups. */
  bool operator==( const KColorGroup& rColorGroup ) const
  { return (*pQColorGroup) == (*rColorGroup.pQColorGroup); }

  /** Compare two KColorGroups. */
  bool operator!=( const KColorGroup& rColorGroup ) const
  { return (*pQColorGroup) != (*rColorGroup.pQColorGroup); }

  /** Load color values from a KConfig object. The group "ColorGroup"
    is used. If pString is not 0, the value of pString is prepended
    to the group name.
    */
  void load( KConfig& rConfig, QString* pString = 0L );

  /** Save color values to a KConfig object. The group "ColorGroup" is
    used. If pString is not 0, the value of pString is prepended to
    the group name.
    */
  void save( KConfig& rConfig, QString* pString = 0L );
  
  /** Return the QColorGroup object (for drawing shades e.g.).
   */
  const QColorGroup *colorGroup() const;
};

#endif
