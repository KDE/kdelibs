/* This file is part of the KDE libraries
    Copyright (C) 1999 Lars Knoll (knoll@kde.org)
    $Id$

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
#ifndef KCHARSETS_H
#define KCHARSETS_H

#include <qstring.h>
#include <qfont.h>
#include <qstringlist.h>
#include <qlist.h>

class KGlobal;
class KCharsetsPrivate;

class QTextCodec;

/**
 * Charset font and encoder/decoder handling.
 *
 * This is needed, because Qt's font matching algorithm gives the font
 * family a higher priority than the charset.  For many applications
 * this is not acceptable, since it can totally obscure the output,
 * in languages which use non iso-8859-1 charsets.
 *
 * @author Lars Knoll <knoll@kde.org>
 * @version $Id$
 */
class KCharsets
{
    friend class KGlobal;

protected:
    /** protected constructor. If you need the kcharsets object, use
        @ref KGlobal::charsets() instead.
    */
    KCharsets();

public:

    /**
     * destructor
     */
    virtual ~KCharsets();

    /**
     * provided for compatibility.
     */
    QTextCodec *codecForName(const QString &name) const;
    
    /**
     * tries to find a QTextCodec to convert the given encoding from and to
     * Unicode. If no codec could be found the latin1 codec will be returned an
     * ok will be set to false.
     */
    QTextCodec *codecForName(const QString &n, bool &ok) const;

    /**
     * converts an entity to a character. The string must contain only the
     * entity without the trailing ';'.
     *  @returns QChar::null if the entity could not be decoded.
     */
    static QChar fromEntity(const QString &str);
    /**
     * Overloaded member function. Tries to find an entity in the
     * @ref QString str.
     * @returns a decoded entity if one could be found, QChar::null
     * otherwise
     * @param len is a return value, that gives the length of the decoded
     * entity.
     */
    static QChar fromEntity(const QString &str, int &len);

    /**
     * converts a QChar to an entity. The returned string does already
     * contain the leading '&' and the trailing ';'.
     */
    static QString toEntity(const QChar &ch);

    /**
     * Scans the given string for entities (like &amp;amp;) and resolves them
     * using fromEntity.
     */
    static QString resolveEntities( const QString &text );

    /**
     * Lists all available encodings as names.
     */
    QStringList availableEncodingNames();

    /**
     * Lists the available encoding names together with a more descriptive language.
     */
    QStringList descriptiveEncodingNames();

    /**
     * Lists all languages.
     */
    QStringList languages();
    
    /**
     * Lists all available encodings for language name 'language'.
     */
    QStringList encodingsForLanguage( const QString &language );
    
    /**
     * returns the language the encoding is used for. 
     */
    QString languageForEncoding( const QString &encoding );

    /**
     * returns the encoding for a string obtained with descriptiveEncodingNames()
     */
    QString encodingForName( const QString &descriptiveName );
	    
private:
    KCharsetsPrivate *d;
};

#endif
