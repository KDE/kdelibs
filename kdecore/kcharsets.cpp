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

#include <kapplication.h>
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

#define CHARSETS_COUNT 32

static const char * const language_names[] = {
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
        kc = _kc;
        conf = new KConfig( "charsets", true, false );
    }
    ~KCharsetsPrivate()
    {
        delete db;
        delete conf;
    }
    QFontDatabase *db;
    QAsciiDict<QTextCodec> codecForNameDict;
    KCharsets* kc;
    KConfig* conf;
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

QStringList KCharsets::availableEncodingNames()
{
    QStringList available;

    QMap<QString, QString> map = d->conf->entryMap("charsetsForEncoding");
    d->conf->setGroup("charsetsForEncoding");

    QMap<QString, QString>::ConstIterator it;
    for( it = map.begin(); it != map.end(); ++it ) {
        //kdDebug(0) << "key = " << it.key() << " string =" << it.data() << endl;

        //kdDebug(0) << "list is: " << d->conf->readEntry(it.key()) << endl;
        QStringList charsets = d->conf->readListEntry(it.key());

        // iterate thorugh the list and find the first charset that is available
        for ( QStringList::ConstIterator sit = charsets.begin(); sit != charsets.end(); ++sit ) {
            //kdDebug(0) << "checking for " << *sit << endl;
#ifdef __GNUC__
#warning FIXME?
#endif
            if( true ) {
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
    d->conf->setGroup("LanguageForEncoding");

    int lang = d->conf->readNumEntry(encoding, 0 );
    return i18n( language_names[lang] );
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
    QCString key = name;
    if (name.right(8) == "_charset")
       name.truncate(name.length()-8);

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
        d->codecForNameDict.replace(key, codec);
        return codec;
    }

    // these codecs are built into Qt, but the name given for the codec is different,
    // so QTextCodec did not recognise it.
    d->conf->setGroup("builtin");

    QString cname = d->conf->readEntry(name.data());
    if(!cname.isEmpty() && !cname.isNull())
        codec = QTextCodec::codecForName(cname.latin1());

    if(codec)
    {
        d->codecForNameDict.replace(key, codec);
        return codec;
    }

    d->conf->setGroup("general");
    QString dir = d->conf->readEntry("i18ndir", QString::fromLatin1("/usr/share/i18n/charmaps"));
    dir += "/";

    // these are codecs not included in Qt. They can be build up if the corresponding charmap
    // is available in the charmap directory.
    d->conf->setGroup("aliases");

    cname = d->conf->readEntry(name.data());
    if(cname.isNull() || cname.isEmpty())
        cname = name;
    cname = cname.upper();

    codec = QTextCodec::loadCharmapFile(dir + cname);

    if(codec) {
        d->codecForNameDict.replace(key, codec);
        return codec;
    }

    // this also failed, the last resort is now to take some compatibility charmap

    d->conf->setGroup("conversionHints");
    cname = cname.lower();
    cname = d->conf->readEntry(cname);

    if(!cname.isEmpty() && !cname.isNull())
        codec = QTextCodec::codecForName(cname.latin1());

    if(codec) {
        d->codecForNameDict.replace(key, codec);
        return codec;
    }

    // could not assign a codec, let's return Latin1
    ok = false;
    return QTextCodec::codecForName("iso8859-1");
}
