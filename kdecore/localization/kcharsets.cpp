/* This file is part of the KDE libraries
    Copyright (C) 1999 Lars Knoll (knoll@kde.org)
    Copyright (C) 2001, 2003, 2004, 2005, 2006 Nicolas GOUTTE <goutte@kde.org>
    Copyright (C) 2007 Nick Shaforostoff <shafff@ukr.net>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "kcharsets.h"

#include "kfilterdev.h"
#include "kentities.c"

#include "kconfig.h"
#include "kdebug.h"
#include "kglobal.h"
#include "klocale.h"

#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QCharRef>
#include <QtCore/QMutableStringListIterator>
#include <QtCore/QTextCodec>

#include <assert.h>
#include <QHash>

/*
 * ### FIXME KDE4: the name of the encodings should mostly be uppercase
 * The names of this list are user-visible
 * Generate with generate_string_table.pl, input data:
ISO 8859-1
i18n:Western European
ISO 8859-15
i18n:Western European
ISO 8859-14
i18n:Western European
cp 1252
i18n:Western European
IBM850
i18n:Western European
ISO 8859-2
i18n:Central European
ISO 8859-3
i18n:Central European
ISO 8859-4
i18n:Baltic
ISO 8859-13
i18n:Baltic
ISO 8859-16
i18n:South-Eastern Europe
cp 1250
i18n:Central European
cp 1254
i18n:Turkish
cp 1257
i18n:Baltic
KOI8-R
i18n:Cyrillic
ISO 8859-5
i18n:Cyrillic
cp 1251
i18n:Cyrillic
KOI8-U
i18n:Cyrillic
IBM866
i18n:Cyrillic
Big5
i18n:Chinese Traditional
Big5-HKSCS
i18n:Chinese Traditional
GB18030
i18n:Chinese Simplified
GBK
i18n:Chinese Simplified
GB2312
i18n:Chinese Simplified
EUC-KR
i18n:Korean
cp 949
i18n:Korean
sjis
i18n:Japanese
jis7
i18n:Japanese
EUC-JP
i18n:Japanese
ISO 8859-7
i18n:Greek
cp 1253
i18n:Greek
ISO 8859-6
i18n:Arabic
cp 1256
i18n:Arabic
ISO 8859-8
i18n:Hebrew
ISO 8859-8-I
i18n:Hebrew
cp 1255
i18n:Hebrew
ISO 8859-9
i18n:Turkish
TIS620
i18n:Thai
ISO 8859-11
i18n:Thai
UTF-8
i18n:Unicode
UTF-16
i18n:Unicode
utf7
i18n:Unicode
ucs2
i18n:Unicode
ISO 10646-UCS-2
i18n:Unicode
winsami2
i18n:Northern Saami
windows-1258
i18n:Other
IBM874
i18n:Other
TSCII
i18n:Other
 */
/*
 * Notes about the table:
 *
 * - The following entries were disabled and removed from the table:
ibm852
i18n:Central European
pt 154
i18n:Cyrillic              // ### TODO "PT 154" seems to have been removed from Qt
 *
 * - ISO 8559-11 is the deprecated name of TIS-620
 * - utf7 is not in Qt
 * - UTF-16 is duplicated as "ucs2" and "ISO 10646-UCS-2"
 * - windows-1258: TODO
 * - IBM874: TODO
 * - TSCII: TODO
 */
