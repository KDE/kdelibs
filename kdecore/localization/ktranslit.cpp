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

#include <ktranslit_p.h>
#include <kdebug.h>

#include <config.h>

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

    if (lang.startsWith(QString::fromAscii("sr@ije"))) {
        fallbacks += QString::fromAscii("sr@ijekavian");
    }
    if (lang.startsWith(QString::fromAscii("sr@"))) {
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

void KTranslit::splitToBaseAndScript (const QString &lang,
                                      const KLocale *locale,
                                      QString &blang, QString &lscr)
{
    blang = lang;
    lscr = "";

    if (lang == QString::fromAscii("sr") || lang.startsWith(QString::fromAscii("sr@"))) {
        QString langHi = lang;
        if (locale) {
            foreach (const QString &l, locale->languageList()) {
                if (l == QString::fromAscii("sr") || l.startsWith(QString::fromAscii("sr@"))) {
                    langHi = l;
                    break;
                }
            }
        }
        splitLangScript(langHi, blang, lscr);
    }
}

QString KTranslit::transliterate (const QString &str,
                                  const QString &script) const
{
    Q_UNUSED(script);
    return str;
}

QString KTranslit::resolveInserts (const QString &str_, int nins, int ind,
                                   const QString &head) const
{
    int hlen = head.length();

    QString str = str_;
    QString rstr;
    while (1) {
        int p = str.indexOf(head);
        if (p < 0) {
            break;
        }

        // Append segment before optional insert to resulting text.
        rstr.append(str.left(p));

        // Must have at least 2 characters after the head.
        if (str.length() < p + hlen + 2) {
            kDebug(173) << QString("Malformed optional inserts list in {%1}, "
                                   "starting here: {%2}").arg(str_, str);
            return str_;
        }

        // Read the separating character and trim original string.
        QChar sep = str[p + hlen];
        str.remove(0, p + hlen + 1);

        // Parse requested number of inserts,
        // choose the one with matching index for resulting text.
        for (int i = 0; i < nins; ++i) {
            // Ending separator for this insert.
            int p = str.indexOf(sep);

            // Must have exactly the requested number of inserts.
            if (p < 0) {
                kDebug(173) << QString("Not enough inserts listed in {%1}, "
                                       "starting here: {%2}").arg(str_, str);
                return str_;
            }

            // If index is matching requested, append to resulting text.
            if (i == ind) {
                rstr.append(str.left(p));
            }

            // Trim original string.
            str.remove(0, p + 1);
        }
    }
    // Append the final segment to resulting text.
    rstr.append(str);

    return rstr;
}

// If the insert is just starting at position i, return the position of the
// first character after the insert (or string length if none).
// If the insert is not starting, return i itself.
static int skipInsert (const QString &str, int i, int ninserts,
                       const QString &head)
{
    int hlen = head.length();

    if (str.mid(i, hlen) == head) {
        int slen = str.length();
        int ia = i + hlen;
        if (ia >= slen) return slen;
        QChar sep = str[ia];
        for (int k = 0; k < ninserts; ++k) {
            ia = str.indexOf(sep, ia + 1);
            if (ia < 0) return slen;
        }
        return ia + 1;
    }
    else {
        return i;
    }
}

// -----------------------------------------------------------------------------
// Serbian.

class KTranslitSerbianPrivate
{
    public:
    QHash<QString, bool> latinNames;
    QHash<QString, bool> ijekavianNames;
    QHash<QChar, QString> dictC2L;
    QVector<QString> tickI2Es;
    QVector<QHash<QString, QString> > dictI2Es;
    QVector<int> minRefLens;
    QVector<int> maxRefLens;

    QString toLatinSimple (const QString& text,
                           const QString &althead = "") const;
    QString toDialectSimple (const QString &text,
                             const QString &tick, const QHash<QString, QString> &dictI2E,
                             int minRefLen, int maxRefLen, bool toIjek) const;
};

KTranslitSerbian::KTranslitSerbian ()
: d(new KTranslitSerbianPrivate())
{
    // Known locale modifiers for dialect and script combinations.
    #define SR_NAME_ENTRY(hash, name) do { \
        hash[QString::fromAscii(name)] = true; \
    } while (0)
    SR_NAME_ENTRY(d->latinNames, "latin");
    SR_NAME_ENTRY(d->latinNames, "Latn");
    SR_NAME_ENTRY(d->latinNames, "ijelatin");
    SR_NAME_ENTRY(d->latinNames, "jekavianlatin");
    SR_NAME_ENTRY(d->latinNames, "ijekavianlatin");
    SR_NAME_ENTRY(d->latinNames, "yekavianlatin");
    SR_NAME_ENTRY(d->latinNames, "iyekavianlatin");
    SR_NAME_ENTRY(d->ijekavianNames, "ije");
    SR_NAME_ENTRY(d->ijekavianNames, "ijelatin");
    SR_NAME_ENTRY(d->ijekavianNames, "jekavian");
    SR_NAME_ENTRY(d->ijekavianNames, "jekavianlatin");
    SR_NAME_ENTRY(d->ijekavianNames, "ijekavian");
    SR_NAME_ENTRY(d->ijekavianNames, "ijekavianlatin");
    SR_NAME_ENTRY(d->ijekavianNames, "yekavian");
    SR_NAME_ENTRY(d->ijekavianNames, "yekavianlatin");
    SR_NAME_ENTRY(d->ijekavianNames, "iyekavian");
    SR_NAME_ENTRY(d->ijekavianNames, "iyekavianlatin");

    // Mapping from Cyrillic to Latin.
    #define SR_DICTC2L_ENTRY(a, b) do { \
        d->dictC2L[QString::fromUtf8(a)[0]] = QString::fromUtf8(b); \
    } while (0)
    SR_DICTC2L_ENTRY("а", "a");
    SR_DICTC2L_ENTRY("б", "b");
    SR_DICTC2L_ENTRY("в", "v");
    SR_DICTC2L_ENTRY("г", "g");
    SR_DICTC2L_ENTRY("д", "d");
    SR_DICTC2L_ENTRY("ђ", "đ");
    SR_DICTC2L_ENTRY("е", "e");
    SR_DICTC2L_ENTRY("ж", "ž");
    SR_DICTC2L_ENTRY("з", "z");
    SR_DICTC2L_ENTRY("и", "i");
    SR_DICTC2L_ENTRY("ј", "j");
    SR_DICTC2L_ENTRY("к", "k");
    SR_DICTC2L_ENTRY("л", "l");
    SR_DICTC2L_ENTRY("љ", "lj");
    SR_DICTC2L_ENTRY("м", "m");
    SR_DICTC2L_ENTRY("н", "n");
    SR_DICTC2L_ENTRY("њ", "nj");
    SR_DICTC2L_ENTRY("о", "o");
    SR_DICTC2L_ENTRY("п", "p");
    SR_DICTC2L_ENTRY("р", "r");
    SR_DICTC2L_ENTRY("с", "s");
    SR_DICTC2L_ENTRY("т", "t");
    SR_DICTC2L_ENTRY("ћ", "ć");
    SR_DICTC2L_ENTRY("у", "u");
    SR_DICTC2L_ENTRY("ф", "f");
    SR_DICTC2L_ENTRY("х", "h");
    SR_DICTC2L_ENTRY("ц", "c");
    SR_DICTC2L_ENTRY("ч", "č");
    SR_DICTC2L_ENTRY("џ", "dž");
    SR_DICTC2L_ENTRY("ш", "š");
    SR_DICTC2L_ENTRY("А", "A");
    SR_DICTC2L_ENTRY("Б", "B");
    SR_DICTC2L_ENTRY("В", "V");
    SR_DICTC2L_ENTRY("Г", "G");
    SR_DICTC2L_ENTRY("Д", "D");
    SR_DICTC2L_ENTRY("Ђ", "Đ");
    SR_DICTC2L_ENTRY("Е", "E");
    SR_DICTC2L_ENTRY("Ж", "Ž");
    SR_DICTC2L_ENTRY("З", "Z");
    SR_DICTC2L_ENTRY("И", "I");
    SR_DICTC2L_ENTRY("Ј", "J");
    SR_DICTC2L_ENTRY("К", "K");
    SR_DICTC2L_ENTRY("Л", "L");
    SR_DICTC2L_ENTRY("Љ", "Lj");
    SR_DICTC2L_ENTRY("М", "M");
    SR_DICTC2L_ENTRY("Н", "N");
    SR_DICTC2L_ENTRY("Њ", "Nj");
    SR_DICTC2L_ENTRY("О", "O");
    SR_DICTC2L_ENTRY("П", "P");
    SR_DICTC2L_ENTRY("Р", "R");
    SR_DICTC2L_ENTRY("С", "S");
    SR_DICTC2L_ENTRY("Т", "T");
    SR_DICTC2L_ENTRY("Ћ", "Ć");
    SR_DICTC2L_ENTRY("У", "U");
    SR_DICTC2L_ENTRY("Ф", "F");
    SR_DICTC2L_ENTRY("Х", "H");
    SR_DICTC2L_ENTRY("Ц", "C");
    SR_DICTC2L_ENTRY("Ч", "Č");
    SR_DICTC2L_ENTRY("Џ", "Dž");
    SR_DICTC2L_ENTRY("Ш", "Š");
    // ...and some accented letters existing as NFC:
    SR_DICTC2L_ENTRY("ѐ", "è");
    SR_DICTC2L_ENTRY("ѝ", "ì");
    SR_DICTC2L_ENTRY("ӣ", "ī");
    SR_DICTC2L_ENTRY("ӯ", "ū");
    SR_DICTC2L_ENTRY("Ѐ", "È");
    SR_DICTC2L_ENTRY("Ѝ", "Ì");
    SR_DICTC2L_ENTRY("Ӣ", "Ī");
    SR_DICTC2L_ENTRY("Ӯ", "Ū");

    // Mapping from Ijekavian to Ekavian.
    #define SR_TICKI2E_ENTRY(t) do { \
        d->tickI2Es.append(QString::fromUtf8(t)); \
        d->dictI2Es.append(QHash<QString, QString>()); \
    } while (0)
    #define SR_DICTI2E_ENTRY(i, e) do { \
        d->dictI2Es.last()[QString::fromUtf8(i)] = QString::fromUtf8(e); \
    } while (0)
    SR_TICKI2E_ENTRY("›");
    SR_DICTI2E_ENTRY("ије", "е");
    SR_DICTI2E_ENTRY("је", "е");
    SR_TICKI2E_ENTRY("‹");
    SR_DICTI2E_ENTRY("иј", "еј");
    SR_DICTI2E_ENTRY("иљ", "ел");
    SR_DICTI2E_ENTRY("ио", "ео");
    SR_DICTI2E_ENTRY("ље", "ле");
    SR_DICTI2E_ENTRY("ње", "не");
    SR_TICKI2E_ENTRY("▹");
    SR_DICTI2E_ENTRY("ије", "и");
    SR_DICTI2E_ENTRY("је", "и");
    SR_TICKI2E_ENTRY("◃");
    SR_DICTI2E_ENTRY("ијел", "ео");
    SR_DICTI2E_ENTRY("ијен", "ењ");
    SR_DICTI2E_ENTRY("ил", "ел");
    SR_DICTI2E_ENTRY("ит", "ет");
    SR_DICTI2E_ENTRY("јел", "ео");
    SR_DICTI2E_ENTRY("тн", "тњ");
    SR_DICTI2E_ENTRY("шње", "сне");
    // derived mappings
    for (int i = 0; i < d->dictI2Es.size(); ++i) {
        QHash<QString, QString> &dictI2E = d->dictI2Es[i];
        // derived Latin mappings (before capitalization and uppercasing)
        foreach (const QString &ref, dictI2E.keys()) {
            QString ekvform = dictI2E.value(ref);
            dictI2E[d->toLatinSimple(ref)] = d->toLatinSimple(ekvform);
        }
        // derived capitalized and uppercase mappings
        foreach (const QString &ref, dictI2E.keys()) {
            QString ekvform = dictI2E.value(ref);
            dictI2E[ref[0].toUpper() + ref.mid(1)] = ekvform[0].toUpper() + ekvform.mid(1);
            dictI2E[ref.toUpper()] = ekvform.toUpper();
        }
        // maximum and minimum length of Ijekavian forms
        int minRefLen = 100;
        int maxRefLen = 0;
        foreach (const QString &ref, dictI2E.keys()) {
            int refLen = ref.length();
            if (minRefLen > refLen) {
                minRefLen = refLen;
            }
            if (maxRefLen < refLen) {
                maxRefLen = refLen;
            }
        }
        d->minRefLens.append(minRefLen);
        d->maxRefLens.append(maxRefLen);
    }
}

