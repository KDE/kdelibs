/* This file is part of the KDE libraries
    Copyright (C) 1999 Lars Knoll (knoll@kde.org)

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

static const char * const language_names[] = {
	I18N_NOOP( "Other" ),
	I18N_NOOP( "Arabic" ),
	I18N_NOOP( "Baltic" ),
	I18N_NOOP( "Central European" ),
	I18N_NOOP( "Chinese Simplified" ),
	I18N_NOOP( "Chinese Traditional" ),
	I18N_NOOP( "Cyrillic" ),
	I18N_NOOP( "Greek" ),
	I18N_NOOP( "Hebrew" ),
	I18N_NOOP( "Japanese" ),
	I18N_NOOP( "Korean" ),
	I18N_NOOP( "Thai" ),
	I18N_NOOP( "Turkish" ),
	I18N_NOOP( "Western European" ),
	I18N_NOOP( "Tamil" ),
	I18N_NOOP( "Unicode" ),
	I18N_NOOP( "Northern Saami" ),
        I18N_NOOP( "Vietnamese" ),
        I18N_NOOP( "South-Eastern Europe" )
};

// this list gives the charsets that can be used to display a file given in a certain encoding.
// the list should be in order of preference
// left side is the name returned by the codec used, right side the name of the charset as
// used in kcharsets.cpp
// 'unicode' will always be chosen as last resort, so it only needs to be added to the list,
// if it should get a higher priority
// every line must end with 0

static const char* const charsets_for_encoding[] = {
    "koi8-r",                "koi8-r","cp 1251","koi8-u","iso-8859-5", 0,
    "koi8-u",                "koi8-u","cp 1251","iso-8859-5","koi8-r", 0,
    "iso 8859-1",            "iso8859-1","iso8859-15", 0,
    "iso 8859-2",            "iso8859-2","unicode","iso8859-1", 0,
    "iso 8859-3",            "iso8859-3","unicode","iso8859-1", 0,
    "iso 8859-4",            "iso8859-4","unicode","iso8859-13", "iso8859-1", 0,
    "iso 8859-5",            "iso8859-5","koi8-u","koi8-r", 0,
    "iso 8859-6",            "unicode","iso8859-6", 0,
    "iso 8859-7",            "iso8859-7", 0,
    "iso 8859-8",            "iso8859-8", 0,
    "iso 8859-8-i",          "iso8859-8", 0,
    "iso 8859-9",            "iso8859-9","unicode","iso8859-1", 0,
    "iso 8859-11",           "iso8859-11", 0,
    "iso 8859-13",           "iso8859-13","unicode","iso8859-4", "iso8859-1", 0,
    "ISO-8859-14",           "ISO-8859-15", "ISO-8859-1", 0,
    "iso 8859-15",           "iso8859-15","unicode","iso8859-1", 0,
    "ISO-8859-16",           "iso8859-2", 0, // ISO-8859-2 is not a replacement
    "utf8",                  "unicode","iso8859-1", 0,
    "utf16",                 "unicode","iso8859-1", 0,
    "iso-10646-ucs-2",       "unicode","iso8859-1", 0,
    "cp 1250",               "iso8859-2", 0,
    "cp 1251",               "cp 1251","koi8-u","koi8-r","iso8859-5", 0,
    "cp 1252",               "iso8859-1", 0,
    "cp 1253",               "iso8859-7", 0,
    "cp 1254",               "iso8859-9", 0,
    "cp 1255",               "iso8859-8", 0,
    "cp 1256",               "unicode","iso8859-6", 0,
    "cp 1257",               "iso8859-13", "iso8859-4", 0,
    "windows-1258",          "iso8859-1", 0, // ISO-8859-1 is not a replacement
    "ibm850",                "ibm850","unicode","iso8859-1", 0,
#if 0
    "ibm852",                "unicode","iso-8859-2", 0,
#endif
    "ibm866",                "ibm866","cp 1251","koi8-u","koi8-r","iso8859-5", 0,
    "tis620",                "iso8859-11", 0,
    "eucjp",                 "eucjp","unicode","iso8859-1", 0,
    "sjis",                  "eucjp","unicode","iso8859-1", 0,
    "jis7",                  "eucjp","unicode","iso8859-1", 0,
    "big5",                  "big5","unicode","iso8859-1", 0,
    "gbk",                   "gb2312.1980-0","gbk-0","unicode","iso8859-1", 0,
    "gb18030",               "gb18030.2000-1", "gb18030.2000-0", "unicode", "gbk-0", "gb2313.1980-0", "iso8859-1", 0,
    "gb2312",                "gb2312.1980-0","unicode","iso8859-1", 0,
    "euckr",                 "euckr","unicode","iso8859-1", 0,
    "TSCII",                 "tscii", 0,
//    "pt 154",                "pt 154","cp 1251","koi8-u","koi8-r","iso8859-5", 0, // ### TODO "PT 154" seems to have been removed from Qt
    "winsami2",              "winsami2", "cp1252", "unicode", 0,
    "IBM874",                "tis620", 0,
    0 }; // extra 0 for end

// 0 other
// 1 Arabic
// 2 Baltic
// 3 Central European
// 4 Chinese Simplified
// 5 Chinese Traditional
// 6 Cyrillic
// 7 Greek
// 8 Hebrew
// 9 Japanese
// 10 Korean
// 11 Thai
// 12 Turkish
// 13 Western European
// 14 Tamil
// 15 Unicode
// 16 Northern Sami
// 17 Vietnamese
// 18 South-Eastern Europe
// ### FIXME KDE4: the name of the encodings should mostly be uppercase
static struct LanguageForEncoding
    {
    const char* index;
    int data;
    } const language_for_encoding[] = {
    { "iso 8859-1", 13 },
    { "iso 8859-15", 13 },
    { "ISO-8859-14", 13 },
    { "cp 1252", 13 },
    { "ibm850", 13 },
    { "iso 8859-2", 3 },
    { "iso 8859-3", 3 },
    { "iso 8859-4", 2 },
    { "iso 8859-13", 2 },
    { "ISO-8859-16", 18 },
    { "cp 1250", 3 },
    { "cp 1254", 12 },
    { "cp 1257", 2 },
#if 0
    { "ibm852", 3 },
#endif
    { "koi8-r", 6 },
    { "iso 8859-5", 6 },
    { "cp 1251", 6 },
    { "koi8-u", 6 },
//    { "pt 154", 6 }, // ### TODO "PT 154" seems to have been removed from Qt
    { "ibm866", 6 },
    { "big5", 5 },
    { "gb18030", 4 },
    { "gbk", 4 },
    { "gb2312", 4 },
    { "euckr", 10 },
    { "sjis", 9 },
    { "jis7", 9 },
    { "eucjp", 9 },
    { "iso 8859-7", 7 },
    { "cp 1253", 7 },
    { "iso 8859-6", 1 },
    { "cp 1256", 1 },
    { "iso 8859-8", 8 },
    { "iso 8859-8-i", 8 },
    { "cp 1255", 8 },
    { "iso 8859-9", 12 },
    { "tis620", 11 },
    { "iso 8859-11", 11 }, // ### TODO: deprecated name of TIS-620
    { "utf8", 15 },
    { "utf16", 15 },
    { "utf7", 15 }, // ### FIXME: UTF-7 is not in Qt
    { "ucs2", 15 }, // ### TODO: same as ISO-10646-UCS-2 (so "triples" UTF-16)
    { "iso-10646-ucs-2", 15 }, // ### TODO: doubles UTF-16
    { "winsami2", 16},
    { 0, 0 } };

// defines some different names for codecs that are built into Qt.
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
    { "x-utf-8", "utf-8" },
    { "x-utf-7", "utf-7" }, // ### FIXME: UTF-7 is not in Qt 
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
    { "x-euc-kr", "euckr" },
    { "euc-jp", "eucjp" },
    { "x-euc-jp", "eucjp" },
    { "jisx0201.1976-0", "eucjp" },
    { "jisx0208.1983-0", "eucjp" },
    { "jisx0208.1990-0", "eucjp" },
    { "jisx0208.1997-0", "eucjp" },
    { "jisx0212.1990-0", "eucjp" },
    { "jisx0213.2000-1", "eucjp" },
    { "jisx0213.2000-2", "eucjp" },
    { "shift_jis", "sjis" },
    { "shift-jis", "sjis" },
    { "x-sjis", "sjis" },
    { "iso-2022-jp", "jis7" }, // ### TODO: ISO-2022-JP is now the default name in Qt4
    { "windows850", "ibm850" },
    { "windows866", "ibm866" },
    { "windows-850", "ibm850" },
    { "windows-866", "ibm866" },
    { "x-windows-850", "ibm850" },
    { "x-windows-866", "ibm866" },
    // ### TODO: Qt4 names them windows-125x now
    { "x-windows-1250", "cp 1250" },
    { "x-windows-1251", "cp 1251" },
    { "x-windows-1252", "cp 1252" },
    { "x-windows-1253", "cp 1253" },
    { "x-windows-1254", "cp 1254" },
    { "x-windows-1255", "cp 1255" },
    { "x-windows-1256", "cp 1256" },
    { "x-windows-1257", "cp 1257" },
    { "x-windows-1258", "windows-1258" },
    { "cp-10000", "apple roman" },
    { "x-cp-850", "ibm850" },
    { "x-cp-866", "ibm866" },
    { "x-cp-1250", "cp 1250" },
    { "x-cp-1251", "cp 1251" },
    { "x-cp-1252", "cp 1252" },
    { "x-cp-1253", "cp 1253" },
    { "x-cp-1254", "cp 1254" },
    { "x-cp-1255", "cp 1255" },
    { "x-cp-1256", "cp 1256" },
    { "x-cp-1257", "cp 1257" },
    { "x-cp-1258", "windows-1258" },
    { "x-cp-10000", "apple roman" },
    { "thai-tis620", "iso 8859-11" }, // ### TODO: TIS-620
    { "windows-874", "ibm874" },
    { "windows874", "ibm874" },
    { "x-windows-874", "ibm874" },
    { "x-cp-874", "ibm874" },
    { "x-ibm874", "ibm874" },
    { "ksc5601.1987-0", "euckr" },
    { "ks_c_5601-1987", "euckr" },
    { "x-winsami2", "winsami2" },
    { "x-mac-roman", "apple roman" },
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
    // KDE had always "CP 1251" as best fallback to PT 154. Now that Qt does not offer this encoding anymore, it is our fallback.
    { "pt 154", "windows-1251" },
    { "paratype-154", "windows-1251" },
    { "pt-154", "windows-1251" },
    { 0, 0 }};
#endif

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
    QHash<QByteArray,QTextCodec*> codecForNameDict;
    KCharsets* kc;
};

// --------------------------------------------------------------------------

KCharsets::KCharsets()
{
    d = new KCharsetsPrivate(this);
}

KCharsets::~KCharsets()
{
    delete d;
}

QChar KCharsets::fromEntity(const QString &str)
{
    QChar res = QChar::Null;

    int pos = 0;
    if(str[pos] == '&') pos++;

    // Check for '&#000' or '&#x0000' sequence
    if (str[pos] == '#' && str.length()-pos > 1) {
        bool ok;
        pos++;
        if (str[pos] == 'x' || str[pos] == 'X') {
            pos++;
            // '&#x0000', hexadeciaml character reference
            QString tmp(str.unicode()+pos, str.length()-pos);
            res = tmp.toInt(&ok, 16);
        } else {
            //  '&#0000', decimal character reference
            QString tmp(str.unicode()+pos, str.length()-pos);
            res = tmp.toInt(&ok, 10);
        }
        return res;
    }

    const entity *e = kde_findEntity(str.ascii(), str.length());

    if(!e)
    {
        //kdDebug( 0 ) << "unknown entity " << str <<", len = " << str.length() << endl;
        return QChar::Null;
    }
    //kdDebug() << "got entity " << str << " = " << e->code << endl;

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

        const QChar entityValue = KCharsets::fromEntity( QConstString( entityBegin, entityLength ).string() );
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

QStringList KCharsets::availableEncodingNames()
{
    QStringList available;

    const char* const* pos = charsets_for_encoding;
    while( *pos != 0 ) {
        //kdDebug(0) << "key = " << *pos << endl;


        // iterate thorugh the list and find the first charset that is available
        for( const char* const* charsets = pos + 1;
             *charsets != 0;
             ++charsets ) {
            //kdDebug(0) << "checking for " << *charsets << endl;
#ifdef __GNUC__
#warning FIXME?
#endif
            if( true ) {
                //kdDebug(0) << *charsets << " available" << endl;
                available.append( QLatin1String( *pos ));
                break;
            }
        }
        while( *pos != 0 ) // find end of line
            ++pos;
        ++pos; // move to the next line
    }
    return available;
}

QString KCharsets::languageForEncoding( const QString &encoding )
{
    int lang = kcharsets_array_search< LanguageForEncoding, int >
        ( language_for_encoding, encoding.latin1());
    return i18n( language_names[lang] );
}

QString KCharsets::encodingForName( const QString &descriptiveName )
{
    const int left = descriptiveName.findRev( '(' );
    
    if (left<0) // No parenthesis, so assume it is a normal encoding name
	return descriptiveName.trimmed();
    
    QString name(descriptiveName.mid(left+1));
    
    const int right = name.findRev( ')' );
    
    if (right<0) 
        return name;

    return name.left(right).trimmed();
}

QStringList KCharsets::descriptiveEncodingNames()
{
  QStringList encodings = availableEncodingNames();
  QStringList::Iterator it;
  for( it = encodings.begin(); it != encodings.end(); ++it ) {
      QString lang = KGlobal::charsets()->languageForEncoding( *it );
      *it = i18n("Descriptive Encoding Name", "%1 ( %2 )") .arg(lang) .arg(*it);
  }
  encodings.sort();
  return encodings;
}

QTextCodec *KCharsets::codecForName(const QString &n) const
{
    bool b;
    return codecForName( n, b );
}

QTextCodec *KCharsets::codecForName(const QString &n, bool &ok) const
{
    ok = true;

    QTextCodec* codec = 0;
	QString l = "->locale<-";
    // dict lookup is case insensitive anyway
	if ( !n.isEmpty() && d->codecForNameDict.contains( n.toLower().toLatin1() ) ) {
		return d->codecForNameDict.value( n.toLower().toLatin1() );
	} else if ( n.isEmpty() && d->codecForNameDict.contains( l.toLatin1() ) ) {
		return d->codecForNameDict.value( l.toLatin1() );
	}

    if (n.isEmpty()) {
        codec = KGlobal::locale()->codecForEncoding();
        d->codecForNameDict.insert("->locale<-", codec);
        return codec;
    }

    // ### TODO: we should check if the name starts with x- and remove it. That would save many mapping entries
    QByteArray name = n.toLower().latin1();
    QByteArray key = name;
    if (name.right(8) == "_charset")
       name.truncate(name.length()-8);

    if (name.isEmpty()) {
      ok = false;
      return QTextCodec::codecForName("iso8859-1");
    }

    codec = QTextCodec::codecForName(name);

    if(codec) {
        d->codecForNameDict.insert(key.toLower(), codec);
        return codec;
    }

    // these codecs are built into Qt, but the name given for the codec is different,
    // so QTextCodec did not recognize it.
    QByteArray cname = kcharsets_array_search< Builtin, const char* >( builtin, name.data());

    if(!cname.isEmpty())
        codec = QTextCodec::codecForName(cname);

    if(codec)
    {
        d->codecForNameDict.insert(key.toLower(), codec);
        return codec;
    }

#ifdef __GNUC__
#warning is it still usefull with Qt4 ?
#endif
	//dont forget to remove the #if 0 on a few structs at the top also if you reenable that ;)  (search for 852 )
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
    kdDebug() << k_funcinfo << endl << " Trying to find " << cname << " in " << dir << endl;
    
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
        KFilterDev gzip(dir + "/" + charMapFileName);
        if (gzip.open(QIODevice::ReadOnly)) {
            kdDebug() << "Loading gzipped charset..." << endl;
            codec = QTextCodec::loadCharmap(&gzip);
            gzip.close();
        }
        else
            kdWarning() << "Could not open gzipped charset!" << endl;
    }
    else if (!charMapFileName.isEmpty()) {
        codec = QTextCodec::loadCharmapFile(dir + "/" + charMapFileName);
    }

    if(codec) {
        d->codecForNameDict.replace(key, codec);
        return codec;
    }

    // this also failed, the last resort is now to take some compatibility charmap

    cname = kcharsets_array_search< ConversionHints, const char* >( conversion_hints, (const char*)name.data() );

    if(!cname.isEmpty())
        codec = QTextCodec::codecForName(cname);

    if(codec) {
        d->codecForNameDict.replace(key, codec);
        return codec;
    }
#endif

    // could not assign a codec, let's return Latin1
    ok = false;
    return QTextCodec::codecForName("iso8859-1");
}
