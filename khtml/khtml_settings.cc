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
#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kdebug.h>

#define MAXFONTSIZES 15

typedef QMap<QString,KHTMLSettings::KJavaScriptAdvice> PolicyMap;

class KHTMLSettingsPrivate
{
public:
    bool m_bChangeCursor;
    bool m_underlineLink;
    bool m_hoverLink;

    int m_fontSize;
    QValueList<int>     m_fontSizes;
    int m_minFontSize;

#if QT_VERSION < 300
    QFont::CharSet m_charset;
    QFont::CharSet m_script;
    QFont::CharSet m_defaultCharset;
#else
    int m_charset;
#endif
    bool enforceCharset;
    QString m_encoding;
    QString m_userSheet;
    
    QColor m_textColor;
    QColor m_linkColor;
    QColor m_vLinkColor;

    bool m_bAutoLoadImages;
    bool m_bEnableJava;
    bool m_bEnableJavaScript;
    bool m_bEnableJavaScriptDebug;
    bool m_bEnablePlugins;
    bool m_bEnableCSS;
    QMap<QString,KHTMLSettings::KJavaScriptAdvice> javaDomainPolicy;
    QMap<QString,KHTMLSettings::KJavaScriptAdvice> javaScriptDomainPolicy;
#if QT_VERSION < 300
    QMap<QFont::CharSet, QStringList> fontsForCharset;
#else
    QStringList fonts;
#endif
    QStringList defaultFonts;
    QString availFamilies;

    bool m_formCompletionEnabled;
    int m_maxFormCompletionItems;
};


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
  d = new KHTMLSettingsPrivate();
  init();
}

KHTMLSettings::KHTMLSettings(const KHTMLSettings &other)
{
  d = new KHTMLSettingsPrivate();
  *d = *other.d;
}

KHTMLSettings::~KHTMLSettings()
{
  delete d;
}

bool KHTMLSettings::changeCursor()
{
  return d->m_bChangeCursor;
}

bool KHTMLSettings::underlineLink()
{
  return d->m_underlineLink;
}

