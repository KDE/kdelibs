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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <stdlib.h> // getenv

#include <qtextcodec.h>
#include <qfile.h>
#include <qprinter.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#include <qregexp.h>

#include "kcatalogue.h"
#include "kglobal.h"
#include "kstandarddirs.h"
#include "ksimpleconfig.h"
#include "kinstance.h"
#include "kconfig.h"
#include "kdebug.h"
#include "kcalendarsystem.h"
#include "kcalendarsystemfactory.h"
#include "klocale.h"

static const char * const SYSTEM_MESSAGES = "kdelibs";

static const char *maincatalogue = 0;

class KLocalePrivate
{
public:
  int weekStartDay;
  bool nounDeclension;
  bool dateMonthNamePossessive;
  QStringList languageList;
  QStringList catalogNames; // list of all catalogs (regardless of language)
  QValueList<KCatalogue> catalogues; // list of all loaded catalogs, contains one instance per catalog name and language
  QString encoding;
  QTextCodec * codecForEncoding;
  KConfig * config;
  bool formatInited;
  int /*QPrinter::PageSize*/ pageSize;
  KLocale::MeasureSystem measureSystem;
  QStringList langTwoAlpha;
  KConfig *languages;

  QString calendarType;
  KCalendarSystem * calendar;
  bool utf8FileEncoding;
  QString appName;
};

static KLocale *this_klocale = 0;

KLocale::KLocale( const QString & catalog, KConfig * config )
{
  d = new KLocalePrivate;
  d->config = config;
  d->languages = 0;
  d->calendar = 0;
  d->formatInited = false;

  initEncoding(0);
  initFileNameEncoding(0);

  KConfig *cfg = d->config;
  this_klocale = this;
  if (!cfg) cfg = KGlobal::instance()->config();
  this_klocale = 0;
  Q_ASSERT( cfg );

  d->appName = catalog;
  initLanguageList( cfg, config == 0);
  initMainCatalogues(catalog);
}

QString KLocale::_initLanguage(KConfigBase *config)
{
  if (this_klocale)
  {
     // ### HPB Why this cast??
     this_klocale->initLanguageList((KConfig *) config, true);
     // todo: adapt current catalog list: remove unused languages, insert main catalogs, if not already found
     return this_klocale->language();
  }
  return QString::null;
}

void KLocale::initMainCatalogues(const QString & catalog)
{
  // Use the first non-null string.
  QString mainCatalogue = catalog;
  if (maincatalogue)
    mainCatalogue = QString::fromLatin1(maincatalogue);

  if (mainCatalogue.isEmpty()) {
    kdDebug(173) << "KLocale instance created called without valid "
                 << "catalog! Give an argument or call setMainCatalogue "
                 << "before init" << endl;
  }
  else {
    // do not use insertCatalogue here, that would already trigger updateCatalogs
    d->catalogNames.append( mainCatalogue );   // application catalog
    d->catalogNames.append( SYSTEM_MESSAGES ); // always include kdelibs.mo
    d->catalogNames.append( "kio" );            // always include kio.mo
    updateCatalogues(); // evaluate this for all languages
  }
}

void KLocale::initLanguageList(KConfig * config, bool useEnv)
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

      langs << QFile::decodeName( ::getenv("LC_ALL") );
      langs << QFile::decodeName( ::getenv("LC_MESSAGES") );
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

          langs.insert(it, ln);
	}

      languageList += langs;
    }

  // now we have a language list -- let's use the first OK language
  setLanguage( languageList );
}

void KLocale::initPluralTypes()
{
  for ( QValueList<KCatalogue>::Iterator it = d->catalogues.begin();
    it != d->catalogues.end();
    ++it )
  {
    QString language = (*it).language();
    int pt = pluralType( language );
    (*it).setPluralType( pt );
  }
}


int KLocale::pluralType( const QString & language )
{
  for ( QValueList<KCatalogue>::ConstIterator it = d->catalogues.begin();
    it != d->catalogues.end();
    ++it )
  {
    if ( ((*it).name() == SYSTEM_MESSAGES ) && ((*it).language() == language )) {
      return pluralType( *it );
    }
  }
  // kdelibs.mo does not seem to exist for this language
  return -1;
}

