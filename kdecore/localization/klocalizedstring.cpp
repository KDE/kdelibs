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

#include <kglobal.h>
#include <klocale.h>
#include <kcomponentdata.h>
#include <klibloader.h>
#include <kstandarddirs.h>
#include <ktranscript.h>
#include <kdebug.h>


#include <QStringList>
#include <QByteArray>
#include <QChar>
#include <QMap>
#include <QList>
#include <QVector>

// Truncates string, for output of long messages.
QString shorten (const QString &str)
{
    const int maxlen = 20;
    if (str.length() <= maxlen)
        return str;
    else
        return str.left(maxlen).append("|...");
}

typedef qulonglong pluraln;

class KLocalizedStringPrivateStatics;

class KLocalizedStringPrivate
{
    friend class KLocalizedString;

    QStringList args;
    bool numberSet;
    pluraln number;
    int numberOrd;
    QByteArray ctxt;
    QByteArray msg;
    QByteArray plural;

    QString toString (const KLocale *locale) const;
    QString substituteSimple (const QString &trans,
                              const QChar &plchar = '%',
                              bool partial = false) const;
    QString substituteTranscript (const QString &trans,
                                  const QString &lang,
                                  const QString &final) const;
    int parseInterpolation (const QString &trans, int pos,
                            QStringList &args) const;

    static void notifyCatalogsUpdated (const QStringList &languages,
                                       const QStringList &catalogs);
    static void loadTranscript ();

};

class KLocalizedStringPrivateStatics
{
    public:

    const QString theFence;
    const QString startInterp;
    const QString endInterp;
    const QChar scriptPlchar;

    const QString scriptDir;
    QMap<QString, QStringList> scriptModules;
    QList<QStringList> scriptModulesToLoad;

    bool loadTranscriptCalled;
    KTranscript *ktrs;

    KLocalizedStringPrivateStatics () :
        theFence("|/|"),
        startInterp("$["),
        endInterp("]"),
        scriptPlchar('%'),

        scriptDir("LC_SCRIPTS"),
        scriptModules(),
        scriptModulesToLoad(),

        loadTranscriptCalled(false),
        ktrs(NULL)
    {}
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
    return d->toString(KGlobal::locale());
}

QString KLocalizedString::toString (const KLocale *locale) const
{
    return d->toString(locale);
}

