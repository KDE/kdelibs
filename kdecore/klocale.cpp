/* This file is part of the KDE libraries
    Copyright (c) 1997 Stephan Kulow <coolo@kde.org>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 1999-2000 Hans Petter Bieker <bieker@kde.org>

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

#include <stdlib.h>

#include <qtextcodec.h>
#include <qfile.h>
#include <qdatetime.h>

#include "kglobal.h"
#include "kstddirs.h"
#include "ksimpleconfig.h"
#include "kinstance.h"
#include "kconfig.h"
#include "kdebug.h"
#include "klocale.h"
#include "kcharsets.h"

/**
  * Stephan: I don't want to put this in an extra header file, since
  * this would let people think, they can use it within C files, but
  * this is not the case.
  **/

/* Look up MSGID in the DOMAINNAME message catalog for the current CATEGORY
   locale.  */
char *k_dcgettext (const char* __domainname, const char* __msgid,
		   const char* _language);

/* Specify that the DOMAINNAME message catalog will be found
   in DIRNAME rather than in the system locale data base.  */
char *k_bindtextdomain (const char* __domainname,
			const char* __dirname);

static const char * const SYSTEM_MESSAGES = "kdelibs";

static const char *maincatalogue = 0;

class KLocalePrivate {
public:
    int plural_form;
};

void KLocale::splitLocale(const QString& aStr,
			  QString& lang,
			  QString& country,
			  QString& chrset){

    QString str = aStr;

    // just in case, there is another language appended
    int f = str.find(':');
    if (f >= 0) {
	str.truncate(f);
    }

    country = QString::null;
    chrset = QString::null;
    lang = QString::null;

    f = str.find('.');
    if (f >= 0) {
	chrset = str.mid(f + 1);
	str.truncate(f);
    }

    f = str.find('_');
    if (f >= 0) {
	country = str.mid(f + 1);
	str.truncate(f);
    }

    lang = str;
}

QString KLocale::language() const
{
    ASSERT(_inited);
    return lang;
}

QString KLocale::country() const
{
    ASSERT(_inited);
    return _country;
}

