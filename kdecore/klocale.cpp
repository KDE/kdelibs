// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
   Copyright (c) 1997,2001 Stephan Kulow <coolo@kde.org>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1999-2001 Hans Petter Bieker <bieker@kde.org>

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

#include <config.h>

#include <stdlib.h> // getenv

#include <qtextcodec.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qfileinfo.h>

#include "kcatalogue.h"
#include "kglobal.h"
#include "kstddirs.h"
#include "ksimpleconfig.h"
#include "kinstance.h"
#include "kconfig.h"
#include "kdebug.h"
#include "klocale.h"
#include "kcharsets.h"

static const char * const SYSTEM_MESSAGES = "kdelibs";

static const char *maincatalogue = 0;

class KLocalePrivate
{
public:
  int plural_form;
  QString language;
  QStringList languageList;
  QValueList<KCatalogue> catalogues;
  QString encoding;
  QTextCodec * codecForEncoding;
  KConfig * config;
  bool formatInited;
};

extern void qt_set_locale_codec( QTextCodec *codec );

KLocale::KLocale( const QString & catalogue, KConfig * config )
{
  d = new KLocalePrivate;
  d->config = config;

  KConfig *cfg = d->config;
  if (!cfg) cfg = KGlobal::instance()->config();

  ASSERT( cfg );

  initCharset(cfg);
  initEncoding(cfg);
  initCatalogue(catalogue);

  initLanguage(cfg, config == 0);
}

void KLocale::initCatalogue(const QString & catalogue)
{
  // Use the first non-null string.
  QString mainCatalogue = catalogue;
  if (maincatalogue)
    mainCatalogue = QString::fromLatin1(maincatalogue);

  if (mainCatalogue.isEmpty()) {
    kdDebug(173) << "KLocale instance created called without valid "
                 << "catalogue! Give an argument or call setMainCatalogue "
                 << "before init" << endl;
  }
  else
    d->catalogues.append( KCatalogue(mainCatalogue ) );

  // always include kdelibs.mo
  d->catalogues.append( KCatalogue( SYSTEM_MESSAGES ) );
}

void KLocale::initLanguage(KConfig * config, bool useEnv)
{
  KConfigGroupSaver saver(config, "Locale");

  m_country = config->readEntry( "Country" );
  if ( m_country.isEmpty() )
    m_country = defaultCountry();

  // Reset the list and add the new languages
  QStringList languageList;
  if ( useEnv )
    languageList += QStringList::split
      (':', QFile::decodeName( ::getenv("KDE_LANG") ));

  languageList += config->readListEntry("Language", ':');

  // same order as setlocale use
  if ( useEnv )
    {
      // HPB: Only run splitLocale on the environment variables..
      QStringList langs;

      langs << QFile::decodeName( ::getenv("LC_MESSAGES") );
      langs << QFile::decodeName( ::getenv("LC_ALL") );
      langs << QFile::decodeName( ::getenv("LANG") );

      for ( QStringList::Iterator it = langs.begin();
	    it != langs.end();
	    ++it )
	{
	  QString ln, ct, chrset;
	  splitLocale(*it, ln, ct, chrset);

	  if (!ct.isEmpty()) {
	    langs.insert(it, ln + '_' + ct);
	    if (!chrset.isEmpty())
	      langs.insert(it, ln + '_' + ct + '.' + chrset);
	  }

	}

      languageList += langs;
    }

  // now we have a language list -- let's use the first OK language
  setLanguage( languageList );
}

