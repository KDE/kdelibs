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
#include <klocale.h>
#include <kcharsets.h>

#define MAXFONTSIZES 15
const int defaultSmallFontSizes[MAXFONTSIZES] = { 7, 8, 10, 12, 14, 18, 24, 28, 34, 40, 48, 56, 68, 82, 100 };
const int defaultMediumFontSizes[MAXFONTSIZES] = { 8, 10, 12, 14, 18, 24, 28, 34, 40, 48, 56, 68, 82, 100, 120 };
const int defaultLargeFontSizes[MAXFONTSIZES] = { 10, 14, 18, 24, 28, 34, 40, 48, 56, 68, 82, 100, 120, 150 };


KHTMLSettings::KJavaScriptAdvice KHTMLSettings::strToAdvice(const QString& _str)
{
  KJavaScriptAdvice ret = KJavaScriptDunno;

  if (!_str)
        ret = KJavaScriptDunno;

  if (_str.lower() == QString::fromLatin1("accept"))
        ret = KJavaScriptAccept;
  else if (_str.lower() == QString::fromLatin1("reject"))
        ret = KJavaScriptReject;

  return ret;
}

const char* KHTMLSettings::adviceToStr(KJavaScriptAdvice _advice)
{
    switch( _advice ) {
    case KJavaScriptAccept: return I18N_NOOP("Accept");
    case KJavaScriptReject: return I18N_NOOP("Reject");
    default: return 0;
    }
        return 0;
}


void KHTMLSettings::splitDomainAdvice(const QString& configStr, QString &domain,
                                                                          KJavaScriptAdvice &javaAdvice, KJavaScriptAdvice& javaScriptAdvice)
{
    QString tmp(configStr);
    int splitIndex = tmp.find(':');
    if ( splitIndex == -1)
    {
        domain = configStr;
        javaAdvice = KJavaScriptDunno;
                javaScriptAdvice = KJavaScriptDunno;
    }
    else
    {
        domain = tmp.left(splitIndex);
                QString adviceString = tmp.mid( splitIndex+1, tmp.length() );
                int splitIndex2 = adviceString.find( ':' );
                if( splitIndex2 == -1 ) {
                  // Java advice only
                  javaAdvice = strToAdvice( adviceString );
                  javaScriptAdvice = KJavaScriptDunno;
                } else {
                  // Java and JavaScript advice
                  javaAdvice = strToAdvice( adviceString.left( splitIndex2 ) );
                  javaScriptAdvice = strToAdvice( adviceString.mid( splitIndex2+1,
                                                                                                                        adviceString.length() ) );
                }
    }
}


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
    m_fontSize = config->readNumEntry( "FontSize" );
        resetFontSizes();
  }

  if ( reset || config->hasKey( "MinimumFontSize" ) )
  {
    m_minFontSize = config->readNumEntry( "MinimumFontSize", HTML_DEFAULT_MIN_FONT_SIZE );
    if(m_minFontSize < 6)
        m_minFontSize = 6;
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

  if ( reset || config->hasKey( "SerifFont" ) )
  {
    m_strSerifFontName = config->readEntry( "SerifFont" );
    if ( m_strSerifFontName.isEmpty() )
      m_strSerifFontName = HTML_DEFAULT_VIEW_SERIF_FONT;
  }

  if ( reset || config->hasKey( "SansSerifFont" ) )
  {
    m_strSansSerifFontName = config->readEntry( "SansSerifFont" );
    if ( m_strSansSerifFontName.isEmpty() )
      m_strSansSerifFontName = HTML_DEFAULT_VIEW_SANSSERIF_FONT;
  }

  if ( reset || config->hasKey( "CursiveFont" ) )
  {
    m_strCursiveFontName = config->readEntry( "CursiveFont" );
    if ( m_strCursiveFontName.isEmpty() )
      m_strCursiveFontName = HTML_DEFAULT_VIEW_CURSIVE_FONT;
  }

  if ( reset || config->hasKey( "FantasyFont" ) )
  {
    m_strFantasyFontName = config->readEntry( "FantasyFont" );
    if ( m_strFantasyFontName.isEmpty() )
      m_strFantasyFontName = HTML_DEFAULT_VIEW_FANTASY_FONT;
  }

  if ( reset || config->hasKey( "DefaultCharset" ) )
    {
        m_charset = KGlobal::charsets()->nameToID(config->readEntry( "DefaultCharset", "iso-8859-1") );
    };

  if ( reset || config->hasKey( "EnforceDefaultCharset" ) )
    enforceCharset = config->readBoolEntry( "EnforceDefaultCharset", false );

  if ( reset || config->hasKey( "DefaultEncoding" ) )
    m_encoding = config->readEntry( "DefaultEncoding", "iso8859-1" );

  // Behaviour
  if ( reset || config->hasKey( "ChangeCursor" ) )
    m_bChangeCursor = config->readBoolEntry( "ChangeCursor", KDE_DEFAULT_CHANGECURSOR );

  if ( reset || config->hasKey( "UnderlineLinks" ) )
    m_underlineLink = config->readBoolEntry( "UnderlineLinks", true ); //huh, can't find default define
  //    m_underlineLink = config->readBoolEntry( "UnderlineLink", KDE_DEFAULT_UNDERLINELINKS );

  // Colors
  if ( reset || config->hasGroup( "General" ) )
  {
    config->setGroup( "General" ); // group will be restored by cgs anyway
    if ( reset || config->hasKey( "TextColor" ) )
      m_textColor = config->readColorEntry( "foreground", &HTML_DEFAULT_TXT_COLOR );

    if ( reset || config->hasKey( "linkColor" ) )
      m_linkColor = config->readColorEntry( "linkColor", &HTML_DEFAULT_LNK_COLOR );

    if ( reset || config->hasKey( "visitedLinkColor" ) )
      m_vLinkColor = config->readColorEntry( "visitedLinkColor", &HTML_DEFAULT_VLNK_COLOR);
  }

  // Other

  if ( reset || config->hasGroup( "HTML Settings" ) )
  {
    config->setGroup( "HTML Settings" ); // group will be restored by cgs anyway

    if ( reset || config->hasKey( "AutoLoadImages" ) )
      m_bAutoLoadImages = config->readBoolEntry( "AutoLoadImages", true );

    // The global setting for JavaScript
    if ( reset || config->hasKey( "EnableCSS" ) )
        m_bEnableCSS = config->readBoolEntry( "EnableCSS", true );

  }

  if( reset || config->hasGroup( "Java/JavaScript Settings" ) ) {
        config->setGroup( "Java/JavaScript Settings" );

        // The global setting for Java
    if ( reset || config->hasKey( "EnableJava" ) )
      m_bEnableJava = config->readBoolEntry( "EnableJava", false );

        // The global setting for JavaScript
    if ( reset || config->hasKey( "EnableJavaScript" ) )
      m_bEnableJavaScript = config->readBoolEntry( "EnableJavaScript", false );

        // The domain-specific settings.
        if( reset || config->hasKey( "JavaScriptDomainAdvice" ) ) {
          QStringList domainList = config->readListEntry( "JavaScriptDomainAdvice" );
          for (QStringList::ConstIterator it = domainList.begin();
                   it != domainList.end(); ++it) {
                QString domain;
                KJavaScriptAdvice javaAdvice;
                KJavaScriptAdvice javaScriptAdvice;
                splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
                javaDomainPolicy[domain] = javaAdvice;
                javaScriptDomainPolicy[domain] = javaScriptAdvice;
          }
        }
  }
}