QString KLocalizedStringPrivate::toString (const KLocale *locale) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP;

    // Assure the message has been supplied.
    if (msg.isEmpty())
    {
        #ifndef NDEBUG
        kDebug(173) << QString("Trying to convert empty KLocalizedString to QString.") << endl;
        return QString("(I18N_EMPTY_MESSAGE)");
        #else
        return QString();
        #endif
    }

    #ifndef NDEBUG
    // Check whether plural argument has been supplied, if message has plural.
    if (!plural.isEmpty() && !numberSet)
        kDebug(173) << QString("Plural argument to message {%1} not supplied before conversion.")
                              .arg(shorten(QString::fromUtf8(msg))) << endl;
    #endif

    // Get raw translation.
    QString lang, rawtrans;
    if (locale != NULL)
    {
        if (!ctxt.isEmpty() && !plural.isEmpty())
            locale->translateRaw(ctxt, msg, plural, number, &lang, &rawtrans);
        else if (!plural.isEmpty())
            locale->translateRaw(msg, plural, number, &lang, &rawtrans);
        else if (!ctxt.isEmpty())
            locale->translateRaw(ctxt, msg, &lang, &rawtrans);
        else
            locale->translateRaw(msg, &lang, &rawtrans);
    }
    else
    {
        lang = KLocale::defaultLanguage();
        if (!plural.isEmpty())
        {
            if (number == 1)
                rawtrans = QString::fromUtf8(msg);
            else
                rawtrans = QString::fromUtf8(plural);
        }
        else
            rawtrans = QString::fromUtf8(msg);
    }

    // Set ordinary translation and possibly scripted translation.
    QString trans, strans;
    int cdpos = rawtrans.indexOf(s->theFence);
    if (cdpos >= 0)
    {
        // Script fence has been found, strip the scripted from the
        // ordinary translation.
        trans = rawtrans.left(cdpos);

        // Scripted translation.
        strans = rawtrans.mid(cdpos + s->theFence.length());

        // Try to initialize Transcript if not initialized.
        if (!s->loadTranscriptCalled && locale->useTranscript())
        {
            if (KGlobal::hasMainComponent())
                loadTranscript();
            #ifndef NDEBUG
            else
                kDebug(173) << QString("Scripted message {%1} before transcript engine can be loaded.")
                                    .arg(shorten(trans)) << endl;
            #endif
        }
    }
    else
        trans = rawtrans;

    // Substitute placeholders in ordinary translations.
    QString final = substituteSimple(trans);

    if (strans.isEmpty())
        // No script, ordinary translation is final.
        return final;
    else
    {
        // Evaluate scripted translation.
        QString sfinal = substituteTranscript(strans, lang, final);

        if (!sfinal.isEmpty()) // scripted translation evaluated successfully
            return sfinal;
        else // scripted translation failed, fallback to ordinary
            return final;
    }
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
        else
            tpos++; // might be escape for %

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
                                      .arg(QString::number(i + 1), shorten(trans)) << endl;
            }
        // If no gaps, check for mismatch between number of unique placeholders and
        // actually supplied arguments.
        if (!gaps && ords.size() != args.size())
            kDebug(173) << QString("%1 instead of %2 arguments to message {%3} supplied before conversion.")
                                  .arg(args.size()).arg(ords.size()).arg(shorten(trans)) << endl;

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

QString KLocalizedStringPrivate::substituteTranscript (const QString &strans,
                                                       const QString &lang,
                                                       const QString &final) const
{
    KLocalizedStringPrivateStatics *s = staticsKLSP;

    if (s->ktrs == NULL)
        // Scripting engine not available.
        return QString();

    // Iterate by interpolations.
    QString sfinal;
    int ppos = 0;
    int tpos = strans.indexOf(s->startInterp);
    while (tpos >= 0)
    {
        // Resolve substitutions in preceding text.
        QString ptext = substituteSimple(strans.mid(ppos, tpos - ppos),
                                         s->scriptPlchar, true);
        sfinal.append(ptext);

        // Parse interpolation.
        QStringList argv;
        int mpos = tpos;
        tpos = parseInterpolation(strans, tpos, argv);
        if (tpos < 0)
            // Problem while parsing the interpolation
            // (debug info already reported while parsing).
            return QString();

        // Resolve interpolation.
        QString msgctxt = QString::fromUtf8(ctxt);
        QString msgid = QString::fromUtf8(msg);
        QString scriptError;
        bool fallback;
        QString interp = s->ktrs->eval(argv, lang, msgctxt, msgid, args, final,
                                       s->scriptModulesToLoad,
                                       scriptError, fallback);
        // Transcript has loaded new modules, clear list for next invocation.
        s->scriptModulesToLoad.clear();

        // See if transcript evaluation went well.
        if (scriptError.isEmpty())
        {
            if (fallback) // script requested fallback to ordinary translation
                return QString();
            else
                sfinal.append(interp);
        }
        else
        {
            #ifndef NDEBUG
            kDebug(173) << QString("Interpolation {%1} in message {%2} failed: %3")
                                  .arg(strans.mid(mpos, tpos - mpos), shorten(strans), scriptError) << endl;
            #endif
            return QString();
        }

        // On to next interpolation.
        ppos = tpos;
        tpos = strans.indexOf(s->startInterp, tpos);
    }
    // Last text segment.
    sfinal.append(substituteSimple(strans.mid(ppos), s->scriptPlchar,
                                   true));

    return sfinal;
}

