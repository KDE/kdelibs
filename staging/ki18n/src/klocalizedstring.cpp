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

#include <common_helpers_p.h>
#include <kcatalog_p.h>
#include <ktranscript_p.h>
#include <kuitsemantics_p.h>

#include <QDebug>
#include <QMutexLocker>
#include <QStringList>
#include <QByteArray>
#include <QChar>
#include <QHash>
#include <QList>
#include <QVector>
#include <QFile>
#include <QFileInfo>
#include <QLibrary>
#include <QDir>
#include <QCoreApplication>
#include <qstandardpaths.h>

// Truncate string, for output of long messages.
static QString shortenMessage (const QString &str)
{
    const int maxlen = 20;
    if (str.length() <= maxlen)
        return str;
    else
        return str.left(maxlen).append(QLatin1String("..."));
}

// FIXME: Temporary, until locales ready.
static void splitLocale (const QString &aLocale,
                         QString &language, QString &country,
                         QString &modifier, QString &charset)
{
    QString locale = aLocale;

    language.clear();
    country.clear();
    modifier.clear();
    charset.clear();

    // In case there are several concatenated locale specifications,
    // truncate all but first.
    int f = locale.indexOf(QLatin1Char(':'));
    if (f >= 0) {
        locale.truncate(f);
    }

    f = locale.indexOf(QLatin1Char('.'));
    if (f >= 0) {
        charset = locale.mid(f + 1);
        locale.truncate(f);
    }

    f = locale.indexOf(QLatin1Char('@'));
    if (f >= 0) {
        modifier = locale.mid(f + 1);
        locale.truncate(f);
    }

    f = locale.indexOf(QLatin1Char('_'));
    if (f >= 0) {
        country = locale.mid(f + 1);
        locale.truncate(f);
    }

    language = locale;
}

// FIXME: Temporary, until locales ready.
static void appendLanguagesFromVariable (QStringList &languages,
                                         const char *envar, bool isList = false)
{
    QByteArray qenvar(qgetenv(envar));
    if (!qenvar.isEmpty()) {
        QString value = QFile::decodeName(qenvar);
        if (isList) {
            languages += value.split(QLatin1Char(':'));
        } else {
            // Process the value to create possible combinations.
            QString language, country, modifier, charset;
            splitLocale(value, language, country, modifier, charset);

            if (!country.isEmpty() && !modifier.isEmpty()) {
                languages +=   language + QLatin1Char('_')
                             + country + QLatin1Char('@')
                             + modifier;
            }
            // NOTE: Priority is unclear in case both the country and
            // the modifier are present. Should really language@modifier be of
            // higher priority than language_country?
            // In at least one case (Serbian language), it is better this way.
            if (!modifier.isEmpty()) {
                languages += language + QLatin1Char('@') + modifier;
            }
            if (!country.isEmpty()) {
                languages += language + QLatin1Char('_') + country;
            }
            languages += language;
        }
    }
}

// Extract the first country code from a list of language_COUNTRY strings.
// Country code is converted to all lower case letters.
static QString extractCountry (const QStringList &languages)
{
    QString country;
    Q_FOREACH (const QString &language, languages) {
        int pos1 = language.indexOf(QLatin1Char('_'));
        if (pos1 >= 0) {
            ++pos1;
            int pos2 = pos1;
            while (pos2 < language.length() && language[pos2].isLetter()) {
                ++pos2;
            }
            country = language.mid(pos1, pos2 - pos1);
            break;
        }
    }
    country = country.toLower();
    return country;
}

