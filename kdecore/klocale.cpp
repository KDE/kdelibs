
/* This file is part of the KDE libraries
    Copyright (c) 1997 Stephan Kulow <coolo@kde.org>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 1999 Hans Petter Bieker <bieker@kde.org>

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>

// Overloading of all standard locale functions makes no sense
// Let application use them
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <stdlib.h>

#include <qtextcodec.h>
#include <qdir.h>
#include <qdatetime.h>

#include "kglobal.h"
#include "kstddirs.h"
#include "ksimpleconfig.h"
#include <kinstance.h>

/**
  * Stephan: I don't want to put this in an extra header file, since
  * this would let people think, they can use it within C files, but
  * this is not the case.
  **/

/* Look up MSGID in the DOMAINNAME message catalog for the current CATEGORY
   locale.  */
char *k_dcgettext (const char* __domainname, const char* __msgid,
		   const char* _language);

/* Set the current default message catalog to DOMAINNAME.
   If DOMAINNAME is null, return the current default.
   If DOMAINNAME is "", reset to the default of "messages".  */
char *k_textdomain (const QString& __domainname);

/* Specify that the DOMAINNAME message catalog will be found
   in DIRNAME rather than in the system locale data base.  */
char *k_bindtextdomain (const char* __domainname,
			const char* __dirname);


#include "klocale.h"
#include <kapp.h>
#include <kconfig.h>

#if !HAVE_LC_MESSAGES
/* This value determines the behaviour of the gettext() and dgettext()
   function.  But some system does not have this defined.  Define it
   to a default value.  */
# define LC_MESSAGES (-1)
#endif

#define SYSTEM_MESSAGES "kdelibs"

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

    country=QString::null;
    chrset=QString::null;
    lang=QString::null;

    f = str.find('.');
    if (f >= 0) {
	chrset = str.right(str.length() - f - 1);
	str.truncate(f);
    }

    f = str.find('_');
    if (f >= 0) {
	country = str.right(str.length() - f - 1);
	str.truncate(f);
    }

    lang = str;
}

QString KLocale::language() const
{
  ASSERT(_inited);
  return lang;
}

