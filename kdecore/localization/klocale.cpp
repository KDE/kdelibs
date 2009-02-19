/* This file is part of the KDE libraries
   Copyright (c) 1997,2001 Stephan Kulow <coolo@kde.org>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1999-2002 Hans Petter Bieker <bieker@kde.org>
   Copyright (c) 2002 Lukas Tinkl <lukas@kde.org>
   Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>

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

#include "klocale.h"

#include <config.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif

#include <QtCore/QTextCodec>
#include <QtCore/QFile>
#include <QtGui/QPrinter>
#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>
#include <QtCore/QLocale>

#include "kcatalog_p.h"
#include "kglobal.h"
#include "kstandarddirs.h"
#include "kconfig.h"
#include "kcomponentdata.h"
#include "kdebug.h"
#include "kdatetime.h"
#include "kcalendarsystem.h"
#include "klocalizedstring.h"
#include "ktranslit_p.h"
#include "kconfiggroup.h"
#include "kcatalogname_p.h"
#include "common_helpers_p.h"

#ifdef Q_WS_WIN
#include <windows.h>
#endif

static const char *maincatalog = 0;

QDebug operator<<(QDebug debug, const KCatalogName &cn)
{
    return debug << cn.name << cn.loadCount;
}

class KLocalePrivate
{
public:
    KLocalePrivate(const QString& catalog, KConfig *config, const QString &language_ = QString(), const QString &country_ = QString());
  /**
   * @internal Initializes the catalogs appname, kdelibs and kio for all chosen languages.
   *
   * @param config The configuration object used for init
   * @param useEnv True if we should use environment variables
   */
  void initMainCatalogs(const QString & catalog);

  /**
   * @internal Initializes the list of valid languages from the user's point of view. This is the list of
   * languages that the user picks in kcontrol. The config object should be valid and contain the global
   * entries.
   *
   * @param config The configuration object used for init
   * @param useEnv True if we should use environment variables
   */
  void initLanguageList(KConfig *config, bool useEnv);

  /**
   * @internal Figures out which encoding the user prefers.
   */
  void initEncoding();

  /**
   * @internal Figures out which encoding the user prefers for filenames
   * and sets up the appropriate QFile encoding and decoding functions.
   */
  void initFileNameEncoding();

  /**
   * @internal A QFile filename encoding function (QFile::encodeFn).
   */
  static QByteArray encodeFileNameUTF8( const QString & fileName );

  /**
   * @internal QFile filename decoding function (QFile::decodeFn).
   */
  static QString decodeFileNameUTF8( const QByteArray & localFileName );

  /**
   * @internal Reads the format configuration from disk.
   */
  void initFormat(KConfig *config);

  /**
   * @internal The worker of the same-name KLocale API function.
   */
  bool setLanguage(const QString & _language, KConfig *config);

  /**
   * @internal The worker of the same-name KLocale API function.
   */
  bool setLanguage(const QStringList & languages);

  /**
   * @internal The worker of the same-name KLocale API function.
   */
  bool setEncoding(int mibEnum);

  /**
   * @internal function used by the translate versions
   */
  void translate_priv(const char *msgctxt,
                      const char *msgid,
                      const char *msgid_plural = 0,
                      unsigned long n = 0,
                      QString *language = 0,
                      QString *translation = 0) const;

  /**
   * @internal function used to determine if we are using the en_US translation
   */
  bool useDefaultLanguage() const;

  /**
   * @internal evaluate the list of catalogs and check that all instances for all languages are loaded
   * and that they are sorted according to the catalog names
   */
  void updateCatalogs( );

  /**
   * @internal Checks whether or not theFind catalog for given language and given catalog name.
   *
   * @param language language to check
   */
  bool isApplicationTranslatedInto( const QString & language);

  /**
   * @internal Formats a date/time according to specified format.
   */
  static QString formatDateTime(const KLocale *locale, const QDateTime &dateTime,
                                KLocale::DateFormat, bool includeSeconds, int daysToNow);
  /**
   * @internal Formats a date as a 'fancy' date.
   */
  static QString fancyDate(const KLocale *locale, const QDate &date, int daysToNow);

  enum DurationType {
      DaysDurationType = 0,
      HoursDurationType,
      MinutesDurationType,
      SecondsDurationType
  };
  /**
   * @internal Formats a duration according to the given type and number
   */
  static QString formatSingleDuration( DurationType durationType, int n );

  // Numbers and money
  QString decimalSymbol;
  QString thousandsSeparator;
  QString currencySymbol;
  QString monetaryDecimalSymbol;
  QString monetaryThousandsSeparator;
  QString positiveSign;
  QString negativeSign;
  int fracDigits;
  KLocale::SignPosition positiveMonetarySignPosition;
  KLocale::SignPosition negativeMonetarySignPosition;
  bool positivePrefixCurrencySymbol : 1;
  bool negativePrefixCurrencySymbol : 1;

  // Date and time
  QString timeFormat;
  QString dateFormat;
  QString dateFormatShort;
  int weekStartDay;
  int workingWeekStartDay;
  int workingWeekEndDay;
  int weekDayOfPray;

  // Locale
  QString language;
  QString country;

  // Handling of translation catalogs
  QStringList languageList;

  QList<KCatalogName> catalogNames; // list of all catalogs (regardless of language)
  QList<KCatalog> catalogs; // list of all found catalogs, one instance per catalog name and language
  int numberOfSysCatalogs; // number of catalogs that each app draws from
  bool useTranscript; // indicates if scripted messages are to be executed

  // Misc
  QString encoding;
  QTextCodec * codecForEncoding;
  //KSharedConfig::Ptr config;
  int pageSize;
  KLocale::MeasureSystem measureSystem;
  KConfig * languages;

  QString calendarType;
  KCalendarSystem * calendar;
  QString appName;
  bool nounDeclension:1;
  bool dateMonthNamePossessive:1;
  bool utf8FileEncoding:1;
#ifdef Q_WS_WIN
  char win32SystemEncoding[3+7]; //"cp " + lang ID
#endif
};

KLocalePrivate::KLocalePrivate(const QString& catalog, KConfig *config, const QString &language_, const QString &country_)
    : language(language_),
      country(country_),
      useTranscript(false), codecForEncoding(0),
      languages(0), calendar(0), appName(catalog)
{
    initEncoding();
    initFileNameEncoding();

    if (config) {
        initLanguageList(config, false);
    }
    else {
        config = KGlobal::config().data();
        initLanguageList(config, true);
    }

    initMainCatalogs(catalog);

    initFormat(config);
}

KLocale::KLocale( const QString & catalog, KSharedConfig::Ptr config )
	: d(new KLocalePrivate(catalog, config.data()))
{
}

KLocale::KLocale(const QString& catalog, const QString &language, const QString &country, KConfig *config)
	: d(new KLocalePrivate(catalog, config, language, country))
{
}

void KLocalePrivate::initMainCatalogs(const QString & catalog)
{
  // Use the first non-null string.
  QString mainCatalog = catalog;
  if (maincatalog)
    mainCatalog = QString::fromLatin1(maincatalog);

  if (mainCatalog.isEmpty()) {
    kDebug(173) << "KLocale instance created called without valid "
                 << "catalog! Give an argument or call setMainCatalog "
                 << "before init" << endl;
  }
  else {
    // do not use insertCatalog here, that would already trigger updateCatalogs
    catalogNames.append(KCatalogName(mainCatalog));   // application catalog

    // catalogs from which each application can draw translations
    numberOfSysCatalogs = 4;
    catalogNames.append(KCatalogName("libphonon"));
    catalogNames.append(KCatalogName("kio4"));
    catalogNames.append(KCatalogName("kdelibs4"));
    catalogNames.append(KCatalogName("kdeqt"));

    updateCatalogs(); // evaluate this for all languages
  }
}

static inline void getLanguagesFromVariable(QStringList& list, const char* variable)
{
  QByteArray var( qgetenv(variable) );
  if (!var.isEmpty())
    list += QFile::decodeName(var).split(':');
}