typedef qulonglong pluraln;
typedef qlonglong intn;
typedef qulonglong uintn;
typedef double realn;

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

    static void translateRaw (const QStringList &catalogNames,
                              const QStringList &languages,
                              const QByteArray &msgctxt,
                              const QByteArray &msgid,
                              const QByteArray &msgid_plural,
                              qulonglong n,
                              QString &language,
                              QString &translation);

    QString toString (const QStringList &catalogNames,
                      const QStringList &languages) const;
    QString selectForEnglish () const;
    QString substituteSimple (const QString &trans,
                              const QChar &plchar = QLatin1Char('%'),
                              bool partial = false) const;
    QString postFormat (const QString &text,
                        const QString &lang,
                        const QString &ctxt) const;
    QString substituteTranscript (const QString &trans,
                                  const QString &lang,
                                  const QString &ctry,
                                  const QString &ftrans,
                                  bool &fallback) const;
    int resolveInterpolation (const QString &trans, int pos,
                              const QString &lang,
                              const QString &ctry,
                              const QString &ftrans,
                              QString &result,
                              bool &fallback) const;
    QVariant segmentToValue (const QString &arg) const;
    QString postTranscript (const QString &pcall,
                            const QString &lang,
                            const QString &ctry,
                            const QString &ftrans) const;

    static const KCatalog &getCatalog (const QString &catalogName,
                                      const QString &language);
    static void locateScriptingModule (const QString &catalogName,
                                       const QString &language);

    static void loadTranscript ();
};

typedef QHash<QString, KCatalog*> KCatalogPtrHash;

class KLocalizedStringPrivateStatics
{
    public:

    QHash<QString, KCatalogPtrHash> catalogs;
    QStringList catalogNames;
    QStringList languages;

    QString ourCatalogName;
    QString appCatalogName;
    QString codeLanguage;
    QStringList localeLanguages;

    QList<int> catalogInsertCount;

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

    QMutex klspMutex;

    // FIXME: Temporary, until locales ready.
    void initLocaleLanguages ();

    KLocalizedStringPrivateStatics ()
        : catalogs()
        , catalogNames()
        , languages()

        , ourCatalogName(QLatin1String("kdelibs4")) // FIXME: New name in KF 5.
        , appCatalogName()
        , codeLanguage(QLatin1String("en_US"))
        , localeLanguages()

        , catalogInsertCount()

        , theFence(QLatin1String("|/|"))
        , startInterp(QLatin1String("$["))
        , endInterp(QLatin1String("]"))
        , scriptPlchar(QLatin1Char('%'))
        , scriptVachar(QLatin1Char('^'))

        , scriptDir(QLatin1String("LC_SCRIPTS"))
        , scriptModules()
        , scriptModulesToLoad()

        , loadTranscriptCalled(false)
        , ktrs(NULL)

        , formatters()

        , klspMutex(QMutex::Recursive)
    {
        initLocaleLanguages();
        languages = localeLanguages;

        catalogNames.prepend(ourCatalogName);
        catalogInsertCount.prepend(1);
    }

    ~KLocalizedStringPrivateStatics ()
    {
        Q_FOREACH (const KCatalogPtrHash &langCatalogs, catalogs) {
            qDeleteAll(langCatalogs);
        }
        // ktrs is handled by QLibrary.
        //delete ktrs;
        qDeleteAll(formatters);
    }
};

Q_GLOBAL_STATIC(KLocalizedStringPrivateStatics, staticsKLSP)

// FIXME: Temporary, until locales ready.
void KLocalizedStringPrivateStatics::initLocaleLanguages ()
{
    QMutexLocker lock(&klspMutex);

    // Collect translation languages by decreasing priority.

    // KDE_LANG contains list of language codes, not locale string.
    appendLanguagesFromVariable(localeLanguages, "KDE_LANG", true);

    // Collect languages by same order of priority as for gettext(3).
    // LANGUAGE contains list of language codes, not locale string.
    appendLanguagesFromVariable(localeLanguages, "LANGUAGE", true);
    appendLanguagesFromVariable(localeLanguages, "LC_ALL");
    appendLanguagesFromVariable(localeLanguages, "LC_MESSAGES");
    appendLanguagesFromVariable(localeLanguages, "LANG");
}

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

