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
#include "kcharsets.h"

#include "kentities.c"

#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>

#include <qfontinfo.h>
#include <qstrlist.h>
#include <qfontdatabase.h>
#include <kdebug.h>

#include <qtextcodec.h>
#include <qmap.h>
#include <qcstring.h>

template class QList<QFont::CharSet>;

#if QT_VERSION >= 224
#define CHARSETS_COUNT 32
#else
#define CHARSETS_COUNT 30
#endif

static const char * const charsetsStr[CHARSETS_COUNT] = {
    "unicode",
    "iso-8859-1",
    "iso-8859-2",
    "iso-8859-3",
    "iso-8859-4",
    "iso-8859-5",
    "iso-8859-6",
    "iso-8859-7",
    "iso-8859-8",
    "iso-8859-9",
    "iso-8859-10",
    "iso-8859-11",
    "iso-8859-12",
    "iso-8859-13",
    "iso-8859-14",
    "iso-8859-15",
    "koi8r",
    "koi8u",
#if QT_VERSION >= 224
    "cp 1251",
    "pt 154",
#endif
    "eucjp",
    "euckr",
    "set-th-th",
    "set-gbk",
    "set-zh",
    "set-zh-tw",
    "big5",
    "tscii",
    "utf-8",
    "utf-16",
    "iso-8859-11",
    "Any"
};

// these can contain wildcard characters. Needed for fontset matching (CJK fonts)
// sort them by length if they have the same prefix!
static const char * const xNames[CHARSETS_COUNT] = {
    "iso10646-1",
    "iso8859-1",
    "iso8859-2",
    "iso8859-3",
    "iso8859-4",
    "iso8859-5",
    "iso8859-6",
    "iso8859-7",
    "iso8859-8",
    "iso8859-9",
    "iso8859-10",
    "iso8859-11",
    "iso8859-12",
    "iso8859-13",
    "iso8859-14",
    "iso8859-15",
    "koi8-r",
    "koi8-u",
#if QT_VERSION >= 224
    "microsoft-cp1251",
    "paratype-cp154",
#endif
    "jisx0208.1983-0",
    "ksc5601.1987-0",
    "tis620.2533-1",
    "gb2312.1980-0",
    "gb2312.1980-0",
    "cns11643.1986-",
    "big5-0",
    "tscii-0",
    "utf8",
    "utf16",
    "tis620-0",
    ""  // this will always return true...
        // adjust xNameToId if you remove this
};

static const QFont::CharSet charsetsIds[CHARSETS_COUNT] = {
    QFont::Unicode,
    QFont::ISO_8859_1,
    QFont::ISO_8859_2,
    QFont::ISO_8859_3,
    QFont::ISO_8859_4,
    QFont::ISO_8859_5,
    QFont::ISO_8859_6,
    QFont::ISO_8859_7,
    QFont::ISO_8859_8,
    QFont::ISO_8859_9,
    QFont::ISO_8859_10,
    QFont::ISO_8859_11,
    QFont::ISO_8859_12,
    QFont::ISO_8859_13,
    QFont::ISO_8859_14,
    QFont::ISO_8859_15,
    QFont::KOI8R,
    QFont::KOI8U,
#if QT_VERSION >= 224
    QFont::CP1251,
    QFont::PT154,
#endif
    QFont::Set_Ja,
    QFont::Set_Ko,
    QFont::Set_Th_TH,
    QFont::Set_GBK,
    QFont::Set_Zh,
    QFont::Set_Zh_TW,
    QFont::Set_Big5,
    QFont::TSCII,
    QFont::Unicode,
    QFont::Unicode,
    QFont::ISO_8859_11,
    QFont::AnyCharSet
};

static const char * const languages[] = {
    I18N_NOOP( "other" ),
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
	I18N_NOOP( "Unicode" )
};


class KCharsetsPrivate
{
public:
    KCharsetsPrivate(KCharsets* _kc)
        : codecForNameDict(43, false) // case insensitive
    {
        db = 0;
        availableCharsets = 0;
        kc = _kc;
	_conf = 0;
    }
    ~KCharsetsPrivate()
    {
        delete db;
        delete availableCharsets;
        delete _conf;
    }
    KConfig* conf()
	{
	if( _conf == NULL )
            _conf = new KConfig( "charsets", true, false );
	return _conf;
	}
    QFontDatabase *db;
    QMap<QFont::CharSet, QStrList> *availableCharsets;
    QMap<QCString, QFont::CharSet> charsetForEncodingMap;
    QMap<QString, QFont::CharSet> nameToIDMap;
    QAsciiDict<QTextCodec> codecForNameDict;
    KCharsets* kc;
    KConfig* _conf;

