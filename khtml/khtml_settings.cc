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

#include "khtml_settings.h"
#include "khtmldefaults.h"
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <qregexp.h>

/**
 * @internal
 * Contains all settings which are both available globally and per-domain
 */
struct KPerDomainSettings {
    bool m_bEnableJava : 1;
    bool m_bEnableJavaScript : 1;
    bool m_bEnablePlugins : 1;
    // don't forget to maintain the bitfields as the enums grow
    KHTMLSettings::KJSWindowOpenPolicy m_windowOpenPolicy : 2;
    KHTMLSettings::KJSWindowStatusPolicy m_windowStatusPolicy : 1;
    KHTMLSettings::KJSWindowFocusPolicy m_windowFocusPolicy : 1;
    KHTMLSettings::KJSWindowMovePolicy m_windowMovePolicy : 1;
    KHTMLSettings::KJSWindowResizePolicy m_windowResizePolicy : 1;

    void dump(const QString &infix = QString::null) const {
      kdDebug() << "KPerDomainSettings " << infix << " @" << this << ":" << endl;
      kdDebug() << "  m_bEnableJava: " << m_bEnableJava << endl;
      kdDebug() << "  m_bEnableJavaScript: " << m_bEnableJavaScript << endl;
      kdDebug() << "  m_bEnablePlugins: " << m_bEnablePlugins << endl;
      kdDebug() << "  m_windowOpenPolicy: " << m_windowOpenPolicy << endl;
      kdDebug() << "  m_windowStatusPolicy: " << m_windowStatusPolicy << endl;
      kdDebug() << "  m_windowFocusPolicy: " << m_windowFocusPolicy << endl;
      kdDebug() << "  m_windowMovePolicy: " << m_windowMovePolicy << endl;
      kdDebug() << "  m_windowResizePolicy: " << m_windowResizePolicy << endl;
    }
};

typedef QMap<QString,KPerDomainSettings> PolicyMap;

class KHTMLSettingsPrivate
{
public:
    bool m_bChangeCursor : 1;
    bool m_bBackRightClick : 1;
    bool m_underlineLink : 1;
    bool m_hoverLink : 1;
    bool m_bEnableJavaScriptDebug : 1;
    bool enforceCharset : 1;
    bool m_bAutoLoadImages : 1;
    bool m_formCompletionEnabled : 1;

    // the virtual global "domain"
    KPerDomainSettings global;

    int m_fontSize;
    int m_minFontSize;
    int m_maxFormCompletionItems;
    KHTMLSettings::KAnimationAdvice m_showAnimations;

    QString m_encoding;
    QString m_userSheet;

    QColor m_textColor;
    QColor m_linkColor;
    QColor m_vLinkColor;

    PolicyMap domainPolicy;
    QStringList fonts;
    QStringList defaultFonts;
};


/** Returns a writeable per-domains settings instance for the given domain
  * or a deep copy of the global settings if not existent.
  */
static KPerDomainSettings &setup_per_domain_policy(
				KHTMLSettingsPrivate *d,
				const QString &domain) {
  if (domain.isEmpty()) {
    kdWarning() << "setup_per_domain_policy: domain is empty" << endl;
  }
  QString ldomain = domain.lower();
  PolicyMap::iterator it = d->domainPolicy.find(ldomain);
  if (it == d->domainPolicy.end()) {
    // simply copy global domain settings (they should have been initialized
    // by this time)
    it = d->domainPolicy.insert(ldomain,d->global);
  }
  return *it;
}


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

