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
  * this would make people think, they can use it within C files, but
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
    /* Set locale via LC_ALL.  */
    setlocale (LC_ALL, "C");
#endif
    
    if ( ! _catalogue )
      _catalogue = kapp->appName().data();
    
    catalogue = new char[ strlen(_catalogue) + 12 ];
    strcpy(catalogue, _catalogue);
    
    QString languages;
    const char *g_lang = getenv("LANG");
    
    if (! g_lang ) {

	if (kapp) {
	    KConfig* config = kapp->getConfig();
	    config->setGroup("Locale");
	    languages = config->readEntry("Language", "C");
	} else
	    languages = "C";
    } else
      languages = g_lang;
    
    if (languages.isEmpty())
      languages = "C";
    else languages += ":C";
    
    QString directory = KApplication::kdedir() + "/share/locale";
    
    while (1) {
      lang = languages.left(languages.find(':'));
      languages = languages.right(languages.length() - lang.length() - 1);
      if (lang.isEmpty() || lang == "C")
	break;
      QDir d(directory + "/" +  lang + "/LC_MESSAGES");
      if (d.exists(QString(catalogue) + ".mo") && 
	  d.exists(QString(SYSTEM_MESSAGES) + ".mo"))
	break;
    }
    /* Set the text message domain.  */
    k_bindtextdomain ( catalogue , directory);
    k_bindtextdomain ( SYSTEM_MESSAGES,  directory);

}

KLocale::~KLocale()
{
    delete [] catalogue;
}

const char *KLocale::translate(const char *msgid)
{
    char *text = k_dcgettext( catalogue, msgid, lang.data() );

    if (text == msgid) // just compare the pointers
	text = k_dcgettext( SYSTEM_MESSAGES, msgid, lang.data() );
   
    return text;
}

QString KLocale::directory() 
{
    return kapp->kdedir() + "/share/locale/" +  lang;
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
    return kapp->kdedir() + "/share/locale/";
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