void KLocale::doBindInit()
{
  kdDebug(173) << "KLocale::doBindInit" << endl;

  for ( QValueList<KCatalogue>::Iterator it = d->catalogues.begin();
	it != d->catalogues.end();
	++it )
    initCatalogue( *it );

  if ( useDefaultLanguage() )
    d->plural_form = -1;
  else
    {
      QString pf = translate_priv
	( I18N_NOOP("_: Dear translator, please do not translate this string "
		    "in any form, but pick the _right_ value out of "
		    "NoPlural/TwoForms/French.... If not sure what to do mail "
		    "thd@kde.org and coolo@kde.org, they will tell you. "
		    "Better leave that out if unsure, the programs will "
		    "crash!!\nDefinition of PluralForm - to be set by the "
		    "translator of kdelibs.po"), 0);
      if ( pf.isEmpty() ) {
	kdWarning(173) << "found no definition of PluralForm" << endl;
	d->plural_form = -1;
      } else if ( pf == "NoPlural" )
	d->plural_form = 0;
      else if ( pf == "TwoForms" )
	d->plural_form = 1;
      else if ( pf == "French" )
	d->plural_form = 2;
      else if ( pf == "Gaeilge" )
	d->plural_form = 3;
      else if ( pf == "Russian" )
	d->plural_form = 4;
      else if ( pf == "Polish" )
	d->plural_form = 5;
      else if ( pf == "Slovenian" )
	d->plural_form = 6;
      else if ( pf == "Lithuanian" )
	d->plural_form = 7;
      else if ( pf == "Czech" )
	d->plural_form = 8;
      else if ( pf == "Slovak" )
	d->plural_form = 9;
      else if ( pf == "Maltese" )
	d->plural_form = 10;
      else {
	kdWarning(173) << "Definition of PluralForm is none of "
		       << "NoPlural/"
		       << "TwoForms/"
		       << "French/"
		       << "Gaeilge/"
		       << "Russian/"
		       << "Polish/"
		       << "Slovenian/"
		       << "Lithuanian/"
		       << "Czech/"
		       << "Slovak/"
		       << "Maltese: " << pf << endl;
	exit(1);
      }
    }

  d->formatInited = false;
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
  KConfig *config = d->config;
  if (!config) config = KGlobal::instance()->config();
  ASSERT( config );

  kdDebug(173) << "KLocale::initFormat" << endl;

  // make sure the config files are read using the correct locale
  // ### Why not add a KConfigBase::setLocale( const KLocale * )?
  // ### Then we could remove this hack
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = this;

  KConfigGroupSaver saver(config, "Locale");

  KSimpleConfig entry(locate("locale",
                             QString::fromLatin1("l10n/%1/entry.desktop")
                             .arg(m_country)), true);
  entry.setGroup("KCM Locale");

  // Numeric
  m_decimalSymbol = config->readEntry("DecimalSymbol");
  if (m_decimalSymbol.isNull())
    m_decimalSymbol = entry.readEntry("DecimalSymbol",
				     QString::fromLatin1("."));

  m_thousandsSeparator = config->readEntry("ThousandsSeparator");
  if (m_thousandsSeparator.isNull())
    m_thousandsSeparator = entry.readEntry("ThousandsSeparator",
					   QString::fromLatin1(","));
  m_thousandsSeparator.replace( QRegExp(QString::fromLatin1("$0")),
				QString::fromLatin1("") );
  //kdDebug(173) << "m_thousandsSeparator=" << m_thousandsSeparator << endl;

  m_positiveSign = config->readEntry("PositiveSign");
  if (m_positiveSign.isNull())
    m_positiveSign = entry.readEntry("PositiveSign");

  m_negativeSign = config->readEntry("NegativeSign");
  if (m_negativeSign.isNull())
    m_negativeSign = entry.readEntry("NegativeSign",
				     QString::fromLatin1("-"));

  // Monetary
  m_currencySymbol = config->readEntry("CurrencySymbol");
  if (m_currencySymbol.isNull())
    m_currencySymbol = entry.readEntry("CurrencySymbol",
				       QString::fromLatin1("$"));

  m_monetaryDecimalSymbol = config->readEntry("MonetaryDecimalSymbol");
  if (m_monetaryDecimalSymbol.isNull())
    m_monetaryDecimalSymbol = entry.readEntry("MonetaryDecimalSymbol",
					      QString::fromLatin1("."));

  m_monetaryThousandsSeparator = config->readEntry("MonetaryThousendSeparator");
  if (m_monetaryThousandsSeparator.isNull())
    m_monetaryThousandsSeparator = entry.readEntry("MonetaryThousandsSeparator",
						   QString::fromLatin1(","));
  m_monetaryThousandsSeparator.replace(QRegExp(QString::fromLatin1("$0")),
				       QString::null);

  m_fracDigits = config->readNumEntry("FractDigits", -1);
  if (m_fracDigits == -1)
    m_fracDigits = entry.readNumEntry("FractDigits", 2);

  m_positivePrefixCurrencySymbol = entry.readBoolEntry("PositivePrefixCurrencySymbol", true);
  m_positivePrefixCurrencySymbol = config->readNumEntry("PositivePrefixCurrencySymbol", m_positivePrefixCurrencySymbol);

  m_negativePrefixCurrencySymbol = entry.readBoolEntry("NegativePrefixCurrencySymbol", true);
  m_negativePrefixCurrencySymbol = config->readNumEntry("NegativePrefixCurrencySymbol", m_negativePrefixCurrencySymbol);

  m_positiveMonetarySignPosition = (SignPosition)config->readNumEntry("PositiveMonetarySignPosition", -1);
  if (m_positiveMonetarySignPosition == -1)
    m_positiveMonetarySignPosition = (SignPosition)entry.readNumEntry("PositiveMonetarySignPosition", BeforeQuantityMoney);

  m_negativeMonetarySignPosition = (SignPosition)config->readNumEntry("NegativeMonetarySignPosition", -1);
  if (m_negativeMonetarySignPosition == -1)
    m_negativeMonetarySignPosition = (SignPosition)entry.readNumEntry("NegativeMonetarySignPosition", ParensAround);

  // Date and time
  m_timeFormat = config->readEntry("TimeFormat");
  if (m_timeFormat.isNull())
    m_timeFormat = entry.readEntry("TimeFormat",
				   QString::fromLatin1("%H:%M:%S"));

  m_dateFormat = config->readEntry("DateFormat");
  if (m_dateFormat.isNull())
    m_dateFormat = entry.readEntry("DateFormat",
				   QString::fromLatin1("%A %d %B %Y"));

  m_dateFormatShort = config->readEntry("DateFormatShort");
  if (m_dateFormatShort.isNull())
    m_dateFormatShort = entry.readEntry("DateFormatShort",
				    QString::fromLatin1("%Y-%m-%d"));

  m_weekStartsMonday = entry.readBoolEntry("WeekStartsMonday", true);
  m_weekStartsMonday = config->readBoolEntry("WeekStartsMonday",
					     m_weekStartsMonday);

  // end of hack
  KGlobal::_locale = lsave;
}