    void getAvailableCharsets();
};

void KCharsetsPrivate::getAvailableCharsets()
{
    if(availableCharsets)
        return;

    if(!db)
        db = new QFontDatabase;

    availableCharsets = new QMap<QFont::CharSet, QStrList >;

    QStringList f = db->families( false );

    for ( QStringList::ConstIterator it = f.begin(); it != f.end(); ++it ) {
	QStringList chSets = db->charSets(*it, false);
	QCString family = (*it).latin1(); // can only be latin1
        QCString shortFamily; // family without foundry, if "family" contains the foundry
        int dashPos = family.find( '-' );
	if ( dashPos != -1 )
	    shortFamily = family.right( family.length() - dashPos - 1);
	//kdDebug() << "KCharsetsPrivate full-family='" << *it << "' family='" << family << "'  charsets: " << chSets.join(",") << endl;
	for ( QStringList::ConstIterator ch = chSets.begin(); ch != chSets.end(); ++ch ) {
	    //kdDebug() << "    " << *ch << " " << KGlobal::charsets()->xNameToID( *ch ) << endl;
	    QCString cs = (*ch).latin1();
	    QFont::CharSet qcs = kc->xNameToID( cs );
	redo:
	    if ( qcs != QFont::AnyCharSet )
		if( !availableCharsets->contains( qcs ) ) {
		    QStrList strList;
		    strList.append( family );
		    if ( !shortFamily.isEmpty() )
			strList.append( shortFamily );
		    availableCharsets->insert( qcs, strList );
		} else
		{
		    ((*availableCharsets)[qcs]).append(family);
		    if ( !shortFamily.isEmpty() )
			((*availableCharsets)[qcs]).append(shortFamily);
		}
	    // hack to get the duplicated entry for gb2312 right.
	    if ( qcs == QFont::Set_Zh ) {
		qcs = QFont::Set_GBK;
		goto redo;
	    }
	}
    }

    // sort lists to avoid "Arial Black" being found before "Arial"
    for( QMap<QFont::CharSet, QStrList >::Iterator it = availableCharsets->begin();
         it != availableCharsets->end(); ++it )
        it.data().sort();

#if 0
    for( QMap<QFont::CharSet, QValueList<QCString> >::Iterator it = availableCharsets->begin();
         it != availableCharsets->end(); ++it ) {
        kdDebug() << "KCharsetsPrivate::getAvailableCharsets " << it.key() << " " << endl;
    }
#endif
}



// --------------------------------------------------------------------------

KCharsets::KCharsets()
{
    d = new KCharsetsPrivate(this);
}

KCharsets::~KCharsets()
{
    delete d;
}

QChar KCharsets::fromEntity(const QString &str) const
{
    QChar res = QChar::null;

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
            //  '&#0000', deciaml character reference
            QString tmp(str.unicode()+pos, str.length()-pos);
            res = tmp.toInt(&ok, 10);
        }
        return res;
    }

    const entity *e = findEntity(str.ascii(), str.length());

    if(!e)
    {
        //kdDebug( 0 ) << "unknown entity " << str <<", len = " << str.length() << endl;
        return QChar::null;
    }
    //kdDebug() << "got entity " << str << " = " << e->code << endl;

    return QChar(e->code);
}

QChar KCharsets::fromEntity(const QString &str, int &len) const
{
    // entities are never longer than 8 chars... we start from
    // that length and work backwards...
    len = 8;
    while(len > 0)
    {
        QString tmp = str.left(len);
        QChar res = fromEntity(tmp);
        if( res != QChar::null ) return res;
        len--;
    }
    return QChar::null;
}


QString KCharsets::toEntity(const QChar &ch) const
{
    QString ent;
    ent.sprintf("&#0x%x;", ch.unicode());
    return ent;
}