int KLocale::pluralType( const KCatalogue& catalog )
{
    const char* pluralFormString =
    I18N_NOOP("_: Dear translator, please do not translate this string "
      "in any form, but pick the _right_ value out of "
      "NoPlural/TwoForms/French... If not sure what to do mail "
      "thd@kde.org and coolo@kde.org, they will tell you. "
      "Better leave that out if unsure, the programs will "
      "crash!!\nDefinition of PluralForm - to be set by the "
      "translator of kdelibs.po");
    QString pf (catalog.translate( pluralFormString));
    if ( pf.isEmpty() ) {
      return -1;
    }
	else if ( pf == "NoPlural" )
      return 0;
    else if ( pf == "TwoForms" )
      return 1;
    else if ( pf == "French" )
      return 2;
    else if ( pf == "OneTwoRest" )
      return 3;
    else if ( pf == "Russian" )
      return 4;
    else if ( pf == "Polish" )
      return 5;
    else if ( pf == "Slovenian" )
      return 6;
    else if ( pf == "Lithuanian" )
      return 7;
    else if ( pf == "Czech" )
      return 8;
    else if ( pf == "Slovak" )
      return 9;
    else if ( pf == "Maltese" )
      return 10;
    else if ( pf == "Arabic" )
      return 11;
    else if ( pf == "Balcan" )
      return 12;
    else if ( pf == "Macedonian" )
      return 13;
    else if ( pf == "Gaeilge" )
        return 14;
    else {
      kdWarning(173) << "Definition of PluralForm is none of "
		       << "NoPlural/"
		       << "TwoForms/"
		       << "French/"
		       << "OneTwoRest/"
		       << "Russian/"
		       << "Polish/"
		       << "Slovenian/"
		       << "Lithuanian/"
		       << "Czech/"
		       << "Slovak/"
		       << "Arabic/"
		       << "Balcan/"
               << "Macedonian/"
               << "Gaeilge/"
		       << "Maltese: " << pf << endl;
      exit(1);
    }
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
  Q_ASSERT( config );

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
#define readConfigEntry(key, default, save) \
  save = entry.readEntry(key, QString::fromLatin1(default)); \
  save = config->readEntry(key, save);

#define readConfigNumEntry(key, default, save, type) \
  save = (type)entry.readNumEntry(key, default); \
  save = (type)config->readNumEntry(key, save);

#define readConfigBoolEntry(key, default, save) \
  save = entry.readBoolEntry(key, default); \
  save = config->readBoolEntry(key, save);

  readConfigEntry("DecimalSymbol", ".", m_decimalSymbol);
  readConfigEntry("ThousandsSeparator", ",", m_thousandsSeparator);
  m_thousandsSeparator.replace( QString::fromLatin1("$0"), QString::null );
  //kdDebug(173) << "m_thousandsSeparator=" << m_thousandsSeparator << endl;

  readConfigEntry("PositiveSign", "", m_positiveSign);
  readConfigEntry("NegativeSign", "-", m_negativeSign);

  // Monetary
  readConfigEntry("CurrencySymbol", "$", m_currencySymbol);
  readConfigEntry("MonetaryDecimalSymbol", ".", m_monetaryDecimalSymbol);
  readConfigEntry("MonetaryThousandsSeparator", ",",
		  m_monetaryThousandsSeparator);
  m_monetaryThousandsSeparator.replace(QString::fromLatin1("$0"), QString::null);

  readConfigNumEntry("FracDigits", 2, m_fracDigits, int);
  readConfigBoolEntry("PositivePrefixCurrencySymbol", true,
		      m_positivePrefixCurrencySymbol);
  readConfigBoolEntry("NegativePrefixCurrencySymbol", true,
		      m_negativePrefixCurrencySymbol);
  readConfigNumEntry("PositiveMonetarySignPosition", (int)BeforeQuantityMoney,
		     m_positiveMonetarySignPosition, SignPosition);
  readConfigNumEntry("NegativeMonetarySignPosition", (int)ParensAround,
		     m_negativeMonetarySignPosition, SignPosition);


  // Date and time
  readConfigEntry("TimeFormat", "%H:%M:%S", m_timeFormat);
  readConfigEntry("DateFormat", "%A %d %B %Y", m_dateFormat);
  readConfigEntry("DateFormatShort", "%Y-%m-%d", m_dateFormatShort);
  readConfigNumEntry("WeekStartDay", 1, d->weekStartDay, int);

  // other
  readConfigNumEntry("PageSize", (int)QPrinter::A4, d->pageSize, int);
  readConfigNumEntry("MeasureSystem", (int)Metric, d->measureSystem,
		     MeasureSystem);
  readConfigEntry("CalendarSystem", "gregorian", d->calendarType);
  delete d->calendar;
  d->calendar = 0; // ### HPB Is this the correct place?

  //Grammatical
  //Precedence here is l10n / i18n / config file
  KSimpleConfig language(locate("locale",
			        QString::fromLatin1("%1/entry.desktop")
                                .arg(m_language)), true);
  language.setGroup("KCM Locale");
#define read3ConfigBoolEntry(key, default, save) \
  save = entry.readBoolEntry(key, default); \
  save = language.readBoolEntry(key, save); \
  save = config->readBoolEntry(key, save);

  read3ConfigBoolEntry("NounDeclension", false, d->nounDeclension);
  read3ConfigBoolEntry("DateMonthNamePossessive", false,
		       d->dateMonthNamePossessive);

  // end of hack
  KGlobal::_locale = lsave;
}