bool KLocale::setCountry(const QString & country)
{
  // Check if the file exist too??
  if ( country.isEmpty() )
    return false;

  m_country = country;

  d->formatInited = false;

  return true;
}

QString KLocale::catalogueFileName(const QString & language,
				   const KCatalogue & catalogue)
{
  QString path = QString::fromLatin1("%1/LC_MESSAGES/%2.mo")
    .arg( language )
    .arg( catalogue.name() );

  return locate( "locale", path );
}

bool KLocale::isLanguageInstalled(const QString & language) const
{
  // Do not allow empty languages
  if ( language.isEmpty() ) return false;

  bool bRes = true;
  if ( language != defaultLanguage() )
    for ( QValueList<KCatalogue>::ConstIterator it = d->catalogues.begin();
	  it != d->catalogues.end() && bRes;
	  ++it )
      {
	bRes = !catalogueFileName( language, *it ).isNull();
        if ( !bRes )
	  kdDebug(173) << "message catalogue not found: "
		       << (*it).name() << endl;
      }

  return bRes;
}

bool KLocale::setLanguage(const QString & language)
{
  bool bRes = isLanguageInstalled( language );

  if ( bRes )
    {
      d->language = language;

      doBindInit();
    }

  return bRes;
}

bool KLocale::setLanguage(const QStringList & languages)
{
  QStringList languageList(languages);

  // Remove duplicate entries in reverse so that we
  // can keep user's language preference order intact. (DA)
  for( QStringList::Iterator it = languageList.fromLast();
         it != languageList.begin();
         --it )
    if ( languageList.contains(*it) > 1 || (*it).isEmpty() )
      it = languageList.remove( it );

  bool bRes = false;
  for ( QStringList::ConstIterator it = languageList.begin();
	it != languageList.end();
	++it )
    if ( bRes = setLanguage( *it ) )
      break;

  if ( !bRes )
    setLanguage(defaultLanguage());

  d->languageList = languageList;

  return bRes;
}

void KLocale::splitLocale(const QString & aStr,
			  QString & language,
			  QString & country,
			  QString & chrset)
{
  QString str = aStr;

  // just in case, there is another language appended
  int f = str.find(':');
  if (f >= 0)
    str.truncate(f);

  country = QString::null;
  chrset = QString::null;
  language = QString::null;

  f = str.find('.');
  if (f >= 0)
    {
      chrset = str.mid(f + 1);
      str.truncate(f);
    }

  f = str.find('_');
  if (f >= 0)
    {
      country = str.mid(f + 1);
      str.truncate(f);
    }

  language = str;
}

QString KLocale::language() const
{
  return d->language;
}

QString KLocale::country() const
{
  return m_country;
}

QString KLocale::monthName(int i, bool shortName) const
{
  if ( shortName )
    switch ( i )
      {
      case 1:   return translate("January", "Jan");
      case 2:   return translate("February", "Feb");
      case 3:   return translate("March", "Mar");
      case 4:   return translate("April", "Apr");
      case 5:   return translate("May short", "May");
      case 6:   return translate("June", "Jun");
      case 7:   return translate("July", "Jul");
      case 8:   return translate("August", "Aug");
      case 9:   return translate("September", "Sep");
      case 10:  return translate("October", "Oct");
      case 11:  return translate("November", "Nov");
      case 12:  return translate("December", "Dec");
      }
  else
    switch (i)
      {
      case 1:   return translate("January");
      case 2:   return translate("February");
      case 3:   return translate("March");
      case 4:   return translate("April");
      case 5:   return translate("May long", "May");
      case 6:   return translate("June");
      case 7:   return translate("July");
      case 8:   return translate("August");
      case 9:   return translate("September");
      case 10:  return translate("October");
      case 11:  return translate("November");
      case 12:  return translate("December");
      }

  return QString::null;
}