static const char language_for_encoding_string[] =
    "ISO 8859-1\0"
    I18N_NOOP2("@item Text character set", "Western European")"\0"
    "ISO 8859-15\0"
    "ISO 8859-14\0"
    "cp 1252\0"
    "IBM850\0"
    "ISO 8859-2\0"
    I18N_NOOP2("@item Text character set", "Central European")"\0"
    "ISO 8859-3\0"
    "ISO 8859-4\0"
    I18N_NOOP2("@item Text character set", "Baltic")"\0"
    "ISO 8859-13\0"
    "ISO 8859-16\0"
    I18N_NOOP2("@item Text character set", "South-Eastern Europe")"\0"
    "cp 1250\0"
    "cp 1254\0"
    I18N_NOOP2("@item Text character set", "Turkish")"\0"
    "cp 1257\0"
    "KOI8-R\0"
    I18N_NOOP2("@item Text character set", "Cyrillic")"\0"
    "ISO 8859-5\0"
    "cp 1251\0"
    "KOI8-U\0"
    "IBM866\0"
    "Big5\0"
    I18N_NOOP2("@item Text character set", "Chinese Traditional")"\0"
    "Big5-HKSCS\0"
    "GB18030\0"
    I18N_NOOP2("@item Text character set", "Chinese Simplified")"\0"
    "GBK\0"
    "GB2312\0"
    "EUC-KR\0"
    I18N_NOOP2("@item Text character set", "Korean")"\0"
    "cp 949\0"
    "sjis\0"
    I18N_NOOP2("@item Text character set", "Japanese")"\0"
    "jis7\0"
    "EUC-JP\0"
    "ISO 8859-7\0"
    I18N_NOOP2("@item Text character set", "Greek")"\0"
    "cp 1253\0"
    "ISO 8859-6\0"
    I18N_NOOP2("@item Text character set", "Arabic")"\0"
    "cp 1256\0"
    "ISO 8859-8\0"
    I18N_NOOP2("@item Text character set", "Hebrew")"\0"
    "ISO 8859-8-I\0"
    "cp 1255\0"
    "ISO 8859-9\0"
    "TIS620\0"
    I18N_NOOP2("@item Text character set", "Thai")"\0"
    "ISO 8859-11\0"
    "UTF-8\0"
    I18N_NOOP2("@item Text character set", "Unicode")"\0"
    "UTF-16\0"
    "utf7\0"
    "ucs2\0"
    "ISO 10646-UCS-2\0"
    "winsami2\0"
    I18N_NOOP2("@item Text character set", "Northern Saami")"\0"
    "windows-1258\0"
    I18N_NOOP2("@item Text character set", "Other")"\0"
    "IBM874\0"
    "TSCII\0"
    "\0";

static const int language_for_encoding_indices[] = {
       0,   11,   28,   11,   40,   11,   52,   11,
      60,   11,   67,   78,   95,   78,  106,  117,
     124,  117,  136,  148,  169,   78,  177,  185,
     193,  117,  201,  208,  217,  208,  228,  208,
     236,  208,  243,  208,  250,  255,  275,  255,
     286,  294,  313,  294,  317,  294,  324,  331,
     338,  331,  345,  350,  359,  350,  364,  350,
     371,  382,  388,  382,  396,  407,  414,  407,
     422,  433,  440,  433,  453,  433,  461,  185,
     472,  479,  484,  479,  496,  502,  510,  502,
     517,  502,  522,  502,  527,  502,  543,  552,
     567,  580,  586,  580,  593,  580,   -1
};

/*
 * defines some different names for codecs that are built into Qt.
 * The names in this list must be lower-case.
 * input data for generate_string_table.pl:
iso-ir-111
koi8-r
koi unified
koi8-r
us-ascii
iso 8859-1
usascii
iso 8859-1
ascii
iso 8859-1
unicode-1-1-utf-7
utf-7
ucs2
iso-10646-ucs-2
iso10646-1
iso-10646-ucs-2
gb18030.2000-1
gb18030
gb18030.2000-0
gb18030
gbk-0
gbk
gb2312
gbk
gb2312.1980-0
gbk
big5-0
big5
euc-kr
euckr
cp949
cp 949
euc-jp
eucjp
jisx0201.1976-0
eucjp
jisx0208.1983-0
eucjp
jisx0208.1990-0
eucjp
jisx0208.1997-0
eucjp
jisx0212.1990-0
eucjp
jisx0213.2000-1
eucjp
jisx0213.2000-2
eucjp
shift_jis
sjis
shift-jis
sjis
sjis
sjis
iso-2022-jp
jis7
windows850
ibm850
windows866
ibm866
windows-850
ibm850
windows-866
ibm866
cp-10000
apple roman
thai-tis620
iso 8859-11
windows-874
ibm874
windows874
ibm874
cp-874
ibm874
ksc5601.1987-0
euckr
ks_c_5601-1987
euckr
mac-roman
apple roman
macintosh
apple roman
mac
apple roman
csiso2022jp
iso-2022-jp
*/
/*
 * Notes about the table:
 * - using ISO-8859-1 for ASCII is only an approximation (as you cannot test if a character is part of the set)
 * - utf7 is not in Qt
 * - UTF-16 is duplicated as "ucs2" and "ISO 10646-UCS-2"
 * - sjis: appears on the table for x-sjis
 * - jis7: ISO-2022-JP is now the default name in Qt4
 * - cp-874: is it really needed?
 * - mac-roman: appears on the table for x-mac-roman
 * - csiso2022jp: See bug #77243
 */