void KLocalePrivate::initLanguageList(KConfig *config, bool useEnv)
{
  KConfigGroup cg(config, "Locale");

  // Set the country as specified by the KDE config or use default,
  // do not consider environment variables.
  if (country.isEmpty())
    country = cg.readEntry("Country");
  if (country.isEmpty())
      country = KLocale::defaultCountry();

  // Collect possible languages by decreasing priority.
  // The priority is as follows:
  // - KDE_LANG environment variable (can be a list)
  // - KDE configuration (can be a list)
  // - environment variables considered by gettext(3)
  // The environment variables are not considered if useEnv is false.
  QStringList list;

  if (!language.isEmpty())
      list.append(language);

  // Collect languages set by KDE_LANG.
  if (useEnv)
    getLanguagesFromVariable(list, "KDE_LANG");

  // Collect languages set by KDE config.
  QString languages(cg.readEntry("Language", QString()));
  if (!languages.isEmpty())
    list += languages.split(':');

  // Collect languages read from environment variables by gettext(3).
  QStringList rawList;
  if (useEnv) {
    // Collect by same order of priority as for gettext(3).

    // LANGUAGE should contain colon-separated list of exact language codes,
    // so add them directly.
    getLanguagesFromVariable(list, "LANGUAGE");

    // Other environment variables contain locale string, which should
    // be checked for all combinations yielding language codes.
    getLanguagesFromVariable(rawList, "LC_ALL");
    getLanguagesFromVariable(rawList, "LC_MESSAGES");
    getLanguagesFromVariable(rawList, "LANG");
  }
#ifdef Q_WS_WIN // how about Mac?
  rawList += QLocale::system().name(); // fall back to the system language
#endif

#ifndef Q_WS_WIN
  if (useEnv) // Collect languages  - continued...
#endif
  {
    // Process the raw list to create possible combinations.
    foreach (const QString &ln, rawList) {
      QString lang, ctry, modf, cset;
      KLocale::splitLocale(ln, lang, ctry, modf, cset);

      if (!ctry.isEmpty() && !modf.isEmpty()) {
        list += lang + '_' + ctry + '@' + modf;
      }
      // NOTE: The priority is tricky in case both ctry and modf are present.
      // Should really lang@modf be of higher priority than lang_ctry?
      // For at least one case (Serbian language), it is better this way.
      if (!modf.isEmpty()) {
        list += lang + '@' + modf;
      }
      if (!ctry.isEmpty()) {
        list += lang + '_' + ctry;
      }
      list += lang;
    }
  }

  // Send the list to filter for really present languages on the system.
  setLanguage(list);
}

void KLocalePrivate::initFormat(KConfig *config)
{
  Q_ASSERT(config);

  //kDebug(173) << "KLocalePrivate::KLocalePrivate";

  config->setLocale(language);

  KConfigGroup cg(config, "Locale");

  KConfig entryFile(KStandardDirs::locate("locale",
                                           QString::fromLatin1("l10n/%1/entry.desktop")
                                           .arg(country)));
  entryFile.setLocale(language);
  KConfigGroup entry(&entryFile, "KCM Locale");

  // Numeric
#define readConfigEntry(key, default, save) \
  save = entry.readEntry(key, default); \
  save = cg.readEntry(key, save);

#define readConfigNumEntry(key, default, save, type) \
  save = (type)entry.readEntry(key, int(default)); \
  save = (type)cg.readEntry(key, int(save));

  readConfigEntry("DecimalSymbol", ".", decimalSymbol);
  readConfigEntry("ThousandsSeparator", ",", thousandsSeparator);
  thousandsSeparator.remove( QString::fromLatin1("$0") );
  //kDebug(173) << "thousandsSeparator=" << thousandsSeparator;

  readConfigEntry("PositiveSign", "", positiveSign);
  readConfigEntry("NegativeSign", "-", negativeSign);

  // Monetary
  readConfigEntry("CurrencySymbol", "$", currencySymbol);
  readConfigEntry("MonetaryDecimalSymbol", ".", monetaryDecimalSymbol);
  readConfigEntry("MonetaryThousandsSeparator", ",",
		  monetaryThousandsSeparator);
  monetaryThousandsSeparator.remove(QString::fromLatin1("$0"));

  readConfigNumEntry("FracDigits", 2, fracDigits, int);
  readConfigEntry("PositivePrefixCurrencySymbol", true,
		      positivePrefixCurrencySymbol);
  readConfigEntry("NegativePrefixCurrencySymbol", true,
		      negativePrefixCurrencySymbol);
  readConfigNumEntry("PositiveMonetarySignPosition", KLocale::BeforeQuantityMoney,
		     positiveMonetarySignPosition, KLocale::SignPosition);
  readConfigNumEntry("NegativeMonetarySignPosition", KLocale::ParensAround,
		     negativeMonetarySignPosition, KLocale::SignPosition);

  // Date and time
  readConfigEntry("TimeFormat", "%H:%M:%S", timeFormat);
  readConfigEntry("DateFormat", "%A %d %B %Y", dateFormat);
  readConfigEntry("DateFormatShort", "%Y-%m-%d", dateFormatShort);
  readConfigNumEntry("WeekStartDay", 1, weekStartDay, int);                //default to Monday
  readConfigNumEntry("WorkingWeekStartDay", 1, workingWeekStartDay, int);  //default to Monday
  readConfigNumEntry("WorkingWeekEndDay", 5, workingWeekEndDay, int);      //default to Friday
  readConfigNumEntry("WeekDayOfPray", 7, weekDayOfPray, int);              //default to Sunday

  // other
  readConfigNumEntry("PageSize", QPrinter::A4, pageSize,
		     QPrinter::PageSize);
  readConfigNumEntry("MeasureSystem", KLocale::Metric,
		     measureSystem, KLocale::MeasureSystem);
  readConfigEntry("CalendarSystem", "gregorian", calendarType);
  delete calendar;
  calendar = 0; // ### HPB Is this the correct place?

  readConfigEntry("Transcript", true, useTranscript);

  //Grammatical
  //Precedence here is l10n / i18n / config file
  KConfig langCfg(KStandardDirs::locate("locale",
                                        QString::fromLatin1("%1/entry.desktop")
                                        .arg(language)));
  KConfigGroup lang(&langCfg,"KCM Locale");
#define read3ConfigBoolEntry(key, default, save) \
  save = entry.readEntry(key, default); \
  save = lang.readEntry(key, save); \
  save = cg.readEntry(key, save);

  read3ConfigBoolEntry("NounDeclension", false, nounDeclension);
  read3ConfigBoolEntry("DateMonthNamePossessive", false,
		       dateMonthNamePossessive);
}

bool KLocale::setCountry(const QString & aCountry, KConfig *config)
{
  // Check if the file exists too??
  if ( aCountry.isEmpty() )
    return false;

  d->country = aCountry;

  d->initFormat(config);

  return true;
}

bool KLocale::setLanguage(const QString & language, KConfig *config)
{
  return d->setLanguage(language, config);
}

bool KLocalePrivate::setLanguage(const QString & _language, KConfig *config)
{
  if ( languageList.contains( _language ) ) {
 	 languageList.removeAll( _language );
  }
  languageList.prepend( _language ); // let us consider this language to be the most important one

  language = _language; // remember main language for shortcut evaluation

  // important when called from the outside and harmless when called before
  // populating the catalog name list
  updateCatalogs();

  initFormat(config);

  return true; // Maybe the mo-files for this language are empty, but in principle we can speak all languages
}

bool KLocale::setLanguage(const QStringList & languages)
{
  return d->setLanguage(languages);
}

bool KLocalePrivate::setLanguage(const QStringList & languages)
{
  // This list might contain
  // 1) some empty strings that we have to eliminate
  // 2) duplicate entries like in de:fr:de, where we have to keep the first occurrence of a language in order
  //    to preserve the order of precenence of the user
  // 3) languages into which the application is not translated. For those languages we should not even load kdelibs.mo or kio.po.
  //    these languages have to be dropped. Otherwise we get strange side effects, e.g. with Hebrew:
  //    the right/left switch for languages that write from
  //    right to left (like Hebrew or Arabic) is set in kdelibs.mo. If you only have kdelibs.mo
  //    but nothing from appname.mo, you get a mostly English app with layout from right to left.
  //    That was considered to be a bug by the Hebrew translators.
  QStringList list;
  foreach (const QString &language, languages) {
      if (!language.isEmpty() && !list.contains(language) && isApplicationTranslatedInto(language)) {
          list.append(language);
      }
  }

  if ( !list.contains( KLocale::defaultLanguage() ) ) {
    // English should always be added as final possibility; this is important
    // for proper initialization of message text post-processors which are
    // needed for English too, like semantic to visual formatting, etc.
    list.append( KLocale::defaultLanguage() );
  }

  language = list.first(); // keep this for shortcut evaluations

  languageList = list; // keep this new list of languages to use

  // important when called from the outside and harmless when called before populating the
  // catalog name list
  updateCatalogs();

  return true; // we found something. Maybe it's only English, but we found something
}

bool KLocale::isApplicationTranslatedInto( const QString & lang)
{
  return d->isApplicationTranslatedInto( lang );
}

bool KLocalePrivate::isApplicationTranslatedInto( const QString & lang)
{
  if ( lang.isEmpty() ) {
    return false;
  }

  if ( lang == KLocale::defaultLanguage() ) {
    // en_us is always "installed"
    return true;
  }

  if (maincatalog) {
    appName = QString::fromLatin1(maincatalog);
  }

  // Check for this language and possible transliteration fallbacks.
  QStringList possibles;
  possibles += lang;
  possibles += KTranslit::fallbackList(lang);
  foreach (const QString &lang, possibles) {
    if ( ! KCatalog::catalogLocaleDir( appName, lang ).isEmpty() ) {
        return true;
    }
  }
  return false;
}