int KLocalizedStringPrivate::parseInterpolation (const QString &strans, int pos,
                                                 QStringList &argv) const
{
    // pos is the position of opening character sequence.
    // Returns the position of first character after closing sequence,
    // or -1 for non-terminated interpolation.

    KLocalizedStringPrivateStatics *s = staticsKLSP;

    // Split into tokens.
    QStringList tokens;
    int slen = strans.length();
    int elen = s->endInterp.length();
    int tpos = pos + s->startInterp.length();
    while (   tpos < slen
           && strans.mid(tpos, elen) != s->endInterp)
    {
        // Devour whitespace.
        while (   tpos < slen
               && (strans[tpos] == ' ' || strans[tpos] == '\t'))
            ++tpos;

        if (tpos == slen)
        {
            #ifndef NDEBUG
            kDebug(173) << QString("Unexpected end of interpolation {%1} in message {%2}.")
                                  .arg(strans.mid(pos, tpos - pos), shorten(strans)) << endl;
            #endif
            return -1;
        }

        // Token may be quoted and non-quoted.
        // May contain placeholders, substitute them.
        // Mind backslash escapes.
        QString token;
        bool quoted = (strans[tpos] == '\'' ? true : false);
        if (quoted)
        // Quoted token.
        {
            ++tpos; // skip opening quote
            // Find closing quote.
            while (tpos < slen && strans[tpos] != '\'')
            {
                if (strans[tpos] == '\\')
                    ++tpos; // escape next character
                token.append(strans[tpos]);
                ++tpos;
            }

            if (tpos == slen)
            {
                #ifndef NDEBUG
                kDebug(173) << QString("Unclosed quoted token in interpolation {%1} in message {%2}.")
                                      .arg(strans.mid(pos, tpos - pos), shorten(strans)) << endl;
                #endif
                return -1;
            }

            ++tpos; // skip closing quote
        }
        else
        // Non-quoted token.
        {
            // Find whitespace or closing sequence.
            while (   tpos < slen
                   && strans[tpos] != ' ' && strans[tpos] != '\t'
                   && strans.mid(tpos, elen) != s->endInterp)
            {
                if (strans[tpos] == '\\')
                    ++tpos; // escape next character
                token.append(strans[tpos]);
                ++tpos;
            }

            if (tpos == slen)
            {
                #ifndef NDEBUG
                kDebug(173) << QString("Non-terminated interpolation {%1} in message {%2}.")
                                      .arg(strans.mid(pos, tpos - pos), shorten(strans)) << endl;
                #endif
                return -1;
            }
        }

        // Add token to arguments, possibly substituting placeholders.
        if (!token.isEmpty() || quoted) // don't add empty unquoted tokens
            argv.append(substituteSimple(token, s->scriptPlchar, true));
    }
    tpos += elen; // skip to first character after closing sequence

    // NOTE: Why not make this simpler, in two passes -- a simple substitution
    // of placeholders (via substituteSimple) in first pass, then handle
    // interpolations in second pass? Because then user would always have to
    // quote substitutions within interpolations, for safety in case when
    // expansion contains whitespace (like in shell filename expansions).

    return tpos;
}

KLocalizedString KLocalizedString::subs (int a, int fieldWidth, int base,
                                         const QChar &fillChar) const
{
    if (!d->plural.isEmpty() && !d->numberSet)
    {
        d->number = static_cast<pluraln>(abs(a));
        d->numberSet = true;
        d->numberOrd = d->args.size();
    }
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth, base, fillChar));
    return kls;
}

KLocalizedString KLocalizedString::subs (uint a, int fieldWidth, int base,
                                         const QChar &fillChar) const
{
    if (!d->plural.isEmpty() && !d->numberSet)
    {
        d->number = static_cast<pluraln>(a);
        d->numberSet = true;
        d->numberOrd = d->args.size();
    }
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth, base, fillChar));
    return kls;
}