static const char builtin_string[] =
    "iso-ir-111\0"
    "koi8-r\0"
    "koi unified\0"
    "us-ascii\0"
    "iso 8859-1\0"
    "usascii\0"
    "ascii\0"
    "unicode-1-1-utf-7\0"
    "utf-7\0"
    "ucs2\0"
    "iso-10646-ucs-2\0"
    "iso10646-1\0"
    "gb18030.2000-1\0"
    "gb18030\0"
    "gb18030.2000-0\0"
    "gbk-0\0"
    "gbk\0"
    "gb2312\0"
    "gb2312.1980-0\0"
    "big5-0\0"
    "big5\0"
    "euc-kr\0"
    "euckr\0"
    "cp949\0"
    "cp 949\0"
    "euc-jp\0"
    "eucjp\0"
    "jisx0201.1976-0\0"
    "jisx0208.1983-0\0"
    "jisx0208.1990-0\0"
    "jisx0208.1997-0\0"
    "jisx0212.1990-0\0"
    "jisx0213.2000-1\0"
    "jisx0213.2000-2\0"
    "shift_jis\0"
    "sjis\0"
    "shift-jis\0"
    "iso-2022-jp\0"
    "jis7\0"
    "windows850\0"
    "ibm850\0"
    "windows866\0"
    "ibm866\0"
    "windows-850\0"
    "windows-866\0"
    "cp-10000\0"
    "apple roman\0"
    "thai-tis620\0"
    "iso 8859-11\0"
    "windows-874\0"
    "ibm874\0"
    "windows874\0"
    "cp-874\0"
    "ksc5601.1987-0\0"
    "ks_c_5601-1987\0"
    "mac-roman\0"
    "macintosh\0"
    "mac\0"
    "csiso2022jp\0"
    "\0";

static const int builtin_indices[] = {
       0,   11,   18,   11,   30,   39,   50,   39,
      58,   39,   64,   82,   88,   93,  109,   93,
     120,  135,  143,  135,  158,  164,  168,  164,
     175,  164,  189,  196,  201,  208,  214,  220,
     227,  234,  240,  234,  256,  234,  272,  234,
     288,  234,  304,  234,  320,  234,  336,  234,
     352,  362,  367,  362,  362,  362,  377,  389,
     394,  405,  412,  423,  430,  405,  442,  423,
     454,  463,  475,  487,  499,  511,  518,  511,
     529,  511,  536,  208,  551,  208,  566,  463,
     576,  463,  586,  463,  590,  377,   -1
};

/*
 * some last resort hints in case the charmap file couldn't be found.
 * This gives at least a partial conversion and helps making things readable.
 *
 * the name used as input here is already converted to the more canonical
 * name as defined in the aliases array.
 *
 * Input data:
cp1250
iso-8859-2
koi8-r
iso-8859-5
koi8-u
koi8-r
pt 154
windows-1251
paratype-154
windows-1251
pt-154
windows-1251
 */
/* Notes:
 * - KDE had always "CP 1251" as best fallback to PT 154. As Qt does not offer this encoding anymore, the codepage 1251 is used as fallback.
 */
static const char conversion_hints_string[] =
    "cp1250\0"
    "iso-8859-2\0"
    "koi8-r\0"
    "iso-8859-5\0"
    "koi8-u\0"
    "pt 154\0"
    "windows-1251\0"
    "paratype-154\0"
    "pt-154\0"
    "\0";

static const int conversion_hints_indices[] = {
       0,    7,   18,   25,   36,   18,   43,   50,
      63,   50,   76,   50,   -1
};

// search an array of items index/data, find first matching index
// and return data, or return 0
static inline
const char *kcharsets_array_search(const char *start, const int *indices, const char *entry)
{
    for (int i = 0; indices[i] != -1; i += 2)
        if (qstrcmp(start + indices[i], entry) == 0)
            return start + indices[i + 1];
    return 0;
}


class KCharsetsPrivate
{
public:
    KCharsetsPrivate(KCharsets* _kc)
    {
        kc = _kc;
        codecForNameDict.reserve( 43 );
    }
    // Hash for the encoding names (sensitive case)
    QHash<QByteArray,QTextCodec*> codecForNameDict;
    KCharsets* kc;

    //Cache list so QStrings can be implicitly shared
    QList<QStringList> encodingsByScript;
};

// --------------------------------------------------------------------------

KCharsets::KCharsets()
	:d(new KCharsetsPrivate(this))
{
}

KCharsets::~KCharsets()
{
    delete d;
}