void KLocale::splitLocale(const QString &aLocale,
                          QString &language,
                          QString &country,
                          QString &modifier,
                          QString &charset)
{
  QString locale = aLocale;

  language.clear();
  country.clear();
  modifier.clear();
  charset.clear();

  // In case there are several concatenated locale specifications,
  // truncate all but first.
  int f = locale.indexOf(':');
  if (f >= 0) {
    locale.truncate(f);
  }

  f = locale.indexOf('.');
  if (f >= 0) {
    charset = locale.mid(f + 1);
    locale.truncate(f);
  }

  f = locale.indexOf('@');
  if (f >= 0) {
    modifier = locale.mid(f + 1);
    locale.truncate(f);
  }

  f = locale.indexOf('_');
  if (f >= 0) {
    country = locale.mid(f + 1);
    locale.truncate(f);
  }

  language = locale;
}

QString KLocale::language() const
{
  return d->language;
}

QString KLocale::country() const
{
  return d->country;
}

void KLocale::insertCatalog( const QString & catalog )
{
    int pos = d->catalogNames.indexOf(KCatalogName(catalog));
    if (pos != -1) {
        ++d->catalogNames[pos].loadCount;
        return;
    }

    // Insert new catalog just before system catalogs, to preserve the
    // lowest priority of system catalogs.
    d->catalogNames.insert(d->catalogNames.size() - d->numberOfSysCatalogs,
                           KCatalogName(catalog));
    d->updateCatalogs(); // evaluate the changed list and generate the necessary KCatalog objects
}

void KLocalePrivate::updateCatalogs( )
{
  // some changes have occurred. Maybe we have learned or forgotten some languages.
  // Maybe the language precedence has changed.
  // Maybe we have learned or forgotten some catalog names.

  catalogs.clear();

  // Insert possible transliteration fallbacks after each set language.
  QStringList languageListFB;
  foreach (const QString &lang, languageList) {
    languageListFB += lang;
    languageListFB += KTranslit::fallbackList(lang);
  }

  // now iterate over all languages and all wanted catalog names and append or create them in the right order
  // the sequence must be e.g. nds/appname nds/kdelibs nds/kio de/appname de/kdelibs de/kio etc.
  // and not nds/appname de/appname nds/kdelibs de/kdelibs etc. Otherwise we would be in trouble with a language
  // sequende nds,en_US, de. In this case en_US must hide everything below in the language list.
  foreach ( const QString &lang, languageListFB )
    foreach ( const KCatalogName &name, catalogNames )
      // create and add catalog for this name and language if it exists
      if ( ! KCatalog::catalogLocaleDir( name.name, lang ).isEmpty() )
      {
        catalogs.append( KCatalog( name.name, lang ) );
        //kDebug(173) << "Catalog: " << name << ":" << lang;
      }

  // notify KLocalizedString of catalog update.
  KLocalizedString::notifyCatalogsUpdated(languageListFB, catalogNames);
}

void KLocale::removeCatalog(const QString &catalog)
{
    int pos = d->catalogNames.indexOf(KCatalogName(catalog));
    if (pos == -1)
        return;
    if (--d->catalogNames[pos].loadCount > 0)
        return;
    d->catalogNames.removeAt(pos);
    if (KGlobal::hasMainComponent())
        d->updateCatalogs();  // walk through the KCatalog instances and weed out everything we no longer need
}

void KLocale::setActiveCatalog(const QString &catalog)
{
    int pos = d->catalogNames.indexOf(KCatalogName(catalog));
    if (pos == -1)
        return;
    d->catalogNames.move(pos, 0);
	d->updateCatalogs();  // walk through the KCatalog instances and adapt to the new order
}

KLocale::~KLocale()
{
  delete d->calendar;
  delete d->languages;
  delete d;
}

void KLocalePrivate::translate_priv(const char *msgctxt,
                                    const char *msgid,
                                    const char *msgid_plural,
                                    unsigned long n,
                                    QString *language,
                                    QString *translation) const
{
  if ( !msgid || !msgid[0] ) {
    kDebug(173) << "KLocale: trying to look up \"\" in catalog. "
                << "Fix the program" << endl;
    language->clear();
    translation->clear();
    return;
  }
  if ( msgctxt && !msgctxt[0] ) {
    kDebug(173) << "KLocale: trying to use \"\" as context to message. "
                << "Fix the program" << endl;
  }
  if ( msgid_plural && !msgid_plural[0] ) {
    kDebug(173) << "KLocale: trying to use \"\" as plural message. "
                << "Fix the program" << endl;
  }

  // determine the fallback string
  QString fallback;
  if ( msgid_plural == NULL )
    fallback = QString::fromUtf8( msgid );
  else {
    if ( n == 1 )
      fallback = QString::fromUtf8( msgid );
    else
      fallback = QString::fromUtf8( msgid_plural );
  }
  if ( language )
    *language = KLocale::defaultLanguage();
  if ( translation )
    *translation = fallback;

  // shortcut evaluation if en_US is main language: do not consult the catalogs
  if ( useDefaultLanguage() )
    return;

  for ( QList<KCatalog>::ConstIterator it = catalogs.begin();
        it != catalogs.end();
        ++it )
  {
    // shortcut evaluation: once we have arrived at en_US (default language) we cannot consult
    // the catalog as it will not have an assiciated mo-file. For this default language we can
    // immediately pick the fallback string.
    if ( (*it).language() == KLocale::defaultLanguage() )
      return;

    QString text;
    if ( msgctxt != NULL && msgid_plural != NULL )
        text = (*it).translateStrict( msgctxt, msgid, msgid_plural, n );
    else if ( msgid_plural != NULL )
        text = (*it).translateStrict( msgid, msgid_plural, n );
    else if ( msgctxt != NULL )
        text = (*it).translateStrict( msgctxt, msgid );
    else
        text = (*it).translateStrict( msgid );

    if ( !text.isEmpty() ) {
      // we found it
      if ( language )
        *language = (*it).language();
      if ( translation )
        *translation = text;
      return;
    }
  }
}

void KLocale::translateRaw(const char* msg,
                           QString *lang, QString *trans) const
{
  d->translate_priv(0, msg, 0, 0, lang, trans);
}

void KLocale::translateRaw(const char *ctxt, const char *msg,
                           QString *lang, QString *trans) const
{
  d->translate_priv(ctxt, msg, 0, 0, lang, trans);
}

void KLocale::translateRaw(const char *singular, const char *plural, unsigned long n,
                           QString *lang, QString *trans) const
{
  d->translate_priv(0, singular, plural, n, lang, trans);
}

void KLocale::translateRaw(const char *ctxt, const char *singular, const char *plural,
                           unsigned long n,
                           QString *lang, QString *trans) const
{
  d->translate_priv(ctxt, singular, plural, n, lang, trans);
}

QString KLocale::translateQt(const char *context, const char *sourceText,
                             const char *comment) const
{
  // Qt's context is normally the name of the class of the method which makes
  // the tr(sourceText) call. However, it can also be manually supplied via
  // translate(context, sourceText) call.
  //
  // Qt's sourceText is the actual message displayed to the user.
  //
  // Qt's comment is an optional argument of tr() and translate(), like
  // tr(sourceText, comment) and translate(context, sourceText, comment).
  //
  // We handle this in the following way:
  //
  // If the comment is given, then it is considered gettext's msgctxt, so a
  // context call is made.
  //
  // If the comment is not given, but context is given, then we treat it as
  // msgctxt only if it was manually supplied (the one in translate()) -- but
  // we don't know this, so we first try a context call, and if translation
  // is not found, we fallback to ordinary call.
  //
  // If neither comment nor context are given, it's just an ordinary call
  // on sourceText.

  if (!sourceText || !sourceText[0]) {
    kDebug(173) << "KLocale: trying to look up \"\" in catalog. "
                << "Fix the program" << endl;
    return QString();
  }

  if (d->useDefaultLanguage()) {
    return QString();
  }

  QString translation;
  QString language;

  // NOTE: Condition (language != defaultLanguage()) means that translation
  // was found, otherwise we got the original string back as translation.

  if (comment && comment[0]) {
    // Comment given, go for context call.
    d->translate_priv(comment, sourceText, 0, 0, &language, &translation);
  }
  else {
    // Comment not given, go for try-fallback with context.
    if (context && context[0]) {
      d->translate_priv(context, sourceText, 0, 0, &language, &translation);
    }
    if (language.isEmpty() || language == defaultLanguage()) {
      d->translate_priv(0, sourceText, 0, 0, &language, &translation);
    }
  }

  if (language != defaultLanguage()) {
    return translation;
  }

  // No proper translation found, return empty according to Qt's expectation.
  return QString();
}

bool KLocale::nounDeclension() const
{
  return d->nounDeclension;
}

bool KLocale::dateMonthNamePossessive() const
{
  return d->dateMonthNamePossessive;
}

int KLocale::weekStartDay() const
{
  return d->weekStartDay;
}

