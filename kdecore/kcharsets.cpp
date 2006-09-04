/* This file is part of the KDE libraries
    Copyright (C) 1999 Lars Knoll (knoll@kde.org)
    Copyright (C) 2001, 2003, 2004, 2005, 2006 Nicolas GOUTTE <goutte@kde.org>

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

#include <qdir.h>
#include <qfontdatabase.h>
#include <qfontinfo.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#include <assert.h>
#include <QHash>

// ### FIXME KDE4: the name of the encodings should mostly be uppercase
// The names of this list are user-visible
static struct LanguageForEncoding
    {
    const char* index;
    const char* data;
    } const language_for_encoding[] = {
    { "ISO 8859-1", I18N_NOOP( "Western European" ) },
    { "ISO 8859-15", I18N_NOOP( "Western European" ) },
    { "ISO 8859-14", I18N_NOOP( "Western European" ) },
    { "cp 1252", I18N_NOOP( "Western European" ) },
    { "IBM850", I18N_NOOP( "Western European" ) },
    { "ISO 8859-2", I18N_NOOP( "Central European" ) },
    { "ISO 8859-3", I18N_NOOP( "Central European" ) },
    { "ISO 8859-4", I18N_NOOP( "Baltic" ) },
    { "ISO 8859-13", I18N_NOOP( "Baltic" ) },
    { "ISO 8859-16", I18N_NOOP( "South-Eastern Europe" ) },
    { "cp 1250", I18N_NOOP( "Central European" ) },
    { "cp 1254", I18N_NOOP( "Turkish" ) },
    { "cp 1257", I18N_NOOP( "Baltic" ) },
#if 0
    { "ibm852", I18N_NOOP( "Central European" ) },
#endif
    { "KOI8-R", I18N_NOOP( "Cyrillic" ) },
    { "ISO 8859-5", I18N_NOOP( "Cyrillic" ) },
    { "cp 1251", I18N_NOOP( "Cyrillic" ) },
    { "KOI8-U", I18N_NOOP( "Cyrillic" ) },
//    { "pt 154", I18N_NOOP( "Cyrillic" ) }, // ### TODO "PT 154" seems to have been removed from Qt
    { "IBM866", I18N_NOOP( "Cyrillic" ) },
    { "Big5", I18N_NOOP( "Chinese Traditional" ) },
    { "Big5-HKSCS", I18N_NOOP( "Chinese Traditional" ) },
    { "GB18030", I18N_NOOP( "Chinese Simplified" ) },
    { "GBK", I18N_NOOP( "Chinese Simplified" ) },
    { "GB2312", I18N_NOOP( "Chinese Simplified" ) },
    { "EUC-KR", I18N_NOOP( "Korean" ) },
    { "sjis", I18N_NOOP( "Japanese" ) },
    { "jis7", I18N_NOOP( "Japanese" ) },
    { "EUC-JP", I18N_NOOP( "Japanese" ) },
    { "ISO 8859-7", I18N_NOOP( "Greek" ) },
    { "cp 1253", I18N_NOOP( "Greek" ) },
    { "ISO 8859-6", I18N_NOOP( "Arabic" ) },
    { "cp 1256", I18N_NOOP( "Arabic" ) },
    { "ISO 8859-8", I18N_NOOP( "Hebrew" ) },
    { "ISO 8859-8-I", I18N_NOOP( "Hebrew" ) },
    { "cp 1255", I18N_NOOP( "Hebrew" ) },
    { "ISO 8859-9", I18N_NOOP( "Turkish" ) },
    { "TIS620", I18N_NOOP( "Thai" ) },
    { "ISO 8859-11", I18N_NOOP( "Thai" ) }, // ### TODO: DEPRECATED NAME OF TIS-620
    { "UTF-8", I18N_NOOP( "Unicode" ) },
    { "UTF-16", I18N_NOOP( "Unicode" ) },
    { "utf7", I18N_NOOP( "Unicode" ) }, // ### FIXME: UTF-7 is not in Qt
    { "ucs2", I18N_NOOP( "Unicode" ) }, // ### TODO: same as ISO-10646-UCS-2 (so "triples" UTF-16)
    { "ISO 10646-UCS-2", I18N_NOOP( "Unicode") }, // ### TODO: doubles UTF-16
    { "winsami2", I18N_NOOP( "Northern Saami" ) },
    { "windows-1258", I18N_NOOP( "Other" ) }, // ### TODO
    { "IBM874", I18N_NOOP( "Other" ) }, // ### TODO
    { "TSCII", I18N_NOOP( "Other" ) }, // ### TODO
    { 0, 0 } };

// defines some different names for codecs that are built into Qt.
// The names in this list must be lower-case
static struct Builtin
    {
    const char* index;
    const char* data;
    } const builtin[] = {
    { "iso-ir-111", "koi8-r" },
    { "koi unified", "koi8-r" },
    // ### FIXME: at write, using ISO-8859-1 for ASCII is only an approximation (as you cannot test if a character is part of the set).
    { "us-ascii", "iso 8859-1" },
    { "usascii", "iso 8859-1" },
    { "ascii", "iso 8859-1" },
    { "unicode-1-1-utf-7", "utf-7" }, // ### FIXME: UTF-7 is not in Qt
    { "ucs2", "iso-10646-ucs-2" }, // ### TODO: UTF-16
    { "iso10646-1", "iso-10646-ucs-2" }, // ### TODO: UTF-16
    { "gb18030.2000-1", "gb18030" },
    { "gb18030.2000-0", "gb18030" },
    { "gbk-0", "gbk" },
    { "gb2312", "gbk" },
    { "gb2312.1980-0", "gbk" },
    { "big5-0", "big5" },
    { "euc-kr", "euckr" },
    { "euc-jp", "eucjp" },
    { "jisx0201.1976-0", "eucjp" },
    { "jisx0208.1983-0", "eucjp" },
    { "jisx0208.1990-0", "eucjp" },
    { "jisx0208.1997-0", "eucjp" },
    { "jisx0212.1990-0", "eucjp" },
    { "jisx0213.2000-1", "eucjp" },
    { "jisx0213.2000-2", "eucjp" },
    { "shift_jis", "sjis" },
    { "shift-jis", "sjis" },
    { "sjis", "sjis" }, // For x-sjis
    { "iso-2022-jp", "jis7" }, // ### TODO: ISO-2022-JP is now the default name in Qt4
    { "windows850", "ibm850" },
    { "windows866", "ibm866" },
    { "windows-850", "ibm850" },
    { "windows-866", "ibm866" },
    { "cp-10000", "apple roman" },
    { "thai-tis620", "iso 8859-11" }, // ### TODO: TIS-620
    { "windows-874", "ibm874" },
    { "windows874", "ibm874" },
    { "cp-874", "ibm874" }, // ### TODO: really needed?
    { "ksc5601.1987-0", "euckr" },
    { "ks_c_5601-1987", "euckr" },
    { "mac-roman", "apple roman" }, // for x-mac-roman
    { "macintosh", "apple roman" },
    { "mac", "apple roman" },
    { "csiso2022jp", "iso-2022-jp" }, // See bug #77243 
    { 0, 0 }};

#if 0
// some different names for the encodings defined in the charmaps files.
// even though the charmap file names are all uppercase, the names are all lowercase here.
static struct Aliases
    {
    const char* index;
    const char* data;
    } const aliases[] = {
    { "cp852", "ibm852" },
    { "cp-852", "ibm852" },
    { "x-cp-852", "ibm852" },
    { "windows852", "ibm852" },
    { "windows-852", "ibm852" },
    { "x-windows-852", "ibm852" },
    { 0, 0 }};
#endif

// some last resort hints in case the charmap file couldn't be found. This gives at least a partial conversion
// and helps making things readable.
// the name used as input here is already converted to the more canonical name as defined in the aliases array.
static struct ConversionHints
    {
    const char* index;
    const char* data;
    } const conversion_hints[] = {
    { "cp1250", "iso-8859-2" },
    { "koi8-r", "iso-8859-5" },
    { "koi8-u", "koi8-r" },
    // KDE had always "CP 1251" as best fallback to PT 154. As Qt does not offer this encoding anymore, the codepage 1251 is used as fallback.
    { "pt 154", "windows-1251" },
    { "paratype-154", "windows-1251" },
    { "pt-154", "windows-1251" },
    { 0, 0 }};

// search an array of items index/data, index is const char*, data is T, find first matching index
// and return data, or return 0
template< typename T, typename Data >
static Data kcharsets_array_search( const T* start, const char* entry )
{
    for( const T* pos = start;
         pos->index != 0;
         ++pos )
        if( qstrcmp( pos->index, entry ) == 0 )
            return pos->data;
    return 0;
}


class KCharsetsPrivate
{
public:
    KCharsetsPrivate(KCharsets* _kc)
    {
        db = 0;
        kc = _kc;
        codecForNameDict.reserve( 43 );
    }
    ~KCharsetsPrivate()
    {
        delete db;
    }
    QFontDatabase *db;
    // Hash for the encoding names (sensitive case)
    QHash<QByteArray,QTextCodec*> codecForNameDict;
    KCharsets* kc;
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
    if(str[pos] == '&') pos++;

    // Check for '&#000' or '&#x0000' sequence
    if (str[pos] == '#' && str.length()-pos > 1) {
        bool ok;
        pos++;
        if (str[pos] == 'x' || str[pos] == 'X') {
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
        //kDebug( 0 ) << "unknown entity " << str <<", len = " << str.length() << endl;
        return QChar::Null;
    }
    //kDebug() << "got entity " << str << " = " << e->code << endl;

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

        if ( ch == '&' ) {
            ampersand = p;
            scanForSemicolon = true;
            continue;
        }

        if ( ch != ';' || scanForSemicolon == false )
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
    for ( const LanguageForEncoding* pos = language_for_encoding; pos->index; ++pos ) {
        //kDebug(0) << *charsets << " available" << endl;
        available.append( QString::fromUtf8( pos->index ) );
    }
    available.sort();
    return available;
}

QString KCharsets::languageForEncoding( const QString &encoding ) const
{
     const char* lang = kcharsets_array_search< LanguageForEncoding, const char* >
        ( language_for_encoding, encoding.toUtf8());
    if ( lang )
        return i18n( lang );
    else
        return i18n( "Other encoding" );
}

QString KCharsets::encodingForName( const QString &descriptiveName ) const
{
    const int left = descriptiveName.lastIndexOf( '(' );
    
    if (left<0) // No parenthesis, so assume it is a normal encoding name
	return descriptiveName.trimmed();
    
    QString name(descriptiveName.mid(left+1));
    
    const int right = name.lastIndexOf( ')' );
    
    if (right<0) 
        return name;

    return name.left(right).trimmed();
}

QStringList KCharsets::descriptiveEncodingNames() const
{
    QStringList encodings;
    for ( const LanguageForEncoding* pos = language_for_encoding; pos->index; ++pos ) {
        const QString name = QString::fromUtf8( pos->index );
        const QString description = i18n( pos->data );
        encodings.append( i18nc("Descriptive Encoding Name", "%1 ( %2 )",    description ,   name ) );
    }
    encodings.sort();
    return encodings;
}

QTextCodec* KCharsets::codecForName(const QString &n) const
{
    const QByteArray name( n.toLatin1() );
    QTextCodec* codec = codecForNameOrNull( name );
    if ( codec )
        return codec;
    else
        return QTextCodec::codecForName( "iso8859-1" );
}

QTextCodec* KCharsets::codecForName(const QString &n, bool &ok) const
{
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
        return QTextCodec::codecForName( "iso8859-1" );
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
    if (name.endsWith("_charset")) {
       name.chop( 8 );
       changed = true;
    }
    if ( name.startsWith( "x-" ) ) {
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
    QByteArray cname = kcharsets_array_search< Builtin, const char* >( builtin, name.data());

    if(!cname.isEmpty())
        codec = QTextCodec::codecForName(cname);

    if (codec)
    {
        d->codecForNameDict.insert( n, codec );
        return codec;
    }

#ifdef __GNUC__
#warning is it still useful with Qt4 ?
#endif
	//don't forget to remove the #if 0 on a few structs at the top also if you reenable that ;)  (search for 852 )
	//from what I understood, one needs to create a QTextCodecPlugin in order to be able to support a new Codec, but I do not 
	//know how to convert a charmap to a QTextCodec and the real big question is whether we need that at all ...  (mikmak)
        // Yes, it is useful (for examples EBCDIC in Kate or codepages for KOffice filters from/to MS formats) (goutte)
#if 0
    QString dir;
    {
    KConfigGroup cg( KGlobal::config(), "i18n" );
    dir = cg.readPathEntry("i18ndir", QLatin1String("/usr/share/i18n/charmaps"));
    }

    // these are codecs not included in Qt. They can be build up if the corresponding charmap
    // is available in the charmap directory.
    cname = kcharsets_array_search< Aliases, const char* >( aliases, name.data());

    if(cname.isEmpty())
        cname = name;
    cname = cname.toUpper();

    const QString basicName = QLatin1String(cname);
    kDebug() << k_funcinfo << endl << " Trying to find " << cname << " in " << dir << endl;
    
    QString charMapFileName;
    bool gzipped = false; 
    QDir qdir(dir);
    if (!qdir.exists()) {
        // The directory for the charmaps does not even exist... (That is common!)
    }
    else if (qdir.exists(basicName, false)) {
        charMapFileName = basicName;
    }
    else if (qdir.exists(basicName+".gz", false)) {
        charMapFileName = basicName + ".gz";
        gzipped = true;
    }
    else {
        // Check if we are asking a code page
        // If yes, then check "CP99999" and "IBM99999"
        // First we need to find the number of the codepage
        QRegExp regexp("^(X-)?(CP|IBM)(-| )?(0-9)+");
        if ( regexp.search(basicName) != -1) {
            const QString num = regexp.cap(4);
            if (num.isEmpty()) {
                // No number, not a code page (or something went wrong)
            }
            else if (qdir.exists("IBM"+num)) {
                charMapFileName = "IBM"+num;
            }
            else if (qdir.exists("IBM"+num+".gz")) {
                charMapFileName = "IBM"+num+".gz";
                gzipped = true;
            }
            else if (qdir.exists("CP"+num)) {
                charMapFileName = "CP"+num;
            }
            else if (qdir.exists("CP"+num+".gz")) {
                charMapFileName = "CP"+num+".gz";
                gzipped = true;
            }
        }
    }
    
    if (gzipped && !charMapFileName.isEmpty()) {
        KFilterDev gzip(dir + '/' + charMapFileName);
        if (gzip.open(QIODevice::ReadOnly)) {
            kDebug() << "Loading gzipped charset..." << endl;
            codec = QTextCodec::loadCharmap(&gzip);
            gzip.close();
        }
        else
            kWarning() << "Could not open gzipped charset!" << endl;
    }
    else if (!charMapFileName.isEmpty()) {
        codec = QTextCodec::loadCharmapFile(dir + '/' + charMapFileName);
    }

    if(codec) {
        d->codecForNameDict.insert( n, codec );
        return codec;
    }
#endif

    // this also failed, the last resort is now to take some compatibility charmap
    // ### TODO: while emergency conversions might be useful at read, it is not sure if they should be done if the application plans to write.
    cname = kcharsets_array_search< ConversionHints, const char* >( conversion_hints, (const char*)name.data() );

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
