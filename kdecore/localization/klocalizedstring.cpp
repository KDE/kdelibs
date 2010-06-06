/*  This file is part of the KDE libraries
    Copyright (C) 2006 Chusslove Illich <caslav.ilic@gmx.net>

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

#include <klocalizedstring.h>

#include <config.h>

#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <klocale_p.h>
#include <kcomponentdata.h>
#include <klibrary.h>
#include <kstandarddirs.h>
#include <ktranscript_p.h>
#include <kuitsemantics_p.h>
#include "kcatalogname_p.h"

#include <QMutexLocker>
#include <QStringList>
#include <QByteArray>
#include <QChar>
#include <QHash>
#include <QList>
#include <QVector>

// Truncates string, for output of long messages.
static QString shortenMessage (const QString &str)
{
    const int maxlen = 20;
    if (str.length() <= maxlen)
        return str;
    else
        return str.left(maxlen).append("...");
}

typedef qulonglong pluraln;
typedef qlonglong intn;
typedef qulonglong uintn;
typedef double realn;

class KLocalizedStringPrivateStatics;

class KLocalizedStringPrivate
{
    friend class KLocalizedString;

    QStringList args;
    QList<QVariant> vals;
    bool numberSet;
    pluraln number;
    int numberOrd;
    QByteArray ctxt;
    QHash<QString, QString> dynctxt;
    QByteArray msg;
    QByteArray plural;

    QString toString (const KLocale *locale, const QString &catalogName) const;
    QString selectForEnglish () const;
    QString substituteSimple (const QString &trans,
                              const QChar &plchar = '%',
                              bool partial = false) const;
    QString postFormat (const QString &text,
                        const QString &lang,
                        const QString &ctxt) const;
    QString substituteTranscript (const QString &trans,
                                  const QString &lang,
                                  const QString &ctry,
                                  const QString &final,
                                  bool &fallback) const;
    int resolveInterpolation (const QString &trans, int pos,
                              const QString &lang,
                              const QString &ctry,
                              const QString &final,
                              QString &result,
                              bool &fallback) const;
    QVariant segmentToValue (const QString &arg) const;
    QString postTranscript (const QString &pcall,
                            const QString &lang,
                            const QString &ctry,
                            const QString &final) const;

    static void notifyCatalogsUpdated (const QStringList &languages,
                                       const QList<KCatalogName> &catalogs);
    static void loadTranscript ();
};

class KLocalizedStringPrivateStatics
{
    public:

    const QString theFence;
    const QString startInterp;
    const QString endInterp;
    const QChar scriptPlchar;
    const QChar scriptVachar;

    const QString scriptDir;
    QHash<QString, QStringList> scriptModules;
    QList<QStringList> scriptModulesToLoad;

    bool loadTranscriptCalled;
    KTranscript *ktrs;

    QHash<QString, KuitSemantics*> formatters;

    KLocalizedStringPrivateStatics () :
        theFence("|/|"),
        startInterp("$["),
        endInterp("]"),
        scriptPlchar('%'),
        scriptVachar('^'),

        scriptDir("LC_SCRIPTS"),
        scriptModules(),
        scriptModulesToLoad(),

        loadTranscriptCalled(false),
        ktrs(NULL),

        formatters()
    {}

    ~KLocalizedStringPrivateStatics ()
    {
        // ktrs is handled by KLibLoader.
        //delete ktrs;
        qDeleteAll(formatters);
    }
};
K_GLOBAL_STATIC(KLocalizedStringPrivateStatics, staticsKLSP)

KLocalizedString::KLocalizedString ()
: d(new KLocalizedStringPrivate)
{
    d->numberSet = false;
    d->number = 0;
    d->numberOrd = 0;
}

KLocalizedString::KLocalizedString (const char *ctxt,
                                    const char *msg, const char *plural)
: d(new KLocalizedStringPrivate)
{
    d->ctxt = ctxt;
    d->msg = msg;
    d->plural = plural;
    d->numberSet = false;
    d->number = 0;
    d->numberOrd = 0;
}

KLocalizedString::KLocalizedString(const KLocalizedString &rhs)
: d(new KLocalizedStringPrivate(*rhs.d))
{
}

KLocalizedString& KLocalizedString::operator= (const KLocalizedString &rhs)
{
    if (&rhs != this)
    {
        *d = *rhs.d;
    }
    return *this;
}

KLocalizedString::~KLocalizedString ()
{
    delete d;
}

bool KLocalizedString::isEmpty () const
{
    return d->msg.isEmpty();
}

QString KLocalizedString::toString () const
{
    return d->toString(KGlobal::locale(), QString());
}

QString KLocalizedString::toString (const QString &catalogName) const
{
    return d->toString(KGlobal::locale(), catalogName);
}

QString KLocalizedString::toString (const KLocale *locale) const
{
    return d->toString(locale, QString());
}

QString KLocalizedString::toString (const KLocale *locale,
                                    const QString &catalogName) const
{
    return d->toString(locale, catalogName);
}

QString KLocalizedStringPrivate::toString (const KLocale *locale,
                                           const QString &catalogName) const
{
    const KLocalizedStringPrivateStatics *s = staticsKLSP;

    QMutexLocker lock(kLocaleMutex());

    // Assure the message has been supplied.
    if (msg.isEmpty())
    {
        kDebug(173) << "Trying to convert empty KLocalizedString to QString.";
        #ifndef NDEBUG
        return QString("(I18N_EMPTY_MESSAGE)");
        #else
        return QString();
        #endif
    }

    // Check whether plural argument has been supplied, if message has plural.
    if (!plural.isEmpty() && !numberSet)
        kDebug(173) << QString("Plural argument to message {%1} not supplied before conversion.")
                              .arg(shortenMessage(QString::fromUtf8(msg)));

    // Get raw translation.
    QString rawtrans, lang, ctry;
    const char *catname = catalogName.toUtf8();
    if (locale != NULL) {
        if (!ctxt.isEmpty() && !plural.isEmpty()) {
            locale->translateRawFrom(catname, ctxt, msg, plural, number,
                                     &lang, &rawtrans);
        } else if (!plural.isEmpty()) {
            locale->translateRawFrom(catname, msg, plural, number,
                                     &lang, &rawtrans);
        } else if (!ctxt.isEmpty()) {
            locale->translateRawFrom(catname, ctxt, msg,
                                     &lang, &rawtrans);
        } else {
            locale->translateRawFrom(catname, msg,
                                     &lang, &rawtrans);
        }
        ctry = locale->country();
    } else {
        lang = KLocale::defaultLanguage();
        ctry = 'C';
        rawtrans = selectForEnglish();
    }

    // Set ordinary translation and possibly scripted translation.
    QString trans, strans;
    int cdpos = rawtrans.indexOf(s->theFence);
    if (cdpos > 0)
    {
        // Script fence has been found, strip the scripted from the
        // ordinary translation.
        trans = rawtrans.left(cdpos);

        // Scripted translation.
        strans = rawtrans.mid(cdpos + s->theFence.length());

        // Try to initialize Transcript if not initialized, and script not empty.
        if (   !s->loadTranscriptCalled && !strans.isEmpty()
            && locale && locale->useTranscript())
        {
            if (KGlobal::hasMainComponent())
                loadTranscript();
            else
                kDebug(173) << QString("Scripted message {%1} before transcript engine can be loaded.")
                                      .arg(shortenMessage(trans));
        }
    }
    else if (cdpos < 0)
    {
        // No script fence, use translation as is.
        trans = rawtrans;
    }
    else // cdpos == 0
    {
        // The msgstr starts with the script fence, no ordinary translation.
        // This is not allowed, consider message not translated.
        kDebug(173) << QString("Scripted message {%1} without ordinary translation, discarded.")
                               .arg(shortenMessage(trans)) ;
        trans = selectForEnglish();
    }

    // Substitute placeholders in ordinary translation.
    QString final = substituteSimple(trans);
    // Post-format ordinary translation.
    final = postFormat(final, lang, ctxt);

    // If there is also a scripted translation.
    if (!strans.isEmpty()) {
        // Evaluate scripted translation.
        bool fallback;
        QString sfinal = substituteTranscript(strans, lang, ctry, final, fallback);

        // If any translation produced and no fallback requested.
        if (!sfinal.isEmpty() && !fallback) {
            final = postFormat(sfinal, lang, ctxt);
        }
    }

    // Execute any scripted post calls; they cannot modify the final result,
    // but are used to set states.
    if (s->ktrs != NULL)
    {
        QStringList pcalls = s->ktrs->postCalls(lang);
        foreach(const QString &pcall, pcalls)
            postTranscript(pcall, lang, ctry, final);
    }

    return final;
}

QString KLocalizedStringPrivate::selectForEnglish () const
{
    QString trans;

    if (!plural.isEmpty()) {
        if (number == 1) {
            trans = QString::fromUtf8(msg);
        }
        else {
            trans = QString::fromUtf8(plural);
        }
    }
    else {
        trans = QString::fromUtf8(msg);
    }

    return trans;
}

QString KLocalizedStringPrivate::substituteSimple (const QString &trans,
                                                   const QChar &plchar,
                                                   bool partial) const
{
    #ifdef NDEBUG
    Q_UNUSED(partial);
    #endif

    QStringList tsegs; // text segments per placeholder occurrence
    QList<int> plords; // ordinal numbers per placeholder occurrence
    #ifndef NDEBUG
    QVector<int> ords; // indicates which placeholders are present
    #endif
    int slen = trans.length();
    int spos = 0;
    int tpos = trans.indexOf(plchar);
    while (tpos >= 0)
    {
        int ctpos = tpos;

        tpos++;
        if (tpos == slen)
            break;

        if (trans[tpos].digitValue() > 0) // %0 not considered a placeholder
        {
            // Get the placeholder ordinal.
            int plord = 0;
            while (tpos < slen && trans[tpos].digitValue() >= 0)
            {
                plord = 10 * plord + trans[tpos].digitValue();
                tpos++;
            }
            plord--; // ordinals are zero based

            #ifndef NDEBUG
            // Perhaps enlarge storage for indicators.
            // Note that QVector<int> will initialize new elements to 0,
            // as they are supposed to be.
            if (plord >= ords.size())
                ords.resize(plord + 1);

            // Indicate that placeholder with computed ordinal is present.
            ords[plord] = 1;
            #endif

            // Store text segment prior to placeholder and placeholder number.
            tsegs.append(trans.mid(spos, ctpos - spos));
            plords.append(plord);

            // Position of next text segment.
            spos = tpos;
        }

        tpos = trans.indexOf(plchar, tpos);
    }
    // Store last text segment.
    tsegs.append(trans.mid(spos));

    #ifndef NDEBUG
    // Perhaps enlarge storage for plural-number ordinal.
    if (!plural.isEmpty() && numberOrd >= ords.size())
        ords.resize(numberOrd + 1);

    // Message might have plural but without plural placeholder, which is an
    // allowed state. To ease further logic, indicate that plural placeholder
    // is present anyway if message has plural.
    if (!plural.isEmpty())
        ords[numberOrd] = 1;
    #endif

    // Assemble the final string from text segments and arguments.
    QString final;
    for (int i = 0; i < plords.size(); i++)
    {
        final.append(tsegs.at(i));
        if (plords.at(i) >= args.size())
        // too little arguments
        {
            // put back the placeholder
            final.append('%' + QString::number(plords.at(i) + 1));
            #ifndef NDEBUG
            if (!partial)
                // spoof the message
                final.append("(I18N_ARGUMENT_MISSING)");
            #endif
        }
        else
        // just fine
            final.append(args.at(plords.at(i)));
    }
    final.append(tsegs.last());

    #ifndef NDEBUG
    if (!partial)
    {
        // Check that there are no gaps in numbering sequence of placeholders.
        bool gaps = false;
        for (int i = 0; i < ords.size(); i++)
            if (!ords.at(i))
            {
                gaps = true;
                kDebug(173) << QString("Placeholder %%1 skipped in message {%2}.")
                                      .arg(QString::number(i + 1), shortenMessage(trans));
            }
        // If no gaps, check for mismatch between number of unique placeholders and
        // actually supplied arguments.
        if (!gaps && ords.size() != args.size())
            kDebug(173) << QString("%1 instead of %2 arguments to message {%3} supplied before conversion.")
                                  .arg(args.size()).arg(ords.size()).arg(shortenMessage(trans));

        // Some spoofs.
        if (gaps)
            final.append("(I18N_GAPS_IN_PLACEHOLDER_SEQUENCE)");
        if (ords.size() < args.size())
            final.append("(I18N_EXCESS_ARGUMENTS_SUPPLIED)");
        if (!plural.isEmpty() && !numberSet)
            final.append("(I18N_PLURAL_ARGUMENT_MISSING)");
    }
    #endif

    return final;
}

QString KLocalizedStringPrivate::postFormat (const QString &text,
                                             const QString &lang,
                                             const QString &ctxt) const
{
    const KLocalizedStringPrivateStatics *s = staticsKLSP;
    QMutexLocker lock(kLocaleMutex());

    QString final = text;

    // Transform any semantic markup into visual formatting.
    if (s->formatters.contains(lang)) {
        final = s->formatters[lang]->format(final, ctxt);
    }

    return final;
}

QString KLocalizedStringPrivate::substituteTranscript (const QString &strans,
                                                       const QString &lang,
                                                       const QString &ctry,
                                                       const QString &final,
                                                       bool &fallback) const
{
    const KLocalizedStringPrivateStatics *s = staticsKLSP;
    QMutexLocker lock(kLocaleMutex());

    if (s->ktrs == NULL)
        // Scripting engine not available.
        return QString();

    // Iterate by interpolations.
    QString sfinal;
    fallback = false;
    int ppos = 0;
    int tpos = strans.indexOf(s->startInterp);
    while (tpos >= 0)
    {
        // Resolve substitutions in preceding text.
        QString ptext = substituteSimple(strans.mid(ppos, tpos - ppos),
                                         s->scriptPlchar, true);
        sfinal.append(ptext);

        // Resolve interpolation.
        QString result;
        bool fallbackLocal;
        tpos = resolveInterpolation(strans, tpos, lang, ctry, final,
                                    result, fallbackLocal);

        // If there was a problem in parsing the interpolation, cannot proceed
        // (debug info already reported while parsing).
        if (tpos < 0) {
            return QString();
        }
        // If fallback has been explicitly requested, indicate global fallback
        // but proceed with evaluations (other interpolations may set states).
        if (fallbackLocal) {
            fallback = true;
        }

        // Add evaluated interpolation to the text.
        sfinal.append(result);

        // On to next interpolation.
        ppos = tpos;
        tpos = strans.indexOf(s->startInterp, tpos);
    }
    // Last text segment.
    sfinal.append(substituteSimple(strans.mid(ppos), s->scriptPlchar, true));

    // Return empty string if fallback was requested.
    return fallback ? QString() : sfinal;
}

int KLocalizedStringPrivate::resolveInterpolation (const QString &strans,
                                                   int pos,
                                                   const QString &lang,
                                                   const QString &ctry,
                                                   const QString &final,
                                                   QString &result,
                                                   bool &fallback) const
{
    // pos is the position of opening character sequence.
    // Returns the position of first character after closing sequence,
    // or -1 in case of parsing error.
    // result is set to result of Transcript evaluation.
    // fallback is set to true if Transcript evaluation requested so.

    KLocalizedStringPrivateStatics *s = staticsKLSP;
    QMutexLocker lock(kLocaleMutex());

    result.clear();
    fallback = false;

    // Split interpolation into arguments.
    QList<QVariant> iargs;
    int slen = strans.length();
    int islen = s->startInterp.length();
    int ielen = s->endInterp.length();
    int tpos = pos + s->startInterp.length();
    while (1)
    {
        // Skip whitespace.
        while (tpos < slen && strans[tpos].isSpace()) {
            ++tpos;
        }
        if (tpos == slen) {
            kDebug(173) << QString("Unclosed interpolation {%1} in message {%2}.")
                                  .arg(strans.mid(pos, tpos - pos), shortenMessage(strans));
            return -1;
        }
        if (strans.mid(tpos, ielen) == s->endInterp) {
            break; // no more arguments
        }

        // Parse argument: may be concatenated from free and quoted text,
        // and sub-interpolations.
        // Free and quoted segments may contain placeholders, substitute them;
        // recurse into sub-interpolations.
        // Free segments may be value references, parse and record for
        // consideration at the end.
        // Mind backslash escapes throughout.
        QStringList segs;
        QVariant vref;
        while (   !strans[tpos].isSpace()
               && strans.mid(tpos, ielen) != s->endInterp)
        {
            if (strans[tpos] == '\'') { // quoted segment
                QString seg;
                ++tpos; // skip opening quote
                // Find closing quote.
                while (tpos < slen && strans[tpos] != '\'') {
                    if (strans[tpos] == '\\')
                        ++tpos; // escape next character
                    seg.append(strans[tpos]);
                    ++tpos;
                }
                if (tpos == slen) {
                    kDebug(173) << QString("Unclosed quote in interpolation {%1} in message {%2}.")
                                        .arg(strans.mid(pos, tpos - pos), shortenMessage(strans));
                    return -1;
                }

                // Append to list of segments, resolving placeholders.
                segs.append(substituteSimple(seg, s->scriptPlchar, true));

                ++tpos; // skip closing quote
            }
            else if (strans.mid(tpos, islen) == s->startInterp) { // sub-interpolation
                QString resultLocal;
                bool fallbackLocal;
                tpos = resolveInterpolation(strans, tpos, lang, ctry, final,
                                            resultLocal, fallbackLocal);
                if (tpos < 0) { // unrecoverable problem in sub-interpolation
                    // Error reported in the subcall.
                    return tpos;
                }
                if (fallbackLocal) { // sub-interpolation requested fallback
                    fallback = true;
                }
                segs.append(resultLocal);
            }
            else { // free segment
                QString seg;
                // Find whitespace, quote, opening or closing sequence.
                while (   tpos < slen
                       && !strans[tpos].isSpace() && strans[tpos] != '\''
                       && strans.mid(tpos, islen) != s->startInterp
                       && strans.mid(tpos, ielen) != s->endInterp)
                {
                    if (strans[tpos] == '\\')
                        ++tpos; // escape next character
                    seg.append(strans[tpos]);
                    ++tpos;
                }
                if (tpos == slen) {
                    kDebug(173) << QString("Non-terminated interpolation {%1} in message {%2}.")
                                        .arg(strans.mid(pos, tpos - pos), shortenMessage(strans));
                    return -1;
                }

                // The free segment may look like a value reference;
                // in that case, record which value it would reference,
                // and add verbatim to the segment list.
                // Otherwise, do a normal substitution on the segment.
                vref = segmentToValue(seg);
                if (vref.isValid()) {
                    segs.append(seg);
                }
                else {
                    segs.append(substituteSimple(seg, s->scriptPlchar, true));
                }
            }
        }

        // Append this argument to rest of the arguments.
        // If the there was a single text segment and it was a proper value
        // reference, add it instead of the joined segments.
        // Otherwise, add the joined segments.
        if (segs.size() == 1 && vref.isValid()) {
            iargs.append(vref);
        }
        else {
            iargs.append(segs.join(""));
        }
    }
    tpos += ielen; // skip to first character after closing sequence

    // NOTE: Why not substitute placeholders (via substituteSimple) in one
    // global pass, then handle interpolations in second pass? Because then
    // there is the danger of substituted text or sub-interpolations producing
    // quotes and escapes themselves, which would mess up the parsing.

    // Evaluate interpolation.
    QString msgctxt = QString::fromUtf8(ctxt);
    QString msgid = QString::fromUtf8(msg);
    QString scriptError;
    bool fallbackLocal;
    result = s->ktrs->eval(iargs, lang, ctry,
                           msgctxt, dynctxt, msgid,
                           args, vals, final, s->scriptModulesToLoad,
                           scriptError, fallbackLocal);
    // s->scriptModulesToLoad will be cleared during the call.

    if (fallbackLocal) { // evaluation requested fallback
        fallback = true;
    }
    if (!scriptError.isEmpty()) { // problem with evaluation
        fallback = true; // also signal fallback
        if (!scriptError.isEmpty()) {
            kDebug(173) << QString("Interpolation {%1} in {%2} failed: %3")
                                  .arg(strans.mid(pos, tpos - pos), shortenMessage(strans), scriptError);
        }
    }

    return tpos;
}

QVariant KLocalizedStringPrivate::segmentToValue (const QString &seg) const
{
    const KLocalizedStringPrivateStatics *s = staticsKLSP;
    QMutexLocker lock(kLocaleMutex());

    // Return invalid variant if segment is either not a proper
    // value reference, or the reference is out of bounds.

    // Value reference must start with a special character.
    if (seg.left(1) != s->scriptVachar) {
        return QVariant();
    }

    // Reference number must start with 1-9.
    // (If numstr is empty, toInt() will return 0.)
    QString numstr = seg.mid(1);
    if (numstr.left(1).toInt() < 1) {
        return QVariant();
    }

    // Number must be valid and in bounds.
    bool ok;
    int index = numstr.toInt(&ok) - 1;
    if (!ok || index >= vals.size()) {
        return QVariant();
    }

    // Passed all hoops.
    return vals.at(index);
}

QString KLocalizedStringPrivate::postTranscript (const QString &pcall,
                                                 const QString &lang,
                                                 const QString &ctry,
                                                 const QString &final) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP;
    QMutexLocker lock(kLocaleMutex());

    if (s->ktrs == NULL)
        // Scripting engine not available.
        // (Though this cannot happen, we wouldn't be here then.)
        return QString();

    // Resolve the post call.
    QList<QVariant> iargs;
    iargs.append(pcall);
    QString msgctxt = QString::fromUtf8(ctxt);
    QString msgid = QString::fromUtf8(msg);
    QString scriptError;
    bool fallback;
    QString dummy = s->ktrs->eval(iargs, lang, ctry,
                                  msgctxt, dynctxt, msgid,
                                  args, vals, final, s->scriptModulesToLoad,
                                  scriptError, fallback);
    // s->scriptModulesToLoad will be cleared during the call.

    // If the evaluation went wrong.
    if (!scriptError.isEmpty())
    {
        kDebug(173) << QString("Post call {%1} for message {%2} failed: %3")
                              .arg(pcall, shortenMessage(msgid), scriptError);
        return QString();
    }

    return final;
}

static QString wrapNum (const QString &tag, const QString &numstr,
                        int fieldWidth, const QChar &fillChar)
{
    QString optag;
    if (fieldWidth != 0) {
        QString fillString = KuitSemantics::escape(fillChar);
        optag = QString("<%1 width='%2' fill='%3'>")
                       .arg(tag, QString::number(fieldWidth), fillString);
    } else {
        optag = QString("<%1>").arg(tag);
    }
    QString cltag = QString("</%1>").arg(tag);
    return optag + numstr + cltag;
}

KLocalizedString KLocalizedString::subs (int a, int fieldWidth, int base,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    if (!kls.d->plural.isEmpty() && !kls.d->numberSet) {
        kls.d->number = static_cast<pluraln>(abs(a));
        kls.d->numberSet = true;
        kls.d->numberOrd = d->args.size();
    }
    kls.d->args.append(wrapNum(KUIT_NUMINTG, QString::number(a, base),
                               fieldWidth, fillChar));
    kls.d->vals.append(static_cast<intn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs (uint a, int fieldWidth, int base,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    if (!kls.d->plural.isEmpty() && !kls.d->numberSet) {
        kls.d->number = static_cast<pluraln>(a);
        kls.d->numberSet = true;
        kls.d->numberOrd = d->args.size();
    }
    kls.d->args.append(wrapNum(KUIT_NUMINTG, QString::number(a, base),
                               fieldWidth, fillChar));
    kls.d->vals.append(static_cast<uintn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs (long a, int fieldWidth, int base,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    if (!kls.d->plural.isEmpty() && !kls.d->numberSet) {
        kls.d->number = static_cast<pluraln>(abs(a));
        kls.d->numberSet = true;
        kls.d->numberOrd = d->args.size();
    }
    kls.d->args.append(wrapNum(KUIT_NUMINTG, QString::number(a, base),
                               fieldWidth, fillChar));
    kls.d->vals.append(static_cast<intn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs (ulong a, int fieldWidth, int base,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    if (!kls.d->plural.isEmpty() && !kls.d->numberSet) {
        kls.d->number = static_cast<pluraln>(a);
        kls.d->numberSet = true;
        kls.d->numberOrd = d->args.size();
    }
    kls.d->args.append(wrapNum(KUIT_NUMINTG, QString::number(a, base),
                               fieldWidth, fillChar));
    kls.d->vals.append(static_cast<uintn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs (qlonglong a, int fieldWidth, int base,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    if (!kls.d->plural.isEmpty() && !kls.d->numberSet) {
        kls.d->number = static_cast<pluraln>(qAbs(a));
        kls.d->numberSet = true;
        kls.d->numberOrd = d->args.size();
    }
    kls.d->args.append(wrapNum(KUIT_NUMINTG, QString::number(a, base),
                               fieldWidth, fillChar));
    kls.d->vals.append(static_cast<intn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs (qulonglong a, int fieldWidth, int base,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    if (!kls.d->plural.isEmpty() && !kls.d->numberSet) {
        kls.d->number = static_cast<pluraln>(a);
        kls.d->numberSet = true;
        kls.d->numberOrd = d->args.size();
    }
    kls.d->args.append(wrapNum(KUIT_NUMINTG, QString::number(a, base),
                               fieldWidth, fillChar));
    kls.d->vals.append(static_cast<uintn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs (double a, int fieldWidth,
                                         char format, int precision,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->args.append(wrapNum(KUIT_NUMREAL,
                               QString::number(a, format, precision),
                               fieldWidth, fillChar));
    kls.d->vals.append(static_cast<realn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs (QChar a, int fieldWidth,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth, fillChar));
    kls.d->vals.append(QString(a));
    return kls;
}

KLocalizedString KLocalizedString::subs (const QString &a, int fieldWidth,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    // if (!KuitSemantics::mightBeRichText(a)) { ...
    // Do not try to auto-escape non-rich-text alike arguments;
    // breaks compatibility with 4.0. Perhaps for KDE 5?
    // Perhaps bad idea alltogether (too much surprise)?
    kls.d->args.append(QString("%1").arg(a, fieldWidth, fillChar));
    kls.d->vals.append(a);
    return kls;
}

KLocalizedString KLocalizedString::inContext (const QString &key,
                                              const QString &text) const
{
    KLocalizedString kls(*this);
    kls.d->dynctxt[key] = text;
    return kls;
}

KLocalizedString ki18n (const char* msg)
{
    return KLocalizedString(NULL, msg, NULL);
}

KLocalizedString ki18nc (const char* ctxt, const char *msg)
{
    return KLocalizedString(ctxt, msg, NULL);
}

KLocalizedString ki18np (const char* singular, const char* plural)
{
    return KLocalizedString(NULL, singular, plural);
}

KLocalizedString ki18ncp (const char* ctxt,
                          const char* singular, const char* plural)
{
    return KLocalizedString(ctxt, singular, plural);
}

extern "C"
{
    typedef KTranscript *(*InitFunc)();
}

void KLocalizedStringPrivate::loadTranscript ()
{
    KLocalizedStringPrivateStatics *s = staticsKLSP;
    QMutexLocker lock(kLocaleMutex());

    s->loadTranscriptCalled = true;
    s->ktrs = NULL; // null indicates that Transcript is not available

    KLibrary lib(QLatin1String("ktranscript"));
    if (!lib.load()) {
        kDebug(173) << "Cannot load transcript plugin:" << lib.errorString();
        return;
    }

    InitFunc initf = (InitFunc) lib.resolveFunction("load_transcript");
    if (!initf) {
        lib.unload();
        kDebug(173) << "Cannot find function load_transcript in transcript plugin.";
        return;
    }

    s->ktrs = initf();
}

void KLocalizedString::notifyCatalogsUpdated (const QStringList &languages,
                                              const QList<KCatalogName> &catalogs)
{
    KLocalizedStringPrivate::notifyCatalogsUpdated(languages, catalogs);
}

void KLocalizedStringPrivate::notifyCatalogsUpdated (const QStringList &languages,
                                                     const QList<KCatalogName> &catalogs)
{
    if (staticsKLSP.isDestroyed()) {
        return;
    }
    KLocalizedStringPrivateStatics *s = staticsKLSP;
    // Very important: do not the mutex here.
    //QMutexLocker lock(kLocaleMutex());

    // Find script modules for all included language/catalogs that have them,
    // and remember their paths.
    // A more specific module may reference the calls from a less specific,
    // and the catalog list is ordered from more to less specific. Therefore,
    // work on reversed list of catalogs.
    foreach (const QString &lang, languages) {
        for (int i = catalogs.size() - 1; i >= 0; --i) {
            const KCatalogName &cat(catalogs[i]);

            // Assemble module's relative path.
            QString modrpath =   lang + '/' + s->scriptDir + '/'
                            + cat.name + '/' + cat.name + ".js";

            // Try to find this module.
            QString modapath = KStandardDirs::locate("locale", modrpath);

            // If the module exists and hasn't been already included.
            if (   !modapath.isEmpty()
                && !s->scriptModules[lang].contains(cat.name))
            {
                // Indicate that the module has been considered.
                s->scriptModules[lang].append(cat.name);

                // Store the absolute path and language of the module,
                // to load on next script evaluation.
                QStringList mod;
                mod.append(modapath);
                mod.append(lang);
                s->scriptModulesToLoad.append(mod);
            }
        }
    }

    // Create visual formatters for each new language.
    foreach (const QString &lang, languages) {
        if (!s->formatters.contains(lang)) {
            s->formatters.insert(lang, new KuitSemantics(lang));
        }
    }
}
