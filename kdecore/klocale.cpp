
/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>

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

#include <qdir.h>
#include <qdatetime.h>

#include "kglobal.h"
#include "kstddirs.h"
#include "ksimpleconfig.h"

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
#include <kcharsets.h>

#if !HAVE_LC_MESSAGES
/* This value determines the behaviour of the gettext() and dgettext()
   function.  But some system does not have this defined.  Define it
   to a default value.  */
# define LC_MESSAGES (-1)
#endif
 
#define SYSTEM_MESSAGES "kdelibs"

#ifdef ENABLE_NLS
static const char *_categories[]={"LC_MESSAGES","LC_CTYPE","LC_COLLATE",
                            "LC_TIME","LC_NUMERIC","LC_MONETARY",0};
#endif

QString KLocale::mergeLocale(const QString& lang,const QString& country,
				   const QString& charset)
{
    if (lang.isEmpty()) 
	return "C";
    QString ret = lang;
    if (!country.isEmpty()) 
	ret += "_" + country;
    if (!charset.isEmpty()) 
	ret+= "." +charset;
    return ret;
}

void KLocale::splitLocale(const QString& aStr,
			  QString& lang,
			  QString& country,
			  QString& chset){
   
    QString str = aStr.copy();

    // just in case, there is another language appended
    int f = str.find(':');
    if (f >= 0) {
	str.truncate(f);
    }

    country=QString::null;
    chset=QString::null;
    lang=QString::null;
    
    f = str.find('.');
    if (f >= 0) {
	chset = str.right(str.length() - f - 1);
	str.truncate(f);
    }
    
    f = str.find('_');
    if (f >= 0) { 
	country = str.right(str.length() - f - 1);
	str.truncate(f);
    }
    
    lang = str;
    
    if (chset.isEmpty() && kapp != 0){
	QString dir, location;

	if (!country.isEmpty()) {
	    dir = lang + "_" + country;
	    location = locate("locale", dir + "/charset");
	}
	
	if (location.isNull()){
	    dir = lang;
	    location = locate("locale", lang + "/charset");
	}
	if (!location.isNull()){
	    QFile f(location);   
	    if (f.exists() && f.open(IO_ReadOnly)){
		char *buf=new char[256];
		int l=f.readLine(buf,256);
		if (l>0){
		    if (buf[l-1]=='\n') buf[l-1]=0;
		    if (KGlobal::charsets()->isAvailable(buf)) chset=buf;
		}
		delete [] buf;
		f.close();
	    }
	}    
    }  
}

#ifdef ENABLE_NLS

KLocale::KLocale( const QString& _catalogue )
  : _inited(false), lang(0)
{
#ifdef HAVE_SETLOCALE
    /* "" instructs setlocale to use the default locale
       which is selected from the environment variable LANG.
    */
    setlocale (LC_ALL, "");
#endif
    chset="us-ascii";
    
    QString catalogue;

    if (_catalogue.isNull())
      catalogue = kapp->name();
    else
      catalogue = _catalogue;
    
    catalogues = new QStrList(true);
        
    initLanguage(KGlobal::_config, catalogue);
}