void KLocalizedStringPrivate::translateRaw (const QStringList &catalogNames,
                                            const QStringList &languages,
                                            const QByteArray &msgctxt,
                                            const QByteArray &msgid,
                                            const QByteArray &msgid_plural,
                                            qulonglong n,
                                            QString &language,
                                            QString &translation)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    // Empty msgid would result in returning the catalog header,
    // which is never intended, so warn and return empty translation.
    if (msgid.isNull() || msgid.isEmpty()) {
        qWarning() << QString::fromLatin1(
            "KLocalizedString: "
            "Trying to look up translation of \"\", fix the code.");
        language.clear();
        translation.clear();
        return;
    }
    // Gettext semantics allows empty context, but it is pointless, so warn.
    if (!msgctxt.isNull() && msgctxt.isEmpty()) {
        qWarning() << QString::fromLatin1(
            "KLocalizedString: "
            "Using \"\" as context, fix the code.");
    }
    // Gettext semantics allows empty plural, but it is pointless, so warn.
    if (!msgid_plural.isNull() && msgid_plural.isEmpty()) {
        qWarning() << QString::fromLatin1(
            "KLocalizedString: "
            "Using \"\" as plural text, fix the code.");
    }

    // Set translation to text in code language, in case no translation found.
    translation =   msgid_plural.isNull() || n == 1
                  ? QString::fromUtf8(msgid)
                  : QString::fromUtf8(msgid_plural);
    language = s->codeLanguage;

    // Languages are ordered from highest to lowest priority.
    // Priority of catalogs is undefined.
    Q_FOREACH (const QString &testLanguage, languages) {
        // If code language reached, no catalog lookup is needed.
        if (testLanguage == s->codeLanguage) {
            break;
        }
        // Skip this language if there is no application catalog for it.
        if (!KLocalizedString::isApplicationTranslatedInto(testLanguage)) {
            continue;
        }
        Q_FOREACH (const QString &catalogName, catalogNames) {
            const KCatalog &catalog = getCatalog(catalogName, testLanguage);
            QString text;
            if (!msgctxt.isNull() && !msgid_plural.isNull()) {
                text = catalog.translate(msgctxt, msgid, msgid_plural, n);
            } else if (!msgid_plural.isNull()) {
                text = catalog.translate(msgid, msgid_plural, n);
            } else if (!msgctxt.isNull()) {
                text = catalog.translate(msgctxt, msgid);
            } else {
                text = catalog.translate(msgid);
            }
            if (!text.isEmpty()) {
                // Translation found.
                language = testLanguage;
                translation = text;
                break;
            }
        }
    }
}

QString KLocalizedString::toString () const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();
    return d->toString(s->catalogNames, s->languages);
}

QString KLocalizedString::toString (const QString &catalogName) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();
    QStringList catalogNames;
    catalogNames.append(catalogName);
    return d->toString(catalogNames, s->languages);
}

QString KLocalizedString::toString (const QStringList &languages) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();
    return d->toString(s->catalogNames, languages);
}

QString KLocalizedString::toString (const QString &catalogName,
                                    const QStringList &languages) const
{
    QStringList catalogNames;
    catalogNames.append(catalogName);
    return d->toString(catalogNames, languages);
}