QString KLocale::MonthName(int i) const
{
    switch (i)
    {
      case 1:   return translate("January");
      case 2:   return translate("February");
      case 3:   return translate("March");
      case 4:   return translate("April");
      case 5:   return translate("May");
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

QString KLocale::WeekDayName (int i) const
{
    switch (i )
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

#ifdef ENABLE_NLS

KLocale::KLocale( const QString& _catalogue )
  : _inited(false), _codec( 0 )
{
#ifdef HAVE_SETLOCALE
    /* "" instructs setlocale to use the default locale
       which is selected from the environment variable LANG.
    */
    setlocale (LC_MESSAGES, "");
#endif
    KConfig *config = KGlobal::instance()->_config;
    if (config)
    {
      KConfigGroupSaver saver(config, QString::fromLatin1("Locale"));
      chset = config->readEntry(QString::fromLatin1("Charset"), QString::fromLatin1("unicode"));
    }
    QString catalogue;

    if (_catalogue.isNull())
      catalogue = kapp->name();
    else
      catalogue = _catalogue;

    catalogues = new QStrList(true);

    initLanguage(config, catalogue);
    initFormat(config);
}

void KLocale::setEncodingLang(const QString &_lang)
{
  QString location = locate("locale", _lang + QString::fromLatin1("/charset"));
  if (!location.isNull())
  {
    QFile f(location);
    if (f.open(IO_ReadOnly))
    {
      char *buf = new char[256];
      int l=f.readLine(buf,256);
      if (l>0)
      {
        if (buf[l-1]=='\n') buf[l-1]=0;
        _codec = QTextCodec::codecForName( buf );
      }
      f.close();
      delete [] buf;
    }
  }
  // default to ISO 8859-1
  if (!_codec)
  {
    debug("charset file invalide or not found. Defaulting to latin 1");
    _codec = QTextCodec::codecForName( "ISO 8859-1" );
  }
}

void KLocale::initLanguage(KConfig *config, const QString& catalogue)
{
  if (!config || _inited)
    return;

  KConfigGroupSaver saver(config, QString::fromLatin1("Locale"));

  const char *g_lang = getenv("KDE_LANG");
  QString languages = QString::fromLatin1(g_lang);

  if (!g_lang)
    languages = config->readEntry(QString::fromLatin1("Language"), QString::fromLatin1("default"));

#ifdef HAVE_SETLOCALE
  // setlocale reads variables LC_* and LANG, and it may use aliases,
  // so we don't have to do it
  g_lang = setlocale(LC_MESSAGES, 0);
#else
  g_lang = getenv("LANG");
#endif

  if (languages.isEmpty() || (languages == QString::fromLatin1("default"))) {
    if (g_lang && g_lang[0]!=0) // LANG value is set and is not ""
      languages = g_lang;
    else
      languages = "C";
  } else
    languages = languages + QString::fromLatin1(":C");

  QString ln,ct,chrset;

  QString _lang;

  // save languages list requested by user
  langs=languages;
  while (1) {
    int f = languages.find(':');
    if (f > 0) {
      _lang = languages.left(f);
      languages.remove(0, _lang.length() + 1);
    } else {
      _lang = languages;
      languages = "";
    }
	
    if (_lang.isEmpty() || _lang == QString::fromLatin1("C"))
      break;

    splitLocale(_lang,ln,ct,chrset);	

    QString lng[4];
    int counter = 0;
    if (!ct.isEmpty()) {
      if (!chrset.isEmpty())
	lng[counter++]=ln+QString::fromLatin1("_")+ct+QString::fromLatin1(".")+chrset;
      lng[counter++]=ln+QString::fromLatin1("_")+ct;
    }
    lng[counter++]=ln;

    int i;
    for(i=0; !lng[i].isNull(); i++)
      if (!locate("locale", lng[i] + QString::fromLatin1("/LC_MESSAGES/") + catalogue + QString::fromLatin1(".mo")).isNull() &&
	  !locate("locale", lng[i] + QString::fromLatin1("/LC_MESSAGES/" SYSTEM_MESSAGES ".mo")).isNull())
	{
	  _lang = lng[i];
	  break;
	}
	
    if (i != 3)
      break;
  }
  lang = _lang; // taking deep copy

  setEncodingLang(lang);
#ifdef HAVE_SETLOCALE
  setlocale(LC_MESSAGES,lang.ascii());
#endif

  insertCatalogue( catalogue );
  insertCatalogue( QString::fromLatin1(SYSTEM_MESSAGES) );

  aliases.setAutoDelete(true);

  number = config->readEntry(QString::fromLatin1("Numeric"), lang);
  money = config->readEntry(QString::fromLatin1("Monetary"), lang);
  time = config->readEntry(QString::fromLatin1("Time"), lang);

  _inited = true;
}

// init some stuff for format*()
void KLocale::initFormat(KConfig *config)
{
  if (!config)
    return;

  KConfigGroupSaver saver(config, QString::fromLatin1("Locale"));

  // Numeric
  KSimpleConfig numentry(locate("locale", QString::fromLatin1("l10n/") + number + QString::fromLatin1("/entry.desktop")), true);
  QString str = config->readEntry(QString::fromLatin1("Numeric"), lang);
  numentry.setGroup(QString::fromLatin1("KCM Locale"));

  _decimalSymbol = config->readEntry(QString::fromLatin1("DecimalSymbol"));
  if (_decimalSymbol.isEmpty())
    _decimalSymbol = numentry.readEntry(QString::fromLatin1("DecimalSymbol"), QString::fromLatin1("."));

  _thousandsSeparator = config->readEntry(QString::fromLatin1("ThousandsSeparator"));
  if (_thousandsSeparator.isEmpty())
    _thousandsSeparator = numentry.readEntry(QString::fromLatin1("ThousandsSeparator"), QString::fromLatin1(","));

  _positiveSign = config->readEntry(QString::fromLatin1("PositiveSign"));
  if (_positiveSign.isEmpty())
    _positiveSign = numentry.readEntry(QString::fromLatin1("PositiveSign"));

  config->readEntry(QString::fromLatin1("NegativeSign"));
  _negativeSign = config->readEntry(QString::fromLatin1("NegativeSign"));
  if (_negativeSign.isEmpty())
    _negativeSign = numentry.readEntry(QString::fromLatin1("NegativeSign"), QString::fromLatin1("-"));

  // Monetary
  KSimpleConfig monentry(locate("locale", QString::fromLatin1("l10n/") + money + QString::fromLatin1("/entry.desktop")), true);
  monentry.setGroup(QString::fromLatin1("KCM Locale"));

  _currencySymbol = config->readEntry(QString::fromLatin1("CurrencySymbol"));
  if (_currencySymbol.isEmpty())
    _currencySymbol = monentry.readEntry(QString::fromLatin1("CurrencySymbol"), QString::fromLatin1("$"));

  _monetaryDecimalSymbol = config->readEntry(QString::fromLatin1("MonetaryDecimalSymbol"));
  if (_monetaryDecimalSymbol.isEmpty())
    _monetaryDecimalSymbol = monentry.readEntry(QString::fromLatin1("MonetaryDecimalSymbol"), QString::fromLatin1("."));

  _monetaryThousandsSeparator = config->readEntry(QString::fromLatin1("MonetaryThousendSeparator"));
  if (_monetaryThousandsSeparator.isEmpty())
    _monetaryThousandsSeparator = monentry.readEntry(QString::fromLatin1("MonetaryThousandsSeparator"), QString::fromLatin1(","));

  _fracDigits = config->readNumEntry(QString::fromLatin1("FractDigits"), -1);
  if (_fracDigits == -1)
    _fracDigits = monentry.readNumEntry(QString::fromLatin1("FractDigits"), 2);

  _positivePrefixCurrencySymbol = monentry.readBoolEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), true);
  _positivePrefixCurrencySymbol = config->readNumEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), _positivePrefixCurrencySymbol);

  _negativePrefixCurrencySymbol = monentry.readBoolEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), true);
  _negativePrefixCurrencySymbol = config->readNumEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), _negativePrefixCurrencySymbol);

  _positiveMonetarySignPosition = (SignPosition)config->readNumEntry(QString::fromLatin1("PositiveMonetarySignPosition"), -1);
  if (_positiveMonetarySignPosition == -1)
    _positiveMonetarySignPosition = (SignPosition)monentry.readNumEntry(QString::fromLatin1("PositiveMonetarySignPosition"), BeforeQuantityMoney);

  _negativeMonetarySignPosition = (SignPosition)config->readNumEntry(QString::fromLatin1("NegativeMonetarySignPosition"), -1);
  if (_negativeMonetarySignPosition == -1)
    _negativeMonetarySignPosition = (SignPosition)monentry.readNumEntry(QString::fromLatin1("NegativeMonetarySignPosition"), ParensAround);

  // date and time
  KSimpleConfig timentry(locate("locale", QString::fromLatin1("l10n/") + time + QString::fromLatin1("/entry.desktop")), true);
  timentry.setGroup(QString::fromLatin1("KCM Locale"));

  _timefmt = config->readEntry(QString::fromLatin1("TimeFormat"));
  if (_timefmt.isEmpty())
    _timefmt = timentry.readEntry(QString::fromLatin1("TimeFormat"), QString::fromLatin1("%I:%M:%S %p"));

  _datefmt = config->readEntry(QString::fromLatin1("DateFormat"));
  if (_datefmt.isEmpty())
    _datefmt = timentry.readEntry(QString::fromLatin1("DateFormat"), QString::fromLatin1("%A %d %B %Y"));

  _datefmtshort = config->readEntry(QString::fromLatin1("DateFormatShort"));
  if (_datefmtshort.isEmpty())
    _datefmtshort = timentry.readEntry(QString::fromLatin1("DateFormatShort"), QString::fromLatin1("%m/%d/%y"));
}

