/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow (coolo@kde.org)

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

// Overloading of all standard locale functions makes no sense
// Let application use them
#ifdef HAVE_LOCALE_H 
#include <locale.h>
#endif

#include <stdlib.h>

#include <qdir.h>
/**
  * Stephan: I don't want to put this in an extra header file, since
  * this would let people think, they can use it within C files, but
  * this is not the case.
  **/

/* Look up MSGID in the DOMAINNAME message catalog for the current CATEGORY
   locale.  */
char *k_dcgettext (const char *__domainname, const char *__msgid,
		   const char *_language);

/* Set the current default message catalog to DOMAINNAME.
   If DOMAINNAME is null, return the current default.
   If DOMAINNAME is "", reset to the default of "messages".  */
char *k_textdomain (const char *__domainname);

/* Specify that the DOMAINNAME message catalog will be found
   in DIRNAME rather than in the system locale data base.  */
char *k_bindtextdomain (const char *__domainname,
			const char *__dirname);  


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
 
#define SYSTEM_MESSAGES "kde"

#ifdef ENABLE_NLS

KLocale::KLocale( const char *catalogue )
{
#ifdef HAVE_SETLOCALE
    /* Set locale via LC_ALL according to environment variables  */
    setlocale (LC_ALL, "");
#endif
    chset="us-ascii";
    if ( ! catalogue )
	catalogue = kapp->appName().data();
    
    catalogues = new QStrList(true);
    catalogues->append(catalogue);
    
    QString languages;
    const char *g_lang = getenv("KDE_LANG");
    languages = g_lang;
    
    bool set_locale_vars=false;

    if (kapp) {
	QString setting;
	KConfig* config = kapp->getConfig();
	config->setGroup("Locale");
	if (!g_lang) 
	    languages = config->readEntry("Language", "default");
#ifdef HAVE_SETLOCALE
	setting = config->readEntry("Collate", "default");
	if (setting!="default") 
	    setlocale (LC_COLLATE, setting);
	setting = config->readEntry("Time", "default");
	if (setting!="default") 
	    setlocale (LC_TIME, setting);
	setting = config->readEntry("Monetary", "default");
	if (setting!="default") 
	    setlocale (LC_MONETARY, setting);
	setting = config->readEntry("CType", "default");
	if (setting!="default") 
	    setlocale (LC_CTYPE, setting);
	setting = config->readEntry("Numeric", "default");
	if (setting!="default") 
	    setlocale (LC_NUMERIC, setting);
#endif
	set_locale_vars  = config->readBoolEntry("SetLocaleVariables"
						 , false);
    }
    else 
	if (!g_lang) languages = "default";
    
#ifdef HAVE_SETLOCALE
    // setlocale reads variables LC_* and LANG, and it may use aliases,
    // so we don't have to do it
    g_lang = setlocale(LC_MESSAGES,0);
#else   
    g_lang = getenv("LANG");
#endif
    
    if (languages.isEmpty() || (languages == "default")) {
	if (g_lang)
	    languages = g_lang;
	else
	    languages = "C";
    } else 
	languages += ":C";

    QString directory = KApplication::kde_localedir();
    QString ln,ct,chrset;
    
    while (1) {
      int f = languages.find(':');
	if (f > 0) {
	    lang = languages.left(f);
	    languages = languages.right(languages.length() - 
					lang.length() - 1);
	} else {
	    lang = languages;
	    languages = "";
	}
	
	if (lang.isEmpty() || lang == "C")
	    break;

        splitLocale(lang,ln,ct,chrset);	

	QString lng[3];
	lng[0]=ln+"_"+ct+"."+chrset;
	lng[1]=ln+"_"+ct;
	lng[2]=ln;
	int i;
	for(i=0; i<3; i++) {
	  QDir d(directory + "/" + lng[i] + "/LC_MESSAGES");
	  if (d.exists(QString(catalogue) + ".mo") &&
	      d.exists(QString(SYSTEM_MESSAGES) + ".mo")) 
	      {
		  lang = lng[i];
		  break;
	      }
        }
	
	if (i != 3)
	    break;
    }
    
    chset=chrset;
#ifdef HAVE_SETLOCALE
    lc_numeric=setlocale(LC_NUMERIC,0);
    setlocale(LC_NUMERIC,"");          // by default disable LC_NUMERIC
    setlocale(LC_MESSAGES,lang);       
    if (set_locale_vars){
	putenv( QString("LC_NUMERIC=")  + getLocale(LC_NUMERIC) );
	putenv( QString("LC_COLLATE=")  + getLocale(LC_COLLATE) );
	putenv( QString("LC_MONETARY=") + getLocale(LC_MONETARY));
	putenv( QString("LC_TIME=")     + getLocale(LC_TIME)    );
	putenv( QString("LC_MESSAGES=") + getLocale(LC_MESSAGES));
	putenv( QString("LC_CTYPE=")    + getLocale(LC_CTYPE)   );
    }
#else
    lc_numeric="C";
#endif
    numeric_enabled=false;

    insertCatalogue( catalogue );
    insertCatalogue( SYSTEM_MESSAGES );

}