QString KLocale::monthName(int i, bool _short) const
{
  if ( _short )
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

QString KLocale::weekDayName (int i, bool _short) const
{
  if ( _short )
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

extern void qt_set_locale_codec( QTextCodec *codec );

KLocale::KLocale( const QString& _catalogue )
  : _inited(false), _codec( 0 )
{
    d = new KLocalePrivate;
    d->plural_form = 0;

    KConfig *config = KGlobal::instance()->config();
    if (config)
    {
      KConfigGroupSaver saver(config, QString::fromLatin1("Locale"));
      chset = config->readEntry(QString::fromLatin1("Charset"));
    }

    if (chset.isEmpty())
    {
        chset = QString::fromLatin1("iso-8859-1");
        qt_set_locale_codec(QTextCodec::codecForMib(4)); // latin-1
        // ### we should default to Qt's default, as thats always more intelligent
    }
    else
    {
        bool ok;
        QTextCodec* nc = KGlobal::charsets()->codecForName(chset, ok);
        // if !ok, we have a problem. it will return latin-1 then, but thats
        // obviously not what the user wants
        if(!ok)
            qWarning("** Warning: charset %s is not known. using ISO 8859-1 instead.", chset.latin1());

        // Lars said: "A utf16 locale doesn't exist. It can't interact with other Unix applications because utf16 has 0's in the string."
        if (!strcmp(nc->name(), "ISO-10646-UCS-2"))
            nc = QTextCodec::codecForName( "UTF-8" );

        qt_set_locale_codec(nc);
    }

    catalogues = new QStrList(true);

    /*
     * Use the first non-null string.
     */
    QString catalogue = _catalogue;
    if (maincatalogue)
      catalogue = QString::fromLatin1(maincatalogue);

    if (catalogue.isEmpty()) {
        kdDebug() << "KLocale instance created called without valid catalogue! Give an argument or call setMaintCatalogue before init\n";
        catalogue = QString::fromLatin1("kdelibs");
    }
    initLanguage(config, catalogue);
    initFormat(config);
}

void KLocale::setEncodingLang(const QString &_lang)
{
  _codec = 0;
  QString location = locate("locale", _lang + QString::fromLatin1("/charset"));
  if (!location.isNull())
  {
    QFile f(location);
    if (f.open(IO_ReadOnly))
    {
      char buf[256];
      f.readLine(buf, 256);
      _codec = KGlobal::charsets()->codecForName( buf );
      f.close();
    }
  }
  // default to UTF8
  if (!_codec)
      _codec = QTextCodec::codecForName( "UTF-8" );
}

void KLocale::initLanguage(KConfig *config, const QString& catalogue)
{
  if (!config || _inited)
    return;

  KConfigGroupSaver saver(config, QString::fromLatin1("Locale"));

  QStringList langlist = config->readListEntry("Language", ':');

  QStringList kdelangs = QStringList::split(':', QFile::decodeName(getenv("KDE_LANG")) );
  for ( QStringList::Iterator kit = kdelangs.begin(); kit != kdelangs.end(); ++kit )
    langlist.prepend( *kit );

  // same order as setlocale use
  langlist << QFile::decodeName(getenv("LC_MESSAGES"));
  langlist << QFile::decodeName(getenv("LC_ALL"));
  langlist << QFile::decodeName(getenv("LANG"));
  langlist << QString::fromLatin1("C");

  for ( QStringList::Iterator it = langlist.begin();
        it != langlist.end();
        ++it )
  {
    QString ln, ct, chrset;
    splitLocale(*it, ln, ct, chrset);

    if (!ct.isEmpty()) {
      langlist.insert(it, ln + '_' + ct);
      if (!chrset.isEmpty())
        langlist.insert(it, ln + '_' + ct + '.' + chrset);
    }
  }

  for ( QStringList::Iterator it = langlist.begin();
        it != langlist.end();
        ++it)
  {
    lang = (*it).latin1();

    if (lang.isEmpty()) continue;

    // This always exists
    if (lang == "C") break;

    QString path = QString::fromLatin1("%1/LC_MESSAGES/%2.mo").arg(lang);
    if (!locate("locale", path.arg(catalogue)).isNull() &&
        !locate("locale", path.arg(SYSTEM_MESSAGES)).isNull())
      break;
  }

  // Remove duplicate entries in reverse so that we
  // can keep user's language preference order intact. (DA)
  for( QStringList::Iterator it = langlist.fromLast();
         it != langlist.begin();
         --it )
  {
    if ( langlist.contains(*it) > 1 )
        it = langlist.remove( it );
  }

  langs = langlist.join(QString::fromLatin1(":"));
  setEncodingLang(lang);

  insertCatalogue( catalogue );
  insertCatalogue( QString::fromLatin1(SYSTEM_MESSAGES) );

  _country = config->readEntry(QString::fromLatin1("Country"));

  _inited = true;
}

// init some stuff for format*()
void KLocale::initFormat(KConfig *config)
{
  if (!config)
    return;
  //kdDebug() << "KLocale::initFormat" << endl;

  // make sure the config files are read using the correct locale
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = this;

  KConfigGroupSaver saver(config, QString::fromLatin1("Locale"));

  KSimpleConfig entry(locate("locale",
                             QString::fromLatin1("l10n/%1/entry.desktop")
                             .arg(_country)), true);
  entry.setGroup(QString::fromLatin1("KCM Locale"));

  // Numeric
  _decimalSymbol = config->readEntry(QString::fromLatin1("DecimalSymbol"));
  if (_decimalSymbol.isNull())
    _decimalSymbol = entry.readEntry(QString::fromLatin1("DecimalSymbol"),
				     QString::fromLatin1("."));

  _thousandsSeparator = config->readEntry(QString::fromLatin1("ThousandsSeparator"));
  if (_thousandsSeparator.isNull())
    _thousandsSeparator = entry.readEntry(QString::fromLatin1("ThousandsSeparator"), QString::fromLatin1(","));
  _thousandsSeparator.replace( QRegExp(QString::fromLatin1("$0")),
				       "" );
  //kdDebug() << "_thousandsSeparator=" << _thousandsSeparator << endl;

  _positiveSign = config->readEntry(QString::fromLatin1("PositiveSign"));
  if (_positiveSign.isNull())
    _positiveSign = entry.readEntry(QString::fromLatin1("PositiveSign"));

  _negativeSign = config->readEntry(QString::fromLatin1("NegativeSign"));
  if (_negativeSign.isNull())
    _negativeSign = entry.readEntry(QString::fromLatin1("NegativeSign"),
				    QString::fromLatin1("-"));

  // Monetary
  _currencySymbol = config->readEntry(QString::fromLatin1("CurrencySymbol"));
  if (_currencySymbol.isNull())
    _currencySymbol = entry.readEntry(QString::fromLatin1("CurrencySymbol"),
				      QString::fromLatin1("$"));

  _monetaryDecimalSymbol = config->readEntry(QString::fromLatin1("MonetaryDecimalSymbol"));
  if (_monetaryDecimalSymbol.isNull())
    _monetaryDecimalSymbol = entry.readEntry(QString::fromLatin1("MonetaryDecimalSymbol"), QString::fromLatin1("."));

  _monetaryThousandsSeparator = config->readEntry(QString::fromLatin1("MonetaryThousendSeparator"));
  if (_monetaryThousandsSeparator.isNull())
    _monetaryThousandsSeparator = entry.readEntry(QString::fromLatin1("MonetaryThousandsSeparator"), QString::fromLatin1(","));
  _monetaryThousandsSeparator.replace(QRegExp(QString::fromLatin1("$0")),
				      QString::null);

  _fracDigits = config->readNumEntry(QString::fromLatin1("FractDigits"), -1);
  if (_fracDigits == -1)
    _fracDigits = entry.readNumEntry(QString::fromLatin1("FractDigits"), 2);

  _positivePrefixCurrencySymbol = entry.readBoolEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), true);
  _positivePrefixCurrencySymbol = config->readNumEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), _positivePrefixCurrencySymbol);

  _negativePrefixCurrencySymbol = entry.readBoolEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), true);
  _negativePrefixCurrencySymbol = config->readNumEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), _negativePrefixCurrencySymbol);

  _positiveMonetarySignPosition = (SignPosition)config->readNumEntry(QString::fromLatin1("PositiveMonetarySignPosition"), -1);
  if (_positiveMonetarySignPosition == -1)
    _positiveMonetarySignPosition = (SignPosition)entry.readNumEntry(QString::fromLatin1("PositiveMonetarySignPosition"), BeforeQuantityMoney);

  _negativeMonetarySignPosition = (SignPosition)config->readNumEntry(QString::fromLatin1("NegativeMonetarySignPosition"), -1);
  if (_negativeMonetarySignPosition == -1)
    _negativeMonetarySignPosition = (SignPosition)entry.readNumEntry(QString::fromLatin1("NegativeMonetarySignPosition"), ParensAround);

  // Date and time
  _timefmt = config->readEntry(QString::fromLatin1("TimeFormat"));
  if (_timefmt.isNull())
    _timefmt = entry.readEntry(QString::fromLatin1("TimeFormat"),
			       QString::fromLatin1("%H:%M:%S"));

  _datefmt = config->readEntry(QString::fromLatin1("DateFormat"));
  if (_datefmt.isNull())
    _datefmt = entry.readEntry(QString::fromLatin1("DateFormat"),
				 QString::fromLatin1("%A %d %B %Y"));

  _datefmtshort = config->readEntry(QString::fromLatin1("DateFormatShort"));
  if (_datefmtshort.isNull())
    _datefmtshort = entry.readEntry(QString::fromLatin1("DateFormatShort"),
				    QString::fromLatin1("%Y-%m-%d"));

  m_weekStartsMonday = entry.readBoolEntry(QString::fromLatin1("WeekStartsMonday"), true);
  m_weekStartsMonday = config->readBoolEntry(QString::fromLatin1("WeekStartsMonday"), m_weekStartsMonday);

  if (lang != "C") {

  QString pf = translate_priv( I18N_NOOP("_: Definition of PluralForm\nLook at klocale.cpp for now"), 0);
  if ( pf.isEmpty() )
      kdWarning() << "found no definition of PluralForm" << endl;
  else if ( pf == "NoPlural" )
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
  else {
      kdWarning() << "Definition of PluralForm is none of NoPlural/TwoForms/French/Gaeilge/Russian/Polish/Slovenian/Lithuanian/Czech/Slovak: " << pf << endl;
      exit(1);
  }
  }

  KGlobal::_locale = lsave;
}

