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
      * The constructor looks for the environment variable $LANG,
      * if not set, it looks for an entry Locale/Language in the system
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
      * There is a KDE wide message file, that contains the most
      * often used phrases, so we can avoid to duplicate the
      * translation of this phrases. If a phrase is not found
      * in the catalogue given to the constructor, it will search
      * in the system catalogue. This makes it possible to over-
      * ride some phrases for your needs.
      * @param index the lookup text and default text, if not found
      */
    const char *translate( const char *index );
    
    /**
      * Creates an alias for the string text. It will be translated
      + and stored under the integer constant index.
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
    const char* getAlias( long key ) const;

    /**
      * Returns the language used by this object. The domain AND the
      * library translation must be available in this language. 
      * 'C' is default, if no other available.
      */
    const QString& language() const { return lang; }

    /**
      * Return the base directory, where you can find the language
      * specific things like messages.
      * Please note, that this function is unused currently and may
      * not work.
      */
    QString directory();

private:
    char *catalogue;
    QString lang;
    QIntDict<char> aliases;
};

#endif