QString KLocalizedStringPrivate::toString (const QStringList &catalogNames,
                                           const QStringList &languages) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    // Assure the message has been supplied.
    if (msg.isEmpty())
    {
        qWarning() << QString::fromLatin1(
            "Trying to convert empty KLocalizedString to QString.");
        #ifndef NDEBUG
        return QString::fromLatin1("(I18N_EMPTY_MESSAGE)");
        #else
        return QString();
        #endif
    }

    // Check whether plural argument has been supplied, if message has plural.
    if (!plural.isEmpty() && !numberSet)
        qWarning() << QString::fromLatin1(
            "Plural argument to message {%1} not supplied before conversion.")
            .arg(shortenMessage(QString::fromUtf8(msg)));

    // Get raw translation.
    QString rawtrans, lang;
    translateRaw(catalogNames, languages,
                 ctxt, msg, plural, number,
                 lang, rawtrans);

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
        // FIXME: And also if Transcript not disabled: where to configure this?
        if (!s->loadTranscriptCalled && !strans.isEmpty())
        {
            loadTranscript();

            // Definitions from this library's scripting module
            // must be available to all other modules.
            // So force creation of this library's catalog here,
            // to make sure the scripting module is loaded.
            getCatalog(s->ourCatalogName, lang);
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
        qWarning() << QString::fromLatin1(
            "Scripted message {%1} without ordinary translation, discarded.")
            .arg(shortenMessage(trans)) ;
        trans = selectForEnglish();
    }

    // Substitute placeholders in ordinary translation.
    QString ftrans = substituteSimple(trans);
    // Post-format ordinary translation.
    ftrans = postFormat(ftrans, lang, QString::fromLatin1(ctxt));

    QString ctry = extractCountry(languages);

    // If there is also a scripted translation.
    if (!strans.isEmpty()) {
        // Evaluate scripted translation.
        bool fallback;
        QString sfinal = substituteTranscript(strans, lang, ctry, ftrans, fallback);

        // If any translation produced and no fallback requested.
        if (!sfinal.isEmpty() && !fallback) {
            ftrans = postFormat(sfinal, lang, QString::fromLatin1(ctxt));
        }
    }

    // Execute any scripted post calls; they cannot modify the final result,
    // but are used to set states.
    if (s->ktrs != NULL)
    {
        QStringList pcalls = s->ktrs->postCalls(lang);
        Q_FOREACH(const QString &pcall, pcalls) {
            postTranscript(pcall, lang, ctry, ftrans);
        }
    }

    return ftrans;
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
    QString ftrans;
    for (int i = 0; i < plords.size(); i++)
    {
        ftrans.append(tsegs.at(i));
        if (plords.at(i) >= args.size())
        // too little arguments
        {
            // put back the placeholder
            ftrans.append(QLatin1Char('%') + QString::number(plords.at(i) + 1));
            #ifndef NDEBUG
            if (!partial)
                // spoof the message
                ftrans.append(QLatin1String("(I18N_ARGUMENT_MISSING)"));
            #endif
        }
        else
        // just fine
            ftrans.append(args.at(plords.at(i)));
    }
    ftrans.append(tsegs.last());

    #ifndef NDEBUG
    if (!partial)
    {
        // Check that there are no gaps in numbering sequence of placeholders.
        bool gaps = false;
        for (int i = 0; i < ords.size(); i++)
            if (!ords.at(i))
            {
                gaps = true;
                qWarning() << QString::fromLatin1(
                    "Placeholder %%1 skipped in message {%2}.")
                    .arg(QString::number(i + 1), shortenMessage(trans));
            }
        // If no gaps, check for mismatch between the number of
        // unique placeholders and actually supplied arguments.
        if (!gaps && ords.size() != args.size())
            qWarning() << QString::fromLatin1(
                "%1 instead of %2 arguments to message {%3} "
                "supplied before conversion.")
                .arg(args.size()).arg(ords.size()).arg(shortenMessage(trans));

        // Some spoofs.
        if (gaps)
            ftrans.append(QLatin1String("(I18N_GAPS_IN_PLACEHOLDER_SEQUENCE)"));
        if (ords.size() < args.size())
            ftrans.append(QLatin1String("(I18N_EXCESS_ARGUMENTS_SUPPLIED)"));
        if (!plural.isEmpty() && !numberSet)
            ftrans.append(QLatin1String("(I18N_PLURAL_ARGUMENT_MISSING)"));
    }
    #endif

    return ftrans;
}

QString KLocalizedStringPrivate::postFormat (const QString &text,
                                             const QString &lang,
                                             const QString &ctxt) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QString ftrans = text;

    QHash<QString, KuitSemantics*>::iterator formatter = s->formatters.find(lang);
    if (formatter == s->formatters.end()) {
        formatter = s->formatters.insert(lang, new KuitSemantics(lang));
    }

    ftrans = (*formatter)->format(ftrans, ctxt);

    return ftrans;
}