int KLocale::workingWeekStartDay() const
{
  return d->workingWeekStartDay;
}

int KLocale::workingWeekEndDay() const
{
  return d->workingWeekEndDay;
}

int KLocale::weekDayOfPray() const
{
  return d->weekDayOfPray;
}


QString KLocale::decimalSymbol() const
{
  return d->decimalSymbol;
}

QString KLocale::thousandsSeparator() const
{
  return d->thousandsSeparator;
}

QString KLocale::currencySymbol() const
{
  return d->currencySymbol;
}

QString KLocale::monetaryDecimalSymbol() const
{
  return d->monetaryDecimalSymbol;
}

QString KLocale::monetaryThousandsSeparator() const
{
  return d->monetaryThousandsSeparator;
}

QString KLocale::positiveSign() const
{
  return d->positiveSign;
}

QString KLocale::negativeSign() const
{
  return d->negativeSign;
}

int KLocale::fracDigits() const
{
  return d->fracDigits;
}

bool KLocale::positivePrefixCurrencySymbol() const
{
  return d->positivePrefixCurrencySymbol;
}

bool KLocale::negativePrefixCurrencySymbol() const
{
  return d->negativePrefixCurrencySymbol;
}

KLocale::SignPosition KLocale::positiveMonetarySignPosition() const
{
  return d->positiveMonetarySignPosition;
}

KLocale::SignPosition KLocale::negativeMonetarySignPosition() const
{
  return d->negativeMonetarySignPosition;
}

static inline void put_it_in( QChar *buffer, int& index, const QString &s )
{
  for ( int l = 0; l < s.length(); l++ )
    buffer[index++] = s.at( l );
}

static inline void put_it_in( QChar *buffer, int& index, int number )
{
  buffer[index++] = number / 10 + '0';
  buffer[index++] = number % 10 + '0';
}

// insert (thousands)-"separator"s into the non-fractional part of str
static void _insertSeparator(QString &str, const QString &separator,
			     const QString &decimalSymbol)
{
  // leave fractional part untouched
  const int decimalSymbolPos = str.indexOf(decimalSymbol);
  const int start = decimalSymbolPos == -1 ? str.length() : decimalSymbolPos;
  for (int pos = start - 3; pos > 0; pos -= 3)
    str.insert(pos, separator);
}

QString KLocale::formatMoney(double num,
			     const QString & symbol,
			     int precision) const
{
  // some defaults
  QString currency = symbol.isNull()
    ? currencySymbol()
    : symbol;
  if (precision < 0) precision = fracDigits();

  // the number itself
  bool neg = num < 0;
  QString res = QString::number(neg?-num:num, 'f', precision);

  // Replace dot with locale decimal separator
  res.replace(QChar('.'), monetaryDecimalSymbol());

  // Insert the thousand separators
  _insertSeparator(res, monetaryThousandsSeparator(), monetaryDecimalSymbol());

  // set some variables we need later
  int signpos = neg
    ? negativeMonetarySignPosition()
    : positiveMonetarySignPosition();
  QString sign = neg
    ? negativeSign()
    : positiveSign();

  switch (signpos)
    {
    case ParensAround:
      res.prepend(QLatin1Char('('));
      res.append (QLatin1Char(')'));
      break;
    case BeforeQuantityMoney:
      res.prepend(sign);
      break;
    case AfterQuantityMoney:
      res.append(sign);
      break;
    case BeforeMoney:
      currency.prepend(sign);
      break;
    case AfterMoney:
      currency.append(sign);
      break;
    }

  if (neg?negativePrefixCurrencySymbol():
      positivePrefixCurrencySymbol())
    {
      res.prepend(QLatin1Char(' '));
      res.prepend(currency);
    } else {
      res.append (QLatin1Char(' '));
      res.append (currency);
    }

  return res;
}


QString KLocale::formatNumber(double num, int precision) const
{
  if (precision == -1) precision = 2;
  // no need to round since QString::number does this for us
  return formatNumber(QString::number(num, 'f', precision), false, 0);
}

QString KLocale::formatLong(long num) const
{
  return formatNumber((double)num, 0);
}


// increase the digit at 'position' by one
static void _inc_by_one(QString &str, int position)
{
  for (int i = position; i >= 0; i--)
    {
      char last_char = str[i].toLatin1();
      switch(last_char)
	{
	case '0':
	  str[i] = '1';
	  break;
	case '1':
	  str[i] = '2';
	  break;
	case '2':
	  str[i] = '3';
	  break;
	case '3':
	  str[i] = '4';
	  break;
	case '4':
	  str[i] = '5';
	  break;
	case '5':
	  str[i] = '6';
	  break;
	case '6':
	  str[i] = '7';
	  break;
	case '7':
	  str[i] = '8';
	  break;
	case '8':
	  str[i] = '9';
	  break;
	case '9':
	  str[i] = '0';
	  if (i == 0) str.prepend('1');
	  continue;
	case '.':
	  continue;
	}
      break;
    }
}

// Cut off if more digits in fractional part than 'precision'
static void _round(QString &str, int precision)
{
  int decimalSymbolPos = str.indexOf('.');

  if (decimalSymbolPos == -1) {
    if (precision == 0)  return;
    else if (precision > 0) // add dot if missing (and needed)
      {
	str.append('.');
	decimalSymbolPos = str.length() - 1;
      }
  }
  // fill up with more than enough zeroes (in case fractional part too short)
  str.reserve(str.length() + precision);
  for (int i = 0; i < precision; ++i)
    str.append('0');

  // Now decide whether to round up or down
  char last_char = str[decimalSymbolPos + precision + 1].toLatin1();
  switch (last_char)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
      // nothing to do, rounding down
      break;
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      _inc_by_one(str, decimalSymbolPos + precision);
      break;
    default:
      break;
    }

  decimalSymbolPos = str.indexOf('.');
  str.truncate(decimalSymbolPos + precision + 1);

  // if precision == 0 delete also '.'
  if (precision == 0) str = str.left(decimalSymbolPos);

  str.squeeze();
}

QString KLocale::formatNumber(const QString &numStr, bool round,
			      int precision) const
{
  QString tmpString = numStr;
  if (round && precision < 0)
    return numStr;

  // Skip the sign (for now)
  const bool neg = (tmpString[0] == '-');
  if (neg || tmpString[0] == '+') tmpString.remove(0, 1);

  //kDebug(173)<<"tmpString:"<<tmpString;

  // Split off exponential part (including 'e'-symbol)
  const int expPos = tmpString.indexOf('e'); // -1 if not found
  QString mantString = tmpString.left(expPos); // entire string if no 'e' found
  QString expString;
  if (expPos > -1) {
    expString = tmpString.mid(expPos); // includes the 'e', or empty if no 'e'
    if (expString.length() == 1)
      expString.clear();
  }

  //kDebug(173)<<"mantString:"<<mantString;
  //kDebug(173)<<"expString:"<<expString;
  if (mantString.isEmpty() || !mantString[0].isDigit()) // invalid number
    mantString = "0";

  if (round)
    _round(mantString, precision);

  // Replace dot with locale decimal separator
  mantString.replace(QChar('.'), decimalSymbol());

  // Insert the thousand separators
  _insertSeparator(mantString, thousandsSeparator(), decimalSymbol());

  // How can we know where we should put the sign?
  mantString.prepend(neg?negativeSign():positiveSign());

  return mantString + expString;
}

// If someone wants the SI-standard prefixes kB/MB/GB/TB, I would recommend
// a hidden kconfig option and getting the code from #57240 into the same
// method, so that all KDE apps use the same unit, instead of letting each app decide.

QString KLocale::formatByteSize( double size ) const
{
    // Per IEC 60027-2

    // Binary prefixes
    //Tebi-byte             TiB             2^40    1,099,511,627,776 bytes
    //Gibi-byte             GiB             2^30    1,073,741,824 bytes
    //Mebi-byte             MiB             2^20    1,048,576 bytes
    //Kibi-byte             KiB             2^10    1,024 bytes

    QString s;
    // Gibi-byte
    if ( size >= 1073741824.0 )
    {
        size /= 1073741824.0;
        if ( size > 1024 ) // Tebi-byte
            s = i18n( "%1 TiB", formatNumber(size / 1024.0, 1));
        else
            s = i18n( "%1 GiB", formatNumber(size, 1));
    }
    // Mebi-byte
    else if ( size >= 1048576.0 )
    {
        size /= 1048576.0;
        s = i18n( "%1 MiB", formatNumber(size, 1));
    }
    // Kibi-byte
    else if ( size >= 1024.0 )
    {
        size /= 1024.0;
        s = i18n( "%1 KiB", formatNumber(size, 1));
    }
    // Just byte
    else if ( size > 0 )
    {
        s = i18n( "%1 B", formatNumber(size, 0));
    }
    // Nothing
    else
    {
        s = i18n( "0 B" );
    }
    return s;
}

