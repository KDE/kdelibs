#ifndef _KTRANSLATER_H
#define _KTRANSLATER_H

#include <qintdict.h>

/**
  *
  * KLocale provides support for country specific stuff like
  * the national language.
  * Currently it supports only translating, but it's planned
  * to add here methods for getting the currency sign and other
  * things too.
  * 
  * @author Stephan Kulow <coolo@kde.org>
  * @short class for supporting locale settings and national language
  */
class KLocale {

public:
    /**
      * Create a KLocale with the given catalogue name.
      * If no catalogue is given, the application name is used.
      * the constructor looks for the environment variable $LANG,
      * if not set it looks for an entry NLS/Language in the system
      * configuration file. The format for LANG is de:fr:.., if de
      * (german) is your prefered language and fr (french) is your
      * second prefered language. You can add as many languages as
      * you want. If none of them can be find, the default (C) will 
      * be used.
      * @param catalogue the name of the language file      
      */
    KLocale( const char *catalogue = 0L );
    /**
      * Destructor.
      */
    ~KLocale();

    /**
      * Translate the string into the corresponding string in 
      * the national language, if available. If not, returns
      * the string itself.
      * There should be a KDE wide message file, that contains
      * the most often used phrases, so we can avoid to duplicate
      * the translation of this phrases. If a phrase is not found
      * in the catalogue given to the constructor, it will search
      * in the system catalogue. This makes it possible to over-
      * ride some phrases for your needs.
      * @param index the lookup text and default text, if not found
      */
    const char *translate( const char *index );
    
    /**
      * Translate the string index into the corresponing string 
      * in the national language. If not available, it returns
      * the string text.
      * This function can be used, if you prefer small lookup
      * strings for long textes. 
      * Be aware, that this is, since it differs to the way of
      * the standard gettext method, may confuse the translater.
      * So, I suggest to use constants, that make clear, that
      * the lookup string is not ment to be translated roughly.
      * For example you can use a leading underscore or similar
      * things for it.
      * @param index the lookup text 
      * @param text the default text, if index is not found
      */
    const char *translate( const char *index, const char *text);

    /**
      * Creates an alias for the string text. It will be translated
      + and stored under the integer constant index.
      * This can help you to avoid repeated translation. 
      * Be aware, that this is only needed in cases, where you want
      * to translate it in loops or something like that.
      * In every other case, one of the @ref translate methods are fast
      * enough.
      */
    void alias( long index, const char *text);
    
    /** 
      * Returns an alias, that you have set before or NULL, if not
      * set. This method uses @ref QIntDict.
      */
    char *operator[] ( long key) const;

private:
    char *catalogue;
    QIntDict<char> aliases;
};

#endif