QList<QFont::CharSet> KCharsets::availableCharsets(QString family)
{
    if(!d->db)
        d->db = new QFontDatabase;

    d->getAvailableCharsets();

    QList<QFont::CharSet> chSets;
    //chSets.setAutoDelete(true);
    QCString f = family.latin1();
    for( QMap<QFont::CharSet, QStrList >::Iterator it = d->availableCharsets->begin();
         it != d->availableCharsets->end(); ++it ) {
	if ( f.isEmpty() || it.data().contains( f.data() ) ) {
	    QFont::CharSet *i = new QFont::CharSet;
	    *i = it.key();
	    chSets.append( i );
	}
    }
    return chSets;
}

QStringList KCharsets::availableCharsetNames(QString family)
{
    QList<QFont::CharSet> list = availableCharsets( family );
    list.setAutoDelete( true );
    QStringList chsetNames;
    QFont::CharSet *chset;
    for ( chset = list.first(); chset != 0; chset = list.next() ) {
	chsetNames.append( xCharsetName( *chset ) );
    }
    return chsetNames;
}


QStringList KCharsets::availableEncodingNames()
{
    QStringList available;

    QMap<QString, QString> map = d->conf()->entryMap("charsetsForEncoding");
    d->conf()->setGroup("charsetsForEncoding");

    QMap<QString, QString>::ConstIterator it;
    for( it = map.begin(); it != map.end(); ++it ) {
        //kdDebug(0) << "key = " << it.key() << " string =" << it.data() << endl;

        //kdDebug(0) << "list is: " << d->conf->readEntry(it.key()) << endl;
        QStringList charsets = d->conf()->readListEntry(it.key());

        // iterate thorugh the list and find the first charset that is available
        for ( QStringList::ConstIterator sit = charsets.begin(); sit != charsets.end(); ++sit ) {
            //kdDebug(0) << "checking for " << *sit << endl;
            if( const_cast<KCharsets *>(this)->isAvailable(*sit) ) {
                //kdDebug(0) << *sit << " available" << endl;
                available.append(it.key());
                break;
            }
        }
    }
    return available;
}

QString KCharsets::languageForEncoding( const QString &encoding )
{
    d->conf()->setGroup("LanguageForEncoding");

    int lang = d->conf()->readNumEntry(encoding, 0 );
    return i18n( languages[lang] );
}

QString KCharsets::encodingForName( const QString &descriptiveName )
{
    int left = descriptiveName.find( "( " );
    return descriptiveName.mid( left + 2, descriptiveName.find( " )" )
      - left - 2 );
}

QStringList KCharsets::descriptiveEncodingNames()
{
  QStringList encodings = availableEncodingNames();
  QStringList::Iterator it;
  for( it = encodings.begin(); it != encodings.end(); ++it ) {
      QString lang = KGlobal::charsets()->languageForEncoding( *it );
      *it = lang + " ( " + *it + " )";
  }
  encodings.sort();
  return encodings;
}

QFont KCharsets::fontForChar( const QChar &c, const QFont &_f ) const
{
    QFontInfo fi(_f);

    // unicode can display any char...
    if (fi.charSet() == QFont::Unicode) return _f;

    // here comes the work...
    // we look at the range the char is in, and try charsets which can
    // map the char...
    QFont f = _f;
    int uc = c.unicode();

    if( uc < 0xff ) // 8859-1
        setQFont( f, QFont::Latin1 );
    else if ( uc > 0x0400 && uc < 0x0460 )
        setQFont( f, QFont::Latin5 );
    else if ( uc > 0x0600 && uc < 0x0660 )
        setQFont( f, QFont::Latin6 );
    else if ( uc > 0x0380 && uc < 0x03e0 )
        setQFont( f, QFont::Latin7 );
    else if ( uc >= 0x05d0 && uc < 0x05f0 )
        setQFont( f, QFont::Latin8 );
    else if ( hasUnicode( f ) ) // don't know, let's try Unicode
        setQFont( f, QFont::Unicode );

    return f;
}

QStringList KCharsets::availableFamilies( QFont::CharSet ch )
{
    QStringList families;
    QStrList chFamilies = (*d->availableCharsets)[ch];

    // list Unicode as available if nothing else found
    if ( ( (int)ch != QFont::Unicode ) && chFamilies.isEmpty() )
	chFamilies = (*d->availableCharsets)[QFont::Unicode];

    for ( unsigned i = 0; i < chFamilies.count(); ++i )
	families.append( QString::fromLatin1( chFamilies.at( i ) ) );

    return families;
}

