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
#include <X11/Xlib.h>
#include <kdebug.h>

#include <qtextcodec.h>

template class QList<KFontStruct>;
template class QList<QFont::CharSet>;

// This will be enabled when we start using Qt v2.2.x
#ifdef USE_TSCII
#define CHARSETS_COUNT 28
#else
#define CHARSETS_COUNT 27
#endif
static const char *charsetsStr[CHARSETS_COUNT] = {
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
    "Any"
};

// these can contain wildcard characters. Needed for fontset matching (CJK fonts)
static const char *xNames[CHARSETS_COUNT] = {
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
    QFont::AnyCharSet
};


KFontStruct::KFontStruct()
{
    family = QString::null;
    charset = QFont::AnyCharSet;
    fixed = KCharsets::FixedUnknown;
    slant = KCharsets::SlantUnknown;
    weight = KCharsets::WeightUnknown;
    scalable = false;
}

KFontStruct &KFontStruct::operator = (const QFont &f)
{
    family = f.family();
    charset = f.charSet();
    fixed = f.fixedPitch() ? KCharsets::Fixed : KCharsets::Proportional;
    slant = f.italic() ? KCharsets::Italic : KCharsets::Normal;
    weight = f.bold() ? KCharsets::Bold : KCharsets::Medium;
    scalable = false; // we don't know...

    return *this;
}

KFontStruct &KFontStruct::operator = (const KFontStruct &fs)
{
    family = fs.family;
    charset = fs.charset;
    fixed = fs.fixed;
    slant = fs.slant;
    weight = fs.weight;
    scalable = fs.scalable;

    return *this;
}

KFontStruct::operator QFont()
{
    QFont f;
    f.setFamily(family);
    f.setCharSet(charset);
    if( fixed == KCharsets::Fixed )
       f.setFixedPitch(true);
    else
       f.setFixedPitch(false);
    if( slant == KCharsets::Italic )
       f.setItalic(true);
    else
       f.setItalic(false);
    if( weight == KCharsets::Bold )
       f.setBold(true);
    else
       f.setBold(false);

    return f;
}


// --------------------------------------------------------------------------

KCharsets::KCharsets()
{
    // do some initialization
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
	kdDebug( 0 ) << "unknown entity " << str.ascii() <<", len = " << str.length() << endl;
	return QChar::null;
    }
    //printf("got entity %s = %x\n", str.ascii(), e->code);	

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
    KFontStruct mask;
    mask.family = family;

    KFontStructList lst;

    getFontList(mask, lst);

    QList<QFont::CharSet> chList;
    KFontStruct *fs;

    for(fs = lst.first(); fs != 0; fs = lst.next() )
    {
	if(!chList.contains(&(fs->charset)))
	{
	    QFont::CharSet *c = new QFont::CharSet;
	    *c = fs->charset;
	    chList.append(c);
	}
	return chList;
    }
}

QStringList KCharsets::availableCharsetNames(QString family)
{
    KFontStruct mask;
    mask.family = family;

    KFontStructList lst;
    getFontList(mask, lst);

    QStringList chList;
    KFontStruct *fs;

    for(fs = lst.first(); fs != 0; fs = lst.next() )
    {
	if(fs->charset != QFont::AnyCharSet )
	{
	    QString name = this->name(fs->charset);
	    if(!chList.contains(name))
		chList.append(name);
	}
    }
    return chList;
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
    KFontStruct mask;
    KFontStructList list;
    mask = f;

    mask.charset = charset;
    getFontList(mask, list);
    if(!list.isEmpty()) {
        f.setCharSet(charset);
        return;
    }

    // let's try unicode...
    mask.charset = QFont::Unicode;
    getFontList(mask, list);
    if(!list.isEmpty()) {
        // just setting the charset to unicode should work
        f.setCharSet(QFont::Unicode);
        return;
    }

    // ok... we don't have the charset in the specified family, let's
    // try to find a replacement.

    // the prefered charset and any family
    mask.charset = charset;
    mask.family = QString::null;
    getFontList(mask, list);
    if(!list.isEmpty()) {
        f.setFamily(list.first()->family);
        f.setCharSet(charset);
        return;
    }

    // Unicode and any family
    mask.charset = QFont::Unicode;
    getFontList(mask, list);
    if(!list.isEmpty()) {
        f.setFamily(list.first()->family);
        f.setCharSet(QFont::Unicode);
        return;
    }

    // give up -- just use something
    f.setCharSet(QFont::AnyCharSet);
    return;
}

bool KCharsets::isAvailable(const QString &charset)
{
    return isAvailable(nameToID(charset));
}

bool KCharsets::isAvailable(QFont::CharSet charset)
{
    KFontStruct fs;
    fs.charset = charset;

    KFontStructList list;

    getFontList(fs, list);

    if(!list.isEmpty())
        return true;

    return false;
}


void KCharsets::getFontList(KFontStruct mask, KFontStructList& lst) const
{
    char **fontNames;
    int numFonts;
    Display *kde_display;

    kde_display = kapp->getDisplay();
    lst.setAutoDelete(true);
    lst.clear();

    QString maskStr("-*-");
    if(!mask.family.isEmpty())
        maskStr += mask.family;
    else
        maskStr += "*";

    // we sort out wrong slants afterwards...
    switch ( mask.weight ) {
    case WeightUnknown:
        maskStr += "-*-*-*-*-";
        break;
    case Medium:
        maskStr += "-medium-*-*-*-";
        break;
    case Bold:
        maskStr += "-bold-*-*-*-";
        break;
    }

    if(mask.scalable)
        maskStr += "0-0-*-*-*-"; // perhaps "0-0-0-0-*-" ????
    else
        maskStr += "*-*-*-*-*-";

    maskStr += xCharsetName(mask.charset);

    fontNames = XListFonts(kde_display, maskStr.latin1(), 32767, &numFonts);

    for(int i = 0; i < numFonts; i++) {
        KFontStruct *f = new KFontStruct;

        QCString qfontname = fontNames[i];
        int dash = qfontname.find ('-', 1, true); // find next dash
        if (dash == -1) continue;

        // the font name is between the second and third dash so:
        // let's find the third dash:

        int dash_two = qfontname.find ('-', dash + 1 , true);
        if (dash == -1) continue;
        // fish the name of the font info string
        f->family = qfontname.mid(dash +1, dash_two - dash -1);

        if(qfontname.find("-p-") != -1)
            f->fixed = Proportional;
        else
            f->fixed = Fixed;
        if(qfontname.find("-r-") != -1)
            f->slant = Normal;
        else
            f->slant = Italic;
        if(qfontname.find("-0-0-") != -1)
            f->scalable = true;
        if(qfontname.find("-b-") != -1)
            f->weight = Bold;
        else
            f->weight = Medium;

        // get the charset...
        dash = qfontname.findRev('-');
        dash = qfontname.findRev('-', dash-1);
        QString xname = qfontname.right(qfontname.length()-dash-1);
        f->charset = xNameToID(xname);

        lst.append(f);
    }

    XFreeFontNames(fontNames);
}

QFont::CharSet KCharsets::charsetForLocale()
{
    return nameToID(KGlobal::locale()->charset());
}

bool KCharsets::hasUnicode(QString family) const
{
    KFontStruct fs;
    fs.family = family;
    fs.charset = QFont::Unicode;
    KFontStructList l;

    getFontList(fs, l);

    return !l.isEmpty();
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
	return "koi8-*";
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
	QRegExp r(xNames[i]);
	if( r.match(name) != -1 )
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
    
    kdDebug(0) << "list is: " << conf.readEntry(encoding) << endl;
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