QChar KCharsets::fromEntity(const QString &str)
{
    QChar res = QChar::Null;

    if ( str.isEmpty() )
        return QChar::Null;

    int pos = 0;
    if(str[pos] == QLatin1Char('&')) pos++;

    // Check for '&#000' or '&#x0000' sequence
    if (str[pos] == QLatin1Char('#') && str.length()-pos > 1) {
        bool ok;
        pos++;
        if (str[pos] == QLatin1Char('x') || str[pos] == QLatin1Char('X')) {
            pos++;
            // '&#x0000', hexadecimal character reference
            const QString tmp( str.mid( pos ) );
            res = tmp.toInt(&ok, 16);
        } else {
            //  '&#0000', decimal character reference
            const QString tmp( str.mid( pos ) );
            res = tmp.toInt(&ok, 10);
        }
        if ( ok )
            return res;
        else
            return QChar::Null;
    }

    const QByteArray raw ( str.toLatin1() );
    const entity *e = kde_findEntity( raw, raw.length() );

    if(!e)
    {
        //kDebug( 0 ) << "unknown entity " << str <<", len = " << str.length();
        return QChar::Null;
    }
    //kDebug() << "got entity " << str << " = " << e->code;

    return QChar(e->code);
}

QChar KCharsets::fromEntity(const QString &str, int &len)
{
    // entities are never longer than 8 chars... we start from
    // that length and work backwards...
    len = 8;
    while(len > 0)
    {
        QString tmp = str.left(len);
        QChar res = fromEntity(tmp);
        if( res != QChar::Null ) return res;
        len--;
    }
    return QChar::Null;
}


QString KCharsets::toEntity(const QChar &ch)
{
    QString ent;
    ent.sprintf("&#0x%x;", ch.unicode());
    return ent;
}

QString KCharsets::resolveEntities( const QString &input )
{
    QString text = input;
    const QChar *p = text.unicode();
    const QChar *end = p + text.length();
    const QChar *ampersand = 0;
    bool scanForSemicolon = false;

    for ( ; p < end; ++p ) {
        const QChar ch = *p;

        if ( ch == QLatin1Char('&') ) {
            ampersand = p;
            scanForSemicolon = true;
            continue;
        }

        if ( ch != QLatin1Char(';') || scanForSemicolon == false )
            continue;

        assert( ampersand );

        scanForSemicolon = false;

        const QChar *entityBegin = ampersand + 1;

        const uint entityLength = p - entityBegin;
        if ( entityLength == 0 )
            continue;

        const QChar entityValue = KCharsets::fromEntity( QString( entityBegin, entityLength ) );
        if ( entityValue.isNull() )
            continue;

        const uint ampersandPos = ampersand - text.unicode();

        text[ (int)ampersandPos ] = entityValue;
        text.remove( ampersandPos + 1, entityLength + 1 );
        p = text.unicode() + ampersandPos;
        end = text.unicode() + text.length();
        ampersand = 0;
    }

    return text;
}

QStringList KCharsets::availableEncodingNames() const
{
    QStringList available;
    for ( const int *p = language_for_encoding_indices; *p != -1; p += 2)
        available.append( QString::fromUtf8( language_for_encoding_string + *p ) );
    available.sort();
    return available;
}

#ifndef KDE_NO_DEPRECATED
QString KCharsets::languageForEncoding( const QString &encoding ) const
{
    const char* lang = kcharsets_array_search( (const char*)language_for_encoding_string,
                                               language_for_encoding_indices,
                                               encoding.toUtf8().constData() );
    if ( lang )
        return i18nc( "@item Text character set", lang );
    else
        return i18nc( "@item Text character set", "Other" );
}
#endif

QString KCharsets::descriptionForEncoding( const QString& encoding ) const
{
    const char* lang = kcharsets_array_search( language_for_encoding_string,
                                               language_for_encoding_indices,
                                               encoding.toUtf8() );
    if ( lang )
        return i18nc( "@item %1 character set, %2 encoding", "%1 ( %2 )",
                      i18nc( "@item Text character set", lang ), encoding );
    else
        return i18nc( "@item", "Other encoding (%1)", encoding );
}

QString KCharsets::encodingForName( const QString &descriptiveName ) const
{
    const int left = descriptiveName.lastIndexOf( QLatin1Char('(') );

    if (left<0) // No parenthesis, so assume it is a normal encoding name
	return descriptiveName.trimmed();

    QString name(descriptiveName.mid(left+1));

    const int right = name.lastIndexOf( QLatin1Char(')') );

    if (right<0)
        return name;

    return name.left(right).trimmed();
}

QStringList KCharsets::descriptiveEncodingNames() const
{
    QStringList encodings;
    for ( const int *p = language_for_encoding_indices; *p != -1; p += 2) {
        const QString name = QString::fromUtf8( language_for_encoding_string + p[0] );
        const QString description = i18nc( "@item Text character set", language_for_encoding_string + p[1] );
        encodings.append( i18nc( "@item Text encoding: %1 character set, %2 encoding", "%1 ( %2 )",
                                 description, name ) );
    }
    encodings.sort();
    return encodings;
}