void KLocale::setLanguage(const QString &_lang)
{
  lang = _lang;
  setEncodingLang(lang);

  QStrList *cats = catalogues;
  catalogues = new QStrList;
  for (const char* catalogue = cats->first(); catalogue;
       catalogue = cats->next())
    insertCatalogue( QString::fromLatin1(catalogue) );
  delete cats;
}

void KLocale::setCountry(const QString &_number, const QString &_money, const QString &_time)
{
  if (!_number.isNull()) number = _number;
  if (!_money.isNull()) money = _money;
  if (!_time.isNull()) time = _time;

  initFormat(KGlobal::instance()->_config);
}

void KLocale::setCountry(const QString &country)
{
  setCountry(country, country, country);
}

void KLocale::insertCatalogue( const QString& catalogue )
{
    k_bindtextdomain ( catalogue.ascii() ,
		       KGlobal::dirs()->findResourceDir("locale",
			     lang + QString::fromLatin1("/LC_MESSAGES/") + catalogue + QString::fromLatin1(".mo")).ascii());

    catalogues->append(catalogue.ascii());
}

KLocale::~KLocale()
{
    delete catalogues;
}

QString KLocale::translate(const char* msgid) const
{
    ASSERT(_inited);

    const char *text = msgid;

    for (const char* catalogue = catalogues->first(); catalogue;
	 catalogue = catalogues->next())
    {
	text = k_dcgettext( catalogue, msgid, lang.ascii());
	if ( text != msgid) // we found it
	    break;
    }

    return _codec->toUnicode( text );
}