bool KHTMLSettings::hoverLink()
{
  return d->m_hoverLink;
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
    if( reset ) {
	d->defaultFonts = QStringList();
	d->defaultFonts.append( config->readEntry( "StandardFont", KGlobalSettings::generalFont().family() ) );
	d->defaultFonts.append( config->readEntry( "FixedFont", KGlobalSettings::fixedFont().family() ) );
	d->defaultFonts.append( config->readEntry( "SerifFont", HTML_DEFAULT_VIEW_SERIF_FONT ) );
	d->defaultFonts.append( config->readEntry( "SansSerifFont", HTML_DEFAULT_VIEW_SANSSERIF_FONT ) );
	d->defaultFonts.append( config->readEntry( "CursiveFont", HTML_DEFAULT_VIEW_CURSIVE_FONT ) );
	d->defaultFonts.append( config->readEntry( "FantasyFont", HTML_DEFAULT_VIEW_FANTASY_FONT ) );
	d->defaultFonts.append( QString( "0" ) ); // font size adjustment
    }

    if ( reset || config->hasKey( "MinimumFontSize" ) )
        d->m_minFontSize = config->readNumEntry( "MinimumFontSize", HTML_DEFAULT_MIN_FONT_SIZE );

    if ( reset || config->hasKey( "MediumFontSize" ) ) {
        d->m_fontSize = config->readNumEntry( "MediumFontSize", 10 );
        resetFontSizes();
    }

#if QT_VERSION < 300
    QStringList chSets = KGlobal::charsets()->availableCharsetNames();
    for ( QStringList::Iterator it = chSets.begin(); it != chSets.end(); ++it ) {
        if ( reset || config->hasKey( *it ) ){
            QStringList fonts = config->readListEntry( *it );
            if( fonts.count() == 6 ) // backwards compatibility
                fonts.append( QString( "0" ) );
            if ( fonts.count() != 7 )
              fonts = d->defaultFonts;
            d->fontsForCharset[KGlobal::charsets()->xNameToID(*it)] = fonts;
        }
    }
#else
    d->fonts = config->readListEntry( "Fonts" );
#endif

    if ( reset || config->hasKey( "DefaultEncoding" ) ) {
        d->m_encoding = config->readEntry( "DefaultEncoding", "" );
        if ( d->m_encoding.isEmpty() )
            d->m_encoding = KGlobal::locale()->charset();

#if QT_VERSION < 300
        d->m_defaultCharset = KGlobal::charsets()->nameToID( d->m_encoding );
        internalSetCharset( d->m_defaultCharset );
#endif
    }

    if ( reset || config->hasKey( "EnforceDefaultCharset" ) )
        d->enforceCharset = config->readBoolEntry( "EnforceDefaultCharset", false );

  // Behaviour
  if ( reset || config->hasKey( "ChangeCursor" ) )
      d->m_bChangeCursor = config->readBoolEntry( "ChangeCursor", KDE_DEFAULT_CHANGECURSOR );

  if ( reset || config->hasKey("UnderlineLinks") )
      d->m_underlineLink = config->readBoolEntry( "UnderlineLinks", true );

  if ( reset || config->hasKey( "HoverLinks" ) )
  {
    if ( ( d->m_hoverLink = config->readBoolEntry( "HoverLinks", false ) ) )
        d->m_underlineLink = false;
  }

  // Colors
  if ( reset || config->hasGroup( "General" ) )
  {
    config->setGroup( "General" ); // group will be restored by cgs anyway
    if ( reset || config->hasKey( "TextColor" ) )
      d->m_textColor = config->readColorEntry( "foreground", &HTML_DEFAULT_TXT_COLOR );

    if ( reset || config->hasKey( "linkColor" ) )
      d->m_linkColor = config->readColorEntry( "linkColor", &HTML_DEFAULT_LNK_COLOR );

    if ( reset || config->hasKey( "visitedLinkColor" ) )
      d->m_vLinkColor = config->readColorEntry( "visitedLinkColor", &HTML_DEFAULT_VLNK_COLOR);
  }

  // Other

  if ( reset || config->hasGroup( "HTML Settings" ) )
  {
    config->setGroup( "HTML Settings" ); // group will be restored by cgs anyway

    if ( reset || config->hasKey( "AutoLoadImages" ) )
      d->m_bAutoLoadImages = config->readBoolEntry( "AutoLoadImages", true );

    // The global setting for CSS
    if ( reset || config->hasKey( "EnableCSS" ) )
        d->m_bEnableCSS = config->readBoolEntry( "EnableCSS", true );

    if ( config->readBoolEntry( "UserStyleSheetEnabled", false ) == true ) {
	if ( reset || config->hasKey( "UserStyleSheet" ) )
	    d->m_userSheet = config->readEntry( "UserStyleSheet", "" );
    }

    d->m_formCompletionEnabled = config->readBoolEntry("FormCompletion", true);
    d->m_maxFormCompletionItems = config->readNumEntry("MaxFormCompletionItems", 10);
  }

  if( reset || config->hasGroup( "Java/JavaScript Settings" ) ) {
        config->setGroup( "Java/JavaScript Settings" );

        // The global setting for Java
    if ( reset || config->hasKey( "EnableJava" ) )
      d->m_bEnableJava = config->readBoolEntry( "EnableJava", false );

        // The global setting for JavaScript
    if ( reset || config->hasKey( "EnableJavaScript" ) )
      d->m_bEnableJavaScript = config->readBoolEntry( "EnableJavaScript", true );

        // The global setting for JavaScript debugging
    if ( reset || config->hasKey( "EnableJavaScriptDebug" ) )
      d->m_bEnableJavaScriptDebug = config->readBoolEntry( "EnableJavaScriptDebug", false );

        // The global setting for Plugins (there's no local setting yet)
    if ( reset || config->hasKey( "EnablePlugins" ) )
      d->m_bEnablePlugins = config->readBoolEntry( "EnablePlugins", true );

    // The domain-specific settings.
    bool check_old_java = true;
	if( reset || config->hasKey( "JavaDomainSettings" ) ){
	  check_old_java = false;
	  QStringList domainList = config->readListEntry( "JavaDomainSettings" );
      for ( QStringList::ConstIterator it = domainList.begin();
                it != domainList.end(); ++it) {
        QString domain;
        KJavaScriptAdvice javaAdvice;
        KJavaScriptAdvice javaScriptAdvice;
        splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
        d->javaDomainPolicy[domain] = javaAdvice;
      }
	}

	bool check_old_ecma = true;
	if( reset || config->hasKey( "ECMADomainSettings" ) ){
 	  check_old_ecma = false;
	  QStringList domainList = config->readListEntry( "ECMADomainSettings" );
      for ( QStringList::ConstIterator it = domainList.begin();
                it != domainList.end(); ++it) {
        QString domain;
        KJavaScriptAdvice javaAdvice;
        KJavaScriptAdvice javaScriptAdvice;
        splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
        d->javaScriptDomainPolicy[domain] = javaScriptAdvice;
      }
	}

    if( reset || config->hasKey( "JavaScriptDomainAdvice" )
	     && ( check_old_java || check_old_ecma ) ) {
      QStringList domainList = config->readListEntry( "JavaScriptDomainAdvice" );
      for ( QStringList::ConstIterator it = domainList.begin();
                it != domainList.end(); ++it) {
        QString domain;
        KJavaScriptAdvice javaAdvice;
        KJavaScriptAdvice javaScriptAdvice;
        splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
		if( check_old_java )
		  d->javaDomainPolicy[domain] = javaAdvice;
		if( check_old_ecma )
          d->javaScriptDomainPolicy[domain] = javaScriptAdvice;
      }

	  //save all the settings into the new keywords if they don't exist
	  if( check_old_java ){
	    QStringList domainConfig;
	  	PolicyMap::Iterator it;
	  	for( it = d->javaDomainPolicy.begin(); it != d->javaDomainPolicy.end(); ++it ){
          QCString javaPolicy = adviceToStr( it.data() );
          QCString javaScriptPolicy = adviceToStr( KJavaScriptDunno );
          domainConfig.append(QString::fromLatin1("%1:%2:%3").arg(it.key()).arg(javaPolicy).arg(javaScriptPolicy));
	  	}
        config->writeEntry( "JavaDomainSettings", domainConfig );
	  }

	  if( check_old_ecma ){
	    QStringList domainConfig;
	    PolicyMap::Iterator it;
	  	for( it = d->javaScriptDomainPolicy.begin(); it != d->javaScriptDomainPolicy.end(); ++it ){
          QCString javaPolicy = adviceToStr( KJavaScriptDunno );
          QCString javaScriptPolicy = adviceToStr( it.data() );
          domainConfig.append(QString::fromLatin1("%1:%2:%3").arg(it.key()).arg(javaPolicy).arg(javaScriptPolicy));
	  	}
        config->writeEntry( "ECMADomainSettings", domainConfig );
	  }
    }
  }
}