QList<QStringList> KCharsets::encodingsByScript() const
{
    if (!d->encodingsByScript.isEmpty())
        return d->encodingsByScript;
    int i;
    for ( const int *p = language_for_encoding_indices; *p != -1; p += 2) {
        const QString name = QString::fromUtf8( language_for_encoding_string + p[0] );
        const QString description = i18nc("@item Text character set", language_for_encoding_string + p[1] );

        for (i=0; i<d->encodingsByScript.size(); ++i) {
            if (d->encodingsByScript.at(i).at(0) == description) {
                d->encodingsByScript[i].append(name);
                break;
            }
        }

        if (i==d->encodingsByScript.size()) {
            d->encodingsByScript.append(QStringList() << description << name);
        }

    }
    return d->encodingsByScript;
}

QTextCodec* KCharsets::codecForName(const QString &n) const
{
    if ( n == QLatin1String("gb2312") || n == QLatin1String("gbk") )
        return QTextCodec::codecForName( "gb18030" );
    const QByteArray name( n.toLatin1() );
    QTextCodec* codec = codecForNameOrNull( name );
    if ( codec )
        return codec;
    else
        return QTextCodec::codecForName( "iso-8859-1" );
}

QTextCodec* KCharsets::codecForName(const QString &n, bool &ok) const
{
    if (n == QLatin1String("gb2312") || n == QLatin1String("gbk")) {
        ok = true;
        return QTextCodec::codecForName( "gb18030" );
    }
    const QByteArray name( n.toLatin1() );
    QTextCodec* codec = codecForNameOrNull( name );
    if ( codec )
    {
        ok = true;
        return codec;
    }
    else
    {
        ok = false;
        return QTextCodec::codecForName( "iso-8859-1" );
    }
}

QTextCodec *KCharsets::codecForNameOrNull( const QByteArray& n ) const
{
    QTextCodec* codec = 0;

    if (n.isEmpty()) {
        // No name, assume locale (KDE's, not Qt's)
        const QByteArray locale = "->locale<-";
        if ( d->codecForNameDict.contains( locale ) )
            return d->codecForNameDict.value( locale );
        codec = KGlobal::locale()->codecForEncoding();
        d->codecForNameDict.insert("->locale<-", codec);
        return codec;
    }
    // For a non-empty name, lookup the "dictionnary", in a case-sensitive way.
    else if ( d->codecForNameDict.contains( n ) ) {
        return d->codecForNameDict.value( n );
    }

    // If the name is not in the hash table, call directly QTextCoded::codecForName.
    // We assume that QTextCodec is smarter and more maintained than this code.
    codec = QTextCodec::codecForName( n );
    if ( codec ) {
        d->codecForNameDict.insert( n, codec );
        return codec;
    }

    // We have had no luck with QTextCodec::codecForName, so we must now process the name, so that QTextCodec::codecForName could work with it.

    QByteArray name = n.toLower();
    bool changed = false;
    if (name.endsWith("_charset")) { // krazy:exclude=strings
       name.chop( 8 );
       changed = true;
    }
    if ( name.startsWith( "x-" ) ) { // krazy:exclude=strings
       name.remove( 0, 2 ); // remove x- at start
       changed = true;
    }

    if (name.isEmpty()) {
      // We have no name anymore, therefore the name is invalid.
      return 0;
    }

    // We only need to check changed names.
    if ( changed ) {
        codec = QTextCodec::codecForName(name);
        if (codec) {
            d->codecForNameDict.insert( n, codec );
            return codec;
        }
        changed = false;
    }

    // these codecs are built into Qt, but the name given for the codec is different,
    // so QTextCodec did not recognize it.
    QByteArray cname = kcharsets_array_search( builtin_string, builtin_indices, name);

    if(!cname.isEmpty())
        codec = QTextCodec::codecForName(cname);

    if (codec)
    {
        d->codecForNameDict.insert( n, codec );
        return codec;
    }

    // this also failed, the last resort is now to take some compatibility charmap
    // ### TODO: while emergency conversions might be useful at read, it is not sure if they should be done if the application plans to write.
    cname = kcharsets_array_search( conversion_hints_string, conversion_hints_indices, name );

    if (!cname.isEmpty()) {
        codec = QTextCodec::codecForName(cname);
        if (codec) {
            d->codecForNameDict.insert( n, codec );
            return codec;
        }
    }

    // we could not assign a codec, therefore return NULL
    return 0;
}