QString KLocale::formatDuration( unsigned long mSec) const
{
   if( mSec >= 24*3600000) {
      return i18n( "%1 days", formatNumber( mSec/(24*3600000), 3));
   } else if(mSec >= 3600000)
   {
      return i18n( "%1 hours", formatNumber( mSec/3600000.0, 2));
   } else if(mSec >= 60000)
   {
      return i18n( "%1 minutes", formatNumber( mSec/60000.0, 2));
   } else if(mSec >= 1000)
   {
      return i18n( "%1 seconds", formatNumber( mSec/1000.0, 2));
   }

   return i18n( "%1 milliseconds", formatNumber(mSec, 0));
}

QString KLocalePrivate::formatSingleDuration( DurationType durationType, int n )
{
    switch (durationType) {
        case DaysDurationType:
            return i18ncp("@item:intext", "1 day", "%1 days", n);
        case HoursDurationType:
            return i18ncp("@item:intext", "1 hour", "%1 hours", n);
        case MinutesDurationType:
            return i18ncp("@item:intext", "1 minute", "%1 minutes", n);
        case SecondsDurationType:
            return i18ncp("@item:intext", "1 second", "%1 seconds", n);
    }
    return QString();
}

QString KLocale::prettyFormatDuration( unsigned long mSec ) const
{
    unsigned long ms = mSec;
    int days = ms/(24*3600000);
    ms = ms%(24*3600000);
    int hours = ms/3600000;
    ms = ms%3600000;
    int minutes = ms/60000;
    ms = ms%60000;
    int seconds = qRound(ms/1000.0);

    // Handle correctly problematic case #1 (look at KLocaleTest::prettyFormatDuration()
    // at klocaletest.cpp)
    if (seconds == 60) {
        return prettyFormatDuration(mSec - ms + 60000);
    }

    if (days && hours) {
        return i18nc("@item:intext days and hours. This uses the previous item:intext messages. If this does not fit the grammar of your language please contact the i18n team to solve the problem", "%1 and %2", d->formatSingleDuration(KLocalePrivate::DaysDurationType, days), d->formatSingleDuration(KLocalePrivate::HoursDurationType, hours));
    } else if (days) {
        return d->formatSingleDuration(KLocalePrivate::DaysDurationType, days);
    } else if (hours && minutes) {
        return i18nc("@item:intext hours and minutes. This uses the previous item:intext messages. If this does not fit the grammar of your language please contact the i18n team to solve the problem", "%1 and %2", d->formatSingleDuration(KLocalePrivate::HoursDurationType, hours), d->formatSingleDuration(KLocalePrivate::MinutesDurationType, minutes));
    } else if (hours) {
        return d->formatSingleDuration(KLocalePrivate::HoursDurationType, hours);
    } else if (minutes && seconds) {
        return i18nc("@item:intext minutes and seconds. This uses the previous item:intext messages. If this does not fit the grammar of your language please contact the i18n team to solve the problem", "%1 and %2", d->formatSingleDuration(KLocalePrivate::MinutesDurationType, minutes), d->formatSingleDuration(KLocalePrivate::SecondsDurationType, seconds));
    } else if (minutes) {
        return d->formatSingleDuration(KLocalePrivate::MinutesDurationType, minutes);
    } else {
        return d->formatSingleDuration(KLocalePrivate::SecondsDurationType, seconds);
    }
}

QString KLocale::formatDate(const QDate &pDate, DateFormat format) const
{
  if (format == FancyShortDate || format == FancyLongDate)
  {
    int days = pDate.daysTo(QDate::currentDate());
    if (days >= 0 && days < 7)
      return KLocalePrivate::fancyDate(this, pDate, days);
    format = (format == FancyShortDate) ? ShortDate : LongDate;
  }
  const QString rst = (format == ShortDate) ? dateFormatShort() : dateFormat();

  QString buffer;

  if ( ! pDate.isValid() ) return buffer;

  bool escape = false;

  int year = calendar()->year(pDate);
  int month = calendar()->month(pDate);

  for ( int format_index = 0; format_index < rst.length(); ++format_index )
    {
      if ( !escape )
	{
	  if ( rst.at( format_index ).unicode() == '%' )
	    escape = true;
	  else
	    buffer.append(rst.at(format_index));
	}
      else
	{
	  switch ( rst.at( format_index ).unicode() )
	    {
	    case '%':
	      buffer.append(QLatin1Char('%'));
	      break;
	    case 'Y':  //Long year numeric
	      buffer.append(calendar()->yearString(pDate, KCalendarSystem::LongFormat));
	      break;
	    case 'y':  //Short year numeric
	      buffer.append(calendar()->yearString(pDate, KCalendarSystem::ShortFormat));
	      break;
	    case 'n':  //Short month numeric
              buffer.append(calendar()->monthString(pDate, KCalendarSystem::ShortFormat));
	      break;
	    case 'e':  //Short day numeric
              buffer.append(calendar()->dayString(pDate, KCalendarSystem::ShortFormat));
	      break;
	    case 'm':  //Long month numeric
              buffer.append(calendar()->monthString(pDate, KCalendarSystem::LongFormat));
	      break;
	    case 'b':  //Short month name
	      if (d->dateMonthNamePossessive)
		buffer.append(calendar()->monthName(month, year, KCalendarSystem::ShortNamePossessive));
	      else
		buffer.append(calendar()->monthName(month, year, KCalendarSystem::ShortName));
	      break;
	    case 'B':  //Long month name
	      if (d->dateMonthNamePossessive)
		buffer.append(calendar()->monthName(month, year, KCalendarSystem::LongNamePossessive));
	      else
		buffer.append(calendar()->monthName(month, year, KCalendarSystem::LongName));
	      break;
	    case 'd':  //Long day numeric
              buffer.append(calendar()->dayString(pDate, KCalendarSystem::LongFormat));
	      break;
	    case 'a':  //Short weekday name
	      buffer.append(calendar()->weekDayName(pDate, KCalendarSystem::ShortDayName));
	      break;
	    case 'A':  //Long weekday name
	      buffer.append(calendar()->weekDayName(pDate, KCalendarSystem::LongDayName));
	      break;
	    default:
	      buffer.append(rst.at(format_index));
	      break;
	    }
	  escape = false;
	}
    }
  return buffer;
}

QString KLocalePrivate::fancyDate(const KLocale *locale, const QDate &date, int days)
{
  switch (days)
  {
    case 0:
      return i18n("Today");
    case 1:
      return i18n("Yesterday");
    default:
      return locale->calendar()->weekDayName(date);
  }
}

void KLocale::setMainCatalog(const char *catalog)
{
  maincatalog = catalog;
}

double KLocale::readNumber(const QString &_str, bool * ok) const
{
  QString str = _str.trimmed();
  bool neg = str.indexOf(negativeSign()) == 0;
  if (neg)
    str.remove( 0, negativeSign().length() );

  /* will hold the scientific notation portion of the number.
     Example, with 2.34E+23, exponentialPart == "E+23"
  */
  QString exponentialPart;
  int EPos;

  EPos = str.indexOf('E', 0, Qt::CaseInsensitive);

  if (EPos != -1)
  {
    exponentialPart = str.mid(EPos);
    str = str.left(EPos);
  }

  int pos = str.indexOf(decimalSymbol());
  QString major;
  QString minor;
  if ( pos == -1 )
    major = str;
  else
    {
      major = str.left(pos);
      minor = str.mid(pos + decimalSymbol().length());
    }

  // Remove thousand separators
  int thlen = thousandsSeparator().length();
  int lastpos = 0;
  while ( ( pos = major.indexOf( thousandsSeparator() ) ) > 0 )
  {
    // e.g. 12,,345,,678,,922 Acceptable positions (from the end) are 5, 10, 15... i.e. (3+thlen)*N
    int fromEnd = major.length() - pos;
    if ( fromEnd % (3+thlen) != 0 // Needs to be a multiple, otherwise it's an error
        || pos - lastpos > 3 // More than 3 digits between two separators -> error
        || pos == 0          // Can't start with a separator
        || (lastpos>0 && pos-lastpos!=3))   // Must have exactly 3 digits between two separators
    {
      if (ok) *ok = false;
      return 0.0;
    }

    lastpos = pos;
    major.remove( pos, thlen );
  }
  if (lastpos>0 && major.length()-lastpos!=3)   // Must have exactly 3 digits after the last separator
  {
    if (ok) *ok = false;
    return 0.0;
  }

  QString tot;
  if (neg) tot = '-';

  tot += major + '.' + minor + exponentialPart;

  return tot.toDouble(ok);
}

