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
// Revision 1.8  1999/04/19 15:49:35  kulow
// cleaning up yet some more header files (fixheaders is your friend).
// Adding copy constructor to KPixmap to avoid casting while assingment.
//
// The rest of the fixes in kdelibs and kdebase I will commit silently
//
// Revision 1.7  1998/06/16 06:03:19  kalle
// Implemented copy constructors and assignment operators or disabled them
//
// Revision 1.6  1998/01/18 14:38:34  kulow
// reverted the changes, Jacek commited.
// Only the RCS comments were affected, but to keep them consistent, I
// thought, it's better to revert them.
// I checked twice, that only comments are affected ;)
//
// Revision 1.4  1997/10/16 11:14:28  torben
// Kalle: Copyright headers
// kdoctoolbar removed
//
// Revision 1.3  1997/10/09 11:46:19  kalle
// Assorted patches by Fritz Elfert, Rainer Bawidamann, Bernhard Kuhn and Lars Kneschke
//
// Revision 1.2  1997/10/04 19:42:45  kalle
// new KConfig
//
// Revision 1.1.1.1  1997/04/13 14:42:41  cvsuser
// Source imported
//
// Revision 1.1.1.1  1997/04/09 00:28:05  cvsuser
// Sources imported
//
// Revision 1.2  1996/12/14 12:49:11  kalle
// method names start with a small letter
//
// Revision 1.1  1996/11/24 12:54:16  kalle
// Initial revision
//
//
// KColorGroup implementation
//
// (C) 1996 Matthias Kalle Dalheimer <mda@stardivision.de>

#include "kcolorgroup.h"
#include "kconfig.h"

KColorGroup::KColorGroup()
{
  pQColorGroup = new QColorGroup();
}

KColorGroup::KColorGroup( const QColor& rForeground, const QColor& rBackground,
			  const QColor& rLight, const QColor& rMid, 
			  const QColor& rDark, const QColor& rText,
			  const QColor& rBase )
{
  pQColorGroup = new QColorGroup( rForeground, rBackground, rLight, rMid,
				  rDark, rText, rBase );
}

KColorGroup::~KColorGroup()
{
  delete pQColorGroup;
}


KColorGroup::KColorGroup( const KColorGroup& group )
{
  pQColorGroup = new QColorGroup( *group.pQColorGroup );
}


KColorGroup& KColorGroup::operator= ( const KColorGroup& group )
{
  if( this != &group ) {
	delete pQColorGroup;
	pQColorGroup = new QColorGroup( *group.pQColorGroup );
  }

  return *this;
}
	

void KColorGroup::load( KConfig& rConfig, QString* pString )
{
  QString aOldGroup = rConfig.group();

  QString aGroupName;
  if (pString) 
	aGroupName = *pString;
  aGroupName += "ColorGroup";
  rConfig.setGroup( aGroupName );

  QColor aForeColor( rConfig.readColorEntry( "Foreground" ) );
  QColor aBackColor( rConfig.readColorEntry( "Background" ) );
  QColor aLightColor( rConfig.readColorEntry( "Light" ) );
  QColor aMidColor( rConfig.readColorEntry( "Mid" ) );
  QColor aDarkColor( rConfig.readColorEntry( "Dark" ) );
  QColor aTextColor( rConfig.readColorEntry( "Text" ) );
  QColor aBaseColor( rConfig.readColorEntry( "Base" ) );

  pQColorGroup = new QColorGroup( aForeColor, aBackColor, aLightColor, 
								  aMidColor, aDarkColor, aTextColor, 
								  aBaseColor );

  rConfig.setGroup( aOldGroup );
}

void KColorGroup::save( KConfig& rConfig, QString* pString )
{
  QString aOldGroup = rConfig.group();

  QString aGroupName;
  if (pString) 
	aGroupName = *pString;
  aGroupName += "ColorGroup";
  rConfig.setGroup( aGroupName );
  

  rConfig.writeEntry( "Foreground", pQColorGroup->foreground() );
  rConfig.writeEntry( "Background", pQColorGroup->background() );
  rConfig.writeEntry( "Light", pQColorGroup->light() );
  rConfig.writeEntry( "Mid", pQColorGroup->mid() );
  rConfig.writeEntry( "Dark", pQColorGroup->dark() );
  rConfig.writeEntry( "Text", pQColorGroup->text() );
  rConfig.writeEntry( "Base", pQColorGroup->base() );

  rConfig.setGroup( aOldGroup );
}

const QColorGroup* KColorGroup::colorGroup() const
{
  return pQColorGroup;
}
