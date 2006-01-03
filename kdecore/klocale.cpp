// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
   Copyright (c) 1997,2001 Stephan Kulow <coolo@kde.org>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1999-2002 Hans Petter Bieker <bieker@kde.org>
   Copyright (c) 2002 Lukas Tinkl <lukas@kde.org>

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

#undef QT3_SUPPORT
#include "config.h"
#include "klocale.h"

#include <stdlib.h> // getenv

#include <qtextcodec.h>
#include <qfile.h>
#include <qprinter.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#include <qregexp.h>

#include "kcatalog.h"
#include "kglobal.h"
#include "kstandarddirs.h"
#include "ksimpleconfig.h"
#include "kinstance.h"
#include "kconfig.h"
#include "kdebug.h"
#include "kcalendarsystem.h"
#include "kcalendarsystemfactory.h"

#ifdef Q_WS_WIN
#include <windows.h>
#endif

static const char * const SYSTEM_MESSAGES = "kdelibs";

static const char *maincatalog = 0;

class KLocale::Private
{
public:
  int weekStartDay;
  QStringList languageList;
  QStringList catalogNames; // list of all catalogs (regardless of language)
  QList<KCatalog> catalogs; // list of all found catalogs, one instance per catalog name and language
  int numberOfSysCatalogs; // number of catalogs that each app draws from
  QString encoding;
  QTextCodec * codecForEncoding;
  KConfigBase * config;
  int pageSize;
  KLocale::MeasureSystem measureSystem;
  QStringList langTwoAlpha;
  KConfigBase * languages;

  QString calendarType;
  KCalendarSystem * calendar;
  QString appName;
  bool nounDeclension:1;
  bool dateMonthNamePossessive:1;
  bool utf8FileEncoding:1;
  bool formatInited:1;
#ifdef Q_WS_WIN
  char win32SystemEncoding[3+7]; //"cp " + lang ID
#endif
};

static KLocale *this_klocale = 0;

KLocale::KLocale( const QString & catalog, KConfigBase * config )
	: d(new Private)
{
  d->config = config;
  d->languages = 0;
  d->calendar = 0;
  d->formatInited = false;

  initEncoding(0);
  initFileNameEncoding(0);

  KConfigBase *cfg = d->config;
  this_klocale = this;
  if (!cfg) cfg = KGlobal::instance()->config();
  this_klocale = 0;
  Q_ASSERT( cfg );

  d->appName = catalog;
  initLanguageList( cfg, config == 0);
  initMainCatalogs(catalog);
}

QString KLocale::_initLanguage(KConfigBase *config)
{
  if (this_klocale)
  {
     this_klocale->initLanguageList(config, true);
     return this_klocale->language();
  }
  return QString();
}

void KLocale::initMainCatalogs(const QString & catalog)
{
  // Use the first non-null string.
  QString mainCatalog = catalog;
  if (maincatalog)
    mainCatalog = QString::fromLatin1(maincatalog);

  if (mainCatalog.isEmpty()) {
    kdDebug(173) << "KLocale instance created called without valid "
                 << "catalog! Give an argument or call setMainCatalog "
                 << "before init" << endl;
  }
  else {
    // do not use insertCatalog here, that would already trigger updateCatalogs
    d->catalogNames.append( mainCatalog );   // application catalog

    // catalogs from which each application can draw translations
    d->numberOfSysCatalogs = 2;
    d->catalogNames.append( SYSTEM_MESSAGES ); // always include kdelibs.mo
    d->catalogNames.append( "kio" );            // always include kio.mo

    updateCatalogs(); // evaluate this for all languages
  }
}

void KLocale::initLanguageList(KConfigBase * config, bool useEnv)
{
  KConfigGroup cg(config, "Locale");

  m_country = cg.readEntry( "Country" );
  if ( m_country.isEmpty() )
    m_country = defaultCountry();

  // Reset the list and add the new languages
  QStringList list;
  if ( useEnv )
    list += QFile::decodeName( ::getenv("KDE_LANG") ).split(':');

  list += cg.readListEntry("Language", ':');

  // same order as setlocale use
  if ( useEnv )
    {
      // HPB: Only run splitLocale on the environment variables..
      QStringList langs;

      langs << QFile::decodeName( ::getenv("LC_ALL") );
      langs << QFile::decodeName( ::getenv("LC_MESSAGES") );
      langs << QFile::decodeName( ::getenv("LANG") );

      foreach (QString lang, langs)
	{
	  QString ln, aCountry, chrset;
	  splitLocale(lang, ln, aCountry, chrset);

	  if (!aCountry.isEmpty()) {
	    list += ln + '_' + aCountry;
	    if (!chrset.isEmpty())
	      list += ln + '_' + aCountry + '.' + chrset;
	  }
	  list += ln;
	  list += lang;
	}
    }

  // now we have a language list -- let's use the first OK language
  setLanguage( list );
}

