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