// Local helper for isJavaEnabled & isJavaScriptEnabled.
static bool lookup_hostname_policy(const QString& hostname,
                                   const PolicyMap& policy,
                                   const bool default_retval)
{
  if (hostname.isEmpty()) {
    return default_retval;
  }

  // First check whether there is a perfect match.
  if( policy.contains( hostname ) ) {
    // yes, use it (unless dunno)
    KHTMLSettings::KJavaScriptAdvice adv = policy[ hostname ];
    if( adv == KHTMLSettings::KJavaScriptReject ) {
      return false;
    } else if( adv == KHTMLSettings::KJavaScriptAccept ) {
      return true;
    }
  }

  // Now, check for partial match.  Chop host from the left until
  // there's no dots left.
  QString host_part = hostname;
  int dot_idx = -1;
  while( (dot_idx = host_part.find(QChar('.'))) >= 0 ) {
    host_part.remove(0,dot_idx);
    if( policy.contains( host_part ) ) {
      KHTMLSettings::KJavaScriptAdvice adv = policy[ host_part ];
      if( adv == KHTMLSettings::KJavaScriptReject ) {
        return false;
      } else if( adv == KHTMLSettings::KJavaScriptAccept ) {
        return true;
      }
    }
    // assert(host_part[0] == QChar('.'));
    host_part.remove(0,1); // Chop off the dot.
  }

  // No domain-specific entry, or was dunno: use global setting
  return default_retval;
}