KTranslitSerbian::~KTranslitSerbian ()
{
    delete d;
}

QString KTranslitSerbianPrivate::toLatinSimple (const QString &text,
                                                const QString &altHead) const
{
    // NOTE: This loop has been somewhat optimized for speed.
    int slen = text.length();
    bool haveAlts = !altHead.isEmpty() && text.indexOf(altHead) >= 0;
    QString ntext;
    ntext.reserve(slen + 5);
    for (int i = 0; i < slen; ++i) {
        // Skip alternatives directives altogether, so that they can be used
        // as a mean to exclude from transliteration.
        if (haveAlts) {
            int to = skipInsert(text, i, 2, altHead);
            if (to > i) {
                ntext.append(text.mid(i, to - i));
                if (to >= slen) break;
                i = to;
            }
        }
        // Transliterate current character.
        QChar c = text[i];
        QString r = dictC2L[c];
        if (!r.isEmpty()) {
            if (   r.length() > 1 && c.isUpper()
                && (   (i + 1 < slen && text[i + 1].isUpper())
                    || (i > 0 && text[i - 1].isUpper()))) {
                ntext.append(r.toUpper());
            } else {
                ntext.append(r);
            }
        } else {
            ntext.append(c);
        }
    }
    return ntext;
}

QString KTranslitSerbianPrivate::toDialectSimple (const QString &str,
                                                  const QString &tick,
                                                  const QHash<QString, QString> &dictI2E,
                                                  int minRefLen,
                                                  int maxRefLen,
                                                  bool toIjek) const
{
    QString nstr;
    int p = 0;
    while (true) {
        int pp = p;
        p = str.indexOf(tick, p);
        if (p < 0) {
            nstr.append(str.mid(pp));
            break;
        }
        nstr.append(str.mid(pp, p - pp));
        p += tick.length();
        if (p >= str.length() || !str[p].isLetter()) {
            nstr.append(tick);
            continue;
        }

        QString ijkfrm;
        QString ekvfrm;
        int refLen = minRefLen;
        while (refLen <= maxRefLen && ekvfrm.isEmpty()) {
            ijkfrm = str.mid(p, refLen);
            ekvfrm = dictI2E.value(ijkfrm);
            ++refLen;
        }
        if (!ekvfrm.isEmpty()) {
            nstr.append(!toIjek ? ekvfrm : ijkfrm);
            p += ijkfrm.length();
        } else {
            nstr.append(tick);
        }
    }
    return nstr;
}

QString KTranslitSerbian::transliterate (const QString &str_,
                                         const QString &script) const
{
    static QString insHead("~@");
    static QString insHeadIje("~#");

    QString str = str_;

    // Resolve Ekavian/Ijekavian.
    bool toIjek;
    int toAlt;
    if (d->ijekavianNames.contains(script)) {
        toIjek = true;
        toAlt = 1;
    } else {
        toIjek = false;
        toAlt = 0;
    }
    for (int i = 0; i < d->tickI2Es.size(); ++i) {
        str = d->toDialectSimple(str, d->tickI2Es[i], d->dictI2Es[i],
                                 d->minRefLens[i], d->maxRefLens[i], toIjek);
    }
    str = resolveInserts(str, 2, toAlt, insHeadIje);

    // Resolve Cyrillic/Latin.
    if (d->latinNames.contains(script)) {
        str = d->toLatinSimple(str, insHead);
        str = resolveInserts(str, 2, 1, insHead);
    } else {
        str = resolveInserts(str, 2, 0, insHead);
    }

    return str;
}