QString KLocalizedStringPrivate::substituteTranscript (const QString &strans,
                                                       const QString &lang,
                                                       const QString &ctry,
                                                       const QString &ftrans,
                                                       bool &fallback) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

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
        tpos = resolveInterpolation(strans, tpos, lang, ctry, ftrans,
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
                                                   const QString &ftrans,
                                                   QString &result,
                                                   bool &fallback) const
{
    // pos is the position of opening character sequence.
    // Returns the position of first character after closing sequence,
    // or -1 in case of parsing error.
    // result is set to result of Transcript evaluation.
    // fallback is set to true if Transcript evaluation requested so.

    KLocalizedStringPrivateStatics *s = staticsKLSP();

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
            qWarning() << QString::fromLatin1(
                "Unclosed interpolation {%1} in message {%2}.")
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
            if (strans[tpos] == QLatin1Char('\'')) { // quoted segment
                QString seg;
                ++tpos; // skip opening quote
                // Find closing quote.
                while (tpos < slen && strans[tpos] != QLatin1Char('\'')) {
                    if (strans[tpos] == QLatin1Char('\\'))
                        ++tpos; // escape next character
                    seg.append(strans[tpos]);
                    ++tpos;
                }
                if (tpos == slen) {
                    qWarning() << QString::fromLatin1(
                        "Unclosed quote in interpolation {%1} in message {%2}.")
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
                tpos = resolveInterpolation(strans, tpos, lang, ctry, ftrans,
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
                       && !strans[tpos].isSpace() && strans[tpos] != QLatin1Char('\'')
                       && strans.mid(tpos, islen) != s->startInterp
                       && strans.mid(tpos, ielen) != s->endInterp)
                {
                    if (strans[tpos] == QLatin1Char('\\'))
                        ++tpos; // escape next character
                    seg.append(strans[tpos]);
                    ++tpos;
                }
                if (tpos == slen) {
                    qWarning() << QString::fromLatin1(
                        "Non-terminated interpolation {%1} in message {%2}.")
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
            iargs.append(segs.join(QString()));
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
                           args, vals, ftrans, s->scriptModulesToLoad,
                           scriptError, fallbackLocal);
    // s->scriptModulesToLoad will be cleared during the call.

    if (fallbackLocal) { // evaluation requested fallback
        fallback = true;
    }
    if (!scriptError.isEmpty()) { // problem with evaluation
        fallback = true; // also signal fallback
        if (!scriptError.isEmpty()) {
            qWarning() << QString::fromLatin1(
                "Interpolation {%1} in {%2} failed: %3")
                .arg(strans.mid(pos, tpos - pos), shortenMessage(strans),
                     scriptError);
        }
    }

    return tpos;
}

QVariant KLocalizedStringPrivate::segmentToValue (const QString &seg) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

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
                                                 const QString &ftrans) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

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
                                  args, vals, ftrans, s->scriptModulesToLoad,
                                  scriptError, fallback);
    // s->scriptModulesToLoad will be cleared during the call.

    // If the evaluation went wrong.
    if (!scriptError.isEmpty())
    {
        qWarning() << QString::fromLatin1(
            "Post call {%1} for message {%2} failed: %3")
            .arg(pcall, shortenMessage(msgid), scriptError);
        return QString();
    }

    return ftrans;
}

static QString wrapNum (const QString &tag, const QString &numstr,
                        int fieldWidth, const QChar &fillChar)
{
    QString optag;
    if (fieldWidth != 0) {
        QString fillString = KuitSemantics::escape(fillChar);
        optag = QString::fromLatin1("<%1 width='%2' fill='%3'>")
                       .arg(tag, QString::number(fieldWidth), fillString);
    } else {
        optag = QString::fromLatin1("<%1>").arg(tag);
    }
    QString cltag = QString::fromLatin1("</%1>").arg(tag);
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
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMINTG), QString::number(a, base),
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
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMINTG), QString::number(a, base),
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
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMINTG), QString::number(a, base),
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
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMINTG), QString::number(a, base),
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
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMINTG), QString::number(a, base),
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
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMINTG), QString::number(a, base),
                               fieldWidth, fillChar));
    kls.d->vals.append(static_cast<uintn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs (double a, int fieldWidth,
                                         char format, int precision,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMREAL),
                               QString::number(a, format, precision),
                               fieldWidth, fillChar));
    kls.d->vals.append(static_cast<realn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs (QChar a, int fieldWidth,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->args.append(QString::fromLatin1("%1").arg(a, fieldWidth, fillChar));
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
    kls.d->args.append(QString::fromLatin1("%1").arg(a, fieldWidth, fillChar));
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

void KLocalizedString::insertCatalog (const QString &catalogName)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    int pos = s->catalogNames.indexOf(catalogName);
    if (pos < 0) {
        // Catalog priority is undefined, but to minimize damage
        // due to message conflicts, put later inserted catalogs at front.
        s->catalogNames.prepend(catalogName);
        s->catalogInsertCount.prepend(1);
        // Catalog is created when first queried for translation.
    } else {
        ++s->catalogInsertCount[pos];
    }
}

void KLocalizedString::removeCatalog (const QString &catalogName)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    int pos = s->catalogNames.indexOf(catalogName);
    if (pos >= 0 && --s->catalogInsertCount[pos] == 0) {
        s->catalogNames.removeAt(pos);
        s->catalogInsertCount.removeAt(pos);
        s->catalogs.remove(catalogName);
    }
}