void KLocale::doFormatInit() const
{
  if ( d->formatInited ) return;

  KLocale * that = const_cast<KLocale *>(this);
  that->initFormat();

  d->formatInited = true;
}

void KLocale::initFormat()
{
  KConfigBase *config = d->config;
  if (!config) config = KGlobal::instance()->config();
  Q_ASSERT( config );

  kdDebug(173) << "KLocale::initFormat" << endl;

  // make sure the config files are read using the correct locale
  // ### Why not add a KConfigBase::setLocale( const KLocale * )?
  // ### Then we could remove this hack
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = this;

  KConfigGroup cg(config, "Locale");

  KSimpleConfig entry(locate("locale",
                             QString::fromLatin1("l10n/%1/entry.desktop")
                             .arg(m_country)), true);
  entry.setGroup("KCM Locale");

  // Numeric
#define readConfigEntry(key, default, save) \
  save = entry.readEntry(key, default); \
  save = cg.readEntry(key, save);

#define readConfigNumEntry(key, default, save, type) \
  save = (type)entry.readEntry(key, QVariant(default)).toInt(); \
  save = (type)cg.readEntry(key, save).toInt();

#define readConfigBoolEntry(key, default, save) \
  save = entry.readEntry(key, QVariant(default)).toBool(); \
  save = cg.readEntry(key, save).toBool();

  readConfigEntry("DecimalSymbol", ".", m_decimalSymbol);
  readConfigEntry("ThousandsSeparator", ",", m_thousandsSeparator);
  m_thousandsSeparator.replace( QString::fromLatin1("$0"), QString() );
  //kdDebug(173) << "m_thousandsSeparator=" << m_thousandsSeparator << endl;

  readConfigEntry("PositiveSign", "", m_positiveSign);
  readConfigEntry("NegativeSign", "-", m_negativeSign);

  // Monetary
  readConfigEntry("CurrencySymbol", "$", m_currencySymbol);
  readConfigEntry("MonetaryDecimalSymbol", ".", m_monetaryDecimalSymbol);
  readConfigEntry("MonetaryThousandsSeparator", ",",
		  m_monetaryThousandsSeparator);
  m_monetaryThousandsSeparator.replace(QString::fromLatin1("$0"), QString());

  readConfigNumEntry("FracDigits", 2, m_fracDigits, int);
  readConfigBoolEntry("PositivePrefixCurrencySymbol", true,
		      m_positivePrefixCurrencySymbol);
  readConfigBoolEntry("NegativePrefixCurrencySymbol", true,
		      m_negativePrefixCurrencySymbol);
  readConfigNumEntry("PositiveMonetarySignPosition", BeforeQuantityMoney,
		     m_positiveMonetarySignPosition, SignPosition);
  readConfigNumEntry("NegativeMonetarySignPosition", ParensAround,
		     m_negativeMonetarySignPosition, SignPosition);

  // Date and time
  readConfigEntry("TimeFormat", "%H:%M:%S", m_timeFormat);
  readConfigEntry("DateFormat", "%A %d %B %Y", m_dateFormat);
  readConfigEntry("DateFormatShort", "%Y-%m-%d", m_dateFormatShort);
  readConfigNumEntry("WeekStartDay", 1, d->weekStartDay, int);

  // other
  readConfigNumEntry("PageSize", QPrinter::A4, d->pageSize,
		     QPrinter::PageSize);
  readConfigNumEntry("MeasureSystem", Metric, d->measureSystem,
		     MeasureSystem);
  readConfigEntry("CalendarSystem", "gregorian", d->calendarType);
  delete d->calendar;
  d->calendar = 0; // ### HPB Is this the correct place?

  //Grammatical
  //Precedence here is l10n / i18n / config file
  KSimpleConfig lang(locate("locale",
			     QString::fromLatin1("%1/entry.desktop")
                             .arg(m_language)), true);
  lang.setGroup("KCM Locale");
#define read3ConfigBoolEntry(key, default, save) \
  save = entry.readEntry(key, QVariant(default)).toBool(); \
  save = lang.readEntry(key, save).toBool(); \
  save = cg.readEntry(key, save).toBool();

  read3ConfigBoolEntry("NounDeclension", false, d->nounDeclension);
  read3ConfigBoolEntry("DateMonthNamePossessive", false,
		       d->dateMonthNamePossessive);

  // end of hack
  KGlobal::_locale = lsave;
}