bool KLocale::setCountry(const QString & country)
{
  // Check if the file exists too??
  if ( country.isEmpty() )
    return false;

  m_country = country;

  d->formatInited = false;

  return true;
}

QString KLocale::catalogueFileName(const QString & language,
				   const KCatalogue & catalog)
{
  QString path = QString::fromLatin1("%1/LC_MESSAGES/%2.mo")
    .arg( language )
    .arg( catalog.name() );

  return locate( "locale", path );
}

bool KLocale::setLanguage(const QString & language)
{
  if ( d->languageList.contains( language ) ) {
 	 d->languageList.remove( language );
  }
  d->languageList.prepend( language ); // let us consider this language to be the most important one

  m_language = language; // remember main language for shortcut evaluation

  // important when called from the outside and harmless when called before populating the
  // catalog name list
  updateCatalogues();

  d->formatInited = false;

  return true; // Maybe the mo-files for this language are empty, but in principle we can speak all languages
}

bool KLocale::setLanguage(const QStringList & languages)
{
  QStringList languageList( languages );
  // This list might contain
  // 1) some empty strings that we have to eliminate
  // 2) duplicate entries like in de:fr:de, where we have to keep the first occurrance of a language in order
  //    to preserve the order of precenence of the user => iterate backwards
  // 3) languages into which the application is not translated. For those languages we should not even load kdelibs.mo or kio.po.
  //    these langugage have to be dropped. Otherwise we get strange side effects, e.g. with Hebrew:
  //    the right/left switch for languages that write from
  //    right to left (like Hebrew or Arabic) is set in kdelibs.mo. If you only have kdelibs.mo
  //    but nothing from appname.mo, you get a mostly English app with layout from right to left.
  //    That was considered to be a bug by the Hebrew translators.
  for( QStringList::Iterator it = languageList.fromLast();
    it != languageList.begin(); --it )
  {
    // kdDebug() << "checking " << (*it) << endl;
    bool bIsTranslated = isApplicationTranslatedInto( *it );
    if ( languageList.contains(*it) > 1 || (*it).isEmpty() || (!bIsTranslated) ) {
      // kdDebug() << "removing " << (*it) << endl;
      it = languageList.remove( it );
    }
  }
  // now this has left the first element of the list unchecked.
  // The question why this is the case is left as an exercise for the reader...
  // Besides the list might have been empty all the way, so check that too.
  if ( languageList.begin() != languageList.end() ) {
     QStringList::Iterator it = languageList.begin(); // now pointing to the first element
     // kdDebug() << "checking " << (*it) << endl;
     if( (*it).isEmpty() || !(isApplicationTranslatedInto( *it )) ) {
        // kdDebug() << "removing " << (*it) << endl;
     	languageList.remove( it ); // that's what the iterator was for...
     }
  }

  if ( languageList.isEmpty() ) {
	// user picked no language, so we assume he/she speaks English.
	languageList.append( defaultLanguage() );
  }
  m_language = languageList.first(); // keep this for shortcut evaluations

  d->languageList = languageList; // keep this new list of languages to use
  d->langTwoAlpha.clear(); // Flush cache

  // important when called from the outside and harmless when called before populating the
  // catalog name list
  updateCatalogues();

  return true; // we found something. Maybe it's only English, but we found something
}