bool KHTMLSettings::isJavaEnabled( const QString& hostname )
{
  // First check whether there is a Domain-specific entry.
  if( javaDomainPolicy.contains( hostname ) ) {
        // yes, use it (unless dunno)
        KJavaScriptAdvice adv = javaDomainPolicy[ hostname ];
        if( adv == KJavaScriptReject )
          return false;
        else if( adv == KJavaScriptAccept )
          return true;
  }

  // No domain-specific entry, or was dunno: use global setting
  return m_bEnableJava;
}


bool KHTMLSettings::isJavaScriptEnabled( const QString& hostname )
{
  // First check whether there is a Domain-specific entry.
  if( javaScriptDomainPolicy.contains( hostname ) ) {
        // yes, use it (unless dunno)
        KJavaScriptAdvice adv = javaScriptDomainPolicy[ hostname ];
        if( adv == KJavaScriptReject )
          return false;
        else if( adv == KJavaScriptAccept )
          return true;
  }

  // No domain-specific entry, or was dunno: use global setting
  return m_bEnableJavaScript;
}

bool KHTMLSettings::isCSSEnabled( const QString& /*hostname*/ )
{
#if 0
      // First check whether there is a Domain-specific entry.
  if( cssDomainPolicy.contains( hostname ) ) {
        // yes, use it (unless dunno)
        KJavaScriptAdvice adv = cssDomainPolicy[ hostname ];
        if( adv == KJavaScriptReject )
          return false;
        else if( adv == KJavaScriptAccept )
          return true;
  }

#endif
  // No domain-specific entry, or was dunno: use global setting
  return m_bEnableCSS;
}


void KHTMLSettings::resetFontSizes()
{
  m_fontSizes.clear();
  for ( int i = 0; i < MAXFONTSIZES; i++ )
        if( m_fontSize == 0 ) // small
          m_fontSizes << defaultSmallFontSizes[ i ];
        else if( m_fontSize == 2 ) // large
          m_fontSizes << defaultLargeFontSizes[ i ];
        else
          m_fontSizes << defaultMediumFontSizes[ i ];
}

void KHTMLSettings::setFontSizes(const QValueList<int> &_newFontSizes )
{
    QValueList<int> newFontSizes;
    newFontSizes = _newFontSizes;
    while ( newFontSizes.count() > m_fontSizes.count() )
      newFontSizes.remove( newFontSizes.fromLast() );

    QValueList<int>::ConstIterator it = newFontSizes.begin();
    QValueList<int>::ConstIterator end = newFontSizes.end();
    int i = 0;
    for (; it != end; it++ )
      m_fontSizes[ i++ ] = *it;
}


QString KHTMLSettings::settingsToCSS() const
{
    // lets start with the link properties
    QString str = "a[href] {\ncolor: ";    
    str += m_linkColor.name();
    str += ";";
    if(m_underlineLink)
	str += "\ntext-decoration: underline;";
    if( m_underlineLink )
	str += "\ncursor: pointer;";
    str += "\n}\n";
    return str;
}
