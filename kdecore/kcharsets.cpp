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
#include <qfontdatabase.h>
#include <kdebug.h>

#include <qtextcodec.h>
#include <qmap.h>

template class QList<QFont::CharSet>;

// This will be enabled when we start using Qt v2.2.x
#ifdef USE_TSCII
#define CHARSETS_COUNT 29
#else
#define CHARSETS_COUNT 28
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
    "eucjp",
    "euckr",
    "set-th-th",
    "set-gbk",
    "set-zh",
    "set-zh-tw",
    "set-big5",
#ifdef USE_TSCII
    "tscii",
#endif
    "utf-8",
    "utf-16",
	"iso-8859-11"
	"Any"
};

// these can contain wildcard characters. Needed for fontset matching (CJK fonts)
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
    "jisx0208.1983-0",
    /* ### not sure about the next seven ones. Lars */
    "ksx1001.1997-0",
    "ksc5601.1987-0",
    "gb2312*",
    "unknown",
    "unknown",
    "big5*-0",
#ifdef USE_TSCII
    "tscii-0",
#endif
    "utf8",
    "utf16",
	"tis620-*",
	""  // this will always return true...
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
    QFont::Set_Ja,
    QFont::Set_Ko,
    QFont::Set_Th_TH,
    QFont::Set_GBK,
    QFont::Set_Zh,
    QFont::Set_Zh_TW,
    QFont::Set_Big5,
#ifdef USE_TSCII
    QFont::TSCII,
#endif
    QFont::Unicode,
    QFont::Unicode,
	QFont::ISO_8859_11,
	QFont::AnyCharSet
};


class KCharsetsPrivate
{
public:
    KCharsetsPrivate()
    {
        db = 0;
        availableCharsets = 0;
    }
    ~KCharsetsPrivate()
    {
        delete db;
        delete availableCharsets;
    }
    QFontDatabase *db;
    QMap<QFont::CharSet, QStringList> *availableCharsets;

    void getAvailableCharsets(const KCharsets *charsets);
};

void KCharsetsPrivate::getAvailableCharsets(const KCharsets *charsets)
{
    if(availableCharsets)
        return;
    if(!db)
        db = new QFontDatabase;

    availableCharsets = new QMap<QFont::CharSet, QStringList>;

    QStringList f = db->families(false);

    for ( QStringList::Iterator it = f.begin(); it != f.end(); ++it ) {
        QStringList chSets = db->charSets(*it, false);
	QString family = *it;
	if ( family. contains('-') ) // remove foundry
	    family = family.right( family.length() - family.find('-' ) - 1);
	//kdDebug() << "KCharsetsPrivate::getAvailableCharsets " << *it << " " << family <<endl;
        for ( QStringList::Iterator ch = chSets.begin(); ch != chSets.end(); ++ch ) {
	    //kdDebug() << "KCharsetsPrivate::getAvailableCharsets " << *ch << " " << charsets->xNameToID( *ch ) << endl;
            if( !availableCharsets->contains( charsets->xNameToID( *ch ) ) ) {
                QStringList strList;
                strList.append( family );
                availableCharsets->insert( charsets->xNameToID( *ch ), strList );
            } else
                ((*availableCharsets)[charsets->xNameToID( *ch )]).append(family);
        }
    }

#if 0
    for( QMap<QFont::CharSet, QStringList>::Iterator it = availableCharsets->begin();
         it != availableCharsets->end(); ++it ) {
        kdDebug() << "KCharsetsPrivate::getAvailableCharsets " << it.key() << " " << endl;
    }
#endif
}

// --------------------------------------------------------------------------

KCharsets::KCharsets()
{
    d = new KCharsetsPrivate;
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
        kdDebug( 0 ) << "unknown entity " << str <<", len = " << str.length() << endl;
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
    ent.sprintf("&0x%x;", ch.unicode());
    return ent;
}

QList<QFont::CharSet> KCharsets::availableCharsets(QString family)
{
    if(!d->db)
        d->db = new QFontDatabase;

    if( !family.isNull() ) {
        QStringList lst = d->db->charSets(family, false);
        QList<QFont::CharSet> chSets;
        chSets.setAutoDelete(true);
        for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
            QFont::CharSet *i = new QFont::CharSet;
            *i = xNameToID(*it);
            chSets.append(i);
        }
        return chSets;
    }

    d->getAvailableCharsets(this);

    QList<QFont::CharSet> chSets;
    chSets.setAutoDelete(true);
    for( QMap<QFont::CharSet, QStringList>::Iterator it = d->availableCharsets->begin();
         it != d->availableCharsets->end(); ++it ) {
            QFont::CharSet *i = new QFont::CharSet;
            *i = it.key();
            chSets.append( i );
    }
    return chSets;
}

QStringList KCharsets::availableCharsetNames(QString family)
{
    if(!d->db)
        d->db = new QFontDatabase;

    if( !family.isNull() )
        return d->db->charSets(family, false);

    d->getAvailableCharsets(this);

    QStringList chSets;
    for( QMap<QFont::CharSet, QStringList>::Iterator it = d->availableCharsets->begin();
         it != d->availableCharsets->end(); ++it )
            chSets.append( xCharsetName(it.key()) );
    return chSets;
}


