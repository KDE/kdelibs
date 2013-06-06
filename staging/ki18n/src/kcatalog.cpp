/* This file is part of the KDE libraries
   Copyright (c) 2001 Hans Petter Bieker <bieker@kde.org>
   Copyright (c) 2012 Chusslove Illich <caslav.ilic@gmx.net>

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

#include "kcatalog_p.h"

#include <qstandardpaths.h>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QDebug>
#include <QByteArray>

#include <stdlib.h>
#include <locale.h>
#include "gettext.h"



// not defined on win32 :(
#ifdef _WIN32
# ifndef LC_MESSAGES
#  define LC_MESSAGES 42
# endif
#endif

static char *langenv = 0;
static const int langenvMaxlen = 42;
// = "LANGUAGE=" + 32 chars for language code + terminating zero

class KCatalogStaticData
{
public:
    KCatalogStaticData () {}

    QMutex mutex;
};

Q_GLOBAL_STATIC(KCatalogStaticData, catalogStaticData)

class KCatalogPrivate
{
public:
    KCatalogPrivate()
        : bindDone(false)
    {}

    QByteArray language;
    QByteArray name;
    QByteArray localeDir;

    QByteArray systemLanguage;
    bool bindDone;

    static QByteArray currentLanguage;

    void setupGettextEnv ();
    void resetSystemLanguage ();
};

QByteArray KCatalogPrivate::currentLanguage;

KCatalog::KCatalog (const QString &name, const QString &language)
: d(new KCatalogPrivate)
{

    // Find locale directory for this catalog.
    QString localeDir = catalogLocaleDir(name, language);

    d->language = QFile::encodeName(language);
    d->name = QFile::encodeName(name);
    d->localeDir = QFile::encodeName(localeDir);

    // Always get translations in UTF-8, regardless of user's environment.
    bind_textdomain_codeset(d->name, "UTF-8");

    // Invalidate current language, to trigger binding at next translate call.
    KCatalogPrivate::currentLanguage.clear();

    if (!langenv) {
        // Call putenv only here, to initialize LANGUAGE variable.
        // Later only change langenv to what is currently needed.
        langenv = new char[langenvMaxlen];
        QByteArray lang = qgetenv("LANGUAGE");
        qsnprintf(langenv, langenvMaxlen, "LANGUAGE=%s", lang.constData());
        putenv(langenv);
    }
}

KCatalog::~KCatalog ()
{
    delete d;
}

QString KCatalog::catalogLocaleDir (const QString &name, const QString &language)
{
    const QString relpath = QString::fromLatin1("%1/LC_MESSAGES/%2.mo")
                                               .arg(language).arg(name);
    const QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                QString::fromLatin1("locale/") + relpath);
    if (file.isEmpty()) {
        return QString();
    } else {
        // Path of the locale/ directory must be returned.
        return QFileInfo(file.left(file.size() - relpath.size())).absolutePath();
    }
}

QString KCatalog::name () const
{
    return QFile::decodeName(d->name);
}

QString KCatalog::language () const
{
    return QFile::decodeName(d->language);
}

QString KCatalog::localeDir () const
{
    return QFile::decodeName(d->localeDir);
}

#ifdef Q_OS_WIN
extern "C" int __declspec(dllimport) _nl_msg_cat_cntr;
#endif

void KCatalogPrivate::setupGettextEnv ()
{
    // Point Gettext to current language, recording system value for recovery.
    systemLanguage = qgetenv("LANGUAGE");
    if (systemLanguage != language) {
        // putenv has been called in the constructor,
        // it is enough to change the string set there.
        qsnprintf(langenv, langenvMaxlen, "LANGUAGE=%s", language.constData());
    }

    // Rebind text domain if language actually changed from the last time,
    // as locale directories may differ for different languages of same catalog.
    if (language != currentLanguage || !bindDone) {

        currentLanguage = language;
        bindDone = true;

        //kDebug() << "bindtextdomain" << name << localeDir;
        bindtextdomain(name, localeDir);

        // Magic to make sure Gettext doesn't use stale cached translation
        // from previous language.
#ifndef _MSC_VER
        extern int _nl_msg_cat_cntr;
#endif
        ++_nl_msg_cat_cntr;
    }
}

void KCatalogPrivate::resetSystemLanguage ()
{
    if (language != systemLanguage) {
        qsnprintf(langenv, langenvMaxlen, "LANGUAGE=%s", systemLanguage.constData());
    }
}

QString KCatalog::translate (const QByteArray &msgid) const
{
    QMutexLocker locker(&catalogStaticData()->mutex);
    d->setupGettextEnv();
    const char *msgstr = dgettext(d->name.constData(), msgid.constData());
    d->resetSystemLanguage();
    return   msgstr != msgid
           ? QString::fromUtf8(msgstr)
           : QString();
}

QString KCatalog::translate (const QByteArray &msgctxt,
                             const QByteArray &msgid) const
{
    QMutexLocker locker(&catalogStaticData()->mutex);
    d->setupGettextEnv();
    const char *msgstr = dpgettext_expr(d->name.constData(), msgctxt.constData(), msgid.constData());
    d->resetSystemLanguage();
    return   msgstr != msgid
           ? QString::fromUtf8(msgstr)
           : QString();
}

QString KCatalog::translate (const QByteArray &msgid,
                             const QByteArray &msgid_plural,
                             qulonglong n) const
{
    QMutexLocker locker(&catalogStaticData()->mutex);
    d->setupGettextEnv();
    const char *msgstr = dngettext(d->name.constData(), msgid.constData(), msgid_plural.constData(), n);
    d->resetSystemLanguage();
    return   msgstr != msgid && msgstr != msgid_plural
           ? QString::fromUtf8(msgstr)
           : QString();
}

QString KCatalog::translate (const QByteArray &msgctxt,
                             const QByteArray &msgid,
                             const QByteArray &msgid_plural,
                             qulonglong n) const
{
    QMutexLocker locker(&catalogStaticData()->mutex);
    d->setupGettextEnv();
    const char *msgstr = dnpgettext_expr(d->name.constData(), msgctxt.constData(), msgid.constData(), msgid_plural.constData(), n);
    d->resetSystemLanguage();
    return   msgstr != msgid && msgstr != msgid_plural
           ? QString::fromUtf8(msgstr)
           : QString();
}

