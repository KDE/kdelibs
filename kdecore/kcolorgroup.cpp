// $Id$
// Revision 1.1.1.3  1997/12/11 07:19:11  jacek
// $Log$
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
#include <kcolorgroup.h>
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
  QString aGroupName = *pString;
KColorGroup::~KColorGroup()
{
  delete pQColorGroup;
  QColor aForeColor( rConfig.readEntry( "Foreground-Red" ).toInt(),
		     rConfig.readEntry( "Foreground-Green" ).toInt(),
		     rConfig.readEntry( "Foreground-Blue" ).toInt() );
  
  QColor aBackColor( rConfig.readEntry( "Background-Red" ).toInt(),
		     rConfig.readEntry( "Background-Green" ).toInt(),
		     rConfig.readEntry( "Background-Blue" ).toInt() );
  
  QColor aLightColor( rConfig.readEntry( "Light-Red" ).toInt(),
		      rConfig.readEntry( "Light-Green" ).toInt(),
		      rConfig.readEntry( "Light-Blue" ).toInt() );
  
  QColor aMidColor( rConfig.readEntry( "Mid-Red" ).toInt(),
		    rConfig.readEntry( "Mid-Green" ).toInt(),
		    rConfig.readEntry( "Mid-Blue" ).toInt() );
  
  QColor aDarkColor( rConfig.readEntry( "Dark-Red" ).toInt(),
		     rConfig.readEntry( "Dark-Green" ).toInt(),
		     rConfig.readEntry( "Dark-Blue" ).toInt() );
  
  QColor aTextColor( rConfig.readEntry( "Text-Red" ).toInt(),
		     rConfig.readEntry( "Text-Green" ).toInt(),
		     rConfig.readEntry( "Text-Blue" ).toInt() );
  
  QColor aBaseColor( rConfig.readEntry( "Base-Red" ).toInt(),
		     rConfig.readEntry( "Base-Green" ).toInt(),
		     rConfig.readEntry( "Base-Blue" ).toInt() );
  
  delete pQColorGroup;
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
  QString aGroupName = *pString;
								  aBaseColor );

  rConfig.setGroup( aOldGroup );
  QString aRedName;
  QString aGreenName;
  QString aBlueName;

  aRedName.setNum( pQColorGroup->foreground().red() );
  aGreenName.setNum( pQColorGroup->foreground().green() );
  aBlueName.setNum( pQColorGroup->foreground().blue() );
  rConfig.writeEntry( "Foreground-Red", aRedName );  
  rConfig.writeEntry( "Foreground-Green", aGreenName );  
  rConfig.writeEntry( "Foreground-Blue", aBlueName );

  aRedName.setNum( pQColorGroup->background().red() );
  aGreenName.setNum( pQColorGroup->background().green() );
  aBlueName.setNum( pQColorGroup->background().blue() );
  rConfig.writeEntry( "Background-Red", aRedName );  
  rConfig.writeEntry( "Background-Green", aGreenName );  
  rConfig.writeEntry( "Background-Blue", aBlueName );

  aRedName.setNum( pQColorGroup->light().red() );
  aGreenName.setNum( pQColorGroup->light().green() );
  aBlueName.setNum( pQColorGroup->light().blue() );
  rConfig.writeEntry( "Light-Red", aRedName );  
  rConfig.writeEntry( "Light-Green", aGreenName );  
  rConfig.writeEntry( "Light-Blue", aBlueName );

  aRedName.setNum( pQColorGroup->mid().red() );
  aGreenName.setNum( pQColorGroup->mid().green() );
  aBlueName.setNum( pQColorGroup->mid().blue() );
  rConfig.writeEntry( "Mid-Red", aRedName );  
  rConfig.writeEntry( "Mid-Green", aGreenName );  
  rConfig.writeEntry( "Mid-Blue", aBlueName );

  aRedName.setNum( pQColorGroup->dark().red() );
  aGreenName.setNum( pQColorGroup->dark().green() );
  aBlueName.setNum( pQColorGroup->dark().blue() );
  rConfig.writeEntry( "Dark-Red", aRedName );  
  rConfig.writeEntry( "Dark-Green", aGreenName );  
  rConfig.writeEntry( "Dark-Blue", aBlueName );

  aRedName.setNum( pQColorGroup->text().red() );
  aGreenName.setNum( pQColorGroup->text().green() );
  aBlueName.setNum( pQColorGroup->text().blue() );
  rConfig.writeEntry( "Text-Red", aRedName );  
  rConfig.writeEntry( "Text-Green", aGreenName );  
  rConfig.writeEntry( "Text-Blue", aBlueName );

  aRedName.setNum( pQColorGroup->base().red() );
  aGreenName.setNum( pQColorGroup->base().green() );
  aBlueName.setNum( pQColorGroup->base().blue() );
  rConfig.writeEntry( "Base-Red", aRedName );  
  rConfig.writeEntry( "Base-Green", aGreenName );  
  rConfig.writeEntry( "Base-Blue", aBlueName );
	aGroupName = *pString;
  aGroupName += "ColorGroup";
  rConfig.setGroup( aGroupName );
  
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
