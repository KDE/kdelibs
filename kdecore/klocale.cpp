
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

/**
  * Stephan: I don't want to put this in an extra header file, since
  * this would make people think, they can use it within C files, but
  * this is not the case.
  **/

/* Look up MSGID in the current default message catalog for the current
   LC_MESSAGES locale.  If not found, returns MSGID itself (the default
   text).  */
char *k_gettext (const char *__msgid);

/* Look up MSGID in the DOMAINNAME message catalog for the current
   LC_MESSAGES locale.  */
char *k_dgettext (const char *__domainname, const char *__msgid);

/* Look up MSGID in the DOMAINNAME message catalog for the current CATEGORY
   locale.  */
char *k_dcgettext (const char *__domainname, const char *__msgid,
		   const int __category);

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

KLocale::KLocale( const char *_catalogue ) 
{
    char *buffer;

#ifdef HAVE_SETLOCALE
    /* Set locale via LC_ALL.  */
    setlocale (LC_ALL, "");
#endif
    
    if ( ! _catalogue )
	_catalogue = kapp->appName().data();
    
    catalogue = new char[ strlen(_catalogue) + 1 ];
    strcpy(catalogue, _catalogue);
    
    if (! getenv("LANG") ) {
	KConfig config;
	QString languages = "C";
	config.setGroup("Locale");
	languages = config.readEntry("Language", &languages);
	// putenv needs an extra malloc!
	buffer = new char[languages.length() + 6];
	sprintf(buffer, "LANG=%s",languages.data());
	putenv(buffer);
    }
    
    /* Set the text message domain.  */
    k_bindtextdomain ( catalogue , kapp->kdedir() + "/locale");
    k_bindtextdomain ( SYSTEM_MESSAGES,  kapp->kdedir() + "/locale");

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

    char *text = k_dcgettext( catalogue, msgid, LC_MESSAGES);

    if (text == msgid) // just compare the pointers
	return k_dcgettext( SYSTEM_MESSAGES, msgid, LC_MESSAGES);
    else
	return text;
}

/*
const char *KLocale::translate(const char *index, const char *d_text)
{
    if (!enabled)
      return index;

    char *text = k_dcgettext( catalogue, index, LC_MESSAGES);

    if (text == index) { // just compare the pointers
	text =  k_dcgettext( SYSTEM_MESSAGES, index, LC_MESSAGES);
	if (text == index)
	    return d_text;
    }
    return text;
    
}
*/

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

/*
const char *KLocale::translate(const char *, const char *d_text)
{
    return d_text;
}
*/

void KLocale::alias(long index, const char* text)
{
    aliases.insert(index, text);
}

#endif /* ENABLE_NLS */

KLocale *KLocale::klocale()
{
    if (pLocale != NULL)
	return pLocale;

    warning("A new KLocale instance will be created. This one may not be removed.");
    KLocale *kl = new KLocale(); // the constructor will set pLocale!
    kl->enabled = 0;
    return kl;
}

const char *KLocale::getAlias(long key) const
{
    return aliases[key];
}