QString KLocale::weekDayName (int i, bool shortName) const
{
  if ( shortName )
    switch ( i )
      {
      case 1:  return translate("Monday", "Mon");
      case 2:  return translate("Tuesday", "Tue");
      case 3:  return translate("Wednesday", "Wed");
      case 4:  return translate("Thursday", "Thu");
      case 5:  return translate("Friday", "Fri");
      case 6:  return translate("Saturday", "Sat");
      case 7:  return translate("Sunday", "Sun");
      }
  else
    switch ( i )
      {
      case 1:  return translate("Monday");
      case 2:  return translate("Tuesday");
      case 3:  return translate("Wednesday");
      case 4:  return translate("Thursday");
      case 5:  return translate("Friday");
      case 6:  return translate("Saturday");
      case 7:  return translate("Sunday");
      }

  return QString::null;
}

void KLocale::insertCatalogue( const QString & catalogue )
{
  KCatalogue cat( catalogue );

  initCatalogue( cat );

  d->catalogues.append( cat );
}

KLocale::~KLocale()
{
  delete d;
}

QString KLocale::translate_priv(const char *msgid,
				const char *fallback,
				const char **translated) const
{
  if (!msgid || !msgid[0])
    {
      kdDebug(173) << "KLocale: trying to look up \"\" in catalogue. "
		   << "Fix the program" << endl;
      return QString::null;
    }

  if ( useDefaultLanguage() )
    return QString::fromUtf8( fallback );

  for ( QValueList<KCatalogue>::ConstIterator it = d->catalogues.begin();
	it != d->catalogues.end();
	++it )
    {
      const char * text = (*it).translate( msgid );

      if ( text )
	{
	  // we found it
	  if (translated)
	    *translated = text;
	  return QString::fromUtf8( text );
	}
    }

  // Always use UTF-8 if the string was not found
  return QString::fromUtf8( fallback );
}

QString KLocale::translate(const char* msgid) const
{
  return translate_priv(msgid, msgid);
}

QString KLocale::translate( const char *index, const char *fallback) const
{
  if (!index || !index[0] || !fallback || !fallback[0])
    {
      kdDebug(173) << "KLocale: trying to look up \"\" in catalogue. "
		   << "Fix the program" << endl;
      return QString::null;
    }

  if ( useDefaultLanguage() )
    return QString::fromUtf8( fallback );

  char *newstring = new char[strlen(index) + strlen(fallback) + 5];
  sprintf(newstring, "_: %s\n%s", index, fallback);
  // as copying QString is very fast, it looks slower as it is ;/
  QString r = translate_priv(newstring, fallback);
  delete [] newstring;

  return r;
}

QString put_n_in(const QString &orig, unsigned long n)
{
  QString ret = orig;
  int index = ret.find("%n");
  if (index == -1)
    return ret;
  ret.replace(index, 2, QString::number(n));
  return ret;
}

#define EXPECT_LENGTH(x) \
   if (forms.count() != x) { \
      kdError() << "translation of \"" << singular << "\" doesn't contain " << x << " different plural forms as expected\n"; \
      return QString( "BROKEN TRANSLATION %1" ).arg( singular ); }