KLocalizedString KLocalizedString::subs (long a, int fieldWidth, int base,
                                         const QChar &fillChar) const
{
    if (!d->plural.isEmpty() && !d->numberSet)
    {
        d->number = static_cast<pluraln>(abs(a));
        d->numberSet = true;
        d->numberOrd = d->args.size();
    }
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth, base, fillChar));
    return kls;
}

KLocalizedString KLocalizedString::subs (ulong a, int fieldWidth, int base,
                                         const QChar &fillChar) const
{
    if (!d->plural.isEmpty() && !d->numberSet)
    {
        d->number = static_cast<pluraln>(a);
        d->numberSet = true;
        d->numberOrd = d->args.size();
    }
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth, base, fillChar));
    return kls;
}

KLocalizedString KLocalizedString::subs (qlonglong a, int fieldWidth, int base,
                                         const QChar &fillChar) const
{
    if (!d->plural.isEmpty() && !d->numberSet)
    {
        d->number = static_cast<pluraln>(qAbs(a));
        d->numberSet = true;
        d->numberOrd = d->args.size();
    }
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth, base, fillChar));
    return kls;
}

KLocalizedString KLocalizedString::subs (qulonglong a, int fieldWidth, int base,
                                         const QChar &fillChar) const
{
    if (!d->plural.isEmpty() && !d->numberSet)
    {
        d->number = static_cast<pluraln>(a);
        d->numberSet = true;
        d->numberOrd = d->args.size();
    }
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth, base, fillChar));
    return kls;
}

KLocalizedString KLocalizedString::subs (double a, int fieldWidth,
                                         char format, int precision,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth, format, precision,
                                         fillChar));
    return kls;
}

KLocalizedString KLocalizedString::subs (QChar a, int fieldWidth,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth, fillChar));
    return kls;
}

KLocalizedString KLocalizedString::subs (const QString &a, int fieldWidth,
                                         const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth, fillChar));
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

    s->loadTranscriptCalled = true;
    s->ktrs = NULL; // null indicates that Transcript is not available

    KLibrary *lib = KLibLoader::self()->library("ktranscript");
    if (!lib) {
        #ifndef NDEBUG
        kDebug(173) << QString("Cannot load transcript plugin: %1")
                              .arg(KLibLoader::self()->lastErrorMessage()) << endl;
        #endif
        return;
    }

    InitFunc initf = (InitFunc) lib->resolveFunction("load_transcript");
    if (!initf) {
        lib->unload();
        #ifndef NDEBUG
        kDebug(173) << QString("Cannot find function load_transcript in transcript plugin.") << endl;
        #endif
        return;
    }

    s->ktrs = initf();
}

void KLocalizedString::notifyCatalogsUpdated (const QStringList &languages,
                                              const QStringList &catalogs)
{
    KLocalizedStringPrivate::notifyCatalogsUpdated(languages, catalogs);
}

void KLocalizedStringPrivate::notifyCatalogsUpdated (const QStringList &languages,
                                                     const QStringList &catalogs)
{
    KLocalizedStringPrivateStatics *s = staticsKLSP;

    // Find script modules for all included language/catalogs that have them,
    // and remember their paths.
    foreach (const QString &lang, languages)
        foreach (const QString &cat, catalogs)
        {
            // Assemble module's relative path.
            QString modrpath =   lang + '/' + s->scriptDir + '/'
                               + cat + '/' + cat + ".js";

            // Try to find this module.
            QString modapath = KStandardDirs::locate("locale", modrpath);

            // If the module exists and hasn't been already included.
            if (   !modapath.isEmpty()
                && !s->scriptModules[lang].contains(cat))
            {
                // Indicate that the module has been considered.
                s->scriptModules[lang].append(cat);

                // Store the absolute path and language of the module,
                // to load on next script evaluation.
                QStringList mod;
                mod.append(modapath);
                mod.append(lang);
                s->scriptModulesToLoad.append(mod);
            }
        }
}
