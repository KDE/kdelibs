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

#ifdef HAVE_MONETARY_H
#include <monetary.h>
#endif

#include <stdlib.h>

#include <qdir.h>
#include <qdatetime.h>

#include "kglobal.h"
#include "kstddirs.h"

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
static char *_categories[]={"LC_MESSAGES","LC_CTYPE","LC_COLLATE",
                            "LC_TIME","LC_NUMERIC","LC_MONETARY",0};
#endif

const QString KLocale::mergeLocale(const QString& lang,const QString& country,
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
	str = str.left(f);
    }

    country=QString::null;
    chset=QString::null;
    lang=QString::null;
    
    f = str.find('.');
    if (f >= 0) {
	chset = str.right(str.length() - f - 1);
	str = str.left(f);
    }
    
    f = str.find('_');
    if (f >= 0) { 
	country = str.right(str.length() - f - 1);
	str = str.left(f);
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

KLocale::KLocale( QString catalogue ) 
  : lang(0)
{
#ifdef HAVE_SETLOCALE
    /* "" instructs setlocale to use the default locale
       which is selected from the environment variable LANG.
    */
    setlocale (LC_ALL, "");
#endif
    chset="us-ascii";
    if (catalogue.isNull())
        catalogue = kapp->name();
    
    catalogues = new QStrList(true);
    
    const char *g_lang = getenv("KDE_LANG");
    QString languages(g_lang);
    
    bool set_locale_vars=false;

    if (kapp) {
	QString setting;
	KConfig* config = kapp->getConfig();
	config->setGroup("Locale");
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
	    languages = languages.right(languages.length() - 
					_lang.length() - 1);
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
    setlocale(LC_MESSAGES,lang);
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
}

void KLocale::insertCatalogue( const QString& catalogue )
{
    k_bindtextdomain ( catalogue.ascii() , 
		       KGlobal::dirs()->findResourceDir("locale", 
			     lang + "/LC_MESSAGES/" + catalogue + ".mo"));

    catalogues->append(catalogue.ascii());
}

KLocale::~KLocale()
{
    delete catalogues;
}

const QString KLocale::translate(const char* msgid)
{
    const char *text = msgid;

    for (const char* catalogue = catalogues->first(); catalogue; 
	 catalogue = catalogues->next()) 
    {
	text = k_dcgettext( catalogue, msgid, lang);
	if ( text != msgid) // we found it
	    break;
    }

    return QString::fromLocal8Bit( text );
}

QString KLocale::decimalSymbol() const {
#ifdef HAVE_SETLOCALE
  return QString(localeconv()->decimal_point);
#else
  return QString(".");
#endif
}

QString KLocale::thousandsSeparator() const {
#ifdef HAVE_SETLOCALE
  return QString(localeconv()->thousands_sep);
#else
  return QString(",");
#endif
}

QString KLocale::currencySymbol() const {
#ifdef HAVE_SETLOCALE
  return QString(localeconv()->currency_symbol);
#else
  return QString("$");
#endif
}

QString KLocale::monetaryDecimalSymbol() const {
#ifdef HAVE_SETLOCALE
  return QString(localeconv()->mon_decimal_point);
#else
  return QString(".");
#endif
}

QString KLocale::monetaryThousandsSeparator() const {
#ifdef HAVE_SETLOCALE
  return QString(localeconv()->mon_thousands_sep);
#else
  return QString(",");
#endif
}

QString KLocale::positiveSign() const {
#ifdef HAVE_SETLOCALE
  return QString(localeconv()->positive_sign);
#else
  return QString("+");
#endif
}

QString KLocale::negativeSign() const {
#ifdef HAVE_SETLOCALE
  return QString(localeconv()->negative_sign);
#else
  return QString("-");
#endif
}

int KLocale::fracDigits() const {
#ifdef HAVE_SETLOCALE
  return (int) localeconv()->int_frac_digits;
#else
  return 2;
#endif
}

bool KLocale::positivePrefixCurrencySymbol() const {
#ifdef HAVE_SETLOCALE
  return (bool) localeconv()->p_cs_precedes;
#else
  return true;
#endif
}

bool KLocale::negativePrefixCurrencySymbol() const {
#ifdef HAVE_SETLOCALE
  return (bool) localeconv()->n_cs_precedes;
#else
  return true;
#endif
}

KLocale::SignPosition KLocale::positiveMonetarySignPosition() const
{
#ifdef HAVE_SETLOCALE
  return (SignPosition) localeconv()->p_sign_posn;
#else
  return BeforeQuantityMoney;
#endif
}

KLocale::SignPosition KLocale::negativeMonetarySignPosition() const
{
#ifdef HAVE_SETLOCALE
  return (SignPosition) localeconv()->n_sign_posn;
#else
  return ParensAround;
#endif
}

QString KLocale::formatMoney(double num) const
{
#if defined(HAVE_SETLOCALE) && defined(HAVE_STRFMON)
  if (numeric_enabled) {
    char s[256];
    
    if (strfmon(s, 255, "%(n", num) != -1) {
      return QString(s);
    } else {
      return QString().setNum(num);
    }
  } else 
#endif
    return QString().setNum(num);
}

QString KLocale::formatMoney(const QString &numStr) const
{
  return formatMoney(numStr.toDouble());
}

QString KLocale::formatNumber(double num) const
{
#ifdef HAVE_SETLOCALE
  return QString().sprintf("%'.2f", num);
#endif
  return QString().setNum(num);
}

QString KLocale::formatNumber(const QString &numStr) const
{
  return formatNumber(numStr.toDouble());
}

QString KLocale::formatDate(const QDate &pDate) const
{
  struct tm tm;
  char s[256];

  tm.tm_mday = pDate.day();
  tm.tm_mon = pDate.month() - 1;
  tm.tm_year = pDate.year() - 1900;

  if (strftime(s, 255, "%x", &tm) != 0)
    return QString(s);
  else
    return pDate.toString();
}

QString KLocale::formatTime(const QTime &pTime) const
{
  struct tm tm;
  char s[256];

  tm.tm_sec = pTime.second();
  tm.tm_min = pTime.minute();
  tm.tm_hour = pTime.hour();

  if (strftime(s, 255, "%X", &tm) != 0)
    return QString(s);
  else
    return pTime.toString();
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
const QString KLocale::getLocale(const QString& cat){

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

KLocale::KLocale( QString ) : lang(0)
{
}

KLocale::~KLocale() 
{
}

const QString KLocale::translate(const char* msgid)
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

const QString KLocale::getLocale(const QString& ){
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


const QString KLocale::getAlias(long key) const
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

  KApplication *app = KApplication::getKApplication();
  if (app)
    KGlobal::_locale = new KLocale(app->name());
 
}