QString KLocale::decimalSymbol() const {
    return _decimalSymbol;
}

QString KLocale::thousandsSeparator() const {
    return _thousandsSeparator;
}

QString KLocale::currencySymbol() const {
  return _currencySymbol;
}

QString KLocale::monetaryDecimalSymbol() const {
  return _monetaryDecimalSymbol;
}

QString KLocale::monetaryThousandsSeparator() const {
  return _monetaryThousandsSeparator;
}

QString KLocale::positiveSign() const {
  return _positiveSign;
}

QString KLocale::negativeSign() const {
  return _negativeSign;
}

int KLocale::fracDigits() const {
  return _fracDigits;
}

bool KLocale::positivePrefixCurrencySymbol() const {
  return _positivePrefixCurrencySymbol;
}

bool KLocale::negativePrefixCurrencySymbol() const {
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


QString KLocale::formatMoney(double num) const
{
    // the number itself
    bool neg = num < 0;
    // QString craches if fracDigits() > 80...
    QString res = QString::number(neg?-num:num, 'f', fracDigits()>80?80:fracDigits());
    int pos = res.find('.');
    if (pos == -1) pos = res.length();
    else res.replace(pos, 1, monetaryDecimalSymbol());
    while (0 < (pos -= 3))
        res.insert(pos, monetaryThousandsSeparator()); // thousend sep

    // set some variables we need later
    int signpos = neg?negativeMonetarySignPosition():positiveMonetarySignPosition();
    QString sign = neg?negativeSign():positiveSign();
    QString currency = currencySymbol();

    switch (signpos)
    {
    case KLocale::ParensAround:
        res.prepend('(');
        res.append (')');
        break;
    case KLocale::BeforeQuantityMoney:
        res.prepend(sign);
        break;
    case KLocale::AfterQuantityMoney:
        res.append(sign);
        break;
    case KLocale::BeforeMoney:
        currency.prepend(sign);
        break;
    case KLocale::AfterMoney:
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
    QString res = QString::number(neg?-num:num, 'f', (precision==-1) ? 2 : precision);
    int pos = res.find('.');
    if (pos == -1) pos = res.length();
    else res.replace(pos, 1, decimalSymbol());

    if (pos > 4)
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

double KLocale::readMoney(const QString &str, bool * ok) const
{
    bool neg;
    if (negativeMonetarySignPosition() == ParensAround)
    {
        int i1 = str.find('(');
        neg = i1 != -1 && i1 < str.find(')');
    }
    else
        neg = str.find(negativeSign()) != -1;

    QString buf = monetaryDecimalSymbol();
    int pos = str.find(buf);

    QString major = str.left(pos);
    QString minior;
    if (pos != -1) minior = str.mid(pos + buf.length());

    for (pos = major.length(); pos >= 0; pos--)
        if (!major.at(pos).isNumber()) major.remove(pos, 1);

    for (pos = minior.length(); pos >= 0; pos--)
        if (!minior.at(pos).isNumber()) minior.remove(pos, 1);

    QString tot;
    if (neg) tot = '-';
    tot += major + QString::fromLatin1(".") + minior;
    return tot.toDouble(ok);
}

double KLocale::readNumber(const QString &str, bool * ok) const
{
    bool neg = str.find(negativeSign()) == 0;
    QString buf = decimalSymbol();
    int pos = str.find(buf);

    QString major = str.left(pos);
    QString minior;
    if (pos != -1) minior = str.mid(pos + buf.length());

    for (pos = major.length(); pos >= 0; pos--)
        if (!major.at(pos).isNumber()) major.remove(pos, 1);

    for (pos = minior.length(); pos >= 0; pos--)
        if (!minior.at(pos).isNumber()) minior.remove(pos, 1);

    QString tot;
    if (neg) tot = '-';
    tot += major + QString::fromLatin1(".") + minior;
    return tot.toDouble(ok);
}


QString KLocale::formatDate(const QDate &pDate, bool shortfmt) const
{
    QString rst = shortfmt?_datefmtshort:_datefmt;
    QString tmp;

    int i = -1;
    while ( (i = rst.findRev('%', i)) != -1 )
        switch ( rst.at(i + 1).latin1() )
        {
        case 'Y':
                rst.replace(i, 2, QString().sprintf("%02d", pDate.year()));
		continue;
	case 'y':
                rst.replace(i, 2, QString().sprintf("%02d", pDate.year() % 100));
		continue;
	case 'C':
                rst.replace(i, 2, QString().sprintf("%02d", pDate.year() / 100));
		continue;
	case 'n':
                rst.replace(i, 2, QString().sprintf("%2d", pDate.month()));
		continue;
	case 'm':
                rst.replace(i, 2, QString().sprintf("%02d", pDate.month()));
		continue;
	case 'b':
	case 'h':
                rst.replace(i, 2, MonthName(pDate.month()).left(3));
		continue;
	case 'B':
                rst.replace(i, 2, MonthName(pDate.month()));
		continue;
	case 'd':
                rst.replace(i, 2, QString().sprintf("%02d", pDate.day() ));
		continue;
	case 'e':
                rst.replace(i, 2, QString().sprintf("%2d", pDate.day() ));
		continue;
	case 'j':
                rst.replace(i, 2, QString().sprintf("%03d", pDate.dayOfYear()) );
		continue;
	case 'a':
                rst.replace(i, 2, WeekDayName(pDate.dayOfWeek()).left(3));
		continue;
	case 'A':
                rst.replace(i, 2, WeekDayName(pDate.dayOfWeek()));
		continue;
	case 'u':
                rst.replace(i, 2, QString::number( pDate.dayOfWeek() ) );
		continue;
	case 'w':
                rst.replace(i, 2, QString::number( pDate.dayOfWeek()==7?0:pDate.dayOfWeek() ) );
		continue;
	default:
                rst.remove(i, 1);
		i--;
		continue;
        }

    // return _nothing_ if length > 12...
    if (shortfmt && rst.length() > 12) return QString::null;
    return rst;
}

QString KLocale::formatTime(const QTime &pTime, bool includeSecs) const
{
  QString rst(_timefmt);

  int i = -1;
  while ( (i = rst.findRev('%', i)) != -1 )
    switch ( rst.at(i + 1).latin1() )
      {
      case 'H':
	rst.replace(i, 2, QString().sprintf("%02d", pTime.hour()));
	continue;
      case 'k':
	rst.replace(i, 2, QString().sprintf("%2d", pTime.hour()));
	continue;
      case 'I':
	rst.replace(i, 2, QString().sprintf("%02d", pTime.hour() % 12?pTime.hour() % 12 : 12));
	continue;
      case 'l':
	rst.replace(i, 2, QString().sprintf("%2d", (pTime.hour() - 1) % 12 + 1));
	continue;
      case 'M':
	rst.replace(i, 2, QString().sprintf("%02d", pTime.minute()));
	continue;
      case 'S':
	if (includeSecs)
	  rst.replace(i, 2, QString().sprintf("%02d", pTime.second()));
	else
	{
	  i--;
	  rst.remove(i, 3);
	}
	continue;
      case 'p':
	rst.replace(i, 2, pTime.hour() >= 12?translate("pm"):translate("am"));
	continue;
      default:
	rst.remove(i, 1);
	i--;
	continue;
      }

  return rst;
}


QString KLocale::formatDateTime(const QDateTime &pDateTime) const
{
  return formatDate(pDateTime.date()) + QString::fromLatin1(" ") + formatTime(pDateTime.time());
}

void KLocale::aliasLocale( const char* text, long int index)
{
    aliases.insert(index, new QString(translate(text)));
}

QStringList KLocale::languageList() const
{

// a list to be returned
    QStringList list;
// temporary copy of language list
    QString str(langs);

    while(!str.isEmpty()){
      int f = str.find(':');
      if (f >= 0) {
  	list.append(str.left(f));
        str=str.right(str.length()-f-1);
      }
      else{
        list.append(str);
        str="";
      }
    }
    return list;
}

#else /* ENABLE_NLS */

KLocale::KLocale( const QString & ) : _inited(true), lang(0)
{
}

KLocale::~KLocale()
{
}

QString KLocale::translate(const char* msgid) const
{
    return msgid;
}

QString KLocale::decimalSymbol() const {
  return QString(".");
}

QString KLocale::thousandsSeparator() const {
  return QString(",");
}

QString KLocale::currencySymbol() const {
  return QString("$");
}

QString KLocale::monetaryDecimalSymbol() const {
  return QString(".");
}

QString KLocale::monetaryThousandsSeparator() const {
  return QString(",");
}

QString KLocale::positiveSign() const {
  return QString::null;
}

QString KLocale::negativeSign() const {
  return QString("-");
}

int KLocale::fracDigits() const {
  return 2;
}

bool KLocale::positivePrefixCurrencySymbol() const {
  return true;
}

bool KLocale::negativePrefixCurrencySymbol() const {
  return true;
}

KLocale::SignPosition KLocale::positiveMonetarySignPosition() const
{
  return BeforeQuantityMoney;
}

KLocale::SignPosition KLocale::negativeMonetarySignPosition() const
{
  return ParensAround;
}

QString KLocale::formatMoney(double num) const
{
  return QString::number( num );
}

QString KLocale::formatMoney(const QString &numStr) const
{
  return numStr;
}

QString KLocale::formatNumber(double num, int precision) const
{
  return QString::number( num, 'f', precision==-1?2:precision);
}

QString KLocale::formatNumber(const QString &numStr) const
{
  return numStr;
}

QString KLocale::formatDate(const QDate &pDate, bool) const
{
  QString buf;
  buf.sprintf("%.2d/%.2d/%.2d", pDate.month(), pDate.day(), pDate.year() % 100);
  return buf;
}

QString KLocale::formatTime(const QTime &pTime, bool includeSecs) const
{
  QString result = pTime.toString();
  if (!includeSecs)
     result = result.left(5);
  return result;
}

QString KLocale::formatDateTime(const QDateTime &pDateTime) const
{
  return formatDate(pDateTime.date()) + formatTime(pDateTime.time());
}

void KLocale::aliasLocale(const char *text, long int index)
{
    aliases.insert(index, new QString(text));
}

QStringList KLocale::languageList() const {
    return QStringList();
}

void KLocale::insertCatalogue( const QString& ) {
}

void KLocale::setLanguage(
  const QString & /* _lang */)
{
}

void KLocale::setCountry(
  const QString &/*_number*/,
  const QString &/*_money*/,
  const QString &/*_time*/)
{
}

void KLocale::setCountry(
  const QString & /* _country */)
{
}

#endif /* ENABLE_NLS */

QString KLocale::getAlias(long key) const
{
    return *aliases[key];
}

QString i18n(const char* text) {
#ifdef ENABLE_NLS
  register KLocale *instance = KGlobal::locale();
  if (instance)
     return instance->translate(text);
#endif
  return QString::fromLatin1(text);
}

void KLocale::initInstance() {
  if (KGlobal::_locale)
     return;

  KInstance *app = KGlobal::instance();
  if (app)
    KGlobal::_locale = new KLocale(app->instanceName());
  else
    debug("no app name available using KLocale - nothing to do");
}