void KLocale::insertCatalogue( const char *catalogue )
{
    const char *ret = k_bindtextdomain ( catalogue , KApplication::kde_localedir() );
    catalogues->append(catalogue);
}

KLocale::~KLocale()
{
    delete catalogues;
}

const char *KLocale::translate(const char *msgid)
{
    const char *text = msgid;
    for (const char *catalogue = catalogues->first(); catalogue; 
	 catalogue = catalogues->next()) 
    {
	text = k_dcgettext( catalogue, msgid, lang);
	if ( text != msgid) // we found it
	    break;
    }

    return text;
}

QString KLocale::directory() 
{
    return KApplication::kde_localedir() +  "/" + lang;
}

void KLocale::aliasLocale( const char* text, long int index)
{
    aliases.insert(index, translate(text));
}

const char *KLocale::getLocale(int CATEGORY){

    if (CATEGORY==LC_NUMERIC) 
	return lc_numeric;
    else 
	return setlocale(CATEGORY,0);
}

void KLocale::splitLocale(const QString& aStr,
			  QString& lang,
			  QString& country,
			  QString &chset) const {
    
    QString str = aStr.copy();

    // just in case, there is another language appended
    int f = str.find(':');
    if (f >= 0) {
	str = str.left(f);
    }

    country="";
    chset="";
    lang="";
    
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
    
    if (chset.isEmpty()){
	QString directory = KApplication::kde_localedir();
	QString dir=directory+"/"+lang+"_"+country;
	QDir d(dir);
	if (!d.exists("charsets")){
	    dir=directory+"/"+lang;
	    d=QDir(dir);
	}  
	if (d.exists("charsets")){
	    QFile f(dir+"/charset");   
	    if (f.exists() && f.open(IO_ReadOnly)){
		char *buf=new char[256];
		int l=f.readLine(buf,256);
		if (l>0){
		    if (buf[l-1]=='\n') buf[l-1]=0;
		    if (KCharset(buf).ok()) chset=buf;
		}
		delete buf;
		f.close();
	    }
	}    
    }  
}

const QString KLocale::mergeLocale(const QString& lang,const QString& country,
				   const QString &charset) const
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

void KLocale::enableNumericLocale(bool on){
#ifdef HAVE_SETLOCALE
    if (on) 
	setlocale(LC_NUMERIC,lc_numeric);
    else 
	setlocale(LC_NUMERIC,"C");  
    numeric_enabled=on;
#else  
    numeric_enabled=false;
#endif
}

bool KLocale::numericLocaleEnabled()const{
    return numeric_enabled;
}
 
#else /* ENABLE_NLS */

KLocale::KLocale( const char *) 
{
}

KLocale::~KLocale() 
{
}

const char *KLocale::translate(const char *msgid)
{
    return msgid;
}

QString KLocale::directory() 
{
    return KApplication::kde_localedir();
}

void KLocale::aliasLocale(const char* text, long int index)
{
    aliases.insert(index, text);
}

void  KLocale::getLocale(int){
}

void  KLocale::splitLocale(const QString&,QString& lang,
			   QString& country,QString& charset) const
{
    lang=country=charset="";
}

QString  KLocale::mergeLocale(const QString&,const QString&,
			      const QString &)const
{
    return "";
}

void  KLocale::enableNumericLocale(bool){
}

bool  KLocale::numericLocaleEnabled()const{
    return false;
}
 
#endif /* ENABLE_NLS */


const char *KLocale::getAlias(long key) const
{
    return aliases[key];
}