bool KHTMLSettings::isJavaEnabled( const QString& hostname )
{
  return lookup_hostname_policy(hostname, d->javaDomainPolicy, d->m_bEnableJava);
}

bool KHTMLSettings::isJavaScriptEnabled( const QString& hostname )
{
  return lookup_hostname_policy(hostname, d->javaScriptDomainPolicy, d->m_bEnableJavaScript);
}

bool KHTMLSettings::isJavaScriptDebugEnabled( const QString& /*hostname*/ )
{
  // debug setting is global for now, but could change in the future
  return d->m_bEnableJavaScriptDebug;
}

bool KHTMLSettings::isPluginsEnabled( const QString& hostname )
{
  // FIXME: hostname is ignored (dnaber, 2001-01-03)
  return d->m_bEnablePlugins;
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
  return d->m_bEnableCSS;
}


void KHTMLSettings::resetFontSizes()
{
    d->m_fontSizes.clear();
    int sizeAdjust = d->m_fontSize + lookupFont(d->m_charset,6).toInt() + 3;
    if ( sizeAdjust < 0 )
	sizeAdjust = 0;
    if ( sizeAdjust > 9 )
	sizeAdjust = 9;
    //kdDebug(6050) << "KHTMLSettings::resetFontSizes adjustment is " << sizeAdjust << endl;
    const float factor = 1.2;
    float scale = 1.0 / ( factor*factor*factor );
    for ( int i = 0; i < MAXFONTSIZES; i++ ) {
      	d->m_fontSizes << ( KMAX( int( d->m_fontSize * scale + 0.5), d->m_minFontSize ) );
        scale *= factor;
    }
}

int KHTMLSettings::minFontSize() const
{
  return d->m_minFontSize;
}

void KHTMLSettings::setFontSizes(const QValueList<int> &_newFontSizes )
{
    QValueList<int> newFontSizes;
    newFontSizes = _newFontSizes;
    while ( newFontSizes.count() > d->m_fontSizes.count() )
      newFontSizes.remove( newFontSizes.fromLast() );

    QValueList<int>::ConstIterator it = newFontSizes.begin();
    QValueList<int>::ConstIterator end = newFontSizes.end();
    for (int i = 0; it != end; it++ )
      d->m_fontSizes[ i++ ] = *it;
}


QString KHTMLSettings::settingsToCSS() const
{
    // lets start with the link properties
    QString str = "a:link {\ncolor: ";
    str += d->m_linkColor.name();
    str += ";";
    if(d->m_underlineLink)
	str += "\ntext-decoration: underline;";

    if( d->m_bChangeCursor )
    {
	str += "\ncursor: pointer;";
        str += "\n}\ninput[type=image] { cursor: pointer;";
    }
    str += "\n}\n";
    str += "a:visited {\ncolor: ";
    str += d->m_vLinkColor.name();
    str += ";";
    if(d->m_underlineLink)
	str += "\ntext-decoration: underline;";

    if( d->m_bChangeCursor )
	str += "\ncursor: pointer;";
    str += "\n}\n";

    if(d->m_hoverLink)
        str += "a:link:hover, a:visited:hover { text-decoration: underline; }\n";

    return str;
}

