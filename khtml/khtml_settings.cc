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

#include <qfontdatabase.h>
#include <qregexp.h>

#include "khtml_settings.h"
#include "khtmldefaults.h"
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <qregexp.h>


typedef QMap<QString,KHTMLSettings::KJavaScriptAdvice> PolicyMap;

class KHTMLSettingsPrivate
{
public:
    bool m_bChangeCursor : 1;
    bool m_underlineLink : 1;
    bool m_hoverLink : 1;
    bool enforceCharset : 1;
    bool m_bAutoLoadImages : 1;
    bool m_bEnableJava : 1;
    bool m_bEnableJavaScript : 1;
    bool m_bEnableJavaScriptDebug : 1;
    bool m_bEnablePlugins : 1;
    bool m_formCompletionEnabled : 1;

    int m_fontSize;
    int m_minFontSize;
    int m_maxFormCompletionItems;
    KHTMLSettings::KAnimationAdvice m_showAnimations;

    QString m_encoding;
    QString m_userSheet;

    QColor m_textColor;
    QColor m_linkColor;
    QColor m_vLinkColor;

    QMap<QString,KHTMLSettings::KJavaScriptAdvice> javaDomainPolicy;
    QMap<QString,KHTMLSettings::KJavaScriptAdvice> javaScriptDomainPolicy;
    QStringList fonts;
    QStringList defaultFonts;
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
        domain = configStr.lower();
        javaAdvice = KJavaScriptDunno;
        javaScriptAdvice = KJavaScriptDunno;
    }
    else
    {
        domain = tmp.left(splitIndex).lower();
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

bool KHTMLSettings::changeCursor() const
{
  return d->m_bChangeCursor;
}

bool KHTMLSettings::underlineLink() const
{
  return d->m_underlineLink;
}

bool KHTMLSettings::hoverLink() const
{
  return d->m_hoverLink;
}

void KHTMLSettings::init()
{
  KConfig global( "khtmlrc", true, false );
  init( &global, true );

  KConfig *local = KGlobal::config();
  if ( !local )
    return;

  init( local, false );
}

void KHTMLSettings::init( KConfig * config, bool reset )
{
  QString group_save = config->group();
  if (reset || config->hasGroup("HTML Settings"))
  {
    config->setGroup( "HTML Settings" );
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

    if ( reset || config->hasKey( "MediumFontSize" ) )
        d->m_fontSize = config->readNumEntry( "MediumFontSize", 10 );

    d->fonts = config->readListEntry( "Fonts" );

    if ( reset || config->hasKey( "DefaultEncoding" ) ) {
        d->m_encoding = config->readEntry( "DefaultEncoding", "" );
        if ( d->m_encoding.isEmpty() )
            d->m_encoding = KGlobal::locale()->encoding();
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

    // Other
    if ( reset || config->hasKey( "AutoLoadImages" ) )
      d->m_bAutoLoadImages = config->readBoolEntry( "AutoLoadImages", true );

    if ( reset || config->hasKey( "ShowAnimations" ) )
    {
      QString value = config->readEntry( "ShowAnimations").lower();
      if (value == "disabled")
         d->m_showAnimations = KAnimationDisabled;
      else if (value == "looponce")
         d->m_showAnimations = KAnimationLoopOnce;
      else
         d->m_showAnimations = KAnimationEnabled;
    }

    if ( config->readBoolEntry( "UserStyleSheetEnabled", false ) == true ) {
        if ( reset || config->hasKey( "UserStyleSheet" ) )
            d->m_userSheet = config->readEntry( "UserStyleSheet", "" );
    }

    d->m_formCompletionEnabled = config->readBoolEntry("FormCompletion", true);
    d->m_maxFormCompletionItems = config->readNumEntry("MaxFormCompletionItems", 10);
  }

  // Colors
  if ( reset || config->hasGroup( "General" ) )
  {
    config->setGroup( "General" ); // group will be restored by cgs anyway
    if ( reset || config->hasKey( "foreground" ) )
      d->m_textColor = config->readColorEntry( "foreground", &HTML_DEFAULT_TXT_COLOR );

    if ( reset || config->hasKey( "linkColor" ) )
      d->m_linkColor = config->readColorEntry( "linkColor", &HTML_DEFAULT_LNK_COLOR );

    if ( reset || config->hasKey( "visitedLinkColor" ) )
      d->m_vLinkColor = config->readColorEntry( "visitedLinkColor", &HTML_DEFAULT_VLNK_COLOR);
  }


  if( reset || config->hasGroup( "Java/JavaScript Settings" ) )
  {
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
    if( reset || config->hasKey( "JavaDomainSettings" ) )
    {
      check_old_java = false;
      QStringList domainList = config->readListEntry( "JavaDomainSettings" );
      for ( QStringList::ConstIterator it = domainList.begin();
                it != domainList.end(); ++it)
      {
        QString domain;
        KJavaScriptAdvice javaAdvice;
        KJavaScriptAdvice javaScriptAdvice;
        splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
        d->javaDomainPolicy[domain] = javaAdvice;
      }
    }

    bool check_old_ecma = true;
    if( reset || config->hasKey( "ECMADomainSettings" ) )
    {
      check_old_ecma = false;
      QStringList domainList = config->readListEntry( "ECMADomainSettings" );
      for ( QStringList::ConstIterator it = domainList.begin();
                it != domainList.end(); ++it)
      {
        QString domain;
        KJavaScriptAdvice javaAdvice;
        KJavaScriptAdvice javaScriptAdvice;
        splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
        d->javaScriptDomainPolicy[domain] = javaScriptAdvice;
      }
    }

    if( reset || config->hasKey( "JavaScriptDomainAdvice" )
             && ( check_old_java || check_old_ecma ) )
    {
      QStringList domainList = config->readListEntry( "JavaScriptDomainAdvice" );
      for ( QStringList::ConstIterator it = domainList.begin();
                it != domainList.end(); ++it)
      {
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
      if( check_old_java )
      {
        QStringList domainConfig;
        PolicyMap::Iterator it;
        for( it = d->javaDomainPolicy.begin(); it != d->javaDomainPolicy.end(); ++it )
        {
          QCString javaPolicy = adviceToStr( it.data() );
          QCString javaScriptPolicy = adviceToStr( KJavaScriptDunno );
          domainConfig.append(QString::fromLatin1("%1:%2:%3").arg(it.key()).arg(javaPolicy).arg(javaScriptPolicy));
        }
        config->writeEntry( "JavaDomainSettings", domainConfig );
      }

      if( check_old_ecma )
      {
        QStringList domainConfig;
        PolicyMap::Iterator it;
        for( it = d->javaScriptDomainPolicy.begin(); it != d->javaScriptDomainPolicy.end(); ++it )
        {
          QCString javaPolicy = adviceToStr( KJavaScriptDunno );
          QCString javaScriptPolicy = adviceToStr( it.data() );
          domainConfig.append(QString::fromLatin1("%1:%2:%3").arg(it.key()).arg(javaPolicy).arg(javaScriptPolicy));
        }
        config->writeEntry( "ECMADomainSettings", domainConfig );
      }
    }
  }
  config->setGroup(group_save);
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
  return lookup_hostname_policy(hostname.lower(), d->javaDomainPolicy, d->m_bEnableJava);
}

bool KHTMLSettings::isJavaScriptEnabled( const QString& hostname )
{
  return lookup_hostname_policy(hostname.lower(), d->javaScriptDomainPolicy, d->m_bEnableJavaScript);
}

bool KHTMLSettings::isJavaScriptDebugEnabled( const QString& /*hostname*/ )
{
  // debug setting is global for now, but could change in the future
  return d->m_bEnableJavaScriptDebug;
}

bool KHTMLSettings::isPluginsEnabled( const QString& /* hostname */ )
{
  // FIXME: hostname is ignored (dnaber, 2001-01-03)
  return d->m_bEnablePlugins;
}

int KHTMLSettings::mediumFontSize() const
{
    return d->m_fontSize;
}

int KHTMLSettings::minFontSize() const
{
  return d->m_minFontSize;
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

const QString &KHTMLSettings::availableFamilies()
{
    if ( !avFamilies ) {
        avFamilies = new QString;
        QFontDatabase db;
        QStringList families = db.families();
        QStringList s;
        QRegExp foundryExp(" \\[.+\\]");

        //remove foundry info
        for ( QStringList::Iterator f = families.begin(); f != families.end(); ++f ) {
                (*f).replace( foundryExp, "");
                if (!s.contains(*f))
                        s << *f;
        }
        s.sort();

        *avFamilies = s.join(",");
    }

  return *avFamilies;
}

QString KHTMLSettings::lookupFont(int i) const
{
    QString font;
    if (d->fonts.count() > (uint) i)
       font = d->fonts[i];
    if (font.isEmpty())
        font = d->defaultFonts[i];
    return font;
}

QString KHTMLSettings::stdFontName() const
{
    return lookupFont(0);
}

QString KHTMLSettings::fixedFontName() const
{
    return lookupFont(1);
}

QString KHTMLSettings::serifFontName() const
{
    return lookupFont(2);
}

QString KHTMLSettings::sansSerifFontName() const
{
    return lookupFont(3);
}

QString KHTMLSettings::cursiveFontName() const
{
    return lookupFont(4);
}

QString KHTMLSettings::fantasyFontName() const
{
    return lookupFont(5);
}

void KHTMLSettings::setStdFontName(const QString &n)
{
    while(d->fonts.count() <= 0)
        d->fonts.append(QString::null);
    d->fonts[0] = n;
}

void KHTMLSettings::setFixedFontName(const QString &n)
{
    while(d->fonts.count() <= 1)
        d->fonts.append(QString::null);
    d->fonts[1] = n;
}

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

const QColor& KHTMLSettings::textColor() const
{
  return d->m_textColor;
}

const QColor& KHTMLSettings::linkColor() const
{
  return d->m_linkColor;
}

const QColor& KHTMLSettings::vLinkColor() const
{
  return d->m_vLinkColor;
}

bool KHTMLSettings::autoLoadImages() const
{
  return d->m_bAutoLoadImages;
}

KHTMLSettings::KAnimationAdvice KHTMLSettings::showAnimations() const
{
  return d->m_showAnimations;
}
