/* This file is part of the KDE libraries
    Copyright (C) 1999 Lars Knoll (knoll@mpi-hd.mpg.de)
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

#include <kglobal.h>
#include <klocale.h>

struct KFontStruct;
class KFontStructList;

class KCharsets
{
    friend class KGlobal;

protected:
    /** protected constructor. If you need the kcharsets object, use
	@ref KGlobal::charsets() instead.
    */
    KCharsets();

public:

    /** destructor */
    virtual ~KCharsets(){}

    /** converts an entity to a character. 
	@returns QChar::null if the entity could not be decoded
    */
    QChar fromEntity(const QString &str) const;
    QChar fromEntity(const QString &str, int &len) const;

    /** converts a QChar to an entity */
    QString toEntity(const QChar &ch) const;

    /** lists all available charsets for a given family.
	if family is omitted, it will return all charsets available.*/
    QList<QFont::CharSet> availableCharsets(QString family = QString::null);

    /** as above, but returns the names of the charsets */
    QStringList availableCharsetNames(QString family = QString::null);

    /** returns a QFont, which can print the character given, and is closest 
	to the font given
    */
    QFont fontForChar( const QChar &ch, const QFont &f ) const;

    //FIXME: setQfont without ecnod arg should return charsetforlocale
    // or unicode...
    void setQFont(QFont &f, QFont::CharSet charset = QFont::Unicode);
    void setQFont(QFont &f, QString charset);

    QString name(const QFont &f);
    QString name(QFont::CharSet c);


    bool isAvailable(QFont::CharSet charset);
    bool isAvailable(const QString &charset);

    QFont::CharSet charsetForLocale()
	{  return nameToID(KGlobal::locale()->charset()); };

    bool hasUnicode(QString family);
    bool hasUnicode(QFont &font);

    enum FixedType { FixedUnknown, Fixed, Proportional }; 
    enum WeightType { WeightUnknown, Medium, Bold };
    enum SlantType { SlantUnknown, Normal, Italic };

protected:
    KFontStructList getFontList(KFontStruct mask);
    QString xCharsetName(QFont::CharSet);
    QFont::CharSet nameToID(QString name);
    QFont::CharSet xNameToID(QString name);

};

struct KFontStruct {
    KFontStruct();
    KFontStruct & operator = (QFont &f);
    KFontStruct & operator = (KFontStruct &fs);
    operator QFont();

    QString family;
    QFont::CharSet charset;
    KCharsets::FixedType fixed;
    KCharsets::WeightType weight;
    KCharsets::SlantType slant;
    bool scalable;
};

class KFontStructList : public QList<KFontStruct>
{
public:
    KFontStructList() { } //setAutoDelete(true); }
};

#endif