bool KLocale::setCountry(const QString & aCountry)
{
  // Check if the file exists too??
  if ( aCountry.isEmpty() )
    return false;

  m_country = aCountry;

  d->formatInited = false;

  return true;
}

bool KLocale::setLanguage(const QString & _language)
{
  if ( d->languageList.contains( _language ) ) {
 	 d->languageList.removeAll( _language );
  }
  d->languageList.prepend( _language ); // let us consider this language to be the most important one

  m_language = _language; // remember main language for shortcut evaluation

  // important when called from the outside and harmless when called before
  // populating the catalog name list
  updateCatalogs();

  d->formatInited = false;

  return true; // Maybe the mo-files for this language are empty, but in principle we can speak all languages
}

bool KLocale::setLanguage(const QStringList & languages)
{
  // This list might contain
  // 1) some empty strings that we have to eliminate
  // 2) duplicate entries like in de:fr:de, where we have to keep the first occurrance of a language in order
  //    to preserve the order of precenence of the user
  // 3) languages into which the application is not translated. For those languages we should not even load kdelibs.mo or kio.po.
  //    these langugage have to be dropped. Otherwise we get strange side effects, e.g. with Hebrew:
  //    the right/left switch for languages that write from
  //    right to left (like Hebrew or Arabic) is set in kdelibs.mo. If you only have kdelibs.mo
  //    but nothing from appname.mo, you get a mostly English app with layout from right to left.
  //    That was considered to be a bug by the Hebrew translators.
  QStringList list;
  foreach ( QString language, languages )
    if (    !language.isEmpty()
         && !list.contains( language )
         && isApplicationTranslatedInto( language ) )
      list.append( language );

  if ( list.isEmpty() ) {
	// user picked no language, so we assume he/she speaks English.
	list.append( defaultLanguage() );
  }
  m_language = list.first(); // keep this for shortcut evaluations

  d->languageList = list; // keep this new list of languages to use
  d->langTwoAlpha.clear(); // Flush cache

  // important when called from the outside and harmless when called before populating the
  // catalog name list
  updateCatalogs();

  return true; // we found something. Maybe it's only English, but we found something
}

bool KLocale::isApplicationTranslatedInto( const QString & lang)
{
  if ( lang.isEmpty() ) {
    return false;
  }

  if ( lang == defaultLanguage() ) {
    // en_us is always "installed"
    return true;
  }

  QString appName = d->appName;
  if (maincatalog) {
    appName = QString::fromLatin1(maincatalog);
  }
  return ! KCatalog::catalogLocaleDir( appName, lang ).isEmpty();
}

void KLocale::splitLocale(const QString & aStr,
			  QString & language,
			  QString & country,
			  QString & chrset)
{
  QString str = aStr;

  // just in case, there is another language appended
  int f = str.indexOf(':');
  if (f >= 0)
    str.truncate(f);

  country.clear();
  chrset.clear();
  language.clear();

  f = str.indexOf('.');
  if (f >= 0)
    {
      chrset = str.mid(f + 1);
      str.truncate(f);
    }

  f = str.indexOf('_');
  if (f >= 0)
    {
      country = str.mid(f + 1);
      str.truncate(f);
    }

  language = str;
}

QString KLocale::language() const
{
  return m_language;
}

QString KLocale::country() const
{
  return m_country;
}

void KLocale::insertCatalog( const QString & catalog )
{
  if ( !d->catalogNames.contains( catalog) ) {
    // Insert new catalog just before system catalogs, to preserve the
    // lowest priority of system catalogs.
    d->catalogNames.insert( d->catalogNames.size() - d->numberOfSysCatalogs,
                            catalog );
  }
  updateCatalogs( ); // evaluate the changed list and generate the neccessary KCatalog objects
}

void KLocale::updateCatalogs( )
{
  // some changes have occured. Maybe we have learned or forgotten some languages.
  // Maybe the language precedence has changed.
  // Maybe we have learned or forgotten some catalog names.

  d->catalogs.clear();

  // now iterate over all languages and all wanted catalog names and append or create them in the right order
  // the sequence must be e.g. nds/appname nds/kdelibs nds/kio de/appname de/kdelibs de/kio etc.
  // and not nds/appname de/appname nds/kdelibs de/kdelibs etc. Otherwise we would be in trouble with a language
  // sequende nds,en_US, de. In this case en_US must hide everything below in the language list.
  foreach ( QString lang, d->languageList )
    foreach ( QString name, d->catalogNames )
      // create and add catalog for this name and language if it exists
      if ( ! KCatalog::catalogLocaleDir( name, lang ).isEmpty() )
      {
        d->catalogs.append( KCatalog( name, lang ) );
        //kdDebug(173) << "Catalog: " << name << ":" << lang << endl;
      }
}