void KLocalizedString::setApplicationCatalog (const QString &catalogName)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    s->appCatalogName = catalogName;
    insertCatalog(catalogName);
}

QString KLocalizedString::applicationCatalog()
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);
    return s->appCatalogName;
}

void KLocalizedString::setLanguages (const QStringList &languages)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    s->languages = languages;
}

void KLocalizedString::clearLanguages ()
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    s->languages = s->localeLanguages;
}

bool KLocalizedString::isApplicationTranslatedInto (const QString &language)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    return language == s->codeLanguage || !KCatalog::catalogLocaleDir(s->appCatalogName, language).isEmpty();
}

const KCatalog &KLocalizedStringPrivate::getCatalog (const QString &catalogName,
                                                     const QString &language)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    QHash<QString, KCatalogPtrHash>::iterator langCatalogs = s->catalogs.find(catalogName);
    if (langCatalogs == s->catalogs.end()) {
        langCatalogs = s->catalogs.insert(catalogName, KCatalogPtrHash());
    }
    KCatalogPtrHash::iterator catalog = langCatalogs->find(language);
    if (catalog == langCatalogs->end()) {
        catalog = langCatalogs->insert(language, new KCatalog(catalogName, language));
        locateScriptingModule(catalogName, language);
    }
    return **catalog;
}

void KLocalizedStringPrivate::locateScriptingModule (const QString &catalogName,
                                                     const QString &language)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    // Assemble module's relative path.
    QString modrpath =   language + QLatin1Char('/')
                        + s->scriptDir + QLatin1Char('/')
                        + catalogName + QLatin1Char('/')
                        + catalogName + QLatin1String(".js");

    // Try to find this module.
    QString modapath = QStandardPaths::locate(
        QStandardPaths::GenericDataLocation,
        QLatin1String("locale") + QLatin1Char('/') + modrpath);

    // If the module exists and hasn't been already included.
    if (   !modapath.isEmpty()
        && !s->scriptModules[language].contains(catalogName))
    {
        // Indicate that the module has been considered.
        s->scriptModules[language].append(catalogName);

        // Store the absolute path and language of the module,
        // to load on next script evaluation.
        QStringList module;
        module.append(modapath);
        module.append(language);
        s->scriptModulesToLoad.append(module);
    }
}

extern "C"
{
    typedef KTranscript *(*InitFunc)();
}

void KLocalizedStringPrivate::loadTranscript ()
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    s->loadTranscriptCalled = true;
    s->ktrs = NULL; // null indicates that Transcript is not available

    #if 0
    // FIXME: Automatic plugin path resolution does not work at the moment,
    // so search manually through library paths.
    QString pluginPathNoExt = QLatin1String("kf5/ktranscript");
    #else
    QString pluginPathNoExt;
    QStringList nameFilters;
    QString pluginSubdir = QLatin1String("kf5");
    QString pluginName = QLatin1String("ktranscript");
    nameFilters.append(pluginName + QLatin1String(".*"));
    Q_FOREACH (const QString &dirPath, QCoreApplication::libraryPaths()) {
        QString dirPathKf = dirPath + QLatin1Char('/') + pluginSubdir;
        if (!QDir(dirPathKf).entryList(nameFilters).isEmpty()) {
            pluginPathNoExt = dirPathKf + QLatin1Char('/') + pluginName;
            break;
        }
    }
    if (pluginPathNoExt.isEmpty()) {
        qWarning() << QString::fromLatin1("Cannot find Transcript plugin.");
        return;
    }
    #endif

    QLibrary lib(pluginPathNoExt);
    if (!lib.load()) {
        qWarning() << QString::fromLatin1("Cannot load Transcript plugin:")
                   << lib.errorString();
        return;
    }

    InitFunc initf = (InitFunc) lib.resolve("load_transcript");
    if (!initf) {
        lib.unload();
        qWarning() << QString::fromLatin1(
            "Cannot find function load_transcript in Transcript plugin.");
        return;
    }

    s->ktrs = initf();
}