void KCharsets::setQFont(QFont &f, QString charset) const
{
    QFont::CharSet cs = nameToID(charset);
    if ( cs == QFont::AnyCharSet ) {
	cs = charsetForEncoding(charset);
    }
    setQFont( f, cs );
}

void KCharsets::setQFont(QFont &f, QFont::CharSet charset) const
{
    if(QFontInfo(f).charSet() == charset)
        return;

    if(charset == QFont::AnyCharSet)
    {
        f.setCharSet(QFont::AnyCharSet);
        return;
    }

    if(!d->db)
        d->db = new QFontDatabase;

    d->getAvailableCharsets();

    QCString family = f.family().latin1();

    //kdDebug() << "KCharsets::setQFont family=" << family << endl;

    QStrList chFamilies = (*d->availableCharsets)[charset];
    if(chFamilies.contains(family)) {
	//kdDebug() << "KCharsets::setQFont: charsetAvailable in family" << endl;
        f.setCharSet(charset);
        return;
    }

    // ok... we don't have the charset in the specified family, let's
    // try to find a replacement.

    if(chFamilies.count() != 0) {
	//kdDebug() << "KCharsets::setQFont: using family " << chFamilies.first() << " in native charset " << charset << endl;
        f.setFamily(chFamilies.first());
        f.setCharSet(charset);
        return;
    }

    QStrList ucFamilies = (*d->availableCharsets)[QFont::Unicode];
    if(ucFamilies.contains(family)) {
	//kdDebug() << "KCharsets::setQFont: using unicode" << endl;
	// just setting the charset to unicode should work
        f.setCharSet(QFont::Unicode);
        return;
    }

    // Unicode and any family
    if(ucFamilies.count() != 0) {
	//kdDebug() << "KCharsets::setQFont: using family " << chFamilies.first() << " in unicode" << endl;
        f.setFamily(ucFamilies.first());
        f.setCharSet(QFont::Unicode);
        return;
    }

    // give up -- just use something
    f.setCharSet(QFont::AnyCharSet);
    return;
}

bool KCharsets::isAvailable(const QString &charset)
{
    QFont::CharSet cs = nameToID(charset);
    return cs == QFont::AnyCharSet ? false : isAvailable(cs);
}

bool KCharsets::isAvailable(QFont::CharSet charset)
{
    d->getAvailableCharsets();

    return d->availableCharsets->contains(charset);
}

QFont::CharSet KCharsets::charsetForLocale()
{
    return nameToID(KGlobal::locale()->charset());
}

bool KCharsets::hasUnicode(QString family) const
{
    d->getAvailableCharsets();
    if(!d->availableCharsets->contains(QFont::Unicode))
        return false;
    QStrList lst = (*d->availableCharsets)[QFont::Unicode];
    if(lst.contains(family.latin1()))
        return true;
    return false;
}

bool KCharsets::hasUnicode(QFont &font) const
{
    return hasUnicode(font.family());
}


QString KCharsets::xCharsetName(QFont::CharSet charSet) const
{
    switch( charSet )
    {
    case QFont::Unicode:
        return "iso10646-1";
    case QFont::ISO_8859_1:
        return "iso8859-1";
    case QFont::ISO_8859_2:
        return "iso8859-2";
    case QFont::ISO_8859_3:
        return "iso8859-3";
    case QFont::ISO_8859_4:
        return "iso8859-4";
    case QFont::ISO_8859_5:
        return "iso8859-5";
    case QFont::ISO_8859_6:
        return "iso8859-6";
    case QFont::ISO_8859_7:
        return "iso8859-7";
    case QFont::ISO_8859_8:
        return "iso8859-8";
    case QFont::ISO_8859_9:
        return "iso8859-9";
    case QFont::ISO_8859_10:
        return "iso8859-10";
    case QFont::ISO_8859_11: // most of them are actually named as tis620
        return "tis620-0";
    case QFont::ISO_8859_12:
        return "iso8859-12";
    case QFont::ISO_8859_13:
        return "iso8859-13";
    case QFont::ISO_8859_14:
        return "iso8859-14";
    case QFont::ISO_8859_15:
        return "iso8859-15";
    case QFont::KOI8R:
        return "koi8-r";
    case QFont::KOI8U:
        return "koi8-u";
    case QFont::Set_Ko:
        return "ksc5601.1987-0";
    case QFont::Set_Ja:
        return "jisx0208.1983-0";
    case QFont::TSCII:
        return "tscii-0";
    case QFont::Set_Th_TH:
	return "unknown";
	case QFont::Set_GBK:
        return "gbk-0";
	case QFont::Set_Zh:
	 return "gb2312.1980-0";
    case QFont::Set_Zh_TW:
        return "cns11643.1986-*";
    case QFont::Set_Big5:
	return "big5-0";
#if QT_VERSION >= 224
    case QFont::CP1251:
	return "microsoft-cp1251";
    case QFont::PT154:
	return "paratype-cp154";
#endif
    case QFont::AnyCharSet:
    default:
        break;
    }
    return "*-*";
}