void KLocale::setLanguage(const QString &_lang)
{
  lang = _lang.latin1();
  // k_dcgettext doesn't like NULL pointers, so default to C
  if (lang.isEmpty()) lang = "C";

  setEncodingLang(lang);

  QStrList *cats = catalogues;
  catalogues = new QStrList;
  for (const char* catalogue = cats->first(); catalogue;
       catalogue = cats->next())
    insertCatalogue( QString::fromLatin1(catalogue) );
  delete cats;
}

void KLocale::insertCatalogue( const QString& catalogue )
{
  QString str = QString::fromLatin1("%1/LC_MESSAGES/%2.mo")
    .arg(lang)
    .arg(catalogue);
  str = KGlobal::dirs()->findResourceDir("locale", str);

  if (!str.isEmpty())
      k_bindtextdomain ( catalogue.ascii(),
                         QFile::encodeName(str));

  catalogues->append(catalogue.ascii());
}

KLocale::~KLocale()
{
    delete d;
    delete catalogues;
}

QString KLocale::translate_priv(const char *msgid, const char *fallback, const char **translated) const
{
  ASSERT(_inited);

  if (!msgid || !msgid[0])
  {
    kdDebug() << "KLocale: trying to look up \"\" in catalouge. Fix the program" << endl;
    return QString::null;
  }

  if(lang == "C")
      return QString::fromUtf8( fallback );

  for (const char* catalogue = catalogues->first(); catalogue;
       catalogue = catalogues->next())
  {
    const char *text = k_dcgettext( catalogue, msgid, lang);
    if ( text != msgid) { // we found it
        if (translated)
            *translated = text;
        return _codec->toUnicode( text );
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
    kdDebug() << ("KLocale: trying to look up \"\" in catalouge. Fix the program");
    return QString::null;
  }

  if (lang == "C")
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
   if (forms.count() != x) \
      kdFatal() << "translation of \"" << singular << "\" doesn't contain " << x << " different plural forms as expected\n";

QString KLocale::translate( const char *singular, const char *plural,
                            unsigned long n ) const
{
    if (!singular || !singular[0] || !plural || !plural[0])
    {
        kdDebug() << ("KLocale: trying to look up \"\" in catalouge. Fix the program");
        return QString::null;
    }

    char *newstring = new char[strlen(singular) + strlen(plural) + 6];
    sprintf(newstring, "_n: %s\n%s", singular, plural);
    // as copying QString is very fast, it looks slower as it is ;/
    QString r = translate_priv(newstring, 0);
    delete [] newstring;

    if ( r.isEmpty() || lang == "C") {
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
        break;
    case 1: // TwoForms
        EXPECT_LENGTH( 2 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else
            return put_n_in( forms[1], n);
        break;
    case 2: // French
        EXPECT_LENGTH( 2 );
        if ( n == 1 || n == 0 )
            return put_n_in( forms[0], n);
        else
            return put_n_in( forms[1], n);
        break;
    case 3: // Gaeilge
        EXPECT_LENGTH( 3 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else if ( n == 2 )
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
        break;
    case 4: // Russian, corrected by mok
        EXPECT_LENGTH( 3 );
        if ( n%10 == 1  &&  n%100 != 11)
            return put_n_in( forms[0], n); // odin fail
        else if (( n%10 >= 2 && n%10 <=4 ) && (n%100<10 || n%100>20))
            return put_n_in( forms[1], n); // dva faila
        else
            return put_n_in( forms[2], n); // desyat' failov
        break;
    case 5: // Polish
        EXPECT_LENGTH( 3 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else if ( n%10 >= 2 && n%10 <=4 && (n%100<10 || n%100>=20) )
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
        break;
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
        break;
    case 7: // Lithuanian
        EXPECT_LENGTH( 3 );
        if ( n%10 == 0 || (n%100>=11 && n%100<=19) )
            return put_n_in( forms[2], n);
        else if ( n%10 == 1 )
            return put_n_in( forms[0], n);
        else
            return put_n_in( forms[1], n);
        break;
    case 8: // Czech
        EXPECT_LENGTH( 3 );
        if ( n%100 == 1 )
            return put_n_in( forms[0], n);
        else if (( n%100 >= 2 ) && ( n%100 <= 4 ))
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
        break;
    case 9: // Slovak
        EXPECT_LENGTH( 3 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else if (( n >= 2 ) && ( n <= 4 ))
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
        break;
    }
    kdFatal() << "The function should have been returned in another way\n";

    return QString::null;
}

QString KLocale::translateQt( const char *index, const char *fallback) const
{
    if (!index || !index[0] || !fallback || !fallback[0])
    {
        kdDebug() << ("KLocale: trying to look up \"\" in catalouge. Fix the program");
        return QString::null;
    }

    if (lang == "C")
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
  return m_weekStartsMonday;
}

QString KLocale::decimalSymbol() const
{
  return _decimalSymbol;
}

QString KLocale::thousandsSeparator() const
{
  return _thousandsSeparator;
}

QString KLocale::currencySymbol() const
{
  return _currencySymbol;
}

QString KLocale::monetaryDecimalSymbol() const
{
  return _monetaryDecimalSymbol;
}

QString KLocale::monetaryThousandsSeparator() const
{
  return _monetaryThousandsSeparator;
}

QString KLocale::positiveSign() const
{
  return _positiveSign;
}

QString KLocale::negativeSign() const
{
  return _negativeSign;
}

int KLocale::fracDigits() const
{
  return _fracDigits;
}

bool KLocale::positivePrefixCurrencySymbol() const
{
  return _positivePrefixCurrencySymbol;
}

bool KLocale::negativePrefixCurrencySymbol() const
{
  return _negativePrefixCurrencySymbol;
}

KLocale::SignPosition KLocale::positiveMonetarySignPosition() const
{
  return _positiveMonetarySignPosition;
}

KLocale::SignPosition KLocale::negativeMonetarySignPosition() const
{
  return _negativeMonetarySignPosition;
}

inline void put_it_in( QChar *buffer, uint& index, const QString &s )
{
     for ( uint l = 0; l < s.length(); l++ )
         buffer[index++] = s.at( l );
}

inline void put_it_in( QChar *buffer, uint& index, int number )
{
    buffer[index++] = number / 10 + '0'; \
    buffer[index++] = number % 10 + '0';
}

QString KLocale::formatMoney(double num, const QString &symbol, int precision) const
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
    int signpos = neg?negativeMonetarySignPosition():positiveMonetarySignPosition();
    QString sign = neg?negativeSign():positiveSign();

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

    if(neg?negativePrefixCurrencySymbol():
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

QString KLocale::formatDate(const QDate &pDate, bool shortfmt) const
{
    const QString rst = shortfmt?_datefmtshort:_datefmt;

    // I'm rather safe than sorry
    QChar *buffer = new QChar[rst.length() * 3 / 2 + 50];

    unsigned int index = 0;
    bool escape = false;
    int number = 0;

    for ( uint format_index = 0; format_index < rst.length(); format_index++ )
    {
        switch ( rst.at( format_index ).unicode() )
        {
        case '%':
            if ( escape ) {
                buffer[index++] = '%';
                escape = false;
            } else
                escape = true;
            continue;
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
	for (; str.length() > pos && str.at(pos).isDigit(); pos++) {
		result *= 10;
		result += str.at(pos).digitValue();
	}
	return result;
}

QDate KLocale::readDate(const QString &intstr) const
{
  QDate _date;
  _date = readDate(intstr, true);
  if (_date.isValid()) return _date;
  return readDate(intstr, false);
}

QDate KLocale::readDate(const QString &intstr, bool shortfmt) const
{
	QString str = intstr.simplifyWhiteSpace().lower();
	QString fmt = (shortfmt ? _datefmtshort : _datefmt).simplifyWhiteSpace();

	int day = -1, month = -1, year = -1;
	uint strpos = 0;
	uint fmtpos = 0;


        while (fmt.length() > fmtpos || str.length() > strpos) {
          if ( !(fmt.length() > fmtpos && str.length() > strpos) ) goto error;

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
		switch (c) {
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
			if (c == 'y') {
				if (year < 69) year += 100;
				year += 1900;
			}

			break;
		}
	}
	return QDate(year, month, day);
error:
	return QDate(-1, -1, -1);
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
  QString fmt = _timefmt.simplifyWhiteSpace();
  if (!seconds)
    fmt.replace(QRegExp(QString::fromLatin1(".%S")), QString::null);

  int hour = -1, minute = -1, second = seconds ? -1 : 0; // don't require seconds
  bool g_12h = false;
  bool pm = false;
  uint strpos = 0;
  uint fmtpos = 0;

  while (fmt.length() > fmtpos || str.length() > strpos) {
    if ( !(fmt.length() > fmtpos && str.length() > strpos) ) goto error;

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
    switch (c) {
    case 'p':
      {
	QString s;
	s = translate("pm").lower();
	int len = s.length();
	if (str.mid(strpos, len) == s) {
	  pm = true;
	  strpos += len;
	}
	else {
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
    const QString rst = _timefmt;

    /* only "pm/am" here can grow, the rest shrinks, but
       I'm rather safe than sorry */
    QChar *buffer = new QChar[rst.length() * 3 / 2 + 30];

    uint index = 0;
    bool escape = false;
    int number = 0;

    for ( uint format_index = 0; format_index < rst.length(); format_index++ )
    {
        switch ( rst.at( format_index ).unicode() )
        {
        case '%':
            if ( escape ) {
                buffer[index++] = '%';
                escape = false;
            } else
                escape = true;
            continue;
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
            else { // we remove the seperator sign before the seconds and assume that works everywhere
                --index;
                break;
            }
            break;
        case 'k':
            number = pTime.hour();
            break;
        case 'l':
            // to share the code
            if ( rst.at( format_index ).unicode() == 'l' )
                number = (pTime.hour() + 11) % 12 + 1;
            if ( number / 10 )
                buffer[index++] = number / 10 + '0';
            buffer[index++] = number % 10 + '0';
            break;
        case 'p': {
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
    QString ret( buffer, index );
    delete [] buffer;
    return ret;
}

bool KLocale::use12Clock() const
{
  if ((_timefmt.contains(QString::fromLatin1("%I")) > 0) ||
      (_timefmt.contains(QString::fromLatin1("%l")) > 0))
    return true;
  else
    return false;
}

QString KLocale::languages() const
{
  return langs;
}

QStringList KLocale::languageList() const
{
  return QStringList::split(':', languages());
}

QString KLocale::formatDateTime(const QDateTime &pDateTime, bool shortfmt, bool includeSeconds) const
{
  return translate("concatenation of dates and time", "%1 %2")
    .arg( formatDate( pDateTime.date(), shortfmt ) )
    .arg( formatTime( pDateTime.time(), includeSeconds ) );
}

// #### HPB: This will be merged with the method above in KDE 3
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
    if (app)
        KGlobal::_locale = new KLocale(app->instanceName());
    else
        kdDebug() << "no app name available using KLocale - nothing to do\n";
}