QString KLocalizedString::translateQt (const char *context,
                                       const char *sourceText,
                                       const char *comment,
                                       int n
                                       )
{
    // NOTE: Qt message semantics.
    //
    // Qt's context is normally the name of the class of the method which makes
    // the tr(sourceText) call. However, it can also be manually supplied via
    // translate(context, sourceText) call.
    //
    // Qt's sourceText is the actual message displayed to the user.
    //
    // Qt's comment is an optional argument of tr() and translate(), like
    // tr(sourceText, comment) and translate(context, sourceText, comment).
    //
    // We handle this in the following way:
    //
    // If the comment is given, then it is considered gettext's msgctxt, so a
    // context call is made.
    //
    // If the comment is not given, but context is given, then we treat it as
    // msgctxt only if it was manually supplied (the one in translate()) -- but
    // we don't know this, so we first try a context call, and if translation
    // is not found, we fallback to ordinary call.
    //
    // If neither comment nor context are given, it's just an ordinary call
    // on sourceText.

    KLocalizedStringPrivateStatics *s = staticsKLSP();

    QMutexLocker lock(&s->klspMutex);

    if (!sourceText || !sourceText[0]) {
        qWarning() << QString::fromLatin1(
            "KLocalizedString::translateQt: "
            "Trying to look up translation of \"\", fix the code.");
        return QString();
    }

    // NOTE: Condition (language != s->codeLanguage) means that translation
    // was found, otherwise the original text was returned as translation.
    QString translation;
    QString language;
    if (comment && comment[0]) {
        // Comment given, go for context call.
        KLocalizedStringPrivate::translateRaw(s->catalogNames, s->languages,
                                              comment, sourceText, 0, 0,
                                              language, translation);
    } else {
        // Comment not given, go for try-fallback with context.
        if (context && context[0]) {
            KLocalizedStringPrivate::translateRaw(s->catalogNames, s->languages,
                                                  context, sourceText, 0, 0,
                                                  language, translation);
        }
        if (language.isEmpty() || language == s->codeLanguage) {
            KLocalizedStringPrivate::translateRaw(s->catalogNames, s->languages,
                                                  0, sourceText, 0, 0,
                                                  language, translation);
        }
    }
    if (language != s->codeLanguage) {
        return translation;
    }
    // No proper translation found, return empty according to Qt semantics.
    return QString();
}

QString KLocalizedString::localizedFilePath (const QString &filePath)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP();

    // Check if l10n subdirectory is present, stop if not.
    QFileInfo fileInfo(filePath);
    QString locDirPath =   fileInfo.path() + QLatin1Char('/')
                         + QLatin1String("l10n");
    QFileInfo locDirInfo(locDirPath);
    if (!locDirInfo.isDir()) {
        return filePath;
    }

    // Go through possible localized paths by priority of languages,
    // return first that exists.
    QString fileName = fileInfo.fileName();
    Q_FOREACH (const QString &lang, s->languages) {
        QString locFilePath =   locDirPath + QLatin1Char('/')
                              + lang + QLatin1Char('/')
                              + fileName;
        QFileInfo locFileInfo(locFilePath);
        if (locFileInfo.isFile() && locFileInfo.isReadable()) {
            return locFilePath;
        }
    }

    return filePath;
}

QString KLocalizedString::removeAcceleratorMarker(const QString &label)
{
    return ::removeAcceleratorMarker(label);
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