QString KLocale::translate( const char *singular, const char *plural,
                            unsigned long n ) const
{
  if (!singular || !singular[0] || !plural || !plural[0])
    {
      kdDebug(173) << "KLocale: trying to look up \"\" in catalogue. "
		   << "Fix the program" << endl;
      return QString::null;
    }

  char *newstring = new char[strlen(singular) + strlen(plural) + 6];
  sprintf(newstring, "_n: %s\n%s", singular, plural);
  // as copying QString is very fast, it looks slower as it is ;/
  QString r = translate_priv(newstring, 0);
  delete [] newstring;

  if ( r.isEmpty() || useDefaultLanguage() || d->plural_form == -1) {
    if ( n == 1 )
      return put_n_in( QString::fromUtf8( singular ),  n );
    else
      return put_n_in( QString::fromUtf8( plural ),  n );
  }

  QStringList forms = QStringList::split( "\n", r, false );
  switch ( d->plural_form ) {
  case 0: // NoPlural
    EXPECT_LENGTH( 1 );
    return put_n_in( forms[0], n);
  case 1: // TwoForms
    EXPECT_LENGTH( 2 );
    if ( n == 1 )
      return put_n_in( forms[0], n);
    else
      return put_n_in( forms[1], n);
  case 2: // French
    EXPECT_LENGTH( 2 );
    if ( n == 1 || n == 0 )
      return put_n_in( forms[0], n);
    else
      return put_n_in( forms[1], n);
  case 3: // Gaeilge
    EXPECT_LENGTH( 3 );
    if ( n == 1 )
      return put_n_in( forms[0], n);
    else if ( n == 2 )
      return put_n_in( forms[1], n);
    else
      return put_n_in( forms[2], n);
  case 4: // Russian, corrected by mok
    EXPECT_LENGTH( 3 );
    if ( n%10 == 1  &&  n%100 != 11)
      return put_n_in( forms[0], n); // odin fail
    else if (( n%10 >= 2 && n%10 <=4 ) && (n%100<10 || n%100>20))
      return put_n_in( forms[1], n); // dva faila
    else
      return put_n_in( forms[2], n); // desyat' failov
  case 5: // Polish
    EXPECT_LENGTH( 3 );
    if ( n == 1 )
      return put_n_in( forms[0], n);
    else if ( n%10 >= 2 && n%10 <=4 && (n%100<10 || n%100>=20) )
      return put_n_in( forms[1], n);
    else
      return put_n_in( forms[2], n);
  case 6: // Slovenian
    EXPECT_LENGTH( 4 );
    if ( n%100 == 1 )
      return put_n_in( forms[1], n); // ena datoteka
    else if ( n%100 == 2 )
      return put_n_in( forms[2], n); // dve datoteki
    else if ( n%100 == 3 || n%100 == 4 )
      return put_n_in( forms[3], n); // tri datoteke
    else
      return put_n_in( forms[0], n); // sto datotek
  case 7: // Lithuanian
    EXPECT_LENGTH( 3 );
    if ( n%10 == 0 || (n%100>=11 && n%100<=19) )
      return put_n_in( forms[2], n);
    else if ( n%10 == 1 )
      return put_n_in( forms[0], n);
    else
      return put_n_in( forms[1], n);
  case 8: // Czech
    EXPECT_LENGTH( 3 );
    if ( n%100 == 1 )
      return put_n_in( forms[0], n);
    else if (( n%100 >= 2 ) && ( n%100 <= 4 ))
      return put_n_in( forms[1], n);
    else
      return put_n_in( forms[2], n);
  case 9: // Slovak
    EXPECT_LENGTH( 3 );
    if ( n == 1 )
      return put_n_in( forms[0], n);
    else if (( n >= 2 ) && ( n <= 4 ))
      return put_n_in( forms[1], n);
    else
      return put_n_in( forms[2], n);
  case 10: // Maltese
    EXPECT_LENGTH( 4 );
    if ( n == 1 )
      return put_n_in( forms[0], n );
    else if ( ( n == 0 ) || ( n%100 > 0 && n%100 <= 10 ) )
      return put_n_in( forms[1], n );
    else if ( n%100 > 10 && n%100 < 20 )
      return put_n_in( forms[2], n );
    else
      return put_n_in( forms[3], n );
  }
  kdFatal() << "The function should have been returned in another way\n";

  return QString::null;
}

QString KLocale::translateQt( const char *index, const char *fallback) const
{
  if (!index || !index[0] || !fallback || !fallback[0])
    {
      kdDebug(173) << "KLocale: trying to look up \"\" in catalouge. "
		   << "Fix the program" << endl;
      return QString::null;
    }

    if ( useDefaultLanguage() )
      return QString::null;

    char *newstring = new char[strlen(index) + strlen(fallback) + 5];
    sprintf(newstring, "_: %s\n%s", index, fallback);
    const char *translation = 0;
    // as copying QString is very fast, it looks slower as it is ;/
    QString r = translate_priv(newstring, fallback, &translation);
    delete [] newstring;
    if (translation)
      return r;

    r = translate_priv(fallback, fallback, &translation);
    if (translation)
      return r;
    return QString::null;
}

bool KLocale::weekStartsMonday() const
{
  doFormatInit();
  return m_weekStartsMonday;
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

inline void put_it_in( QChar *buffer, uint& index, const QString &s )
{
  for ( uint l = 0; l < s.length(); l++ )
    buffer[index++] = s.at( l );
}

inline void put_it_in( QChar *buffer, uint& index, int number )
{
  buffer[index++] = number / 10 + '0';
  buffer[index++] = number % 10 + '0';
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
  int pos = res.find('.');
  if (pos == -1) pos = res.length();
  else res.replace(pos, 1, monetaryDecimalSymbol());

  while (0 < (pos -= 3))
    res.insert(pos, monetaryThousandsSeparator()); // thousend sep

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
      res.prepend('(');
      res.append (')');
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
      res.prepend(' ');
      res.prepend(currency);
    } else {
      res.append (' ');
      res.append (currency);
    }

  return res;
}

