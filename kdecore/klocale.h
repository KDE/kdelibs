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
#ifndef _KLOCALE_H
#define _KLOCALE_H

#include <qintdict.h>
#include <qstring.h>
#include <qstringlist.h>

class QStrList;

/*
  #ifndef klocale
  #define klocale KApplication::getKApplication()->getLocale()
  #endif
*/

/**
  *
  * KLocale provides support for country specific stuff like
  * the national language.
  * Currently it supports only translating, but it's planned
  * to add here methods for getting the currency sign and other
  * things too.
  * 
  * @author Stephan Kulow (coolo@kde.org)
  * @short class for supporting locale settings and national language
  */
class KLocale {

public:
    /**
      * Create a KLocale with the given catalogue name.
      * If no catalogue is given, the application name is used.
      * The constructor looks for an entry Locale/Language in the
      * configuration file. 
      * If nothing is set there, it looks for the environment variable
      * $LANG. The format for LANG is de:fr:.., if de
      * (german) is your prefered language and fr (french) is your
      * second prefered language. You can add as many languages as
      * you want. If none of them can be find, the default (C) will 
      * be used.
      * @param catalogue the name of the language file
      */
    KLocale( QString catalogue = QString::null );

    /**
      * Destructor.
      */
    ~KLocale();

    /**
      * Translate the string into the corresponding string in 
      * the national language, if available. If not, returns
      * the string itself.
      * There is a KDE wide message file, that contains the most
      * often used phrases, so we can avoid to duplicate the
      * translation of this phrases. If a phrase is not found
      * in the catalogue given to the constructor, it will search
      * in the system catalogue. This makes it possible to override
      * some phrases for your needs.
      * @param index the lookup text and default text, if not found
      */
    const QString translate( const char *index );
    
    /**
      * Creates an alias for the string text. It will be translated
      * and stored under the integer constant index.
      * This can help you to avoid repeated translation. 
      * Be aware, that this is only needed in cases, where you want
      * to translate it in loops or something like that.
      * In every other case, the @ref translate methods is fast
      * enough.
      */
    void aliasLocale( const char *text, long int index);
    
    /** 
      * Returns an alias, that you have set before or 0, if not
      * set. This method uses @ref QIntDict.
      */
    const QString getAlias( long key ) const;

    /**
      * Returns the language used by this object. The domain AND the
      * library translation must be available in this language. 
      * 'C' is default, if no other available.
      */
    const QString language() const { return lang; }

    /**
      * Returns the languages selected by user.
      *
      * @return String containing locale codes separated by colons
      */
    const QString languages() const { return langs; }

    /**
      * Returns the languages selected by user.
      *
      * @return List of language codes
      */
    QStringList languageList() const;
    
    /**
      * Returns the charset name used by selected locale.
      * Special file with charset name must be available
      * "us-ascii" is default 
      */
    const QString charset() const { return chset; }
    
    /**
      * Returns the base directory, where you can find the language
      * specific things like messages.
      * Please note, that this function is unused currently and may
      * not work.
      */
    QString directory();

    /**
     * adds anther catalogue to search for translation lookup.
     * This function is useful for extern libraries and/or code,
     * that provides it's own messages.
     * 
     * If the catalogue does not exist for the chosen language,
     * it will be ignored and C will be used.
     **/
    void insertCatalogue(const QString& catalogue);

    /**
       The category argument tells the setlocale() function which attributes to
       set.  The choices are:
       
       LC_COLLATE      Changes the behavior of the strcoll() and strxfrm() functions.
       LC_CTYPE        Changes the behavior of the character-handling functions:
                       isalpha(), islower(), isupper(), isprint(), ...
       LC_MESSAGES     Changes the language in which messages are displayed.
       LC_MONETARY     Changes the information returned by localeconv().
       LC_NUMERIC      Changes the radix character for numeric conversions.
       LC_TIME         Changes the behavior of the strftime() function.
       LC_ALL          Changes all of the above.
    **/
    const QString getLocale(const QString& CATEGORY);

    /** 
     * returns the parts of the parameter str understood as language setting
     * the format is language_country.charset
     */ 
    static void splitLocale(const QString& str,QString& language, QString& country,
                     QString &charset);

    /** 
     * does the same as the above, just reverse
     */
    static const QString mergeLocale(const QString& lang, const QString& country,
			      const QString& charset);

    /**
     * if the application can handle localized numeric values, it should
     * call this function.
     * 
     * By default, this is disabled
     **/
    void enableNumericLocale(bool on = true);

    /**
     * returns, if the radix character for numeric conversions is set to
     * locale settings or to the POSIX standards.
     **/
    bool numericLocaleEnabled() const;
     
private:
    QStrList *catalogues;
    QIntDict<QString> aliases;
    const char* lang;
    QString chset;
    QString lc_numeric;
    bool numeric_enabled;
    QString langs;

  // Disallow assignment and copy-construction
  KLocale( const KLocale& );
  KLocale& operator= ( const KLocale& );
};

QString i18n(const char *text);

#endif