void KLocale::removeCatalog(const QString &catalog)
{
  if ( d->catalogNames.contains( catalog )) {
    d->catalogNames.removeAll( catalog );
    if (KGlobal::_instance)
      updateCatalogs();  // walk through the KCatalog instances and weed out everything we no longer need
  }
}

void KLocale::setActiveCatalog(const QString &catalog)
{
  if ( d->catalogNames.contains( catalog ) ) {
	d->catalogNames.removeAll( catalog );
	d->catalogNames.prepend( catalog );
	updateCatalogs();  // walk through the KCatalog instances and adapt to the new order
  }
}

KLocale::~KLocale()
{
  delete d->calendar;
  delete d->languages;
  delete d;
}

QString KLocale::translate_priv(const char *msgctxt,
                                const char *msgid,
                                const char *msgid_plural,
                                unsigned long n,
                                QString *language) const
{
  if ( !msgid || !msgid[0] ) {
    kdWarning() << "KLocale: trying to look up \"\" in catalog. "
                << "Fix the program" << endl;
    return QString();
  }
  if ( msgctxt && !msgctxt[0] ) {
    kdWarning() << "KLocale: trying to use \"\" as context to message. "
                << "Fix the program" << endl;
    return QString();
  }
  if ( msgid_plural && !msgid_plural[0] ) {
    kdWarning() << "KLocale: trying to use \"\" as plural message. "
                << "Fix the program" << endl;
    return QString();
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
    *language = defaultLanguage();

  // shortcut evaluation if en_US is main language: do not consult the catalogs
  if ( useDefaultLanguage() )
    return fallback;

  for ( QList<KCatalog>::ConstIterator it = d->catalogs.begin();
        it != d->catalogs.end();
        ++it )
  {
    // shortcut evaluation: once we have arrived at en_US (default language) we cannot consult
    // the catalog as it will not have an assiciated mo-file. For this default language we can
    // immediately pick the fallback string.
    if ( (*it).language() == defaultLanguage() )
      return fallback;

    QString text;
    if ( msgctxt == NULL ) {
      if (msgid_plural == NULL)
        text = (*it).translate( msgid );
      else
        text = (*it).translate( msgid, msgid_plural, n );
    }
    else {
      if ( msgid_plural == NULL )
        text = (*it).translate( msgctxt, msgid );
      else
        text = (*it).translate( msgctxt, msgid, msgid_plural, n );
    }

    if ( text != fallback ) {
      // we found it
      if ( language )
        *language = (*it).language();
      return text;
    }
  }

  // translation not found, return fallback
  return fallback;
}

QString KLocale::translate(const char* msgid) const
{
  return translate_priv(0, msgid);
}

QString KLocale::translate( const char *index, const char *fallback) const
{
  return translate_priv(index, fallback);
}

static QString put_n_in(const QString &orig, unsigned long n)
{
  QString ret = orig;
  int index = ret.indexOf("%n");
  if (index == -1)
    return ret;
  ret.replace(index, 2, QString::number(n));
  return ret;
}

QString KLocale::translate( const char *singular, const char *plural,
                            unsigned long n ) const
{
  return put_n_in( translate_priv( 0, singular, plural, n ), n );
}

QString KLocale::translateQt( const char *context, const char *source,
			      const char *message) const
{
  if (!source || !source[0]) {
    kdWarning() << "KLocale: trying to look up \"\" in catalog. "
		<< "Fix the program" << endl;
    return QString();
  }

  if ( useDefaultLanguage() ) {
    return QString();
  }

  QString language;
  QString r;

  if ( message && message[0]) {
    r = translate_priv(source, message, 0, 0, &language);
    if (language != defaultLanguage())
      return r;
  }

  if ( context && context[0] && message && message[0]) {
    r = translate_priv(context, message, 0, 0, &language);
    if (language != defaultLanguage())
      return r;
  }

  r = translate_priv(0, source, 0, 0, &language);
  if (language != defaultLanguage())
    return r;
  return QString();
}

bool KLocale::nounDeclension() const
{
  doFormatInit();
  return d->nounDeclension;
}

bool KLocale::dateMonthNamePossessive() const
{
  doFormatInit();
  return d->dateMonthNamePossessive;
}

int KLocale::weekStartDay() const
{
  doFormatInit();
  return d->weekStartDay;
}


QString KLocale::decimalSymbol() const
{
  doFormatInit();
  return m_decimalSymbol;
}

QString KLocale::thousandsSeparator() const
{
  doFormatInit();
  return m_thousandsSeparator;
}

QString KLocale::currencySymbol() const
{
  doFormatInit();
  return m_currencySymbol;
}

QString KLocale::monetaryDecimalSymbol() const
{
  doFormatInit();
  return m_monetaryDecimalSymbol;
}

QString KLocale::monetaryThousandsSeparator() const
{
  doFormatInit();
  return m_monetaryThousandsSeparator;
}

QString KLocale::positiveSign() const
{
  doFormatInit();
  return m_positiveSign;
}

QString KLocale::negativeSign() const
{
  doFormatInit();
  return m_negativeSign;
}

int KLocale::fracDigits() const
{
  doFormatInit();
  return m_fracDigits;
}

bool KLocale::positivePrefixCurrencySymbol() const
{
  doFormatInit();
  return m_positivePrefixCurrencySymbol;
}

bool KLocale::negativePrefixCurrencySymbol() const
{
  doFormatInit();
  return m_negativePrefixCurrencySymbol;
}

KLocale::SignPosition KLocale::positiveMonetarySignPosition() const
{
  doFormatInit();
  return m_positiveMonetarySignPosition;
}

KLocale::SignPosition KLocale::negativeMonetarySignPosition() const
{
  doFormatInit();
  return m_negativeMonetarySignPosition;
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
  QString mainPart = str.section(decimalSymbol, 0, 0);
  QString fracPart = str.section(decimalSymbol, 1, 1,
				 QString::SectionIncludeLeadingSep);
  if (fracPart==decimalSymbol) fracPart.clear();
  for (int pos = mainPart.length() - 3; pos > 0; pos -= 3)
    mainPart.insert(pos, separator);

  str = mainPart + fracPart;
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

  if (decimalSymbolPos == -1)
    if (precision == 0)  return;
    else if (precision > 0) // add dot if missing (and needed)
      {
	str.append('.');
	decimalSymbolPos = str.length() - 1;
      }

  // fill up with more than enough zeroes (in case fractional part too short)
  str.append(QString().fill('0', precision));

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
  if (precision == 0) str = str.section('.', 0, 0);
}

QString KLocale::formatNumber(const QString &numStr, bool round,
			      int precision) const
{
  QString tmpString = numStr;
  if ((round  && precision < 0)  ||
      ! QRegExp("^[+-]?\\d+(\\.\\d+)*(e[+-]?\\d+)?$").exactMatch(tmpString))
    return numStr;


  // Skip the sign (for now)
  bool neg = (tmpString[0] == '-');
  if (neg  ||  tmpString[0] == '+') tmpString.remove(0, 1);

  kdDebug(173)<<"tmpString:"<<tmpString<<endl;

  // Split off exponential part (including 'e'-symbol)
  QString mantString = tmpString.section('e', 0, 0,
					 QString::SectionCaseInsensitiveSeps);
  QString expString = tmpString.section('e', 1, 1,
					QString::SectionCaseInsensitiveSeps |
					QString::SectionIncludeLeadingSep);
  if (expString.length()==1) expString.clear();

  kdDebug(173)<<"mantString:"<<mantString<<endl;
  kdDebug(173)<<"expString:"<<expString<<endl;


  if (round) _round(mantString, precision);

  // Replace dot with locale decimal separator
  mantString.replace(QChar('.'), decimalSymbol());

  // Insert the thousand separators
  _insertSeparator(mantString, thousandsSeparator(), decimalSymbol());

  // How can we know where we should put the sign?
  mantString.prepend(neg?negativeSign():positiveSign());

  return mantString +  expString;
}

QString KLocale::formatDate(const QDate &pDate, bool shortFormat) const
{
  const QString rst = shortFormat?dateFormatShort():dateFormat();

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
	    case 'Y':
	      buffer.append(calendar()->yearString(pDate, false));
	      break;
	    case 'y':
	      buffer.append(calendar()->yearString(pDate, true));
	      break;
	    case 'n':
              buffer.append(calendar()->monthString(pDate, true));
	      break;
	    case 'e':
              buffer.append(calendar()->dayString(pDate, true));
	      break;
	    case 'm':
              buffer.append(calendar()->monthString(pDate, false));
	      break;
	    case 'b':
	      if (d->nounDeclension && d->dateMonthNamePossessive)
		buffer.append(calendar()->monthNamePossessive(month, year, true));
	      else
		buffer.append(calendar()->monthName(month, year, true));
	      break;
	    case 'B':
	      if (d->nounDeclension && d->dateMonthNamePossessive)
		buffer.append(calendar()->monthNamePossessive(month, year, false));
	      else
		buffer.append(calendar()->monthName(month, year, false));
	      break;
	    case 'd':
              buffer.append(calendar()->dayString(pDate, false));
	      break;
	    case 'a':
	      buffer.append(calendar()->weekDayName(pDate, true));
	      break;
	    case 'A':
	      buffer.append(calendar()->weekDayName(pDate, false));
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
  for (; str.length() > pos && str.at(pos).isDigit(); pos++)
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
  //kdDebug(173) << "KLocale::readDate intstr=" << intstr << " fmt=" << fmt << endl;
  QString str = intstr.simplified().toLower();
  int day = -1, month = -1;
  // allow the year to be omitted if not in the format
  int year = calendar()->year(QDate::currentDate());
  int strpos = 0;
  int fmtpos = 0;

  int iLength; // Temporary variable used when reading input

  bool error = false;

  while (fmt.length() > fmtpos && str.length() > strpos && !error)
  {

    QChar c = fmt.at(fmtpos++);

    if (c != '%') {
      if (c.isSpace() && str.at(strpos).isSpace())
        strpos++;
      else if (c != str.at(strpos++))
        error = true;
    }
    else
    {
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
        QString s = calendar()->weekDayName(j, c == 'a').toLower();
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
      if (d->nounDeclension && d->dateMonthNamePossessive) {
        j = 1;
        while (error && (j < 13)) {
          QString s = calendar()->monthNamePossessive(j, year, c == 'b').toLower();
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
        QString s = calendar()->monthName(j, year, c == 'b').toLower();
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

      error = iLength <= 0;
      break;
        }
      }
  }

    /* for a match, we should reach the end of both strings, not just one of
       them */
    if ( fmt.length() > fmtpos || str.length() > strpos )
    {
      error = true;
    }

    //kdDebug(173) << "KLocale::readDate day=" << day << " month=" << month << " year=" << year << endl;
    if ( year != -1 && month != -1 && day != -1 && !error)
    {
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

  while (Format.length() > Formatpos || str.length() > strpos)
    {
      if ( !(Format.length() > Formatpos && str.length() > strpos) ) goto error;

      QChar c = Format.at(Formatpos++);

      if (c != '%')
	{
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
      switch (c.unicode())
	{
	case 'p':
	  {
	    QString s;
	    s = translate("pm").toLower();
	    int len = s.length();
	    if (str.mid(strpos, len) == s)
	      {
		pm = true;
		strpos += len;
	      }
	    else
	      {
		s = translate("am").toLower();
		len = s.length();
		if (str.mid(strpos, len) == s) {
		  pm = false;
		  strpos += len;
		}
		else
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
  // ######## KDE4: remove this
  return QTime(-1, -1, -1); // return invalid date if it didn't work
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
	      else if ( index > 0 )
		{
		  // we remove the separator sign before the seconds and
		  // assume that works everywhere
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
		  put_it_in( buffer, index, translate("pm") );
		else
		  put_it_in( buffer, index, translate("am") );
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

QString KLocale::formatDateTime(const QDateTime &pDateTime,
				bool shortFormat,
				bool includeSeconds) const
{
  return translate("concatenation of dates and time", "%1 %2")
    .arg( formatDate( pDateTime.date(), shortFormat ) )
    .arg( formatTime( pDateTime.time(), includeSeconds ) );
}

QString i18n(const char* text)
{
  register KLocale *instance = KGlobal::locale();
  if (instance)
    return instance->translate(text);
  return QString::fromUtf8(text);
}

QString i18n(const char* index, const char *text)
{
  register KLocale *instance = KGlobal::locale();
  if (instance)
    return instance->translate(index, text);
  return QString::fromUtf8(text);
}

QString i18n(const char* singular, const char* plural, unsigned long n)
{
  register KLocale *instance = KGlobal::locale();
  if (instance)
    return instance->translate(singular, plural, n);
  if (n == 1)
    return put_n_in(QString::fromUtf8(singular), n);
  else
    return put_n_in(QString::fromUtf8(plural), n);
}

void KLocale::initInstance()
{
  if (KGlobal::_locale)
    return;

  KInstance *app = KGlobal::instance();
  if (app) {
    KGlobal::_locale = new KLocale(QString::fromLatin1(app->instanceName()));

    // only do this for the global instance
    QTextCodec::setCodecForLocale(KGlobal::_locale->codecForEncoding());
  }
  else
    kdDebug(173) << "no app name available using KLocale - nothing to do\n";
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
      QStringList::ConstIterator lang;
      for (lang = langs.begin(); lang != langs.end(); ++lang)
	search.append(QString("%1%2/%3").arg(localDoc[id]).arg(*lang).arg(fname));
    }

  // try to locate the file
  QStringList::Iterator it;
  for (it = search.begin(); it != search.end(); ++it)
    {
      kdDebug(173) << "Looking for help in: " << *it << endl;

      QFileInfo info(*it);
      if (info.exists() && info.isFile() && info.isReadable())
	return *it;
    }

  return QString();
}

bool KLocale::useDefaultLanguage() const
{
  return language() == defaultLanguage();
}

void KLocale::initEncoding(KConfigBase *)
{
  const int mibDefault = 4; // ISO 8859-1

  // This all made more sense when we still had the EncodingEnum config key.
  setEncoding( QTextCodec::codecForLocale()->mibEnum() );

  if ( !d->codecForEncoding )
    {
      kdWarning(173) << " Defaulting to ISO 8859-1 encoding." << endl;
      setEncoding(mibDefault);
    }

  Q_ASSERT( d->codecForEncoding );
}

void KLocale::initFileNameEncoding(KConfigBase *)
{
  // If the following environment variable is set, assume all filenames
  // are in UTF-8 regardless of the current C locale.
  d->utf8FileEncoding = getenv("KDE_UTF8_FILENAMES") != 0;
  if (d->utf8FileEncoding)
  {
    QFile::setEncodingFunction(KLocale::encodeFileNameUTF8);
    QFile::setDecodingFunction(KLocale::decodeFileNameUTF8);
  }
  // Otherwise, stay with QFile's default filename encoding functions
  // which, on Unix platforms, use the locale's codec.
}

QByteArray KLocale::encodeFileNameUTF8( const QString & fileName )
{
  return fileName.toUtf8();
}

QString KLocale::decodeFileNameUTF8( const QByteArray & localFileName )
{
  return QString::fromUtf8(localFileName);
}

void KLocale::setDateFormat(const QString & format)
{
  doFormatInit();
  m_dateFormat = format.trimmed();
}

void KLocale::setDateFormatShort(const QString & format)
{
  doFormatInit();
  m_dateFormatShort = format.trimmed();
}

void KLocale::setDateMonthNamePossessive(bool possessive)
{
  doFormatInit();
  d->dateMonthNamePossessive = possessive;
}

void KLocale::setTimeFormat(const QString & format)
{
  doFormatInit();
  m_timeFormat = format.trimmed();
}

void KLocale::setWeekStartDay(int day)
{
  doFormatInit();
  if (day>7 || day<1)
    d->weekStartDay = 1; //Monday is default
  else
    d->weekStartDay = day;
}

QString KLocale::dateFormat() const
{
  doFormatInit();
  return m_dateFormat;
}

QString KLocale::dateFormatShort() const
{
  doFormatInit();
  return m_dateFormatShort;
}

QString KLocale::timeFormat() const
{
  doFormatInit();
  return m_timeFormat;
}

void KLocale::setDecimalSymbol(const QString & symbol)
{
  doFormatInit();
  m_decimalSymbol = symbol.trimmed();
}

void KLocale::setThousandsSeparator(const QString & separator)
{
  doFormatInit();
  // allow spaces here
  m_thousandsSeparator = separator;
}

void KLocale::setPositiveSign(const QString & sign)
{
  doFormatInit();
  m_positiveSign = sign.trimmed();
}

void KLocale::setNegativeSign(const QString & sign)
{
  doFormatInit();
  m_negativeSign = sign.trimmed();
}

void KLocale::setPositiveMonetarySignPosition(SignPosition signpos)
{
  doFormatInit();
  m_positiveMonetarySignPosition = signpos;
}

void KLocale::setNegativeMonetarySignPosition(SignPosition signpos)
{
  doFormatInit();
  m_negativeMonetarySignPosition = signpos;
}

void KLocale::setPositivePrefixCurrencySymbol(bool prefix)
{
  doFormatInit();
  m_positivePrefixCurrencySymbol = prefix;
}

void KLocale::setNegativePrefixCurrencySymbol(bool prefix)
{
  doFormatInit();
  m_negativePrefixCurrencySymbol = prefix;
}

void KLocale::setFracDigits(int digits)
{
  doFormatInit();
  m_fracDigits = digits;
}

void KLocale::setMonetaryThousandsSeparator(const QString & separator)
{
  doFormatInit();
  // allow spaces here
  m_monetaryThousandsSeparator = separator;
}

void KLocale::setMonetaryDecimalSymbol(const QString & symbol)
{
  doFormatInit();
  m_monetaryDecimalSymbol = symbol.trimmed();
}

void KLocale::setCurrencySymbol(const QString & symbol)
{
  doFormatInit();
  m_currencySymbol = symbol.trimmed();
}

int KLocale::pageSize() const
{
  doFormatInit();
  return d->pageSize;
}

void KLocale::setPageSize(int size)
{
  // #### check if it's in range??
  doFormatInit();
  d->pageSize = size;
}

KLocale::MeasureSystem KLocale::measureSystem() const
{
  doFormatInit();
  return d->measureSystem;
}

void KLocale::setMeasureSystem(MeasureSystem value)
{
  doFormatInit();
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

const char * KLocale::encoding() const
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
  QTextCodec * codec = QTextCodec::codecForMib(mibEnum);
  if (codec)
    d->codecForEncoding = codec;

  return codec != 0;
}

QStringList KLocale::languagesTwoAlpha() const
{
  if (d->langTwoAlpha.count())
     return d->langTwoAlpha;

  const QStringList &origList = languageList();

  QStringList result;

  KConfig config(QString::fromLatin1("language.codes"), true, false);
  config.setGroup("TwoLetterCodes");

  for ( QStringList::ConstIterator it = origList.begin();
	it != origList.end();
	++it )
    {
      QString lang = *it;
      QStringList langLst;
      if (config.hasKey( lang ))
         langLst = config.readListEntry( lang );
      else
      {
         int i = lang.indexOf('_');
         if (i >= 0)
            lang.truncate(i);
         langLst << lang;
      }

      for ( QStringList::ConstIterator langIt = langLst.begin();
	    langIt != langLst.end();
	    ++langIt )
	{
	  if ( !(*langIt).isEmpty() && !result.contains( *langIt ) )
	    result += *langIt;
	}
    }
  d->langTwoAlpha = result;
  return result;
}

QStringList KLocale::allLanguagesTwoAlpha() const
{
  if (!d->languages)
    d->languages = new KConfig("all_languages", true, false, "locale");

  return d->languages->groupList();
}

QString KLocale::twoAlphaToLanguageName(const QString &code) const
{
  if (!d->languages)
    d->languages = new KConfig("all_languages", true, false, "locale");

  QString groupName = code;
  int i = groupName.indexOf('_');
  if (i < 0) i = groupName.size();
  groupName.replace(0, i, groupName.left(i).toLower());

  d->languages->setGroup(groupName);
  return d->languages->readEntry("Name");
}

QStringList KLocale::allCountriesTwoAlpha() const
{
  QStringList countries;
  QStringList paths = KGlobal::dirs()->findAllResources("locale", "l10n/*/entry.desktop");
  for(QStringList::ConstIterator it = paths.begin();
      it != paths.end(); ++it)
  {
    QString code = (*it).mid((*it).length()-16, 2);
    if (code != "/C")
       countries.append(code);
  }
  return countries;
}

QString KLocale::twoAlphaToCountryName(const QString &code) const
{
  KConfig cfg("l10n/"+code.toLower()+"/entry.desktop", true, false, "locale");
  cfg.setGroup("KCM Locale");
  return cfg.readEntry("Name");
}

void KLocale::setCalendar(const QString & calType)
{
  doFormatInit();

  d->calendarType = calType;

  delete d->calendar;
  d->calendar = 0;
}

QString KLocale::calendarType() const
{
  doFormatInit();

  return d->calendarType;
}

const KCalendarSystem * KLocale::calendar() const
{
  doFormatInit();

  // Check if it's the correct calendar?!?
  if ( !d->calendar )
    d->calendar = KCalendarSystemFactory::create( d->calendarType, this );

  return d->calendar;
}

KLocale::KLocale(const KLocale & rhs) : d(new Private)
{
  *this = rhs;
}

KLocale & KLocale::operator=(const KLocale & rhs)
{
  // Numbers and money
  m_decimalSymbol = rhs.m_decimalSymbol;
  m_thousandsSeparator = rhs.m_thousandsSeparator;
  m_currencySymbol = rhs.m_currencySymbol;
  m_monetaryDecimalSymbol = rhs.m_monetaryDecimalSymbol;
  m_monetaryThousandsSeparator = rhs.m_monetaryThousandsSeparator;
  m_positiveSign = rhs.m_positiveSign;
  m_negativeSign = rhs.m_negativeSign;
  m_fracDigits = rhs.m_fracDigits;
  m_positivePrefixCurrencySymbol = rhs.m_positivePrefixCurrencySymbol;
  m_negativePrefixCurrencySymbol = rhs.m_negativePrefixCurrencySymbol;
  m_positiveMonetarySignPosition = rhs.m_positiveMonetarySignPosition;
  m_negativeMonetarySignPosition = rhs.m_negativeMonetarySignPosition;

  // Date and time
  m_timeFormat = rhs.m_timeFormat;
  m_dateFormat = rhs.m_dateFormat;
  m_dateFormatShort = rhs.m_dateFormatShort;

  m_language = rhs.m_language;
  m_country = rhs.m_country;

  // the assignment operator works here
  *d = *rhs.d;
  d->languages = 0; // Don't copy languages
  d->calendar = 0; // Don't copy the calendar

  return *this;
}