QString KLocale::formatMoney(const QString &numStr) const
{
  return formatMoney(numStr.toDouble());
}

QString KLocale::formatNumber(double num, int precision) const
{
  bool neg = num < 0;
  if (precision == -1) precision = 2;
  QString res = QString::number(neg?-num:num, 'f', precision);
  int pos = res.find('.');
  if (pos == -1) pos = res.length();
  else res.replace(pos, 1, decimalSymbol());

  while (0 < (pos -= 3))
    res.insert(pos, thousandsSeparator()); // thousend sep

  // How can we know where we should put the sign?
  res.prepend(neg?negativeSign():positiveSign());

  return res;
}

QString KLocale::formatNumber(const QString &numStr) const
{
  return formatNumber(numStr.toDouble());
}

QString KLocale::formatDate(const QDate &pDate, bool shortFormat) const
{
  const QString rst = shortFormat?dateFormatShort():dateFormat();

  // I'm rather safe than sorry
  QChar *buffer = new QChar[rst.length() * 3 / 2 + 50];

  unsigned int index = 0;
  bool escape = false;
  int number = 0;

  for ( uint format_index = 0; format_index < rst.length(); ++format_index )
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
	    case 'Y':
	      put_it_in( buffer, index, pDate.year() / 100 );
	    case 'y':
	      put_it_in( buffer, index, pDate.year() % 100 );
	      break;
	    case 'n':
	      number = pDate.month();
	    case 'e':
	      // to share the code
	      if ( rst.at( format_index ).unicode() == 'e' )
		number = pDate.day();
	      if ( number / 10 )
		buffer[index++] = number / 10 + '0';
	      buffer[index++] = number % 10 + '0';
	      break;
	    case 'm':
	      put_it_in( buffer, index, pDate.month() );
	      break;
	    case 'b':
	      put_it_in( buffer, index, monthName(pDate.month(), true) );
	      break;
	    case 'B':
	      put_it_in( buffer, index, monthName(pDate.month(), false) );
	      break;
	    case 'd':
	      put_it_in( buffer, index, pDate.day() );
	      break;
	    case 'a':
	      put_it_in( buffer, index, weekDayName(pDate.dayOfWeek(), true) );
	      break;
	    case 'A':
	      put_it_in( buffer, index, weekDayName(pDate.dayOfWeek(), false) );
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
  return ret;
}

void KLocale::setMainCatalogue(const char *catalogue)
{
  maincatalogue = catalogue;
}

double KLocale::readNumber(const QString &_str, bool * ok) const
{
  QString str = _str.stripWhiteSpace();
  bool neg = str.find(negativeSign()) == 0;
  if (neg)
    str.remove( 0, negativeSign().length() );

  int pos = str.find(decimalSymbol());
  QString major;
  QString minior;
  if ( pos == -1 )
    major = str;
  else
    {
      major = str.left(pos);
      minior = str.mid(pos + decimalSymbol().length());
    }

  // Remove thousand separators
  while ( ( pos = major.find( thousandsSeparator() ) ) > 0 )
    major.remove( pos, thousandsSeparator().length() );

  QString tot;
  if (neg) tot = '-';
  tot += major + '.' + minior;

  return tot.toDouble(ok);
}

