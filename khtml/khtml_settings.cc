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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
#include <q3valuevector.h>
#include <kmessagebox.h>

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

#ifdef DEBUG_SETTINGS
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
#endif
};

typedef QMap<QString,KPerDomainSettings> PolicyMap;

class KHTMLSettingsPrivate
{
public:
    bool m_bChangeCursor : 1;
    bool m_bOpenMiddleClick : 1;
    bool m_bBackRightClick : 1;
    bool m_underlineLink : 1;
    bool m_hoverLink : 1;
    bool m_bEnableJavaScriptDebug : 1;
    bool m_bEnableJavaScriptErrorReporting : 1;
    bool enforceCharset : 1;
    bool m_bAutoLoadImages : 1;
    bool m_bUnfinishedImageFrame : 1;
    bool m_formCompletionEnabled : 1;
    bool m_autoDelayedActionsEnabled : 1;
    bool m_jsErrorsEnabled : 1;
    bool m_follow_system_colors : 1;
    bool m_allowTabulation : 1;
    bool m_autoSpellCheck : 1;
    bool m_adFilterEnabled : 1;
    bool m_hideAdsEnabled : 1;
    bool m_jsPopupBlockerPassivePopup : 1;
    bool m_accessKeysEnabled : 1;

    // the virtual global "domain"
    KPerDomainSettings global;

    int m_fontSize;
    int m_minFontSize;
    int m_maxFormCompletionItems;
    KHTMLSettings::KAnimationAdvice m_showAnimations;

    QString m_encoding;
    QString m_userSheet;

    QColor m_textColor;
    QColor m_baseColor;
    QColor m_linkColor;
    QColor m_vLinkColor;

    PolicyMap domainPolicy;
    QStringList fonts;
    QStringList defaultFonts;

    QVector<QRegExp> adFilters;
    Q3ValueList< QPair< QString, QChar > > m_fallbackAccessKeysAssignments;
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
  const QString ldomain = domain.toLower();
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

  if (_str.isNull())
        ret = KJavaScriptDunno;