double KLocale::readMoney(const QString &_str, bool * ok) const
{
  QString str = _str.trimmed();
  bool neg = false;
  bool currencyFound = false;
  QString symbol = currencySymbol();

  // First try removing currency symbol from either end
  int pos = str.indexOf(symbol);
  if ( pos == 0 || pos == (int) str.length()-symbol.length() )
    {
      str.remove(pos,symbol.length());
      str = str.trimmed();
      currencyFound = true;
    }
  if (str.isEmpty())
    {
      if (ok) *ok = false;
      return 0;
    }
  // Then try removing negative sign from either end
  // (with a special case for parenthesis)
  if (negativeMonetarySignPosition() == ParensAround)
    {
      if (str[0] == '(' && str[str.length()-1] == ')')
        {
	  neg = true;
	  str.remove(str.length()-1,1);
	  str.remove(0,1);
        }
    }
  else
    {
      int i1 = str.indexOf(negativeSign());
      if ( i1 == 0 || i1 == (int) str.length()-1 )
        {
	  neg = true;
	  str.remove(i1,negativeSign().length());
        }
    }
  if (neg) str = str.trimmed();

  // Finally try again for the currency symbol, if we didn't find
  // it already (because of the negative sign being in the way).
  if ( !currencyFound )
    {
      pos = str.indexOf(symbol);
      if ( pos == 0 || pos == (int) str.length()-symbol.length() )
        {
	  str.remove(pos,symbol.length());
	  str = str.trimmed();
        }
    }

  // And parse the rest as a number
  pos = str.indexOf(monetaryDecimalSymbol());
  QString major;
  QString minior;
  if (pos == -1)
    major = str;
  else
    {
      major = str.left(pos);
      minior = str.mid(pos + monetaryDecimalSymbol().length());
    }

  // Remove thousand separators
  int thlen = monetaryThousandsSeparator().length();
  int lastpos = 0;
  while ( ( pos = major.indexOf( monetaryThousandsSeparator() ) ) > 0 )
  {
    // e.g. 12,,345,,678,,922 Acceptable positions (from the end) are 5, 10, 15... i.e. (3+thlen)*N
    int fromEnd = major.length() - pos;
    if ( fromEnd % (3+thlen) != 0 // Needs to be a multiple, otherwise it's an error
        || pos - lastpos > 3 // More than 3 digits between two separators -> error
        || pos == 0          // Can't start with a separator
        || (lastpos>0 && pos-lastpos!=3))   // Must have exactly 3 digits between two separators
    {
      if (ok) *ok = false;
      return 0.0;
    }
    lastpos = pos;
    major.remove( pos, thlen );
  }
  if (lastpos>0 && major.length()-lastpos!=3)   // Must have exactly 3 digits after the last separator
  {
    if (ok) *ok = false;
    return 0.0;
  }

  QString tot;
  if (neg) tot = '-';
  tot += major + '.' + minior;
  return tot.toDouble(ok);
}

/**
 * helper function to read integers
 * @param str
 * @param pos the position to start at. It will be updated when we parse it.
 * @return the integer read in the string, or -1 if no string
 */
static int readInt(const QString &str, int &pos)
{
  if (!str.at(pos).isDigit()) return -1;
  int result = 0;
  for (; str.length() > pos && str.at(pos).isDigit(); ++pos)
    {
      result *= 10;
      result += str.at(pos).digitValue();
    }

  return result;
}

QDate KLocale::readDate(const QString &intstr, bool* ok) const
{
  QDate date;
  date = readDate(intstr, ShortFormat, ok);
  if (date.isValid()) return date;
  return readDate(intstr, NormalFormat, ok);
}

QDate KLocale::readDate(const QString &intstr, ReadDateFlags flags, bool* ok) const
{
  QString fmt = ((flags & ShortFormat) ? dateFormatShort() : dateFormat()).simplified();
  return readDate( intstr, fmt, ok );
}

QDate KLocale::readDate(const QString &intstr, const QString &fmt, bool* ok) const
{
    //kDebug(173) << "KLocale::readDate intstr=" << intstr << " fmt=" << fmt;
    QString str = intstr.simplified().toLower();
    int day = -1, month = -1;
    // allow the year to be omitted if not in the format
    int year = calendar()->year(QDate::currentDate());
    int strpos = 0;
    int fmtpos = 0;

    int iLength; // Temporary variable used when reading input

    bool error = false;

    while (fmt.length() > fmtpos && str.length() > strpos && !error) {

        QChar c = fmt.at(fmtpos++);
        if (c != '%') {
            if (c.isSpace() && str.at(strpos).isSpace())
                strpos++;
            else if (c != str.at(strpos++))
                error = true;
        } else {
            int j;
            // remove space at the beginning
            if (str.length() > strpos && str.at(strpos).isSpace())
                strpos++;

            c = fmt.at(fmtpos++);
            switch (c.unicode())
            {
            case 'a':
            case 'A':
                error = true;
                j = 1;
                while (error && (j < 8)) {
                    QString s;
                    if ( c == 'a') {
                        s = calendar()->weekDayName(j, KCalendarSystem::ShortDayName).toLower();
                    } else {
                        s = calendar()->weekDayName(j, KCalendarSystem::LongDayName).toLower();
                    }
                    int len = s.length();
                    if (str.mid(strpos, len) == s)
                    {
                        strpos += len;
                        error = false;
                    }
                    j++;
                }
                break;
            case 'b':
            case 'B':
                error = true;
                if (d->dateMonthNamePossessive) {
                    j = 1;
                    while (error && (j < 13)) {
                        QString s;
                        if ( c == 'b' ) {
                            s = calendar()->monthName(j, year, KCalendarSystem::ShortNamePossessive).toLower();
                        } else {
                            s = calendar()->monthName(j, year, KCalendarSystem::LongNamePossessive).toLower();
                        }
                        int len = s.length();
                        if (str.mid(strpos, len) == s) {
                            month = j;
                            strpos += len;
                            error = false;
                        }
                        j++;
                    }
                }
                j = 1;
                while (error && (j < 13)) {
                    QString s;
                    if ( c == 'b' ) {
                        s = calendar()->monthName(j, year, KCalendarSystem::ShortName).toLower();
                    } else {
                        s = calendar()->monthName(j, year, KCalendarSystem::LongName).toLower();
                    }
                    int len = s.length();
                    if (str.mid(strpos, len) == s) {
                        month = j;
                        strpos += len;
                        error = false;
                    }
                    j++;
                }
                break;
            case 'd':
            case 'e':
                day = calendar()->dayStringToInteger(str.mid(strpos), iLength);
                strpos += iLength;

                error = iLength <= 0;
                break;

            case 'n':
            case 'm':
                month = calendar()->monthStringToInteger(str.mid(strpos), iLength);
                strpos += iLength;

                error = iLength <= 0;
                break;

            case 'Y':
            case 'y':
                year = calendar()->yearStringToInteger(str.mid(strpos), iLength);
                strpos += iLength;
                if (c == 'y' && year < 100)
                    year += 2000; // QDate assumes 19xx by default, but this isn't what users want...

                error = iLength <= 0;
                break;
            }
        }
    }

    /* for a match, we should reach the end of both strings, not just one of
       them */
    if (fmt.length() > fmtpos || str.length() > strpos) {
        error = true;
    }

    //kDebug(173) << "KLocale::readDate day=" << day << " month=" << month << " year=" << year;
    if (year != -1 && month != -1 && day != -1 && !error) {
        if (ok) *ok = true;

        QDate result;
        calendar()->setYMD(result, year, month, day);

        return result;
    }

    if (ok) *ok = false;
    return QDate(); // invalid date
}

QTime KLocale::readTime(const QString &intstr, bool *ok) const
{
  QTime _time;
  _time = readTime(intstr, WithSeconds, ok);
  if (_time.isValid()) return _time;
  return readTime(intstr, WithoutSeconds, ok);
}

QTime KLocale::readTime(const QString &intstr, ReadTimeFlags flags, bool *ok) const
{
    QString str = intstr.simplified().toLower();
    QString Format = timeFormat().simplified();
    if (flags & WithoutSeconds)
        Format.remove(QRegExp(".%S"));

    int hour = -1, minute = -1;
    int second = ( (flags & WithoutSeconds) == 0 ) ? -1 : 0; // don't require seconds
    bool g_12h = false;
    bool pm = false;
    int strpos = 0;
    int Formatpos = 0;

    while (Format.length() > Formatpos || str.length() > strpos) {
        if ( !(Format.length() > Formatpos && str.length() > strpos) )
            goto error;

        QChar c = Format.at(Formatpos++);

        if (c != '%') {
            if (c.isSpace())
                strpos++;
            else if (c != str.at(strpos++))
                goto error;
            continue;
	}

        // remove space at the beginning
        if (str.length() > strpos && str.at(strpos).isSpace())
            strpos++;

        c = Format.at(Formatpos++);
        switch (c.unicode()) {
	case 'p':
        {
	    QString s = i18n("pm").toLower();
	    int len = s.length();
	    if (str.mid(strpos, len) == s) {
		pm = true;
		strpos += len;
            } else {
		s = i18n("am").toLower();
		len = s.length();
		if (str.mid(strpos, len) == s) {
                    pm = false;
                    strpos += len;
		} else
                    goto error;
            }
        }
        break;

	case 'k':
	case 'H':
            g_12h = false;
            hour = readInt(str, strpos);
            if (hour < 0 || hour > 23)
                goto error;

            break;

	case 'l':
	case 'I':
            g_12h = true;
            hour = readInt(str, strpos);
            if (hour < 1 || hour > 12)
                goto error;

            break;

	case 'M':
            minute = readInt(str, strpos);
            if (minute < 0 || minute > 59)
                goto error;

            break;

	case 'S':
            second = readInt(str, strpos);
            if (second < 0 || second > 59)
                goto error;

            break;
	}
    }
    if (g_12h) {
        hour %= 12;
        if (pm) hour += 12;
    }

    if (ok) *ok = true;
    return QTime(hour, minute, second);

error:
    if (ok) *ok = false;
    return QTime(); // return invalid date if it didn't work
}