QFont::CharSet KCharsets::nameToID(QString name) const
{
    name = name.lower();
    if(d->nameToIDMap.contains(name))
        return d->nameToIDMap[name]; // cache hit

    int i = 0;
    while(i < CHARSETS_COUNT)
    {
        if( name == charsetsStr[i] )
        {
            d->nameToIDMap.replace(name, charsetsIds[i]);
            return charsetsIds[i];
        }
        i++;
    }

    i = CHARSETS_COUNT-1;
    while( i-- ) {
        if( name.find( xNames[i] ) == 0 ) {
            d->nameToIDMap.replace(name, charsetsIds[i]);
            return charsetsIds[i];
	}
    }
    d->nameToIDMap.replace(name, QFont::AnyCharSet);
    return QFont::AnyCharSet;
}

QString KCharsets::name(const QFont &f)
{
    QFont::CharSet c = f.charSet();

    return name(c);
}

QString KCharsets::name(QFont::CharSet c)
{
    int i = 0;

    while(i < CHARSETS_COUNT)
    {
        if( c == charsetsIds[i] )
            return charsetsStr[i];
        i++;
    }
    return "any";
}


QFont::CharSet KCharsets::xNameToID(QString name) const
{
    name = name.lower();

    // fix this stone age problem
    if ( name == "iso10646" )
	name = xNames[0];

    // try longest names first, then shorter ones
    // to avoid that iso-8859-10 matches iso-8859-1
    int i = CHARSETS_COUNT-1; // avoid the "" entry
    while( i-- ) {
        if( name.find( xNames[i] ) == 0 ) {
	    return charsetsIds[i];
	}
    }

    return QFont::AnyCharSet;
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
    // dict lookup is case insensitive anyway
    if((codec = d->codecForNameDict[n.isEmpty() ? "->locale<-" : n.latin1()]))
        return codec; // cache hit, return

    QCString name = n.lower().latin1();

    if (n.isEmpty()) {
        QString lc = KGlobal::locale()->charset();
        if (lc.isEmpty())
            codec = QTextCodec::codecForName("iso8859-1");
        else
            codec = codecForName(lc);

        d->codecForNameDict.replace("->locale<-", codec);
        return codec;
    }

    codec = QTextCodec::codecForName(name);

    if(codec) {
        d->codecForNameDict.replace(name, codec);
        return codec;
    }

    // these codecs are built into Qt, but the name given for the codec is different,
    // so QTextCodec did not recognise it.
    d->conf()->setGroup("builtin");

    QString cname = d->conf()->readEntry(name.data());
    if(!cname.isEmpty() && !cname.isNull())
        codec = QTextCodec::codecForName(cname.latin1());

    if(codec)
    {
        d->codecForNameDict.replace(name, codec);
        return codec;
    }

    d->conf()->setGroup("general");
    QString dir = d->conf()->readEntry("i18ndir", QString::fromLatin1("/usr/share/i18n/charmaps"));
    dir += "/";

    // these are codecs not included in Qt. They can be build up if the corresponding charmap
    // is available in the charmap directory.
    d->conf()->setGroup("aliases");

    cname = d->conf()->readEntry(name.data());
    if(cname.isNull() || cname.isEmpty())
        cname = name;
    cname = cname.upper();

    codec = QTextCodec::loadCharmapFile(dir + cname);

    if(codec) {
        d->codecForNameDict.replace(name, codec);
        return codec;
    }

    // this also failed, the last resort is now to take some compatibility charmap

    d->conf()->setGroup("conversionHints");
    cname = cname.lower();
    cname = d->conf()->readEntry(cname);

    if(!cname.isEmpty() && !cname.isNull())
        codec = QTextCodec::codecForName(cname.latin1());

    if(codec) {
        d->codecForNameDict.replace(name, codec);
        return codec;
    }

    // could not assign a codec, let's return Latin1
    ok = false;
    return QTextCodec::codecForName("iso8859-1");
}