  if (_str.toLower() == QLatin1String("accept"))
        ret = KJavaScriptAccept;
  else if (_str.toLower() == QLatin1String("reject"))
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
        domain = configStr.toLower();
        javaAdvice = KJavaScriptDunno;
        javaScriptAdvice = KJavaScriptDunno;
    }
    else
    {
        domain = tmp.left(splitIndex).toLower();
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
  QString key = javaPrefix + QLatin1String("EnableJava");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_bEnableJava = config->readBoolEntry( key, false );
  else if ( !global )
    pd_settings.m_bEnableJava = d->global.m_bEnableJava;

  // The setting for Plugins
  key = pluginsPrefix + QLatin1String("EnablePlugins");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_bEnablePlugins = config->readBoolEntry( key, true );
  else if ( !global )
    pd_settings.m_bEnablePlugins = d->global.m_bEnablePlugins;

  // The setting for JavaScript
  key = jsPrefix + QLatin1String("EnableJavaScript");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_bEnableJavaScript = config->readBoolEntry( key, true );
  else if ( !global )
    pd_settings.m_bEnableJavaScript = d->global.m_bEnableJavaScript;

  // window property policies
  key = jsPrefix + QLatin1String("WindowOpenPolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowOpenPolicy = (KJSWindowOpenPolicy)
    		config->readUnsignedNumEntry( key, KJSWindowOpenSmart );
  else if ( !global )
    pd_settings.m_windowOpenPolicy = d->global.m_windowOpenPolicy;

  key = jsPrefix + QLatin1String("WindowMovePolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowMovePolicy = (KJSWindowMovePolicy)
    		config->readUnsignedNumEntry( key, KJSWindowMoveAllow );
  else if ( !global )
    pd_settings.m_windowMovePolicy = d->global.m_windowMovePolicy;

  key = jsPrefix + QLatin1String("WindowResizePolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowResizePolicy = (KJSWindowResizePolicy)
    		config->readUnsignedNumEntry( key, KJSWindowResizeAllow );
  else if ( !global )
    pd_settings.m_windowResizePolicy = d->global.m_windowResizePolicy;

  key = jsPrefix + QLatin1String("WindowStatusPolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowStatusPolicy = (KJSWindowStatusPolicy)
    		config->readUnsignedNumEntry( key, KJSWindowStatusAllow );
  else if ( !global )
    pd_settings.m_windowStatusPolicy = d->global.m_windowStatusPolicy;

  key = jsPrefix + QLatin1String("WindowFocusPolicy");
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

    if ( reset || config->hasKey( "OpenMiddleClick" ) )
        d->m_bOpenMiddleClick = config->readBoolEntry( "OpenMiddleClick", true );

    if ( reset || config->hasKey( "BackRightClick" ) )
        d->m_bBackRightClick = config->readBoolEntry( "BackRightClick", false );
  }

  if (reset || config->hasGroup("Access Keys")) {
      config->setGroup( "Access Keys" );
      d->m_accessKeysEnabled = config->readBoolEntry( "Enabled", true );
  }

  if (reset || config->hasGroup("Filter Settings"))
  {
      config->setGroup( "Filter Settings" );
      d->m_adFilterEnabled = config->readBoolEntry("Enabled", false);
      d->m_hideAdsEnabled = config->readBoolEntry("Shrink", false);

      d->adFilters.clear();

      QMap<QString,QString> entryMap = config->entryMap("Filter Settings");
      QMap<QString,QString>::ConstIterator it;
      d->adFilters.reserve(entryMap.count());
      for( it = entryMap.constBegin(); it != entryMap.constEnd(); ++it )
      {
          QString name = it.key();
          QString url = it.data();

          if (url.startsWith("!"))
              continue;

          if (name.startsWith("Filter"))
          {
              if (url.length()>2 && url[0]=='/' && url[url.length()-1] == '/')
              {
                  QString inside = url.mid(1, url.length()-2);
                  QRegExp rx(inside);
                  d->adFilters.append(rx);
              }
              else
              {
                  QRegExp rx;
                  int left,right;

                  for (right=url.length(); right>0 && url[right-1]=='*' ; --right);
                  for (left=0; left<right && url[left]=='*' ; ++left);

                  rx.setWildcard(true);
                  rx.setPattern(url.mid(left,right-left));

                  d->adFilters.append(rx);
              }
          }
      }
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
        d->m_fontSize = config->readNumEntry( "MediumFontSize", 12 );

    d->fonts = config->readListEntry( "Fonts" );

    if ( reset || config->hasKey( "DefaultEncoding" ) )
        d->m_encoding = config->readEntry( "DefaultEncoding", "" );

    if ( reset || config->hasKey( "EnforceDefaultCharset" ) )
        d->enforceCharset = config->readBoolEntry( "EnforceDefaultCharset", false );

    // Behavior
    if ( reset || config->hasKey( "ChangeCursor" ) )
        d->m_bChangeCursor = config->readBoolEntry( "ChangeCursor", KDE_DEFAULT_CHANGECURSOR );

    if ( reset || config->hasKey("UnderlineLinks") )
        d->m_underlineLink = config->readBoolEntry( "UnderlineLinks", true );

    if ( reset || config->hasKey( "HoverLinks" ) )
    {
        if ( ( d->m_hoverLink = config->readBoolEntry( "HoverLinks", false ) ) )
            d->m_underlineLink = false;
    }

    if ( reset || config->hasKey( "AllowTabulation" ) )
        d->m_allowTabulation = config->readBoolEntry( "AllowTabulation", false );

    if ( reset || config->hasKey( "AutoSpellCheck" ) )
        d->m_autoSpellCheck = config->readBoolEntry( "AutoSpellCheck", true );

    // Other
    if ( reset || config->hasKey( "AutoLoadImages" ) )
      d->m_bAutoLoadImages = config->readBoolEntry( "AutoLoadImages", true );

    if ( reset || config->hasKey( "UnfinishedImageFrame" ) )
      d->m_bUnfinishedImageFrame = config->readBoolEntry( "UnfinishedImageFrame", true );

    if ( reset || config->hasKey( "ShowAnimations" ) )
    {
      QString value = config->readEntry( "ShowAnimations").toLower();
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
    d->m_autoDelayedActionsEnabled = config->readBoolEntry ("AutoDelayedActions", true);
    d->m_jsErrorsEnabled = config->readBoolEntry("ReportJSErrors", true);
    QStringList accesskeys = config->readListEntry("FallbackAccessKeysAssignments");
    d->m_fallbackAccessKeysAssignments.clear();
    for( QStringList::ConstIterator it = accesskeys.begin(); it != accesskeys.end(); ++it )
        if( (*it).length() > 2 && (*it)[ 1 ] == ':' )
            d->m_fallbackAccessKeysAssignments.append( qMakePair( (*it).mid( 2 ), (*it)[ 0 ] ));
  }

  // Colors

  if ( reset || config->hasKey( "FollowSystemColors" ) )
      d->m_follow_system_colors = config->readBoolEntry( "FollowSystemColors", false );

  if ( reset || config->hasGroup( "General" ) )
  {
    config->setGroup( "General" ); // group will be restored by cgs anyway
    if ( reset || config->hasKey( "foreground" ) ) {
      QColor def(HTML_DEFAULT_TXT_COLOR);
      d->m_textColor = config->readColorEntry( "foreground", &def );
    }

    if ( reset || config->hasKey( "linkColor" ) ) {
      QColor def(HTML_DEFAULT_LNK_COLOR);
      d->m_linkColor = config->readColorEntry( "linkColor", &def );
    }

    if ( reset || config->hasKey( "visitedLinkColor" ) ) {
      QColor def(HTML_DEFAULT_VLNK_COLOR);
      d->m_vLinkColor = config->readColorEntry( "visitedLinkColor", &def);
    }

    if ( reset || config->hasKey( "background" ) ) {
      QColor def(HTML_DEFAULT_BASE_COLOR);
      d->m_baseColor = config->readColorEntry( "background", &def);
    }
  }

  if( reset || config->hasGroup( "Java/JavaScript Settings" ) )
  {
    config->setGroup( "Java/JavaScript Settings" );

    // The global setting for JavaScript debugging
    // This is currently always enabled by default
    if ( reset || config->hasKey( "EnableJavaScriptDebug" ) )
      d->m_bEnableJavaScriptDebug = config->readBoolEntry( "EnableJavaScriptDebug", false );

    // The global setting for JavaScript error reporting
    if ( reset || config->hasKey( "ReportJavaScriptErrors" ) )
      d->m_bEnableJavaScriptErrorReporting = config->readBoolEntry( "ReportJavaScriptErrors", false );

    // The global setting for popup block passive popup
    if ( reset || config->hasKey( "PopupBlockerPassivePopup" ) )
      d->m_jsPopupBlockerPassivePopup = config->readBoolEntry("PopupBlockerPassivePopup", true);

    // Read options from the global "domain"
    readDomainSettings(config,reset,true,d->global);
#ifdef DEBUG_SETTINGS
    d->global.dump("init global");
#endif

    // The domain-specific settings.

    static const char *const domain_keys[] = {	// always keep order of keys
    	"ECMADomains", "JavaDomains", "PluginDomains"
    };
    bool check_old_ecma_settings = true;
    bool check_old_java_settings = true;
    // merge all domains into one list
    QMap<QString,int> domainList;	// why can't Qt have a QSet?
    for (unsigned i = 0; i < sizeof domain_keys/sizeof domain_keys[0]; ++i) {
      if ( reset || config->hasKey(domain_keys[i]) ) {
        if (i == 0) check_old_ecma_settings = false;
	else if (i == 1) check_old_java_settings = false;
        const QStringList dl = config->readListEntry( domain_keys[i] );
	const QMap<QString,int>::Iterator notfound = domainList.end();
	QStringList::ConstIterator it = dl.begin();
	const QStringList::ConstIterator itEnd = dl.end();
	for (; it != itEnd; ++it) {
	  const QString domain = (*it).toLower();
	  QMap<QString,int>::Iterator pos = domainList.find(domain);
	  if (pos == notfound) domainList.insert(domain,0);
	}/*next it*/
      }
    }/*next i*/

    if (reset)
      d->domainPolicy.clear();

    QString js_group_save = config->group();
    {
      QMap<QString,int>::ConstIterator it = domainList.begin();
      const QMap<QString,int>::ConstIterator itEnd = domainList.end();
      for ( ; it != itEnd; ++it)
      {
        const QString domain = it.key();
        config->setGroup(domain);
        readDomainSettings(config,reset,false,d->domainPolicy[domain]);
#ifdef DEBUG_SETTINGS
        d->domainPolicy[domain].dump("init "+domain);
#endif
      }
    }
    config->setGroup(js_group_save);

    bool check_old_java = true;
    if( ( reset || config->hasKey( "JavaDomainSettings" ) )
    	&& check_old_java_settings )
    {
      check_old_java = false;
      const QStringList domainList = config->readListEntry( "JavaDomainSettings" );
      QStringList::ConstIterator it = domainList.begin();
      const QStringList::ConstIterator itEnd = domainList.end();
      for ( ; it != itEnd; ++it)
      {
        QString domain;
        KJavaScriptAdvice javaAdvice;
        KJavaScriptAdvice javaScriptAdvice;
        splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
        setup_per_domain_policy(d,domain).m_bEnableJava =
		javaAdvice == KJavaScriptAccept;
#ifdef DEBUG_SETTINGS
	setup_per_domain_policy(d,domain).dump("JavaDomainSettings 4 "+domain);
#endif
      }
    }

    bool check_old_ecma = true;
    if( ( reset || config->hasKey( "ECMADomainSettings" ) )
	&& check_old_ecma_settings )
    {
      check_old_ecma = false;
      const QStringList domainList = config->readListEntry( "ECMADomainSettings" );
      QStringList::ConstIterator it = domainList.begin();
      const QStringList::ConstIterator itEnd = domainList.end();
      for ( ; it != itEnd; ++it)
      {
        QString domain;
        KJavaScriptAdvice javaAdvice;
        KJavaScriptAdvice javaScriptAdvice;
        splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
        setup_per_domain_policy(d,domain).m_bEnableJavaScript =
			javaScriptAdvice == KJavaScriptAccept;
#ifdef DEBUG_SETTINGS
	setup_per_domain_policy(d,domain).dump("ECMADomainSettings 4 "+domain);
#endif
      }
    }

    if( ( reset || config->hasKey( "JavaScriptDomainAdvice" ) )
             && ( check_old_java || check_old_ecma )
	     && ( check_old_ecma_settings || check_old_java_settings ) )
    {
      const QStringList domainList = config->readListEntry( "JavaScriptDomainAdvice" );
      QStringList::ConstIterator it = domainList.begin();
      const QStringList::ConstIterator itEnd = domainList.end();
      for ( ; it != itEnd; ++it)
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
#ifdef DEBUG_SETTINGS
	setup_per_domain_policy(d,domain).dump("JavaScriptDomainAdvice 4 "+domain);
#endif
      }

      //save all the settings into the new keywords if they don't exist
#if 0
      if( check_old_java )
      {
        QStringList domainConfig;
        PolicyMap::Iterator it;
        for( it = d->javaDomainPolicy.begin(); it != d->javaDomainPolicy.end(); ++it )
        {
          QByteArray javaPolicy = adviceToStr( it.data() );
          QByteArray javaScriptPolicy = adviceToStr( KJavaScriptDunno );
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
          QByteArray javaPolicy = adviceToStr( KJavaScriptDunno );
          QByteArray javaScriptPolicy = adviceToStr( it.data() );
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
#ifdef DEBUG_SETTINGS
  kdDebug() << "lookup_hostname_policy(" << hostname << ")" << endl;
#endif
  if (hostname.isEmpty()) {
#ifdef DEBUG_SETTINGS
    d->global.dump("global");
#endif
    return d->global;
  }

  const PolicyMap::const_iterator notfound = d->domainPolicy.end();

  // First check whether there is a perfect match.
  PolicyMap::const_iterator it = d->domainPolicy.find(hostname);
  if( it != notfound ) {
#ifdef DEBUG_SETTINGS
    kdDebug() << "perfect match" << endl;
    (*it).dump(hostname);
#endif
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
#ifdef DEBUG_SETTINGS
      kdDebug() << "partial match" << endl;
      (*it).dump(host_part);
#endif
      return *it;
    }
    // assert(host_part[0] == QChar('.'));
    host_part.remove(0,1); // Chop off the dot.
  }

  // No domain-specific entry: use global domain
#ifdef DEBUG_SETTINGS
  kdDebug() << "no match" << endl;
  d->global.dump("global");
#endif
  return d->global;
}

bool KHTMLSettings::isOpenMiddleClickEnabled()
{
  return d->m_bOpenMiddleClick;
}

bool KHTMLSettings::isBackRightClickEnabled()
{
  return d->m_bBackRightClick;
}

bool KHTMLSettings::accessKeysEnabled() const
{
    return d->m_accessKeysEnabled;
}

bool KHTMLSettings::isAdFilterEnabled() const
{
    return d->m_adFilterEnabled;
}

bool KHTMLSettings::isHideAdsEnabled() const
{
    return d->m_hideAdsEnabled;
}

bool KHTMLSettings::isAdFiltered( const QString &url ) const
{
    if (d->m_adFilterEnabled)
    {
        if (!url.startsWith("data:"))
        {
	  QVector<QRegExp>::iterator it;
	  for (it=d->adFilters.begin(); it != d->adFilters.end(); ++it)
            {
                if ((*it).search(url) != -1)
                {
                    kdDebug( 6080 ) << "Filtered: " << url << endl;
                    return true;
                }
            }
        }
    }
    return false;
}

void KHTMLSettings::addAdFilter( const QString &url )
{
    KConfig config( "khtmlrc", false, false );
    config.setGroup( "Filter Settings" );

    QRegExp rx;
    if (url.length()>2 && url[0]=='/' && url[url.length()-1] == '/')
    {
        QString inside = url.mid(1, url.length()-2);
        rx.setWildcard(false);
        rx.setPattern(inside);
    }
    else
    {
        int left,right;

        rx.setWildcard(true);
        for (right=url.length(); right>0 && url[right-1]=='*' ; --right);
        for (left=0; left<right && url[left]=='*' ; ++left);

        rx.setPattern(url.mid(left,right-left));
    }

    if (rx.isValid())
    {
        int last=config.readNumEntry("Count",0);
        QString key = "Filter-" + QString::number(last);
        config.writeEntry(key, url);
        config.writeEntry("Count",last+1);
        config.sync();

        d->adFilters.append(rx);
    }
    else
    {
        KMessageBox::error(0,
                           rx.errorString(),
                           i18n("Filter error"));
    }
}

bool KHTMLSettings::isJavaEnabled( const QString& hostname )
{
  return lookup_hostname_policy(d,hostname.toLower()).m_bEnableJava;
}

bool KHTMLSettings::isJavaScriptEnabled( const QString& hostname )
{
  return lookup_hostname_policy(d,hostname.toLower()).m_bEnableJavaScript;
}

bool KHTMLSettings::isJavaScriptDebugEnabled( const QString& /*hostname*/ )
{
  // debug setting is global for now, but could change in the future
  return d->m_bEnableJavaScriptDebug;
}

bool KHTMLSettings::isJavaScriptErrorReportingEnabled( const QString& /*hostname*/ ) const
{
  // error reporting setting is global for now, but could change in the future
  return d->m_bEnableJavaScriptErrorReporting;
}

bool KHTMLSettings::isPluginsEnabled( const QString& hostname )
{
  return lookup_hostname_policy(d,hostname.toLower()).m_bEnablePlugins;
}

KHTMLSettings::KJSWindowOpenPolicy KHTMLSettings::windowOpenPolicy(
				const QString& hostname) const {
  return lookup_hostname_policy(d,hostname.toLower()).m_windowOpenPolicy;
}

KHTMLSettings::KJSWindowMovePolicy KHTMLSettings::windowMovePolicy(
				const QString& hostname) const {
  return lookup_hostname_policy(d,hostname.toLower()).m_windowMovePolicy;
}

KHTMLSettings::KJSWindowResizePolicy KHTMLSettings::windowResizePolicy(
				const QString& hostname) const {
  return lookup_hostname_policy(d,hostname.toLower()).m_windowResizePolicy;
}

KHTMLSettings::KJSWindowStatusPolicy KHTMLSettings::windowStatusPolicy(
				const QString& hostname) const {
  return lookup_hostname_policy(d,hostname.toLower()).m_windowStatusPolicy;
}

KHTMLSettings::KJSWindowFocusPolicy KHTMLSettings::windowFocusPolicy(
				const QString& hostname) const {
  return lookup_hostname_policy(d,hostname.toLower()).m_windowFocusPolicy;
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
        QStringList::Iterator f = families.begin();
        const QStringList::Iterator fEnd = families.end();

        for ( ; f != fEnd; ++f ) {
                (*f).replace( foundryExp, "");
                if (!s.contains(*f))
                        s << *f;
        }
        s.sort();

        *avFamilies = ',' + s.join(",") + ',';
    }

  return *avFamilies;
}

QString KHTMLSettings::lookupFont(int i) const
{
    QString font;
    if (d->fonts.count() > i)
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

bool KHTMLSettings::followSystemColors() const
{
    return d->m_follow_system_colors;
}

const QColor& KHTMLSettings::textColor() const
{
  return d->m_textColor;
}

const QColor& KHTMLSettings::baseColor() const
{
  return d->m_baseColor;
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

bool KHTMLSettings::unfinishedImageFrame() const
{
  return d->m_bUnfinishedImageFrame;
}

KHTMLSettings::KAnimationAdvice KHTMLSettings::showAnimations() const
{
  return d->m_showAnimations;
}

bool KHTMLSettings::isAutoDelayedActionsEnabled() const
{
  return d->m_autoDelayedActionsEnabled;
}

bool KHTMLSettings::jsErrorsEnabled() const
{
  return d->m_jsErrorsEnabled;
}

void KHTMLSettings::setJSErrorsEnabled(bool enabled)
{
  d->m_jsErrorsEnabled = enabled;
  // save it
  KConfig *config = KGlobal::config();
  config->setGroup("HTML Settings");
  config->writeEntry("ReportJSErrors", enabled);
  config->sync();
}

bool KHTMLSettings::allowTabulation() const
{
    return d->m_allowTabulation;
}

bool KHTMLSettings::autoSpellCheck() const
{
    return d->m_autoSpellCheck;
}

Q3ValueList< QPair< QString, QChar > > KHTMLSettings::fallbackAccessKeysAssignments() const
{
    return d->m_fallbackAccessKeysAssignments;
}

void KHTMLSettings::setJSPopupBlockerPassivePopup(bool enabled)
{
    d->m_jsPopupBlockerPassivePopup = enabled;
    // save it
    KConfig *config = KGlobal::config();
    config->setGroup("Java/JavaScript Settings");
    config->writeEntry("PopupBlockerPassivePopup", enabled);
    config->sync();
}

bool KHTMLSettings::jsPopupBlockerPassivePopup() const
{
    return d->m_jsPopupBlockerPassivePopup;
}