void KHTMLSettings::readDomainSettings(KConfig *config, bool reset,
	bool global, KPerDomainSettings &pd_settings) {
  QString jsPrefix = global ? QString::null
  				: QString::fromLatin1("javascript.");
  QString javaPrefix = global ? QString::null
  				: QString::fromLatin1("java.");
  QString pluginsPrefix = global ? QString::null
  				: QString::fromLatin1("plugins.");

  // The setting for Java
  QString key = javaPrefix + QString::fromLatin1("EnableJava");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_bEnableJava = config->readBoolEntry( key, false );
  else if ( !global )
    pd_settings.m_bEnableJava = d->global.m_bEnableJava;

  // The setting for Plugins
  key = pluginsPrefix + QString::fromLatin1("EnablePlugins");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_bEnablePlugins = config->readBoolEntry( key, true );
  else if ( !global )
    pd_settings.m_bEnablePlugins = d->global.m_bEnablePlugins;

  // The setting for JavaScript
  key = jsPrefix + QString::fromLatin1("EnableJavaScript");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_bEnableJavaScript = config->readBoolEntry( key, true );
  else if ( !global )
    pd_settings.m_bEnableJavaScript = d->global.m_bEnableJavaScript;

  // window property policies
  key = jsPrefix + QString::fromLatin1("WindowOpenPolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowOpenPolicy = (KJSWindowOpenPolicy)
    		config->readUnsignedNumEntry( key, KJSWindowOpenAllow );
  else if ( !global )
    pd_settings.m_windowOpenPolicy = d->global.m_windowOpenPolicy;

  key = jsPrefix + QString::fromLatin1("WindowMovePolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowMovePolicy = (KJSWindowMovePolicy)
    		config->readUnsignedNumEntry( key, KJSWindowMoveAllow );
  else if ( !global )
    pd_settings.m_windowMovePolicy = d->global.m_windowMovePolicy;

  key = jsPrefix + QString::fromLatin1("WindowResizePolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowResizePolicy = (KJSWindowResizePolicy)
    		config->readUnsignedNumEntry( key, KJSWindowResizeAllow );
  else if ( !global )
    pd_settings.m_windowResizePolicy = d->global.m_windowResizePolicy;

  key = jsPrefix + QString::fromLatin1("WindowStatusPolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowStatusPolicy = (KJSWindowStatusPolicy)
    		config->readUnsignedNumEntry( key, KJSWindowStatusAllow );
  else if ( !global )
    pd_settings.m_windowStatusPolicy = d->global.m_windowStatusPolicy;

  key = jsPrefix + QString::fromLatin1("WindowFocusPolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowFocusPolicy = (KJSWindowFocusPolicy)
    		config->readUnsignedNumEntry( key, KJSWindowFocusAllow );
  else if ( !global )
    pd_settings.m_windowFocusPolicy = d->global.m_windowFocusPolicy;

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
  if (reset || config->hasGroup("MainView Settings"))
  {
    config->setGroup( "MainView Settings" );
    if ( reset || config->hasKey( "BackRightClick" ) )
        d->m_bBackRightClick = config->readBoolEntry( "BackRightClick", false );
  }

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

    // The global setting for JavaScript debugging
    if ( reset || config->hasKey( "EnableJavaScriptDebug" ) )
      d->m_bEnableJavaScriptDebug = config->readBoolEntry( "EnableJavaScriptDebug", false );

    // Read options from the global "domain"
    readDomainSettings(config,reset,true,d->global);
    d->global.dump("init global");

    // The domain-specific settings.
    bool check_old_java_script_settings = true;
    if ( reset || config->hasKey("DomainSettings") ) {
      check_old_java_script_settings = false;
      QStringList domainList = config->readListEntry( "DomainSettings" );
      QString js_group_save = config->group();
      for ( QStringList::ConstIterator it = domainList.begin();
                it != domainList.end(); ++it)
      {
        QString domain = (*it).lower();
	config->setGroup(domain);
        readDomainSettings(config,reset,false,d->domainPolicy[domain]);
	d->domainPolicy[domain].dump("init "+domain);
      }
      config->setGroup(js_group_save);
    }

    bool check_old_java = true;
    if( ( reset || config->hasKey( "JavaDomainSettings" ) )
    	&& check_old_java_script_settings )
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
        setup_per_domain_policy(d,domain).m_bEnableJava =
		javaAdvice == KJavaScriptAccept;
	setup_per_domain_policy(d,domain).dump("JavaDomainSettings 4 "+domain);
      }
    }

    bool check_old_ecma = true;
    if( ( reset || config->hasKey( "ECMADomainSettings" ) )
    	&& check_old_java_script_settings )
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
        setup_per_domain_policy(d,domain).m_bEnableJavaScript =
			javaScriptAdvice == KJavaScriptAccept;
	setup_per_domain_policy(d,domain).dump("ECMADomainSettings 4 "+domain);
      }
    }

    if( ( reset || config->hasKey( "JavaScriptDomainAdvice" ) )
             && ( check_old_java || check_old_ecma )
	     && check_old_java_script_settings )
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
          setup_per_domain_policy(d,domain).m_bEnableJava =
	  		javaAdvice == KJavaScriptAccept;
        if( check_old_ecma )
          setup_per_domain_policy(d,domain).m_bEnableJavaScript =
	  		javaScriptAdvice == KJavaScriptAccept;
	setup_per_domain_policy(d,domain).dump("JavaScriptDomainAdvice 4 "+domain);
      }

      //save all the settings into the new keywords if they don't exist
#if 0
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
#endif
    }
  }
  config->setGroup(group_save);
}


