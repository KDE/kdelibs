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

#include <config.h>
#include <klocalizedstring.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <qstring.h>
#include <qvector.h>

class KLocalizedStringPrivate
{
    friend class KLocalizedString;

    QVector<QString> args;
    bool numberSet;
    long number;
    QByteArray ctxt;
    QByteArray msg;
    QByteArray plural;

    QString toString(const KLocale *locale) const;
};

KLocalizedString::KLocalizedString ()
: d(new KLocalizedStringPrivate)
{
    d->numberSet = false;
    d->number = 0;
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
}

KLocalizedString::KLocalizedString(const KLocalizedString& rhs)
: d(new KLocalizedStringPrivate(*rhs.d))
{
}

KLocalizedString& KLocalizedString::operator= (const KLocalizedString& rhs)
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

    // Check whether plural argument has been supplied, if message has plural.
    if (!plural.isEmpty() && !numberSet)
    {
        #ifndef NDEBUG
        kDebug(173) << QString("Plural argument to message {%1} not supplied before conversion.")
                               .arg(QString::fromUtf8(msg).left(20)) << endl;
        #endif
    }

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

    // Set ordinary translation.
    QString trans;
    int cdpos = rawtrans.indexOf("|/|");
    if (cdpos >= 0)
        // Script fence is found, so strip the script from the translation.
        trans = rawtrans.left(cdpos);
    else
        trans = rawtrans;

    //kDebug() << lang << endl;
    //kDebug() << rawtrans << endl;
    //kDebug() << trans << endl;
    //kDebug() << args.size() << endl;

    QVector<int> plinks; // ordinal numbers per placeholder number
    QVector<QString> tsegs; // text segments per placeholder occurence
    QVector<int> plords; // ordinal numbers per placeholder occurence
    QVector<QString> plnames; // placeholder names per ordinal

    QVector<int> ords; // indicates which placeholders are present
    int slen = trans.length();
    int spos = 0;
    int tpos = trans.indexOf('%');
    while (tpos >= 0)
    {
        int ctpos = tpos;

        tpos++;
        if (tpos == slen)
        break;

        if (trans[tpos].digitValue() >= 0 || trans[tpos] == 'n')
        {
            int plnum = 0;
            if (trans[tpos] == 'n')
            {
                // Plural placeholder is treated as placeholder 0.
                plnum = 0;
                tpos++;
            }
            else
            {
                // Get the numbered placeholder.
                while (tpos < slen && trans[tpos].digitValue() >= 0)
                {
                    plnum = 10 * plnum + trans[tpos].digitValue();
                    tpos++;
                }
                // 0 can be only plural placeholder, so shift numbered one up.
                plnum++;
            }

            // Perhaps enlarge storage for indicators.
            if (plnum >= ords.size())
                ords.resize(plnum + 1);

            // Indicate that placeholder with computed number is present.
            ords[plnum] = 1;

            // Store text segment prior to placeholder and placeholder number.
            tsegs.append(trans.mid(spos, ctpos - spos));
            plords.append(plnum);

            // Position of next text segment.
            spos = tpos;
        }
        else
            tpos++; // might be escape for %

        tpos = trans.indexOf('%', tpos);
    }
    // Store last text segment.
    tsegs.append(trans.mid(spos));

    // Message might have plural but without plural placeholder, which is an
    // allowed state. So, for further logic, indicate plural placeholder is
    // present anyway if message has plural.
    if (!plural.isEmpty())
    {
        if (ords.size() < 1)
            ords.resize(1);
        ords[0] = 1;
    }

    // Set placeholder->ordinal links and unique placeholder names.
    int ord = 0;
    for (int i = 0; i < ords.size(); i++)
        if (ords[i])
        {
            plinks.append(ord);
            ord++;

            if (i == 0)
                plnames.append("%n"); // plural placeholder name
            else
                plnames.append("%" + QString::number(i - 1)); // numbered
        }
        else
            plinks.append(0); // dummy

    // Convert placeholder numbers per occurence to ordinal numbers.
    for (int i = 0; i < plords.size(); i++)
        plords[i] = plinks.at(plords[i]);

    #ifndef NDEBUG
    // Check for mismatch between number of unique placeholders and
    // actually supplied arguments.
    if (plords.size() != args.size())
        kDebug(173) << QString("%1 instead of %2 arguments to message {%3} supplied before conversion.")
                               .arg(args.size()).arg(plords.size()).arg(trans.left(20)) << endl;
    #endif

    // Assemble the final string from text segments and arguments.
    QString final;
    for (int i = 0; i < plords.size(); i++)
    {
        final.append(tsegs.at(i));
        if (!plural.isEmpty() && plords.at(i) == 0 && !numberSet)
        // trying to substitute %n in plural with something strange
        {
            // put back the placeholder
            final.append(plnames.at(plords.at(i)));
            #ifndef NDEBUG
            // spoof the message
            final.append("(I18N_PLURAL_ARGUMENT_NOT_A_NUMBER)");
            #endif
        }
        else if (plords.at(i) >= args.size())
        // too little arguments
        {
            // put back the placeholder
            final.append(plnames.at(plords.at(i)));
            #ifndef NDEBUG
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
    if (ord < args.size())
        final.append("(I18N_EXTRA_ARGUMENTS)");
    #endif

    #ifndef NDEBUG
    if (!plural.isEmpty() && !numberSet)
        final.append("(I18N_PLURAL_ARGUMENT_MISSING)");
    #endif

    return final;
}

KLocalizedString KLocalizedString::subs (int a, int fieldWidth) const
{
    if (!d->plural.isEmpty() && d->args.size() == 0)
    {
        d->number = static_cast<long>(a);
        d->numberSet = true;
    }
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth));
    return kls;
}

KLocalizedString KLocalizedString::subs (uint a, int fieldWidth) const
{
    if (!d->plural.isEmpty() && d->args.size() == 0)
    {
        d->number = static_cast<long>(a);
        d->numberSet = true;
    }
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth));
    return kls;
}

KLocalizedString KLocalizedString::subs (long a, int fieldWidth) const
{
    if (!d->plural.isEmpty() && d->args.size() == 0)
    {
        d->number = static_cast<long>(a);
        d->numberSet = true;
    }
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth));
    return kls;
}

KLocalizedString KLocalizedString::subs (ulong a, int fieldWidth) const
{
    if (!d->plural.isEmpty() && d->args.size() == 0)
    {
        d->number = static_cast<long>(a);
        d->numberSet = true;
    }
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth));
    return kls;
}

KLocalizedString KLocalizedString::subs (double a, int fieldWidth,
                                         char fmt, int prec) const
{
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth, fmt, prec));
    return kls;
}

KLocalizedString KLocalizedString::subs (QChar a, int fieldWidth) const
{
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth));
    return kls;
}

KLocalizedString KLocalizedString::subs (const QString& a, int fieldWidth) const
{
    KLocalizedString kls(*this);
    kls.d->args.append(QString("%1").arg(a, fieldWidth));
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