void KLocale::initLanguage(KConfig *config, const QString& catalogue) 
{
  if (!config || _inited)
    return;
  
  KConfigGroupSaver saver(config, "Locale");

  const char *g_lang = getenv("KDE_LANG");
  QString languages(g_lang);
    
  bool set_locale_vars=false;
  
  if (kapp) {
    QString setting;
    if (!g_lang) 
      languages = config->readEntry("Language", "default");
#ifdef HAVE_SETLOCALE
    // override environment with KDE configuration values
    setting = config->readEntry("Collate", "default");
    if (setting!="default") 
      setlocale (LC_COLLATE, setting.ascii());
    setting = config->readEntry("Time", "default");
    if (setting!="default") 
      setlocale (LC_TIME, setting.ascii());
    setting = config->readEntry("Monetary", "default");
    if (setting!="default") 
      setlocale (LC_MONETARY, setting.ascii());
    setting = config->readEntry("CType", "default");
    if (setting!="default") 
      setlocale (LC_CTYPE, setting.ascii());
    setting = config->readEntry("Numeric", "default");
    if (setting!="default") 
      setlocale (LC_NUMERIC, setting.ascii());
#endif
    set_locale_vars  = config->readBoolEntry("SetLocaleVariables"
					     , false);
  }
  else 
    if (!g_lang) languages = "default";
    
#ifdef HAVE_SETLOCALE
  // setlocale reads variables LC_* and LANG, and it may use aliases,
  // so we don't have to do it
  g_lang = setlocale(LC_MESSAGES, 0);
#else   
  g_lang = getenv("LANG");
#endif
    
  if (languages.isEmpty() || (languages == "default")) {
    if (g_lang && g_lang[0]!=0) // LANG value is set and is not ""
      languages = g_lang;
    else
      languages = "C";
  } else 
    languages = languages + ":C";

  QString ln,ct,chrset;
   
  QString _lang;

  // save languages list requested by user
  langs=languages;    
  while (1) {
    int f = languages.find(':');
    if (f > 0) {
      _lang = languages.left(f);
      languages.remove(0, _lang.length() - 1);
    } else {
      _lang = languages;
      languages = "";
    }
	
    if (_lang.isEmpty() || _lang == "C")
      break;

    splitLocale(_lang,ln,ct,chrset);	

    QString lng[4];
    int counter = 0;
    if (!ct.isEmpty()) {
      if (!chrset.isEmpty())
	lng[counter++]=ln+"_"+ct+"."+chrset;
      lng[counter++]=ln+"_"+ct;
    }
    lng[counter++]=ln;

    int i;
    for(i=0; !lng[i+1].isNull(); i++) 
      if (!locate("locale", lng[i] + "/LC_MESSAGES/" + catalogue + ".mo").isNull() &&
	  !locate("locale", lng[i] + "/LC_MESSAGES/" + SYSTEM_MESSAGES + ".mo").isNull()) 
	{
		    
	  _lang = lng[i];
	  break;
	}
	
    if (i != 3)
      break;
  }
  lang = _lang; // taking deep copy
    
  chset=chrset;
#ifdef HAVE_SETLOCALE
  setlocale(LC_MESSAGES,lang.ascii());
  lc_numeric  = setlocale(LC_NUMERIC, 0); // save these values
  lc_monetary = setlocale(LC_MONETARY, 0); 
  lc_time     = setlocale(LC_TIME, 0);
  setlocale(LC_NUMERIC,"C");          // by default disable LC_NUMERIC
  setlocale(LC_MONETARY, "C");        // by default disable LC_MONETARY
  if (set_locale_vars){
    // set environment variables for all categories
    // maybe we should treat LC_NUMERIC differently (netscape problem)
    QString stmp;
    for(int i=0;_categories[i]!=0;i++) {
      stmp = QString(_categories[i])+ "=" + getLocale(_categories[i]);
      putenv( stmp.ascii() );
    }
  }
  // we should use LC_CTYPE, not LC_MESSAGES for charset
  // however in most cases it should be the same for messages
  // to be readable (there is no i18n messages charset conversion yet)
  // So when LC_CTYPE is not set (is set to "C") better stay
  // with LC_MESSAGES
  QString lc_ctype=getLocale("LC_CTYPE");
  if ( !lc_ctype.isEmpty() && lc_ctype!="C"){
    splitLocale(getLocale("LC_CTYPE"),ln,ct,chrset);
    if (!chrset.isEmpty()) chset=chrset;
  }  
#else
  lc_numeric="C";
  lc_monetary="C";
  lc_time="C";
#endif
  numeric_enabled=false;

  insertCatalogue( catalogue );
  insertCatalogue( SYSTEM_MESSAGES );
  if (chset.isEmpty() || !KGlobal::charsets()->isAvailable(chset)) 
    chset="us-ascii";

  aliases.setAutoDelete(true);

  // init some stuff for format*()
  QString str = config->readEntry("Numeric", lang);
  KSimpleConfig numentry(locate("locale", str + "/entry.desktop"), true);
  numentry.setGroup("KCM Locale");

  _decimalSymbol = config->readEntry("DecimalSymbol");
  if (_decimalSymbol.isNull())
    _decimalSymbol = numentry.readEntry("DecimalSymbol", ".");

  _thousandsSeparator = config->readEntry("ThousandsSeparator");
  if (_thousandsSeparator.isNull())
    _thousandsSeparator = numentry.readEntry("ThousandsSeparator", ",");

  str = config->readEntry("Monetary", lang);
  KSimpleConfig monentry(locate("locale", str + "/entry.desktop"), true);
  monentry.setGroup("KCM Locale");

  _currencySymbol = config->readEntry("CurrencySymbol");
  if (_currencySymbol.isNull())
    _currencySymbol = monentry.readEntry("CurrencySymbol", "$");

  _monetaryDecimalSymbol = config->readEntry("MonetaryDecimalSymbol");
  if (_monetaryDecimalSymbol.isNull())
    _monetaryDecimalSymbol = monentry.readEntry("MonetaryDecimalSymbol", ".");

  _monetaryThousandsSeparator = config->readEntry("MonetaryThousendSeparator");
  if (_monetaryThousandsSeparator.isNull())
    _monetaryThousandsSeparator = monentry.readEntry("MonetaryThousandsSeparator", ",");

  _positiveSign = config->readEntry("PositiveSign");
  if (_positiveSign.isNull())
    _positiveSign = monentry.readEntry("PositiveSign", "+");

  config->readEntry("NegativeSign");
  _negativeSign = config->readEntry("NegativeSign");
  if (_negativeSign.isNull())
    _negativeSign = monentry.readEntry("NegativeSign", "-");

  _fracDigits = config->readNumEntry("FractDigits", -1);
  if (_fracDigits == -1)
    _fracDigits = monentry.readNumEntry("FractDigits", 2);

  _positivePrefixCurrencySymbol = monentry.readBoolEntry("PositivePrefixCurrencySymbol", true);
  _positivePrefixCurrencySymbol = config->readNumEntry("PositivePrefixCurrencySymbol", _positivePrefixCurrencySymbol);

  _negativePrefixCurrencySymbol = monentry.readBoolEntry("NegativePrefixCurrencySymbol", true);
  _negativePrefixCurrencySymbol = config->readNumEntry("NegativePrefixCurrencySymbol", _negativePrefixCurrencySymbol);

  _positiveMonetarySignPosition = (SignPosition)config->readNumEntry("PositiveMonetarySignPosition", -1);
  if (_positiveMonetarySignPosition == -1)
    _positiveMonetarySignPosition = (SignPosition)monentry.readNumEntry("PositiveMonetarySignPosition", BeforeQuantityMoney);

  _negativeMonetarySignPosition = (SignPosition)config->readNumEntry("NegativeMonetarySignPosition", -1);
  if (_negativeMonetarySignPosition == -1)
    _negativeMonetarySignPosition = (SignPosition)monentry.readNumEntry("NegativeMonetarySignPosition", ParensAround);

    // date and time
  str = config->readEntry("Time", lang);
  KSimpleConfig timentry(locate("locale", str + "/entry.desktop"), true);
  timentry.setGroup("KCM Locale");

  _timefmt = config->readEntry("TimeFormat");
  if (_timefmt.isNull())
    _timefmt = timentry.readEntry("TimeFormat", "%I:%M:%S %p");
    
  _datefmt = config->readEntry("DateFormat");
  if (_datefmt.isNull())
    _datefmt = timentry.readEntry("DateFormat", "%m/%d/%y");

  _inited = true;
}