/** Local helper for retrieving per-domain settings.
  *
  * In case of doubt, the global domain is returned.
  */
static const KPerDomainSettings &lookup_hostname_policy(
			const KHTMLSettingsPrivate *d,
			const QString& hostname)
{
  kdDebug() << "lookup_hostname_policy(" << hostname << ")" << endl;
  if (hostname.isEmpty()) {
    d->global.dump("global");
    return d->global;
  }

  PolicyMap::const_iterator notfound = d->domainPolicy.end();

  // First check whether there is a perfect match.
  PolicyMap::const_iterator it = d->domainPolicy.find(hostname);
  if( it != notfound ) {
    kdDebug() << "perfect match" << endl;
    (*it).dump(hostname);
    // yes, use it (unless dunno)
    return *it;
  }

  // Now, check for partial match.  Chop host from the left until
  // there's no dots left.
  QString host_part = hostname;
  int dot_idx = -1;
  while( (dot_idx = host_part.find(QChar('.'))) >= 0 ) {
    host_part.remove(0,dot_idx);
    it = d->domainPolicy.find(host_part);
    Q_ASSERT(notfound == d->domainPolicy.end());
    if( it != notfound ) {
      kdDebug() << "partial match" << endl;
      (*it).dump(host_part);
      return *it;
    }
    // assert(host_part[0] == QChar('.'));
    host_part.remove(0,1); // Chop off the dot.
  }

  // No domain-specific entry: use global domain
  kdDebug() << "no match" << endl;
  d->global.dump("global");
  return d->global;
}

bool KHTMLSettings::isBackRightClickEnabled()
{
  return d->m_bBackRightClick;
}

bool KHTMLSettings::isJavaEnabled( const QString& hostname )
{
  kdDebug() << k_funcinfo << endl;
  return lookup_hostname_policy(d,hostname.lower()).m_bEnableJava;
}

bool KHTMLSettings::isJavaScriptEnabled( const QString& hostname )
{
  kdDebug() << k_funcinfo << endl;
  return lookup_hostname_policy(d,hostname.lower()).m_bEnableJavaScript;
}

bool KHTMLSettings::isJavaScriptDebugEnabled( const QString& /*hostname*/ )
{
  // debug setting is global for now, but could change in the future
  return d->m_bEnableJavaScriptDebug;
}

bool KHTMLSettings::isPluginsEnabled( const QString& hostname )
{
  kdDebug() << k_funcinfo << endl;
  return lookup_hostname_policy(d,hostname.lower()).m_bEnablePlugins;
}

KHTMLSettings::KJSWindowOpenPolicy KHTMLSettings::windowOpenPolicy(
				const QString& hostname) const {
  kdDebug() << k_funcinfo << endl;
  return lookup_hostname_policy(d,hostname.lower()).m_windowOpenPolicy;
}

KHTMLSettings::KJSWindowMovePolicy KHTMLSettings::windowMovePolicy(
				const QString& hostname) const {
  kdDebug() << k_funcinfo << endl;
  return lookup_hostname_policy(d,hostname.lower()).m_windowMovePolicy;
}

KHTMLSettings::KJSWindowResizePolicy KHTMLSettings::windowResizePolicy(
				const QString& hostname) const {
  kdDebug() << k_funcinfo << endl;
  return lookup_hostname_policy(d,hostname.lower()).m_windowResizePolicy;
}

KHTMLSettings::KJSWindowStatusPolicy KHTMLSettings::windowStatusPolicy(
				const QString& hostname) const {
  kdDebug() << k_funcinfo << endl;
  return lookup_hostname_policy(d,hostname.lower()).m_windowStatusPolicy;
}

KHTMLSettings::KJSWindowFocusPolicy KHTMLSettings::windowFocusPolicy(
				const QString& hostname) const {
  kdDebug() << k_funcinfo << endl;
  return lookup_hostname_policy(d,hostname.lower()).m_windowFocusPolicy;
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
