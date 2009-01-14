/*  This file is part of the KDE libraries
    Copyright (C) 2007 Chusslove Illich <caslav.ilic@gmx.net>

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

#include <kuitformats_p.h>

#include <config.h>
#include <kglobal.h>
#include <klocale.h>

#include <QStringList>
#include <QRegExp>

#include <kdebug.h>

class KuitFormatsStaticData
{
    public:

    KuitFormatsStaticData ();

    QHash<QChar, QChar> westToEastArabicDigit;
    QChar easternArabicThSep;
    QChar easternArabicDecSep;
};

KuitFormatsStaticData::KuitFormatsStaticData ()
{
    #define WEA_ENTRY(a, b) do { \
        westToEastArabicDigit[a] = QString::fromUtf8(b)[0]; \
    } while (0)
    WEA_ENTRY('1', "١");
    WEA_ENTRY('2', "٢");
    WEA_ENTRY('3', "٣");
    WEA_ENTRY('4', "٤");
    WEA_ENTRY('5', "٥");
    WEA_ENTRY('6', "٦");
    WEA_ENTRY('7', "٧");
    WEA_ENTRY('8', "٨");
    WEA_ENTRY('9', "٩");
    WEA_ENTRY('0', "٠");

    easternArabicThSep = QString::fromUtf8(".")[0];
    easternArabicDecSep = QString::fromUtf8(",")[0];
}

K_GLOBAL_STATIC(KuitFormatsStaticData, staticData)

static QString insertIntegerSeparators (const QString &istr,
                                        const QChar &sep, int ngrp)
{
    int len = istr.length();
    int flen = (len / ngrp + 1) * (ngrp + 1);
    QString fistr(flen, ' ');
    for (int i = 0, fi = 0; i < len; ++i, ++fi) {
        if (i > 0 && i % 3 == 0) {
            fistr[flen - fi - 1] = sep;
            ++fi;
        }
        fistr[flen - fi - 1] = istr[len - i - 1];
    }
    fistr = fistr.trimmed();
    return fistr;
}

static QString toNumberGeneric (const QString &numstr,
                                const QChar &thosep, const QChar &decsep,
                                int thosepGE = 0)
{
    int len = numstr.length();
    int p1 = 0;
    while (p1 < len && !numstr[p1].isDigit()) {
        ++p1;
    }
    if (p1 == len) {
        return numstr;
    }
    int p2 = p1 + 1;
    while (p2 < len && numstr[p2].isDigit()) {
        ++p2;
    }
    QString intpart = numstr.mid(p1, p2 - p1);
    int intval = intpart.toInt();

    QString pre = numstr.left(p1);
    QString mid = intpart;
    if (intval >= thosepGE) {
        mid = insertIntegerSeparators(intpart, thosep, 3);
    }

    QString post = numstr.mid(p2);
    if (post.startsWith('.')) {
        post[0] = decsep;
    }

    return pre + mid + post;
}

QString KuitFormats::toNumberSystem (const QString &numstr)
{
    return KGlobal::locale()->formatNumber(numstr, false);
}

QString KuitFormats::toNumberUS (const QString &numstr)
{
    return toNumberGeneric(numstr, ',', '.');
}

QString KuitFormats::toNumberEuro (const QString &numstr)
{
    return toNumberGeneric(numstr, '.', ',');
}

QString KuitFormats::toNumberEuro2 (const QString &numstr)
{
    return toNumberGeneric(numstr, ' ', ',');
}

QString KuitFormats::toNumberEuro2ct (const QString &numstr)
{
    return toNumberGeneric(numstr, ' ', ',', 10000);
}

// Translated from a Pascal implementation provided
// by Youssef Chahibi <chahibi@gmail.com>
QString KuitFormats::toNumberEArab (const QString &numstr)
{
    KuitFormatsStaticData *s = staticData;

    const int power = 3;

    // Construct string with proper separators, but Western Arabic digits.
    QString sepnum;

    // Find decimal separator in input.
    int i = 0;
    while ((i < numstr.length()) && (numstr[i] != '.')) {
        ++i;
    }

    // Add thousand separators to integer part.
    int j = power;
    while (j < i) {
        sepnum = s->easternArabicThSep + numstr.mid(i - j, power) + sepnum;
        j += power;
    }
    sepnum = numstr.left(i + power - j) + sepnum;

    // Add decimal part.
    if (i + 1 < numstr.length()) {
        sepnum += s->easternArabicDecSep + numstr.mid(i + 1);
    }

    // Add leading 0 if by any chance the input starts with decimal separator.
    if (numstr.length() > 1 && numstr[0] == '.') {
        sepnum = '0' + sepnum;
    }

    // Replace Western with Eastern Arabic digits in the separated string.
    QString arnum;
    for (int i = 0; i < sepnum.length(); ++i) {
        arnum += s->westToEastArabicDigit.value(sepnum[i], sepnum[i]);
    }

    return arnum;
}

QString KuitFormats::toKeyCombo (const QString &shstr, const QString &delim,
                                 const QHash<QString, QString> &keydict)
{
    static QRegExp delRx("[+-]");

    int p = delRx.indexIn(shstr); // find delimiter

    QStringList keys;
    if (p < 0) { // single-key shortcut, no delimiter found
        keys.append(shstr);
    }
    else { // multi-key shortcut
        QChar oldDelim = shstr[p];
        keys = shstr.split(oldDelim, QString::SkipEmptyParts);
    }

    for (int i = 0; i < keys.size(); ++i) {
        // Normalize key, trim and all lower-case.
        QString nkey = keys[i].trimmed().toLower();
        bool isFunctionKey = nkey.length() > 1 && nkey[1].isDigit();
        if (!isFunctionKey) {
            keys[i] = keydict.contains(nkey) ? keydict[nkey] : keys[i].trimmed();
        }
        else {
            keys[i] = keydict["f%1"].arg(nkey.mid(1));
        }
    }
    return keys.join(delim);
}

QString KuitFormats::toInterfacePath (const QString &inpstr,
                                      const QString &delim)
{
    static QRegExp delRx("\\||->");

    int p = delRx.indexIn(inpstr); // find delimiter
    if (p < 0) { // single-element path, no delimiter found
        return inpstr;
    }
    else { // multi-element path
        QString oldDelim = delRx.capturedTexts().at(0);
        QStringList guiels = inpstr.split(oldDelim, QString::SkipEmptyParts);
        return guiels.join(delim);
    }
}