QStringList KCharsets::availableEncodingNames()
{
    QStringList available;

    KConfig conf( "charsets", true );
    QMap<QString, QString> map = conf.entryMap("charsetsForEncoding");
    conf.setGroup("charsetsForEncoding");

    QMap<QString, QString>::Iterator it;
    for( it = map.begin(); it != map.end(); ++it ) {
        //kdDebug(0) << "key = " << it.key() << " string =" << it.data() << endl;

        //kdDebug(0) << "list is: " << conf.readEntry(it.key()) << endl;
        QStringList charsets = conf.readListEntry(it.key());

        // iterate thorugh the list and find the first charset that is available
        for ( QStringList::Iterator sit = charsets.begin(); sit != charsets.end(); ++sit ) {
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

void KCharsets::setQFont(QFont &f, QString charset) const
{
    setQFont(f, nameToID(charset));
}

void KCharsets::setQFont(QFont &f, QFont::CharSet charset) const
{
    if(!d->db)
        d->db = new QFontDatabase;

    d->getAvailableCharsets(this);

    QString family = f.family();

    QStringList chFamilies = (*d->availableCharsets)[charset];
    if(chFamilies.contains(family)) {
	//kdDebug() << "KCharsets::setQFont: charsetAvailable in family" << endl; 
        f.setCharSet(charset);
        return;
    }

    QStringList ucFamilies = (*d->availableCharsets)[QFont::Unicode];
    if(ucFamilies.contains(family)) {
	//kdDebug() << "KCharsets::setQFont: using unicode" << endl; 
	// just setting the charset to unicode should work
        f.setCharSet(QFont::Unicode);
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
    d->getAvailableCharsets(this);

    if(d->availableCharsets->contains(charset))
        return true;
    return false;
}

QFont::CharSet KCharsets::charsetForLocale()
{
    return nameToID(KGlobal::locale()->charset());
}

bool KCharsets::hasUnicode(QString family) const
{
    d->getAvailableCharsets(this);
    if(!d->availableCharsets->contains(QFont::Unicode))
        return false;
    QStringList lst = (*d->availableCharsets)[QFont::Unicode];
    if(lst.contains(family))
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
    case QFont::ISO_8859_11:
        return "iso8859-11";
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
    case QFont::Set_Ko:
        return "ksc5601.1987-0";
    case QFont::Set_Ja:
        return "jisx0208.1983-0";
#ifdef USE_TSCII
    case QFont::TSCII:
        return "tscii-0";
#endif
    case QFont::Set_Th_TH:
    case QFont::Set_Zh:
    case QFont::Set_Zh_TW:
    case QFont::AnyCharSet:
    default:
        break;
    }
    return "*-*";
}

QFont::CharSet KCharsets::nameToID(QString name) const
{
    name = name.lower();

    int i = 0;
    while(i < CHARSETS_COUNT)
    {
        if( name == charsetsStr[i] )
            return charsetsIds[i];
        i++;
    }

    i = 0;
    while(i < CHARSETS_COUNT)
    {
        if( name == xNames[i] )
            return charsetsIds[i];
        i++;
    }
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

    int i = 0;
    while(i < CHARSETS_COUNT)
    {
       if( name == xNames[i] )
            return charsetsIds[i];
        i++;
    }
    return QFont::AnyCharSet;
}


QTextCodec *KCharsets::codecForName(const QString &n) const
{
    QString name = n.lower();
    QTextCodec *codec = QTextCodec::codecForName(name.latin1());

    if(codec)
        return codec;

    KConfig conf( "charsets", true );

    // these codecs are built into Qt, but the name given for the codec is different,
    // so QTextCodec did not recognise it.
    conf.setGroup("builtin");

    QString cname = conf.readEntry(name);
    if(!cname.isEmpty() && !cname.isNull())
        codec = QTextCodec::codecForName(cname.latin1());

    if(codec) return codec;

    conf.setGroup("general");
    QString dir = conf.readEntry("i18ndir", QString::fromLatin1("/usr/share/i18n/charmaps"));
    dir += "/";

    // these are codecs not included in Qt. They can be build up if the corresponding charmap
    // is available in the charmap directory.
    conf.setGroup("aliases");

    cname = conf.readEntry(name);
    if(cname.isNull() || cname.isEmpty())
        cname = name;
    cname = cname.upper();

    codec = QTextCodec::loadCharmapFile(dir + cname);

    if(codec)
        return codec;

    // this also failed, the last resort is now to take some compatibility charmap

    conf.setGroup("conversionHints");
    cname = cname.lower();
    cname = conf.readEntry(cname);

    if(!cname.isEmpty() && !cname.isNull())
        codec = QTextCodec::codecForName(cname.latin1());

    if(codec)
        return codec;

    // could not assign a codec, let's return Latin1
    return QTextCodec::codecForName("iso8859-1");
}


QFont::CharSet KCharsets::charsetForEncoding(const QString &e) const
{
    QString encoding = e.lower();
    KConfig conf( "charsets", true );
    conf.setGroup("charsetsForEncoding");

    kdDebug(0) << "list for " << encoding << " is: " << conf.readEntry(encoding) << endl;
    QStringList charsets = conf.readListEntry(encoding);

    // iterate thorugh the list and find the first charset that is available
    for ( QStringList::Iterator it = charsets.begin(); it != charsets.end(); ++it ) {
        if( const_cast<KCharsets *>(this)->isAvailable(*it) ) {
            kdDebug(0) << *it << " available" << endl;
            return nameToID(*it);
        }
        kdDebug(0) << *it << " is not available" << endl;
    }
    // let's hope the system has a unicode font...
    return QFont::Unicode;
}