bool KLocale::isApplicationTranslatedInto( const QString & language)
{
  if ( language.isEmpty() ) {
    return false;
  }

  if ( language == defaultLanguage() ) {
    // en_us is always "installed"
    return true;
  }

  QString appName = d->appName;
  if (maincatalogue) {
    appName = QString::fromLatin1(maincatalogue);
  }
  // sorry, catalogueFileName requires catalog object,k which we do not have here
  // path finding was supposed to be moved completely to KCatalogue. The interface cannot
  // be changed that far during deep freeze. So in order to fix the bug now, we have
  // duplicated code for file path evaluation. Cleanup will follow later. We could have e.g.
  // a static method in KCataloge that can translate between these file names.
  // a stat
  QString sFileName = QString::fromLatin1("%1/LC_MESSAGES/%2.mo")
    .arg( language )
    .arg( appName );
  // kdDebug() << "isApplicationTranslatedInto: filename " << sFileName << endl;

  QString sAbsFileName = locate( "locale", sFileName );
  // kdDebug() << "isApplicationTranslatedInto: absname " << sAbsFileName << endl;
  return ! sAbsFileName.isEmpty();
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
  return m_language;
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

QString KLocale::monthNamePossessive(int i, bool shortName) const
{
  if ( shortName )
    switch ( i )
      {
      case 1:   return translate("of January", "of Jan");
      case 2:   return translate("of February", "of Feb");
      case 3:   return translate("of March", "of Mar");
      case 4:   return translate("of April", "of Apr");
      case 5:   return translate("of May short", "of May");
      case 6:   return translate("of June", "of Jun");
      case 7:   return translate("of July", "of Jul");
      case 8:   return translate("of August", "of Aug");
      case 9:   return translate("of September", "of Sep");
      case 10:  return translate("of October", "of Oct");
      case 11:  return translate("of November", "of Nov");
      case 12:  return translate("of December", "of Dec");
      }
  else
    switch (i)
      {
      case 1:   return translate("of January");
      case 2:   return translate("of February");
      case 3:   return translate("of March");
      case 4:   return translate("of April");
      case 5:   return translate("of May long", "of May");
      case 6:   return translate("of June");
      case 7:   return translate("of July");
      case 8:   return translate("of August");
      case 9:   return translate("of September");
      case 10:  return translate("of October");
      case 11:  return translate("of November");
      case 12:  return translate("of December");
      }

  return QString::null;
}

QString KLocale::weekDayName (int i, bool shortName) const
{
  return calendar()->weekDayName(i, shortName);
}

void KLocale::insertCatalogue( const QString & catalog )
{
  if ( !d->catalogNames.contains( catalog) ) {
    d->catalogNames.append( catalog );
  }
  updateCatalogues( ); // evaluate the changed list and generate the neccessary KCatalog objects
}

void KLocale::updateCatalogues( )
{
  // some changes have occured. Maybe we have learned or forgotten some languages.
  // Maybe the language precedence has changed.
  // Maybe we have learned or forgotten some catalog names.
  // Now examine the list of KCatalogue objects and change it according to the new circumstances.

  // this could be optimized: try to reuse old KCatalog objects, but remember that the order of
  // catalogs might have changed: e.g. in this fashion
  // 1) move all catalogs into a temporary list
  // 2) iterate over all languages and catalog names
  // 3.1) pick the catalog from the saved list, if it already exists
  // 3.2) else create a new catalog.
  // but we will do this later.

  for ( QValueList<KCatalogue>::Iterator it = d->catalogues.begin();
	it != d->catalogues.end(); )
  {
     it = d->catalogues.remove(it);
  }

  // now iterate over all languages and all wanted catalog names and append or create them in the right order
  // the sequence must be e.g. nds/appname nds/kdelibs nds/kio de/appname de/kdelibs de/kio etc.
  // and not nds/appname de/appname nds/kdelibs de/kdelibs etc. Otherwise we would be in trouble with a language
  // sequende nds,en_US, de. In this case en_US must hide everything below in the language list.
  for ( QStringList::ConstIterator itLangs =  d->languageList.begin();
	  itLangs != d->languageList.end(); ++itLangs)
  {
    for ( QStringList::ConstIterator itNames =  d->catalogNames.begin();
	itNames != d->catalogNames.end(); ++itNames)
    {
      KCatalogue cat( *itNames, *itLangs ); // create Catalog for this name and this language
      d->catalogues.append( cat );
	}
  }
  initPluralTypes();  // evaluate the plural type for all languages and remember this in each KCatalogue
}




void KLocale::removeCatalogue(const QString &catalog)
{
  if ( d->catalogNames.contains( catalog )) {
    d->catalogNames.remove( catalog );
    if (KGlobal::_instance)
      updateCatalogues();  // walk through the KCatalogue instances and weed out everything we no longer need
  }
}

void KLocale::setActiveCatalogue(const QString &catalog)
{
  if ( d->catalogNames.contains( catalog ) ) {
    d->catalogNames.remove( catalog );
	d->catalogNames.prepend( catalog );
	updateCatalogues();  // walk through the KCatalogue instances and adapt to the new order
  }
}

KLocale::~KLocale()
{
  delete d->calendar;
  delete d->languages;
  delete d;
  d = 0L;
}

QString KLocale::translate_priv(const char *msgid,
				const char *fallback,
				const char **translated,
				int* pluralType ) const
{
  if ( pluralType) {
  	*pluralType = -1; // unless we find something more precise
  }
  if (!msgid || !msgid[0])
    {
      kdWarning() << "KLocale: trying to look up \"\" in catalog. "
		   << "Fix the program" << endl;
      return QString::null;
    }

  if ( useDefaultLanguage() ) { // shortcut evaluation if en_US is main language: do not consult the catalogs
    return QString::fromUtf8( fallback );
  }

  for ( QValueList<KCatalogue>::ConstIterator it = d->catalogues.begin();
	it != d->catalogues.end();
	++it )
    {
	  // shortcut evaluation: once we have arrived at en_US (default language) we cannot consult
	  // the catalog as it will not have an assiciated mo-file. For this default language we can
	  // immediately pick the fallback string.
	  if ( (*it).language() == defaultLanguage() ) {
	  	return QString::fromUtf8( fallback );
	  }

      const char * text = (*it).translate( msgid );

      if ( text )
	{
	  // we found it
	  if (translated) {
	    *translated = text;
	  }
	  if ( pluralType) {
	  	*pluralType = (*it).pluralType(); // remember the plural type information from the catalog that was used
	  }
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
      kdDebug(173) << "KLocale: trying to look up \"\" in catalog. "
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

static QString put_n_in(const QString &orig, unsigned long n)
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
      kdWarning() << "KLocale: trying to look up \"\" in catalog. "
		   << "Fix the program" << endl;
      return QString::null;
    }

  char *newstring = new char[strlen(singular) + strlen(plural) + 6];
  sprintf(newstring, "_n: %s\n%s", singular, plural);
  // as copying QString is very fast, it looks slower as it is ;/
  int pluralType = -1;
  QString r = translate_priv(newstring, 0, 0, &pluralType);
  delete [] newstring;

  if ( r.isEmpty() || useDefaultLanguage() || pluralType == -1) {
    if ( n == 1 ) {
      return put_n_in( QString::fromUtf8( singular ),  n );
	} else {
	  QString tmp = QString::fromUtf8( plural );
#ifndef NDEBUG
	  if (tmp.find("%n") == -1) {
			  kdDebug() << "the message for i18n should contain a '%n'! " << plural << endl;
	  }
#endif
      return put_n_in( tmp,  n );
	}
  }

  QStringList forms = QStringList::split( "\n", r, false );
  switch ( pluralType ) {
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
  case 3: // OneTwoRest
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
  case 8: // Czech - use modern form which is equivalent to Slovak
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
  case 11: // Arabic
    EXPECT_LENGTH( 4 );
    if (n == 1)
      return put_n_in(forms[0], n);
    else if (n == 2)
      return put_n_in(forms[1], n);
    else if ( n < 11)
      return put_n_in(forms[2], n);
    else
      return put_n_in(forms[3], n);
  case 12: // Balcan
     EXPECT_LENGTH( 3 );
     if (n != 11 && n % 10 == 1)
	return put_n_in(forms[0], n);
     else if (n / 10 != 1 && n % 10 >= 2 && n % 10 <= 4)
	return put_n_in(forms[1], n);
     else
	return put_n_in(forms[2], n);
  case 13: // Macedonian
     EXPECT_LENGTH(3);
     if (n % 10 == 1)
	return put_n_in(forms[0], n);
     else if (n % 10 == 2)
	return put_n_in(forms[1], n);
     else
	return put_n_in(forms[2], n);
  case 14: // Gaeilge
      EXPECT_LENGTH(5);
      if (n == 1)                       // "ceann amhain"
          return put_n_in(forms[0], n);
      else if (n == 2)                  // "dha cheann"
          return put_n_in(forms[1], n);
      else if (n < 7)                   // "%n cinn"
          return put_n_in(forms[2], n);
      else if (n < 11)                  // "%n gcinn"
          return put_n_in(forms[3], n);
      else                              // "%n ceann"
          return put_n_in(forms[4], n);
  }
  kdFatal() << "The function should have been returned in another way\n";

  return QString::null;
}

QString KLocale::translateQt( const char *context, const char *source,
			      const char *message) const
{
  if (!source || !source[0]) {
    kdWarning() << "KLocale: trying to look up \"\" in catalog. "
		<< "Fix the program" << endl;
    return QString::null;
  }

  if ( useDefaultLanguage() ) {
    return QString::null;
  }

  char *newstring = 0;
  const char *translation = 0;
  QString r;

  if ( message && message[0]) {
    char *newstring = new char[strlen(source) + strlen(message) + 5];
    sprintf(newstring, "_: %s\n%s", source, message);
    const char *translation = 0;
    // as copying QString is very fast, it looks slower as it is ;/
    r = translate_priv(newstring, source, &translation);
    delete [] newstring;
    if (translation)
      return r;
  }

  if ( context && context[0] && message && message[0]) {
    newstring = new char[strlen(context) + strlen(message) + 5];
    sprintf(newstring, "_: %s\n%s", context, message);
    // as copying QString is very fast, it looks slower as it is ;/
    r = translate_priv(newstring, source, &translation);
    delete [] newstring;
    if (translation)
      return r;
  }

  r = translate_priv(source, source, &translation);
  if (translation)
    return r;
  return QString::null;
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

bool KLocale::weekStartsMonday() const //deprecated
{
  doFormatInit();
  return (d->weekStartDay==1);
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

static inline void put_it_in( QChar *buffer, uint& index, const QString &s )
{
  for ( uint l = 0; l < s.length(); l++ )
    buffer[index++] = s.at( l );
}

static inline void put_it_in( QChar *buffer, uint& index, int number )
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
    res.insert(pos, thousandsSeparator()); // thousand sep

  // How can we know where we should put the sign?
  res.prepend(neg?negativeSign():positiveSign());

  return res;
}

QString KLocale::formatLong(long num) const
{
  return formatNumber((double)num, 0);
}

QString KLocale::formatNumber(const QString &numStr) const
{
  return formatNumber(numStr.toDouble());
}

QString KLocale::formatDate(const QDate &pDate, bool shortFormat) const
{
  const QString rst = shortFormat?dateFormatShort():dateFormat();

  QString buffer;

  if ( ! pDate.isValid() ) return buffer;

  bool escape = false;

  int year = calendar()->year(pDate);
  int month = calendar()->month(pDate);

  for ( uint format_index = 0; format_index < rst.length(); ++format_index )
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
	      buffer.append('%');
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

void KLocale::setMainCatalogue(const char *catalog)
{
  maincatalogue = catalog;
}

double KLocale::readNumber(const QString &_str, bool * ok) const
{
  QString str = _str.stripWhiteSpace();
  bool neg = str.find(negativeSign()) == 0;
  if (neg)
    str.remove( 0, negativeSign().length() );

  /* will hold the scientific notation portion of the number.
     Example, with 2.34E+23, exponentialPart == "E+23"
  */
  QString exponentialPart;
  int EPos;

  EPos = str.find('E', 0, false);

  if (EPos != -1)
  {
    exponentialPart = str.mid(EPos);
    str = str.left(EPos);
  }

  int pos = str.find(decimalSymbol());
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
  while ( ( pos = major.find( thousandsSeparator() ) ) > 0 )
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
  int thlen = monetaryThousandsSeparator().length();
  int lastpos = 0;
  while ( ( pos = major.find( monetaryThousandsSeparator() ) ) > 0 )
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

QDate KLocale::readDate(const QString &intstr, bool* ok) const
{
  QDate date;
  date = readDate(intstr, ShortFormat, ok);
  if (date.isValid()) return date;
  return readDate(intstr, NormalFormat, ok);
}

QDate KLocale::readDate(const QString &intstr, ReadDateFlags flags, bool* ok) const
{
  QString fmt = ((flags & ShortFormat) ? dateFormatShort() : dateFormat()).simplifyWhiteSpace();
  return readDate( intstr, fmt, ok );
}

QDate KLocale::readDate(const QString &intstr, const QString &fmt, bool* ok) const
{
  //kdDebug() << "KLocale::readDate intstr=" << intstr << " fmt=" << fmt << endl;
  QString str = intstr.simplifyWhiteSpace().lower();
  int day = -1, month = -1;
  // allow the year to be omitted if not in the format
  int year = calendar()->year(QDate::currentDate());
  uint strpos = 0;
  uint fmtpos = 0;

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
      switch (c)
      {
	case 'a':
	case 'A':

          error = true;
	  j = 1;
	  while (error && (j < 8)) {
	    QString s = calendar()->weekDayName(j, c == 'a').lower();
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
	      QString s = calendar()->monthNamePossessive(j, year, c == 'b').lower();
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
	    QString s = calendar()->monthName(j, year, c == 'b').lower();
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
  else
  {
    if (ok) *ok = false;
    return QDate(); // invalid date
  }
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
  QString str = intstr.simplifyWhiteSpace().lower();
  QString Format = timeFormat().simplifyWhiteSpace();
  if (flags & WithoutSeconds)
    Format.remove(QRegExp(".%S"));

  int hour = -1, minute = -1;
  int second = ( (flags & WithoutSeconds) == 0 ) ? -1 : 0; // don't require seconds
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

      // remove space at the beginning
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

//BIC: merge with below
QString KLocale::formatTime(const QTime &pTime, bool includeSecs) const
{
  return formatTime( pTime, includeSecs, false );
}

QString KLocale::formatTime(const QTime &pTime, bool includeSecs, bool isDuration) const
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
    return ret.stripWhiteSpace();
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

bool KLocale::useDefaultLanguage() const
{
  return language() == defaultLanguage();
}

void KLocale::initEncoding(KConfig *)
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

void KLocale::initFileNameEncoding(KConfig *)
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

QCString KLocale::encodeFileNameUTF8( const QString & fileName )
{
  return fileName.utf8();
}

QString KLocale::decodeFileNameUTF8( const QCString & localFileName )
{
  return QString::fromUtf8(localFileName);
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

void KLocale::setDateMonthNamePossessive(bool possessive)
{
  doFormatInit();
  d->dateMonthNamePossessive = possessive;
}

void KLocale::setTimeFormat(const QString & format)
{
  doFormatInit();
  m_timeFormat = format.stripWhiteSpace();
}

void KLocale::setWeekStartsMonday(bool start) //deprecated
{
  doFormatInit();
  if (start)
    d->weekStartDay = 1;
  else
    d->weekStartDay = 7;
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

int KLocale::pageSize() const
{
  doFormatInit();
  return d->pageSize;
}

void KLocale::setPageSize(int pageSize)
{
  // #### check if it's in range??
  doFormatInit();
  d->pageSize = pageSize;
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
         int i = lang.find('_');
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
  const int i = groupName.find('_');
  groupName.replace(0, i, groupName.left(i).lower());

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
  KConfig cfg("l10n/"+code.lower()+"/entry.desktop", true, false, "locale");
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

KLocale::KLocale(const KLocale & rhs)
{
  d = new KLocalePrivate;

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

bool KLocale::setCharset(const QString & ) { return true; }
QString KLocale::charset() const { return QString::fromLatin1("UTF-8"); }

// KDE4: remove
#if 0
void nothing() { i18n("&Next"); }
#endif
