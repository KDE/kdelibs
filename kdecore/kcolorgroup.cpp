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
// Revision 1.5  1998/01/18 13:38:29  jacek
// Changes KCharsets*:
// Added KCharset class - small changes in interface. I hope it is all source
//                        and binary compatible. Some applications may now ask to recompile them,
// 		       but no changes should be neccessary.
// Added Unicode maping for adobe-symbol fonts, but they are not well displayable yet.
// And some other changes.
//
// Revision 1.1.1.4  1997/12/13 12:57:11  jacek
// Imported sources from KDE CVS
//
// Revision 1.1.1.3  1997/12/11 07:19:11  jacek
// Imported sources from KDE CVS
//
// Revision 1.1.1.2  1997/12/10 07:08:29  jacek
// Imported sources from KDE CVS
//
// Revision 1.1.1.1  1997/12/09 22:02:45  jacek
// Imported sorces fromkde
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

#ifndef _KCOLORGROUP_H
#include "kcolorgroup.h"
#endif

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

void KColorGroup::load( KConfig& rConfig, QString* pString )
{
  QString aOldGroup = rConfig.getGroup();

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
  QString aOldGroup = rConfig.getGroup();

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