QString KLocale::formatTime(const QTime &pTime, bool includeSecs, bool isDuration) const
{
  const QString rst = timeFormat();

  // only "pm/am" here can grow, the rest shrinks, but
  // I'm rather safe than sorry
  QChar *buffer = new QChar[rst.length() * 3 / 2 + 30];

  int index = 0;
  bool escape = false;
  int number = 0;

  for ( int format_index = 0; format_index < rst.length(); format_index++ )
    {
      if ( !escape )
	{
	  if ( rst.at( format_index ).unicode() == '%' )
	    escape = true;
	  else
	    buffer[index++] = rst.at( format_index );
	}
      else
	{
	  switch ( rst.at( format_index ).unicode() )
	    {
	    case '%':
	      buffer[index++] = '%';
	      break;
	    case 'H':
	      put_it_in( buffer, index, pTime.hour() );
	      break;
	    case 'I':
	      if ( isDuration )
	          put_it_in( buffer, index, pTime.hour() );
	      else
	          put_it_in( buffer, index, ( pTime.hour() + 11) % 12 + 1 );
	      break;
	    case 'M':
	      put_it_in( buffer, index, pTime.minute() );
	      break;
	    case 'S':
	      if (includeSecs)
		put_it_in( buffer, index, pTime.second() );
	      else if (index > 0) {
                  // remove spaces (#164813)
                  while(index > 0 && buffer[index-1].isSpace())
                    --index;
		  // we remove the separator sign before the seconds and
		  // assume that works everywhere
		  --index;
                  // remove spaces (#164813)
                  while(index > 0 && buffer[index-1].isSpace())
                    --index;
		  break;
		}
	      break;
	    case 'k':
	      number = pTime.hour();
	    case 'l':
	      // to share the code
	      if ( rst.at( format_index ).unicode() == 'l' )
		number = isDuration ? pTime.hour() : (pTime.hour() + 11) % 12 + 1;
	      if ( number / 10 )
		buffer[index++] = number / 10 + '0';
	      buffer[index++] = number % 10 + '0';
	      break;
	    case 'p':
	      if ( !isDuration )
	      {
		QString s;
		if ( pTime.hour() >= 12 )
		  put_it_in( buffer, index, i18n("pm") );
		else
		  put_it_in( buffer, index, i18n("am") );
	      }
	      break;
	    default:
	      buffer[index++] = rst.at( format_index );
	      break;
	    }
	  escape = false;
	}
    }
  QString ret( buffer, index );
  delete [] buffer;
  if ( isDuration ) // eliminate trailing-space due to " %p"
    return ret.trimmed();
  else
    return ret;
}

bool KLocale::use12Clock() const
{
  if ((timeFormat().contains(QString::fromLatin1("%I")) > 0) ||
      (timeFormat().contains(QString::fromLatin1("%l")) > 0))
    return true;
  else
    return false;
}

QStringList KLocale::languageList() const
{
  return d->languageList;
}

QString KLocalePrivate::formatDateTime(const KLocale *locale, const QDateTime &dateTime,
                                KLocale::DateFormat format, bool includeSeconds, int daysTo)
{
  QString dateStr = (format == KLocale::FancyShortDate || format == KLocale::FancyLongDate)
                  ? KLocalePrivate::fancyDate(locale, dateTime.date(), daysTo)
                  : locale->formatDate(dateTime.date(), format);
  return i18nc("concatenation of dates and time", "%1 %2",
               dateStr, locale->formatTime(dateTime.time(), includeSeconds));
}

QString KLocale::formatDateTime(const QDateTime &dateTime, DateFormat format,
                                bool includeSeconds) const
{
  QString dateStr;
  int days = -1;
  if (format == FancyShortDate || format == FancyLongDate)
  {
    QDateTime now = QDateTime::currentDateTime();
    days = dateTime.date().daysTo(now.date());
    if ((days == 0 && now.secsTo(dateTime) <= 3600)   // not more than an hour in the future
    ||  (days > 0 && days < 7))
      ;  // use fancy date format
    else
      format = (format == FancyShortDate) ? ShortDate : LongDate;  // fancy date not applicable
  }
  return KLocalePrivate::formatDateTime(this, dateTime, format, includeSeconds, days);
}

QString KLocale::formatDateTime(const KDateTime &dateTime, DateFormat format,
                                DateTimeFormatOptions options) const
{
  QString dt;
  if (dateTime.isDateOnly())
    dt = formatDate( dateTime.date(), format );
  else
  {
    int days = -1;
    if (format == FancyShortDate || format == FancyLongDate)
    {
      // Use the time specification (i.e. time zone, etc.) of 'dateTime' to
      // check whether it's less than a week ago.
      KDateTime now = KDateTime::currentDateTime(dateTime.timeSpec());
      days = dateTime.date().daysTo(now.date());
      if ((days == 0 && now.secsTo(dateTime) <= 3600)   // not more than an hour in the future
      ||  (days > 0 && days < 7))
        ;  // use fancy date format
      else
        format = (format == FancyShortDate) ? ShortDate : LongDate;  // fancy date not applicable
    }
    dt = KLocalePrivate::formatDateTime(this, dateTime.dateTime(), format, (options & Seconds), days);
  }
  if (options & TimeZone)
  {
    QString tz;
    switch (dateTime.timeType())
    {
      case KDateTime::OffsetFromUTC:
        tz = i18n(dateTime.toString("%z").toUtf8());
        break;
      case KDateTime::UTC:
      case KDateTime::TimeZone:
        tz = i18n(dateTime.toString((format == ShortDate) ? "%Z" : "%:Z").toUtf8());
        break;
      case KDateTime::ClockTime:
      default:
        break;
    }
    return i18nc( "concatenation of date/time and time zone", "%1 %2", dt, tz );
  }
  else
    return dt;
}

QString KLocale::langLookup(const QString &fname, const char *rtype)
{
  QStringList search;

  // assemble the local search paths
  const QStringList localDoc = KGlobal::dirs()->resourceDirs(rtype);

  // look up the different languages
  for (int id=localDoc.count()-1; id >= 0; --id)
    {
      QStringList langs = KGlobal::locale()->languageList();
      langs.append( "en" );
      langs.removeAll( defaultLanguage() );
      Q_FOREACH(const QString &lang, langs)
        search.append(QString("%1%2/%3").arg(localDoc[id]).arg(lang).arg(fname));
    }

  // try to locate the file
  Q_FOREACH (const QString &file, search)
    {
      kDebug(173) << "Looking for help in: " << file;

      QFileInfo info(file);
      if (info.exists() && info.isFile() && info.isReadable())
        return file;
    }

  return QString();
}

bool KLocalePrivate::useDefaultLanguage() const
{
  return language == KLocale::defaultLanguage();
}

void KLocalePrivate::initEncoding()
{
  codecForEncoding = 0;

  // This all made more sense when we still had the EncodingEnum config key.
#if defined(HAVE_LANGINFO_H) && !defined(Q_OS_WIN)
  // Qt since 4.2 always returns 'System' as codecForLocale and KDE (for example KEncodingFileDialog)
  // expects real encoding name. So on systems that have langinfo.h use nl_langinfo instead,
  // just like Qt compiled without iconv does. Windows already has its own workaround
  QByteArray systemLocale = nl_langinfo(CODESET);

  if (systemLocale == "ANSI_X3.4-1968") // means ascii, "C"; QTextCodec doesn't know, so avoid warning
    systemLocale = "ISO-8859-1";

  QTextCodec* codec = QTextCodec::codecForName(systemLocale);
  if ( codec )
    setEncoding( codec->mibEnum() );
#else
  setEncoding( QTextCodec::codecForLocale()->mibEnum() );
#endif

  if ( !codecForEncoding ) {
      kWarning() << "Cannot resolve system encoding, defaulting to ISO 8859-1.";
      const int mibDefault = 4; // ISO 8859-1
      setEncoding(mibDefault);
    }

  Q_ASSERT( codecForEncoding );
}