QString KLocale::language() const
{ 
  ASSERT(_inited);
  return lang; 
}

void KLocale::insertCatalogue( const QString& catalogue )
{
    k_bindtextdomain ( catalogue.ascii() , 
		       KGlobal::dirs()->findResourceDir("locale", 
			     lang + "/LC_MESSAGES/" + catalogue + ".mo").ascii());

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

    return QString::fromLocal8Bit( text );
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

QString KLocale::MonthName(int i) const
{
    switch (i)
    {
      case 1:   return translate("January");
      case 2:   return translate("February");
      case 3:   return translate("March");
      case 4:   return translate("April");
      case 5:   return translate("May");
      case 6:   return translate("Juni");
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
      case 4:  return translate("Thuesday");
      case 5:  return translate("Friday");
      case 6:  return translate("Saturday");
      case 7:  return translate("Sunday");
    }

    return QString::null;
}

QString KLocale::formatMoney(double num) const
{
    // the number itself
    bool neg = num < 0;
    QString res = QString::number(neg?-num:num, 'f', fracDigits());
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
    QString res = QString::number(neg?-num:num, 'f', (precision==-1) ? fracDigits() : precision);
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

double KLocale::readMoney(const QString &str, bool * ok) const
{
    bool neg = (negativeMonetarySignPosition() == ParensAround?
        str.find('('):
        str.find(negativeSign())) != -1;

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
    tot += major + "." + minior;
    return tot.toDouble(ok);
}

double KLocale::readNumber(const QString &str, bool * ok) const
{
    bool neg = str.find(negativeSign()) != -1;

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
    tot += major + "." + minior;
    return tot.toDouble(ok);
}


QString KLocale::formatDate(const QDate &pDate) const
{
    QString rst(_datefmt);

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
                rst.replace(i, 2, WeekDayName( pDate.dayOfWeek() ).left(3) );
		continue;
	case 'A':
                rst.replace(i, 2, WeekDayName( pDate.dayOfWeek() ) );
		continue;
	case 'u':
                rst.replace(i, 2, QString::number( pDate.dayOfWeek() ) );
		continue;
	case 'w':
                rst.replace(i, 2, QString::number( pDate.dayOfWeek()==7?0:pDate.dayOfWeek() ) );
		continue;
	case '%':
                rst.remove(i, 1);
		continue;
	case 'n':
                rst.replace(i, 2, "\n");
		continue;
	case 't':
                rst.replace(i, 2, "\t");
                continue;
        }

    return rst;
}

QString KLocale::formatTime(const QTime &pTime) const
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
                rst.replace(i, 2, QString().sprintf("%02d", pTime.second()));
		continue;
        case 'p':
                rst.replace(i, 2, pTime.hour() >= 12?translate("pm"):translate("am"));
		continue;
        case '%':
                rst.remove(i, 1);
		continue;
        case 'n':
                rst.replace(i, 2, "\n");
		continue;
        case 't':
                rst.replace(i, 2, "\t");
		continue;
        }

    return rst;
}


