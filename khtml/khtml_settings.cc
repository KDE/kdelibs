/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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

#include "khtml_settings.h"
#include "khtmldefaults.h"
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kapp.h>
#include <kglobal.h>

KHTMLSettings::KHTMLSettings()
{
  init(); 
} 

void KHTMLSettings::init()
{
  KConfig global( "khtmlrc", true );
  global.setGroup( "HTML Settings" );
  init( &global );
  
  KConfig *local = KGlobal::config();
  if ( !local )
    return;
  
  if ( !local->hasGroup( "HTML Settings" ) )
    return;
  
  local->setGroup( "HTML Settings" );
  init( local, false );
}

void KHTMLSettings::init( KConfig * config, bool reset )
{
  // Fonts and colors

  if ( reset || config->hasKey( "FontSize" ) )
  {
    m_iFontSize = config->readNumEntry( "FontSize", HTML_DEFAULT_VIEW_FONT_SIZE );
    if ( m_iFontSize < 8 )
      m_iFontSize = 8;
    else if ( m_iFontSize > 24 )
      m_iFontSize = 24;
  }
   
  if ( reset || config->hasKey( "StandardFont" ) )
  {
    m_strStdFontName = config->readEntry( "StandardFont" );
    if ( m_strStdFontName.isEmpty() )
      m_strStdFontName = HTML_DEFAULT_VIEW_FONT;
  }
   
  if ( reset || config->hasKey( "FixedFont" ) )
  {
    m_strFixedFontName = config->readEntry( "FixedFont" );
    if ( m_strFixedFontName.isEmpty() )
      m_strFixedFontName = HTML_DEFAULT_VIEW_FIXED_FONT;
  }

  if ( reset || config->hasKey( "BgColor" ) )
    m_bgColor = config->readColorEntry( "BgColor", &HTML_DEFAULT_BG_COLOR );
  
  if ( reset || config->hasKey( "TextColor" ) )
    m_textColor = config->readColorEntry( "TextColor", &HTML_DEFAULT_TXT_COLOR );
  
  if ( reset || config->hasKey( "LinkColor" ) )
    m_linkColor = config->readColorEntry( "LinkColor", &HTML_DEFAULT_LNK_COLOR );
  
  if ( reset || config->hasKey( "VLinkColor" ) )
    m_vLinkColor = config->readColorEntry( "VLinkColor", &HTML_DEFAULT_VLNK_COLOR);

  // Behaviour
  if ( reset || config->hasKey( "ChangeCursor" ) )
    m_bChangeCursor = config->readBoolEntry( "ChangeCursor", KDE_DEFAULT_CHANGECURSOR );
  
  if ( reset || config->hasKey( "UnderlineLink" ) )
    m_underlineLink = config->readBoolEntry( "UnderlineLink", true ); //huh, can't find default define
  //    m_underlineLink = config->readBoolEntry( "UnderlineLink", KDE_DEFAULT_UNDERLINELINKS );

  // Other
  
  if ( reset || config->hasGroup( "HTML Settings" ) )
  {
    config->setGroup( "HTML Settings" ); // group will be restored by cgs anyway
  
    if ( reset || config->hasKey( "AutoLoadImages" ) )
      m_bAutoLoadImages = config->readBoolEntry( "AutoLoadImages", true );
    
    if ( reset || config->hasKey( "EnableJava" ) )
      m_bEnableJava = config->readBoolEntry( "EnableJava", false );
    
    if ( reset || config->hasKey( "EnableJavaScript" ) )
      m_bEnableJavaScript = config->readBoolEntry( "EnableJavaScript", false );
    
  }

}

//static
/*
KonqHTMLSettings * KonqHTMLSettings::defaultHTMLSettings()
{
  if (!s_HTMLSettings)
    s_HTMLSettings = new KonqHTMLSettings();
  return s_HTMLSettings;
}
*/
//static
/*
void KonqHTMLSettings::reparseConfiguration()
{
  if ( s_HTMLSettings )
  {
    KConfig config ( "konquerorrc", true );
    config.setGroup( "HTML Settings" );
    s_HTMLSettings->init( &config );
  }
}
*/
