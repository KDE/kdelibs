
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <qdir.h>
/**
  * Stephan: I don't want to put this in an extra header file, since
  * this would make people think, they can use it within C files, but
  * this is not the case.
  **/

/* Look up MSGID in the DOMAINNAME message catalog for the current CATEGORY
   locale.  */
char *k_dcgettext (const char *__domainname, const char *__msgid,
		   const char *language);

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
#include <Kconfig.h>

#if !HAVE_LC_MESSAGES
/* This value determines the behaviour of the gettext() and dgettext()
   function.  But some system does not have this defined.  Define it
   to a default value.  */
# define LC_MESSAGES (-1)
#endif
 
#define SYSTEM_MESSAGES "kde"

KLocale *KLocale::pLocale = NULL;

#ifdef ENABLE_NLS
/*
bool domain_exist(const char * directory, const char * language, const char * catalogue)
{
    QString tmp;
    tmp.sprintf("%s/%s/%s",directory, language, catalogue);
    return QDir::exists(tmp);
}
*/

KLocale::KLocale( const char *_catalogue ) 
{

#ifdef HAVE_SETLOCALE
    /* Set locale via LC_ALL.  */
    setlocale (LC_ALL, "");
#endif
    
    if ( ! _catalogue )
	_catalogue = kapp->appName().data();
    
    catalogue = new char[ strlen(_catalogue) + 12 ];
    strcpy(catalogue, _catalogue);

    QString languages;
    const char *lang = getenv("LANG");

    if (! lang ) {
	KConfig config;
	config.setGroup("Locale");
	languages = config.readEntry("Language", "C");
    } else
	languages = lang;
   
    if (languages.isEmpty())
	languages = "C";
    else languages += ":C";

    debug("STRING1 %s",languages.data());
    QString directory = kapp->kdedir() + "/share/locale";

    while (1) {
	language = languages.left(languages.find(':'));
	languages = languages.right(languages.length() - language.length() - 1);
	if (language.isEmpty() || language == "C")
	    break;
	QDir d(directory + "/" +  language + "/LC_MESSAGES");
	if (d.exists(QString(catalogue) + ".mo") && d.exists(QString(SYSTEM_MESSAGES) + ".mo"))
	    break;
	debug("STRING %s %s",language.data(),languages.data());
    }
    /* Set the text message domain.  */
    debug("STRING2 %s %s",language.data(),languages.data());
    k_bindtextdomain ( catalogue , directory);
    k_bindtextdomain ( SYSTEM_MESSAGES,  directory);

    enabled = 1;

    if (pLocale == NULL)
	pLocale = this;
}

KLocale::~KLocale()
{
    delete [] catalogue;
    if (pLocale == this)
	pLocale = NULL; // this may be a problem, if some other instance exist
}

const char *KLocale::translate(const char *msgid)
{
    if (!enabled)
      return msgid;

    char *text = k_dcgettext( catalogue, msgid, language.data() );

    if (text == msgid) // just compare the pointers
	return k_dcgettext( SYSTEM_MESSAGES, msgid, language.data() );
    else
	return text;
}

const QString& KLocale::directory() 
{
    return kapp->kdedir() + "/locale/share/" +  language;
}

void KLocale::aliasLocale( const char* text, long int index)
{
    aliases.insert(index, translate(text));
}

#else /* ENABLE_NLS */

KLocale::KLocale( const char *) 
{
    if (pLocale == NULL)
	pLocale = this;  
}

KLocale::~KLocale() 
{
    if (pLocale == this)
	pLocale = NULL; // this may be a problem, if some other instance exis
}

const char *KLocale::translate(const char *msgid)
{
    return msgid;
}

const QString& KLocale::directory() 
{
    return kapp->kdedir() + "/locale/share/";
}

void KLocale::alias(long index, const char* text)
{
    aliases.insert(index, text);
}

#endif /* ENABLE_NLS */


const char *KLocale::getAlias(long key) const
{
    return aliases[key];
}