QString KLocale::formatDateTime(const QDateTime &pDateTime) const
{
  return formatDate(pDateTime.date()) + formatTime(pDateTime.time());
}

void KLocale::aliasLocale( const char* text, long int index)
{
    aliases.insert(index, new QString(translate(text)));
}

// Using strings seems to be more portable (for systems without locale.h
QString KLocale::getLocale(const QString& cat){

    cat.upper();
    if (cat=="LC_NUMERIC") return lc_numeric;
    if (cat=="LC_MONETARY") return lc_monetary;
#ifdef HAVE_SETLOCALE        
    else if (cat=="LC_MESSAGES") return setlocale(LC_MESSAGES,0);
    else if (cat=="LC_COLLATE") return setlocale(LC_COLLATE,0);
    else if (cat=="LC_TIME") return setlocale(LC_TIME,0);
    else if (cat=="LC_CTYPE") return setlocale(LC_CTYPE,0);
#endif	
    else return "C";
}

void KLocale::enableNumericLocale(bool on){
#ifdef HAVE_SETLOCALE
  if (on) {
    setlocale(LC_NUMERIC, lc_numeric.ascii());
    setlocale(LC_MONETARY, lc_monetary.ascii());
  } else {
    setlocale(LC_NUMERIC, "C");
    setlocale(LC_MONETARY, "C");
  }
  numeric_enabled=on;
#else  
  numeric_enabled=false;
#endif
}

bool KLocale::numericLocaleEnabled() const {
  return numeric_enabled;
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

KLocale::KLocale( QString ) : _inited(true), lang(0)
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
  return QString("+");
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
  return QString().setNum(num);
}

QString KLocale::formatMoney(const QString &numStr) const
{
  return numStr;
}

QString KLocale::formatNumber(double num) const
{
  return QString().setNum(num);
}

QString KLocale::formatNumber(const QString &numStr) const
{
  return numStr;
}

QString KLocale::formatDate(const QDate &pDate) const
{
  return pDate.toString();
}

QString KLocale::formatTime(const QTime &pTime) const
{
  return pTime.toString();
}

QString KLocale::formatDateTime(const QDateTime &pDateTime) const
{
  return formatDate(pDateTime.date()) + formatTime(pDateTime.time());
}

void KLocale::aliasLocale(const char *text, long int index)
{
    aliases.insert(index, new QString(text));
}

QString KLocale::getLocale(const QString& ) {
    return "C";
}

void  KLocale::enableNumericLocale(bool enable) {
}
 
QStringList KLocale::languageList() const {
    return QStringList();
}

void KLocale::insertCatalogue( const QString& ) {
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
  return text;
}

void KLocale::initInstance() {
  if (KGlobal::_locale) 
     return;

  KApplication *app = KApplication::kApplication();
  if (app)
    KGlobal::_locale = new KLocale(app->name());
 
}
