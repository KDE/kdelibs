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

KLocale::KLocale( const char *_catalogue ) 
{
#ifdef HAVE_SETLOCALE
    /* Set locale via LC_ALL according to environment variables  */
    setlocale (LC_ALL, "");
#endif
    chset="us-ascii";
    if ( ! _catalogue )
	_catalogue = kapp->appName().data();
    
    catalogue = new char[ strlen(_catalogue) + 12 ];
    strcpy(catalogue, _catalogue);
    
    QString languages;
    const char *g_lang = getenv("KDE_LANG");
    
    if (! g_lang ) {

	if (kapp) {
	    KConfig* config = kapp->getConfig();
	    config->setGroup("Locale");
	    languages = config->readEntry("Language", "C");
	} else
	    languages = "C";
    } else
      languages = g_lang;
    
#ifdef HAVE_SETLOCALE
// setlocale reads variables LC_* and LANG, and it may use aliasses,
// so we don't have to do it
    g_lang = setlocale(LC_MESSAGES,0);
#else   
    g_lang = getenv("LANG");
#endif

    if (languages.isEmpty() || (languages == "C")) {
	if (g_lang)
	    languages = g_lang;
	else
	    languages = "C";
    } else 
	languages += ":C";

    
    QString directory = KApplication::kde_localedir();
    
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
	
	while (!lang.isEmpty()) {
		QDir d(directory + "/" +  lang + "/LC_MESSAGES");
	    if (d.exists(QString(catalogue) + ".mo") && 
		d.exists(QString(SYSTEM_MESSAGES) + ".mo")) 
		goto found; // my first time ;-)
	    f = lang.findRev('_');
	    if (f > 0)
		lang = lang.left(lang.findRev('_'));
	    else 
		lang = "";
	}
    }
 found:
    /* Set the text message domain.  */
    k_bindtextdomain ( catalogue , directory);
    k_bindtextdomain ( SYSTEM_MESSAGES,  directory);
    
    QFile f(directory+"/"+lang+"/charset");   
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

KLocale::~KLocale()
{
    delete [] catalogue;
}

const char *KLocale::translate(const char *msgid)
{
    char *text = k_dcgettext( catalogue, msgid, lang);

    if (text == msgid) // just compare the pointers
	text = k_dcgettext( SYSTEM_MESSAGES, msgid, lang);
   
    return text;
}

QString KLocale::directory() 
{
    return KApplication::kde_localedir() +  lang;
}

void KLocale::aliasLocale( const char* text, long int index)
{
    aliases.insert(index, translate(text));
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

#endif /* ENABLE_NLS */


const char *KLocale::getAlias(long key) const
{
    return aliases[key];
}

