/*  This file is part of the KDE libraries    Copyright (C) 2007 Chusslove Illich <caslav.ilic@gmx.net>

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

#include <ktranslit.h>

#include <QHash>

// -----------------------------------------------------------------------------
// Base class.

class KTranslitPrivate
{
};

KTranslit::KTranslit ()
: d(NULL)
{
}

KTranslit::~KTranslit ()
{
    delete d;
}

KTranslit *KTranslit::create (const QString &lang)
{
    if (lang == QString::fromAscii("sr")) {
        return new KTranslitSerbian();
    }
    else {
        return NULL;
    }
}

QStringList KTranslit::fallbackList (const QString &lang)
{
    QStringList fallbacks;

    if (   lang == QString::fromAscii("sr@latin")
        or lang == QString::fromAscii("sr@Latn")) {
        fallbacks += QString::fromAscii("sr");
    }

    return fallbacks;
}

void splitLangScript (const QString &lang, QString &ln, QString &scr)
{
    ln = lang;
    scr.clear();
    int pos = lang.indexOf('@');
    if (pos >= 0) {
        ln = lang.left(pos);
        scr = lang.mid(pos + 1);
    }
}

QString KTranslit::higherPriorityScript (const QString &lang,
                                         const KLocale *locale)
{
    // Split into pure language and script part.
    QString ln, scr;
    splitLangScript(lang, ln, scr);

    // Search through higher priority languages.
    QString finalScrHi;
    if (lang != KLocale::defaultLanguage()) {
        foreach (const QString &langHi, locale->languageList()) {
            // Don't search lower priority languages.
            if (langHi == lang)
                break;

            // Split current spec into pure language and script parts.
            QString lnHi, scrHi;
            splitLangScript(langHi, lnHi, scrHi);

            // Return current script if languages match.
            if (lnHi == ln) {
                finalScrHi = scrHi;
                break;
            }
        }
    }
    return finalScrHi;
}

QString KTranslit::transliterate (const QString &str,
                                  const QString &script) const
{
    Q_UNUSED(script);
    return str;
}

// -----------------------------------------------------------------------------
// Serbian.

class KTranslitSerbianPrivate
{
    public:
    QHash<QString, bool> latinNames;
    QHash<QString, QString> dictAll;
    QHash<QString, QString> dictSpec;
    QHash<QString, bool> dictIsCap;
};

KTranslitSerbian::KTranslitSerbian ()
: d(new KTranslitSerbianPrivate())
{
    d->latinNames[QString::fromAscii("Latn")] = true;
    d->latinNames[QString::fromAscii("latin")] = true;

    #define SR_DICTALL_ENTRY(a, b, iscap) do { \
        d->dictAll[QString::fromUtf8(a)] = QString::fromUtf8(b); \
        d->dictIsCap[QString::fromUtf8(a)] = iscap; \
    } while (0)
    SR_DICTALL_ENTRY("а", "a", false);
    SR_DICTALL_ENTRY("б", "b", false);
    SR_DICTALL_ENTRY("в", "v", false);
    SR_DICTALL_ENTRY("г", "g", false);
    SR_DICTALL_ENTRY("д", "d", false);
    SR_DICTALL_ENTRY("ђ", "đ", false);
    SR_DICTALL_ENTRY("е", "e", false);
    SR_DICTALL_ENTRY("ж", "ž", false);
    SR_DICTALL_ENTRY("з", "z", false);
    SR_DICTALL_ENTRY("и", "i", false);
    SR_DICTALL_ENTRY("ј", "j", false);
    SR_DICTALL_ENTRY("к", "k", false);
    SR_DICTALL_ENTRY("л", "l", false);
    SR_DICTALL_ENTRY("љ", "lj", false);
    SR_DICTALL_ENTRY("м", "m", false);
    SR_DICTALL_ENTRY("н", "n", false);
    SR_DICTALL_ENTRY("њ", "nj", false);
    SR_DICTALL_ENTRY("о", "o", false);
    SR_DICTALL_ENTRY("п", "p", false);
    SR_DICTALL_ENTRY("р", "r", false);
    SR_DICTALL_ENTRY("с", "s", false);
    SR_DICTALL_ENTRY("т", "t", false);
    SR_DICTALL_ENTRY("ћ", "ć", false);
    SR_DICTALL_ENTRY("у", "u", false);
    SR_DICTALL_ENTRY("ф", "f", false);
    SR_DICTALL_ENTRY("х", "h", false);
    SR_DICTALL_ENTRY("ц", "c", false);
    SR_DICTALL_ENTRY("ч", "č", false);
    SR_DICTALL_ENTRY("џ", "dž", false);
    SR_DICTALL_ENTRY("ш", "š", false);
    SR_DICTALL_ENTRY("А", "A", true);
    SR_DICTALL_ENTRY("Б", "B", true);
    SR_DICTALL_ENTRY("В", "V", true);
    SR_DICTALL_ENTRY("Г", "G", true);
    SR_DICTALL_ENTRY("Д", "D", true);
    SR_DICTALL_ENTRY("Ђ", "Đ", true);
    SR_DICTALL_ENTRY("Е", "E", true);
    SR_DICTALL_ENTRY("Ж", "Ž", true);
    SR_DICTALL_ENTRY("З", "Z", true);
    SR_DICTALL_ENTRY("И", "I", true);
    SR_DICTALL_ENTRY("Ј", "J", true);
    SR_DICTALL_ENTRY("К", "K", true);
    SR_DICTALL_ENTRY("Л", "L", true);
    SR_DICTALL_ENTRY("Љ", "Lj", true);
    SR_DICTALL_ENTRY("М", "M", true);
    SR_DICTALL_ENTRY("Н", "N", true);
    SR_DICTALL_ENTRY("Њ", "Nj", true);
    SR_DICTALL_ENTRY("О", "O", true);
    SR_DICTALL_ENTRY("П", "P", true);
    SR_DICTALL_ENTRY("Р", "R", true);
    SR_DICTALL_ENTRY("С", "S", true);
    SR_DICTALL_ENTRY("Т", "T", true);
    SR_DICTALL_ENTRY("Ћ", "Ć", true);
    SR_DICTALL_ENTRY("У", "U", true);
    SR_DICTALL_ENTRY("Ф", "F", true);
    SR_DICTALL_ENTRY("Х", "H", true);
    SR_DICTALL_ENTRY("Ц", "C", true);
    SR_DICTALL_ENTRY("Ч", "Č", true);
    SR_DICTALL_ENTRY("Џ", "Dž", true);
    SR_DICTALL_ENTRY("Ш", "Š", true);

    #define SR_DICTSPEC_ENTRY(a, b) do { \
        d->dictSpec[QString::fromUtf8(a)] = QString::fromUtf8(b); \
    } while (0)
    SR_DICTSPEC_ENTRY("Љ", "LJ");
    SR_DICTSPEC_ENTRY("Њ", "NJ");
    SR_DICTSPEC_ENTRY("Џ", "DŽ");
}

KTranslitSerbian::~KTranslitSerbian ()
{
    delete d;
}

QString KTranslitSerbian::transliterate (const QString &str,
                                         const QString &script) const
{
    if (d->latinNames.contains(script)) {
        QString nstr;
        int slen = str.length();
        // NOTE: Conversion is a bit more convoluted then necessary
        // because it handles cases in the most-to-least probable order,
        // in order to speed up the execution.
        for (int i = 0; i < slen; ++i) {
            QChar c = str[i];
            QString r = d->dictAll[c];
            if (!r.isEmpty()) {
                if (d->dictIsCap[c]) {
                    QString rc = d->dictSpec[c];
                    if (   !rc.isEmpty()
                        && (   (i + 1 < slen && d->dictIsCap[str[i + 1]])
                            || (i > 0 && d->dictIsCap[str[i - 1]]))) {
                        nstr += rc;
                    }
                    else {
                        nstr += r;
                    }
                }
                else {
                    nstr += r;
                }
            }
            else {
                nstr += c;
            }
        }

        return nstr;
    }
    else {
        return str;
    }
}
