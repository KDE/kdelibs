// $Id$
// Revision 1.1.1.3  1997/12/11 07:19:11  jacek
// $Log$
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