double KLocale::readMoney(const QString &_str, bool * ok) const
{
  QString str = _str.stripWhiteSpace();
  bool neg = false;
  bool currencyFound = false;
  // First try removing currency symbol from either end
  int pos = str.find(currencySymbol());
  if ( pos == 0 || pos == (int) str.length()-1 )
    {
      str.remove(pos,currencySymbol().length());
      str = str.stripWhiteSpace();
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
      int i1 = str.find(negativeSign());
      if ( i1 == 0 || i1 == (int) str.length()-1 )
        {
	  neg = true;
	  str.remove(i1,negativeSign().length());
        }
    }
  if (neg) str = str.stripWhiteSpace();

  // Finally try again for the currency symbol, if we didn't find
  // it already (because of the negative sign being in the way).
  if ( !currencyFound )
    {
      pos = str.find(currencySymbol());
      if ( pos == 0 || pos == (int) str.length()-1 )
        {
	  str.remove(pos,currencySymbol().length());
	  str = str.stripWhiteSpace();
        }
    }

  // And parse the rest as a number
  pos = str.find(monetaryDecimalSymbol());
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
  while ( ( pos = major.find( monetaryThousandsSeparator() ) ) > 0 )
    major.remove( pos, monetaryThousandsSeparator().length() );

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
static int readInt(const QString &str, uint &pos)
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

QDate KLocale::readDate(const QString &intstr) const
{
  QDate date;
  date = readDate(intstr, true);
  if (date.isValid()) return date;
  return readDate(intstr, false);
}

QDate KLocale::readDate(const QString &intstr, bool shortFormat) const
{
  QString fmt = (shortFormat ? dateFormatShort() : dateFormat()).simplifyWhiteSpace();
  return readDate( intstr, fmt );
}

QDate KLocale::readDate(const QString &intstr, const QString &fmt) const
{
  QString str = intstr.simplifyWhiteSpace().lower();
  int day = -1, month = -1;
  // allow the year to be omitted if not in the format
  int year = QDate::currentDate().year();
  uint strpos = 0;
  uint fmtpos = 0;

  while (fmt.length() > fmtpos || str.length() > strpos)
    {
      if ( !(fmt.length() > fmtpos && str.length() > strpos) )
        goto error;

      QChar c = fmt.at(fmtpos++);

      if (c != '%') {
        if (c.isSpace())
          strpos++;
        else if (c != str.at(strpos++))
          goto error;
        continue;
      }

      // remove space at the begining
      if (str.length() > strpos && str.at(strpos).isSpace())
        strpos++;

      c = fmt.at(fmtpos++);
      switch (c)
      {
	case 'a':
	case 'A':
	  // this will just be ignored
	  for (int j = 1; j < 8; j++) {
	    QString s = weekDayName(j, c == 'a').lower();
	    int len = s.length();
	    if (str.mid(strpos, len) == s)
	      strpos += len;
	  }
	  break;
	case 'b':
	case 'B':
	  for (int j = 1; j < 13; j++) {
	    QString s = monthName(j, c == 'b').lower();
	    int len = s.length();
	    if (str.mid(strpos, len) == s) {
	      month = j;
	      strpos += len;
	    }
	  }
	  break;
	case 'd':
	case 'e':
	  day = readInt(str, strpos);
	  if (day < 1 || day > 31)
	    goto error;

	  break;

	case 'n':
	case 'm':
	  month = readInt(str, strpos);
	  if (month < 1 || month > 12)
	    goto error;

	  break;

	case 'Y':
	case 'y':
	  year = readInt(str, strpos);
	  if (year < 0)
	    goto error;
	  // Qt treats a year in the range 0-100 as 1900-1999.
	  // It is nicer for the user if we treat 0-68 as 2000-2068
	  if (year < 69)
	    year += 2000;
	  else if (c == 'y')
	    year += 1900;

	  break;
	}
    }
  //kdDebug(173) << "KLocale::readDate day=" << day << " month=" << month << " year=" << year << endl;
  if ( year != -1 && month != -1 && day != -1 )
    return QDate(year, month, day);
 error:
  return QDate(); // invalid date
}

QTime KLocale::readTime(const QString &intstr) const
{
  QTime _time;
  _time = readTime(intstr, true);
  if (_time.isValid()) return _time;
  return readTime(intstr, false);
}

QTime KLocale::readTime(const QString &intstr, bool seconds) const
{
  QString str = intstr.simplifyWhiteSpace().lower();
  QString Format = timeFormat().simplifyWhiteSpace();
  if (!seconds)
    Format.replace(QRegExp(QString::fromLatin1(".%S")), QString::null);

  int hour = -1, minute = -1, second = seconds ? -1 : 0; // don't require seconds
  bool g_12h = false;
  bool pm = false;
  uint strpos = 0;
  uint Formatpos = 0;

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

      // remove space at the begining
      if (str.length() > strpos && str.at(strpos).isSpace())
	strpos++;

      c = Format.at(Formatpos++);
      switch (c)
	{
	case 'p':
	  {
	    QString s;
	    s = translate("pm").lower();
	    int len = s.length();
	    if (str.mid(strpos, len) == s)
	      {
		pm = true;
		strpos += len;
	      }
	    else
	      {
		s = translate("am").lower();
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
  if (g_12h)
    {
      hour %= 12;
      if (pm) hour += 12;
    }

  return QTime(hour, minute, second);

 error:
  return QTime(-1, -1, -1); // return invalid date if it didn't work
}

QString KLocale::formatTime(const QTime &pTime, bool includeSecs) const
{
  const QString rst = timeFormat();

  // only "pm/am" here can grow, the rest shrinks, but
  // I'm rather safe than sorry
  QChar *buffer = new QChar[rst.length() * 3 / 2 + 30];

  uint index = 0;
  bool escape = false;
  int number = 0;

  for ( uint format_index = 0; format_index < rst.length(); format_index++ )
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
	      put_it_in( buffer, index, ( pTime.hour() + 11) % 12 + 1 );
	      break;
	    case 'M':
	      put_it_in( buffer, index, pTime.minute() );
	      break;
	    case 'S':
	      if (includeSecs)
		put_it_in( buffer, index, pTime.second() );
	      else
		{
		  // we remove the seperator sign before the seconds and
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
		number = (pTime.hour() + 11) % 12 + 1;
	      if ( number / 10 )
		buffer[index++] = number / 10 + '0';
	      buffer[index++] = number % 10 + '0';
	      break;
	    case 'p':
	      {
		QString s;
		if ( pTime.hour() >= 12 )
		  put_it_in( buffer, index, translate("pm") );
		else
		  put_it_in( buffer, index, translate("am") );
		break;
	      }
	    default:
	      buffer[index++] = rst.at( format_index );
	      break;
	    }
	  escape = false;
	}
    }
  QString ret( buffer, index );
  delete [] buffer;
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

QString KLocale::languages() const
{
  return d->languageList.join( QString::fromLatin1(":") );
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

QString KLocale::formatDateTime(const QDateTime &pDateTime) const
{
  return formatDateTime(pDateTime, true);
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
    KGlobal::_locale = new KLocale(app->instanceName());

    // only do this for the global instance
    qt_set_locale_codec(KGlobal::_locale->codecForEncoding());
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
      langs.remove( defaultLanguage() );
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

  return QString::null;
}

QString KLocale::charset() const
{
  return m_charset;
}

bool KLocale::useDefaultLanguage() const
{
  return language() == defaultLanguage();
}

void KLocale::initCharset(KConfig *config)
{
  KConfigGroupSaver saver(config, "Locale");
  m_charset = config->readEntry("Charset");

  if (m_charset.isEmpty())
    {
      m_charset = QString::fromLatin1("iso-8859-1");
    }
  else
    {
      // ### HPB: This code should be rewritten/removed
      bool bOk;
      KGlobal::charsets()->codecForName(m_charset, bOk);
      // if !ok, we have a problem. it will return latin-1 then, but thats
      // obviously not what the user wants
      if(!bOk)
	kdWarning(173) << "charset " << m_charset
		       << " is not known. using ISO 8859-1 instead." << endl;
      //m_charset = QString::fromLatin1("iso-8859-1");
    }
}

void KLocale::initEncoding(KConfig *config)
{
  const int mibDefault = 4; // ISO 8859-1

  int encodingMib = mibDefault; // set to default
  
  KConfigGroupSaver saver(config, "Locale");
  encodingMib = config->readNumEntry("EncodingMib",
				     encodingMib);

  setEncoding( encodingMib );

  if ( !d->codecForEncoding )
    {
      kdWarning(173) << "encodingMib " << encodingMib
		     << " is not known. using ISO 8859-1 instead." << endl;
      setEncoding(mibDefault);
      // ### we should default to Qt's default, as thats always more
      // intelligent
    }

  ASSERT( d->codecForEncoding );
}

void KLocale::initCatalogue( KCatalogue & catalogue )
{
  catalogue.setFileName( catalogueFileName( language(), catalogue ) );
}

bool KLocale::setCharset(const QString & charset)
{
  if ( charset.isEmpty() )
    return false;

  m_charset = charset;

  return true;
}

void KLocale::setDateFormat(const QString & format)
{
  doFormatInit();
  m_dateFormat = format.stripWhiteSpace();
}

void KLocale::setDateFormatShort(const QString & format)
{
  doFormatInit();
  m_dateFormatShort = format.stripWhiteSpace();
}

void KLocale::setTimeFormat(const QString & format)
{
  doFormatInit();
  m_timeFormat = format.stripWhiteSpace();
}

void KLocale::setWeekStartsMonday(bool start)
{
  doFormatInit();
  m_weekStartsMonday = start;
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
  m_decimalSymbol = symbol.stripWhiteSpace();
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
  m_positiveSign = sign.stripWhiteSpace();
}

void KLocale::setNegativeSign(const QString & sign)
{
  doFormatInit();
  m_negativeSign = sign.stripWhiteSpace();
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
  m_monetaryDecimalSymbol = symbol.stripWhiteSpace();
}

void KLocale::setCurrencySymbol(const QString & symbol)
{
  doFormatInit();
  m_currencySymbol = symbol.stripWhiteSpace();
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
  return codecForEncoding()->name();
}

int KLocale::encodingMib() const
{
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

KLocale::KLocale(const KLocale & rhs)
{
  d = new KLocalePrivate;

  *this = rhs;
}

KLocale & KLocale::operator=(const KLocale & rhs)
{
  m_charset = rhs.m_charset;

  m_weekStartsMonday = rhs.m_weekStartsMonday;

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

  m_country = rhs.m_country;

  // the assignment operator works here
  *d = *rhs.d;

  return *this;
}