QString KHTMLSettings::availableFamilies() const
{
  return d->availFamilies;
}

#if QT_VERSION < 300
QString KHTMLSettings::lookupFont(const QFont::CharSet &charset, int i) const
#else
QString KHTMLSettings::lookupFont(int, int i) const
#endif
{
    QString font;
#if QT_VERSION < 300
    const QStringList &fontList = d->fontsForCharset[charset];
    if (fontList.count() > (uint) i)
       font = fontList[i];
#endif
    if (font.isEmpty())
        font = d->defaultFonts[i];
    return font;
}

QString KHTMLSettings::stdFontName() const
{
    return lookupFont(d->m_charset,0);
}

QString KHTMLSettings::fixedFontName() const
{
    return lookupFont(d->m_charset,1);
}

QString KHTMLSettings::serifFontName() const
{
    return lookupFont(d->m_charset,2);
}

QString KHTMLSettings::sansSerifFontName() const
{
    return lookupFont(d->m_charset,3);
}

QString KHTMLSettings::cursiveFontName() const
{
    return lookupFont(d->m_charset,4);
}

QString KHTMLSettings::fantasyFontName() const
{
    return lookupFont(d->m_charset,5);
}

#if QT_VERSION < 300
void KHTMLSettings::setFont(const QFont::CharSet &charset, int i, const QString &n)
{
    QStringList fontList = d->fontsForCharset[charset];
    while (fontList.count() <= (uint)i)
      fontList.append(QString::null);
    fontList[i] = n;
}
#else
void KHTMLSettings::setFont(int, int, const QString &)
{
}
#endif

void KHTMLSettings::setStdFontName(const QString &n)
{
    setFont(d->m_charset, 0, n);
}

void KHTMLSettings::setFixedFontName(const QString &n)
{
    setFont(d->m_charset, 1, n);
}

const QValueList<int> &KHTMLSettings::fontSizes() const
{
  return d->m_fontSizes;
}

#if QT_VERSION < 300
void KHTMLSettings::setDefaultCharset( QFont::CharSet c, bool enforce )
{
    d->m_defaultCharset = c;
    d->enforceCharset = enforce;
    if(enforce)
        internalSetCharset(c);
}

void KHTMLSettings::resetCharset()
{
    internalSetCharset(d->m_defaultCharset);
    setScript( d->m_defaultCharset );
}

QFont::CharSet KHTMLSettings::charset() const
{
  return d->m_charset;
}

void KHTMLSettings::setCharset( QFont::CharSet c)
{
    if (!d->enforceCharset)
       internalSetCharset(c);
}

void KHTMLSettings::internalSetCharset( QFont::CharSet c )
{
    d->m_charset = c;
    d->availFamilies = KGlobal::charsets()->availableFamilies( d->m_charset ).join(",");
    resetFontSizes();
}

QFont::CharSet KHTMLSettings::script() const
{
  return d->m_script;
}

void KHTMLSettings::setScript( QFont::CharSet c )
{
    //kdDebug(6050) << "KHTMLSettings::setScript to " << c << endl;
    d->m_script = c;
}
#endif

QString KHTMLSettings::userStyleSheet() const
{
    return d->m_userSheet;
}

bool KHTMLSettings::isFormCompletionEnabled() const
{
  return d->m_formCompletionEnabled;
}

int KHTMLSettings::maxFormCompletionItems() const
{
  return d->m_maxFormCompletionItems;
}

const QString &KHTMLSettings::encoding() const
{
  return d->m_encoding;
}

const QColor& KHTMLSettings::textColor()
{
  return d->m_textColor;
}

const QColor& KHTMLSettings::linkColor()
{
  return d->m_linkColor;
}

const QColor& KHTMLSettings::vLinkColor()
{
  return d->m_vLinkColor;
}

bool KHTMLSettings::autoLoadImages()
{
  return d->m_bAutoLoadImages;
}