QFont::CharSet KCharsets::charsetForEncoding(const QString &e) const
{
    return charsetForEncoding( e, false );
}

QFont::CharSet KCharsets::charsetForEncoding(const QString &e, bool noUnicode) const
{
    QCString encoding = e.lower().latin1();
    if(!noUnicode && d->charsetForEncodingMap.contains(encoding))
        return d->charsetForEncodingMap[encoding]; // cache hit

    d->conf()->setGroup("charsetsForEncoding");

    //kdDebug(0) << "list for " << encoding << " is: " << d->conf->readEntry(encoding) << endl;

    QString enc = d->conf()->readEntry(encoding.data());
    if(enc.isEmpty()) {
	d->conf()->setGroup("builtin");
	enc = d->conf()->readEntry(encoding.data());
	encoding = enc.lower().latin1();
	d->conf()->setGroup("charsetsForEncoding");
	//kdDebug(0) << "list for " << encoding << " is: " << d->conf->readEntry(encoding) << endl <<endl;
    }

    QStringList charsets;
    charsets = d->conf()->readListEntry(encoding.data());

    // iterate thorugh the list and find the first charset that is available
    for ( QStringList::Iterator it = charsets.begin(); it != charsets.end(); ++it ) {
        QFont::CharSet cs = nameToID(*it);
	if ( noUnicode ) {
	    if ( cs != QFont::Unicode )
		return cs;
	} else {
	    if( const_cast<KCharsets *>(this)->isAvailable(cs) ) {
		//kdDebug(0) << *it << " available" << endl;
		d->charsetForEncodingMap.replace(QCString(e.latin1()), cs);
		return cs;
	    }
	}
        //kdDebug(0) << *it << " is not available" << endl;
    }

    // let's hope the system has a unicode font...
    d->charsetForEncodingMap.replace(QCString(e.latin1()), QFont::Unicode);
    return QFont::Unicode;
}

bool KCharsets::supportsScript( const QFont &f, QFont::CharSet charset )
{
    QChar ch;
    QFont::CharSet fcs = f.charSet();
    switch( charset ) {
	case QFont::ISO_8859_1:
	    ch = 0xc0; break; //Latin A circumflex
	case QFont::ISO_8859_2:
	    ch = 0x013d; break;
	case QFont::ISO_8859_3:
	    ch = 0x0126; break;
	case QFont::ISO_8859_10:
	case QFont::ISO_8859_4:
	    ch = 0x014b; break;
	case QFont::ISO_8859_6:
	    if ( fcs != QFont::Unicode ) return false;
	    ch = 0xfef5; break;
	case QFont::ISO_8859_7:
	    ch = 0x3aa; break;
	case QFont::ISO_8859_8:
	    ch  = 0x05D3; break;
	case QFont::ISO_8859_9:
	    ch = 0x0131; break;
	case QFont::ISO_8859_11:
	case QFont::Set_Th_TH:
	    ch = 0x0E23; break;
	case QFont::ISO_8859_13:
	    ch  = 0x0179; break;
	case QFont::ISO_8859_14:
	    ch  = 0x0177; break;
	case QFont::ISO_8859_15:
	    ch  = 0x0153; break;
	case QFont::ISO_8859_5:
	case QFont::KOI8U:
#if QT_VERSION >= 224
	case QFont::CP1251:
	case QFont::PT154:
#endif
	    ch = 0x0454; break;
	case QFont::KOI8R:
	    ch = 0x0431; break;
	case QFont::Set_Ja:
	    ch  = 0x3041; break;
	case QFont::Set_Ko:
	    ch = 0xac00; break;
	case QFont::Set_Zh:
	case QFont::Set_GBK:
	case QFont::Set_Big5:
	case QFont::Set_Zh_TW:
	    ch = 0x4e00; break;
	case QFont::Unicode:
	    return (fcs == charset);
	case QFont::TSCII:
	    ch = 0x0b90;
	case QFont::ISO_8859_12:
	case QFont::AnyCharSet:
	default:
	    ch = 0x0; break;
    }
    if ( charset == fcs || ch == QChar::null )
	return true;

    return QFontMetrics(f).inFont( ch );
}