void KLocalePrivate::initFileNameEncoding()
{
  // If the following environment variable is set, assume all filenames
  // are in UTF-8 regardless of the current C locale.
  utf8FileEncoding = !qgetenv("KDE_UTF8_FILENAMES").isEmpty();
  if (utf8FileEncoding)
  {
    QFile::setEncodingFunction(KLocalePrivate::encodeFileNameUTF8);
    QFile::setDecodingFunction(KLocalePrivate::decodeFileNameUTF8);
  }
  // Otherwise, stay with QFile's default filename encoding functions
  // which, on Unix platforms, use the locale's codec.
}

QByteArray KLocalePrivate::encodeFileNameUTF8( const QString & fileName )
{
  return fileName.toUtf8();
}

QString KLocalePrivate::decodeFileNameUTF8( const QByteArray & localFileName )
{
  return QString::fromUtf8(localFileName);
}

void KLocale::setDateFormat(const QString & format)
{
  d->dateFormat = format.trimmed();
}

void KLocale::setDateFormatShort(const QString & format)
{
  d->dateFormatShort = format.trimmed();
}

void KLocale::setDateMonthNamePossessive(bool possessive)
{
  d->dateMonthNamePossessive = possessive;
}

void KLocale::setTimeFormat(const QString & format)
{
  d->timeFormat = format.trimmed();
}

void KLocale::setWeekStartDay(int day)
{
  if (day >= 1 && day <= calendar()->daysInWeek(QDate()))
    d->weekStartDay = day;
}

void KLocale::setWorkingWeekStartDay(int day)
{
  if (day >= 1 && day <= calendar()->daysInWeek(QDate()))
    d->workingWeekStartDay = day;
}

void KLocale::setWorkingWeekEndDay(int day)
{
  if (day >= 1 && day <= calendar()->daysInWeek(QDate()))
    d->workingWeekEndDay = day;
}

void KLocale::setWeekDayOfPray(int day)
{
  if (day >= 0 && day <= calendar()->daysInWeek(QDate()))  // 0 = None
    d->weekDayOfPray = day;
}

QString KLocale::dateFormat() const
{
  return d->dateFormat;
}

QString KLocale::dateFormatShort() const
{
  return d->dateFormatShort;
}

QString KLocale::timeFormat() const
{
  return d->timeFormat;
}

void KLocale::setDecimalSymbol(const QString & symbol)
{
  d->decimalSymbol = symbol.trimmed();
}

void KLocale::setThousandsSeparator(const QString & separator)
{
  // allow spaces here
  d->thousandsSeparator = separator;
}

void KLocale::setPositiveSign(const QString & sign)
{
  d->positiveSign = sign.trimmed();
}

void KLocale::setNegativeSign(const QString & sign)
{
  d->negativeSign = sign.trimmed();
}

void KLocale::setPositiveMonetarySignPosition(SignPosition signpos)
{
  d->positiveMonetarySignPosition = signpos;
}

void KLocale::setNegativeMonetarySignPosition(SignPosition signpos)
{
  d->negativeMonetarySignPosition = signpos;
}

void KLocale::setPositivePrefixCurrencySymbol(bool prefix)
{
  d->positivePrefixCurrencySymbol = prefix;
}

void KLocale::setNegativePrefixCurrencySymbol(bool prefix)
{
  d->negativePrefixCurrencySymbol = prefix;
}

void KLocale::setFracDigits(int digits)
{
  d->fracDigits = digits;
}

void KLocale::setMonetaryThousandsSeparator(const QString & separator)
{
  // allow spaces here
  d->monetaryThousandsSeparator = separator;
}

void KLocale::setMonetaryDecimalSymbol(const QString & symbol)
{
  d->monetaryDecimalSymbol = symbol.trimmed();
}

void KLocale::setCurrencySymbol(const QString & symbol)
{
  d->currencySymbol = symbol.trimmed();
}

int KLocale::pageSize() const
{
  return d->pageSize;
}

void KLocale::setPageSize(int size)
{
  // #### check if it's in range??
  d->pageSize = size;
}

KLocale::MeasureSystem KLocale::measureSystem() const
{
  return d->measureSystem;
}

void KLocale::setMeasureSystem(MeasureSystem value)
{
  d->measureSystem = value;
}

QString KLocale::defaultLanguage()
{
  return QString::fromLatin1("en_US");
}

QString KLocale::defaultCountry()
{
  return QString::fromLatin1("C");
}

bool KLocale::useTranscript() const
{
  return d->useTranscript;
}

const QByteArray KLocale::encoding() const
{
#ifdef Q_WS_WIN
  if (0==qstrcmp("System", codecForEncoding()->name()))
  {
    //win32 returns "System" codec name here but KDE apps expect a real name:
    strcpy(d->win32SystemEncoding, "cp ");
    if (GetLocaleInfoA( MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
      LOCALE_IDEFAULTANSICODEPAGE, d->win32SystemEncoding+3, sizeof(d->win32SystemEncoding)-3-1 ))
    {
      return d->win32SystemEncoding;
    }
  }
#endif
  return codecForEncoding()->name();
}

int KLocale::encodingMib() const
{
  return codecForEncoding()->mibEnum();
}

int KLocale::fileEncodingMib() const
{
  if (d->utf8FileEncoding)
     return 106;
  return codecForEncoding()->mibEnum();
}

QTextCodec * KLocale::codecForEncoding() const
{
  return d->codecForEncoding;
}

bool KLocale::setEncoding(int mibEnum)
{
  return d->setEncoding(mibEnum);
}

bool KLocalePrivate::setEncoding(int mibEnum)
{
  QTextCodec * codec = QTextCodec::codecForMib(mibEnum);
  if (codec)
    codecForEncoding = codec;

  return codec != 0;
}

QStringList KLocale::allLanguagesList() const
{
  if (!d->languages)
    d->languages = new KConfig("all_languages", KConfig::NoGlobals, "locale");

  return d->languages->groupList();
}

QString KLocale::languageCodeToName(const QString &language) const
{
  if (!d->languages)
    d->languages = new KConfig("all_languages", KConfig::NoGlobals, "locale");

  KConfigGroup cg(d->languages, language);
  return cg.readEntry("Name");
}

QStringList KLocale::allCountriesList() const
{
  QStringList countries;
  const QStringList paths = KGlobal::dirs()->findAllResources("locale", "l10n/*/entry.desktop");
  for(QStringList::ConstIterator it = paths.begin();
      it != paths.end(); ++it)
  {
    QString code = (*it).mid((*it).length()-16, 2);
    if (code != "/C")
       countries.append(code);
  }
  return countries;
}

QString KLocale::countryCodeToName(const QString &country) const
{
  QString countryName;
  QString entryFile = KStandardDirs::locate("locale", "l10n/"+country.toLower()+"/entry.desktop");
  if (!entryFile.isEmpty()) {
    KConfig cfg(entryFile);
    KConfigGroup cg(&cfg, "KCM Locale");
    countryName = cg.readEntry("Name");
  }
  return countryName;
}

void KLocale::setCalendar(const QString & calType)
{
  d->calendarType = calType;

  delete d->calendar;
  d->calendar = 0;
}

QString KLocale::calendarType() const
{
  return d->calendarType;
}

const KCalendarSystem * KLocale::calendar() const
{
  // Check if it's the correct calendar?!?
  if ( !d->calendar )
    d->calendar = KCalendarSystem::create( d->calendarType, this );

  return d->calendar;
}

KLocale::KLocale(const KLocale & rhs) : d(new KLocalePrivate(*rhs.d))
{
  d->languages = 0; // Don't copy languages
  d->calendar = 0; // Don't copy the calendar
}

KLocale & KLocale::operator=(const KLocale & rhs)
{
  // the assignment operator works here
  *d = *rhs.d;
  d->languages = 0; // Don't copy languages
  d->calendar = 0; // Don't copy the calendar

  return *this;
}

void KLocale::copyCatalogsTo(KLocale *locale)
{
    locale->d->catalogNames = d->catalogNames;
    locale->d->updateCatalogs();
}

QString KLocale::localizedFilePath(const QString &filePath) const
{
    // Stop here if the default language is primary.
    if (d->useDefaultLanguage()) {
        return filePath;
    }

    // Check if l10n sudir is present, stop if not.
    QFileInfo fileInfo(filePath);
    QString locDirPath = fileInfo.path() + "/l10n";
    QFileInfo locDirInfo(locDirPath);
    if (!locDirInfo.isDir()) {
        return filePath;
    }

    // Go through possible localized paths by priority of languages,
    // return first that exists.
    QString fileName = fileInfo.fileName();
    foreach (const QString &lang, d->languageList) {
        // Stop when the default language is reached.
        if (lang == KLocale::defaultLanguage()) {
            return filePath;
        }
        QString locFilePath = locDirPath + '/' + lang + '/' + fileName;
        QFileInfo locFileInfo(locFilePath);
        if (locFileInfo.isFile() && locFileInfo.isReadable()) {
            return locFilePath;
        }
    }

    return filePath;
}

QString KLocale::removeAcceleratorMarker(const QString &label) const
{
    return ::removeAcceleratorMarker(label);
}
