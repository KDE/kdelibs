/* This file is part of the KDE libraries
   Copyright (c) 2001 Hans Petter Bieker <bieker@kde.org>

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
#include "kstandarddirs.h"

#include <config.h>

#include <QtCore/QFile>
#include <QMutexLocker>

#include <kdebug.h>

#include <stdlib.h>
#include <locale.h>
#include "gettext.h"


static bool s_localeSet = false;

// Initialize the locale very early during application startup
// This is necessary for e.g. toLocal8Bit() to work, even before
// a Q[Core]Application exists (David)
int kInitializeLocale()
{
#ifndef _WIN32_WCE
    setlocale(LC_ALL, "");
#endif
    extern Q_CORE_EXPORT bool qt_locale_initialized; // in Qt since 4.5.0
    qt_locale_initialized = true; // as recommended by Thiago
    s_localeSet = true;
    return 1;
}
Q_CONSTRUCTOR_FUNCTION(kInitializeLocale)

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
    KCatalogStaticData() {}

    QMutex mutex;
};

K_GLOBAL_STATIC(KCatalogStaticData, catalogStaticData)

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

QDebug operator<<(QDebug debug, const KCatalog &c)
{
  return debug << c.d->language << " " << c.d->name << " " << c.d->localeDir;
}

QByteArray KCatalogPrivate::currentLanguage;

KCatalog::KCatalog(const QString & name, const QString & language )
  : d( new KCatalogPrivate )
{
    // Set locales if the static initializer didn't work
    if (!s_localeSet) {
        kInitializeLocale();
    }

  // Find locale directory for this catalog.
  QString localeDir = catalogLocaleDir( name, language );

  d->language = QFile::encodeName( language );
  d->name = QFile::encodeName( name );
  d->localeDir = QFile::encodeName( localeDir );

  // Always get translations in UTF-8, regardless of user's environment.
  bind_textdomain_codeset( d->name, "UTF-8" );

  // Invalidate current language, to trigger binding at next translate call.
  KCatalogPrivate::currentLanguage.clear();

  if (!langenv) {
    // Call putenv only here, to initialize LANGUAGE variable.
    // Later only change langenv to what is currently needed.
    langenv = new char[langenvMaxlen];
    QByteArray lang = qgetenv("LANGUAGE");
    snprintf(langenv, langenvMaxlen, "LANGUAGE=%s", lang.constData());
    putenv(langenv);
  }
}

KCatalog::KCatalog(const KCatalog & rhs)
  : d( new KCatalogPrivate )
{
  *this = rhs;
}

KCatalog & KCatalog::operator=(const KCatalog & rhs)
{
  *d = *rhs.d;

  return *this;
}

KCatalog::~KCatalog()
{
  delete d;
}

QString KCatalog::catalogLocaleDir( const QString &name,
                                    const QString &language )
{
  QString relpath =  QString::fromLatin1( "%1/LC_MESSAGES/%2.mo" )
                    .arg( language ).arg( name );
  return KGlobal::dirs()->findResourceDir( "locale", relpath );
}

QString KCatalog::name() const
{
  return d->name;
}

QString KCatalog::language() const
{
  return d->language;
}

QString KCatalog::localeDir() const
{
  return d->localeDir;
}

void KCatalogPrivate::setupGettextEnv ()
{
  // Point Gettext to current language, recording system value for recovery.
  systemLanguage = qgetenv("LANGUAGE");
  if (systemLanguage != language) {
    // putenv has been called in the constructor,
    // it is enough to change the string set there.
    snprintf(langenv, langenvMaxlen, "LANGUAGE=%s", language.constData());
  }

  // Rebind text domain if language actually changed from the last time,
  // as locale directories may differ for different languages of same catalog.
  if (language != currentLanguage || !bindDone) {

    currentLanguage = language;
    bindDone = true;

    //kDebug() << "bindtextdomain" << name << localeDir;
    bindtextdomain(name, localeDir);

    // // Magic to make sure Gettext doesn't use stale cached translation
    // // from previous language.
    // extern int _nl_msg_cat_cntr;
    // ++_nl_msg_cat_cntr;
    //
    // Note: Not needed, caching of translations is not an issue because
    // language is switched only if translation is not found.
  }
}

void KCatalogPrivate::resetSystemLanguage ()
{
  if (language != systemLanguage) {
    snprintf(langenv, langenvMaxlen, "LANGUAGE=%s", systemLanguage.constData());
  }
}

QString KCatalog::translate(const char * msgid) const
{
  QMutexLocker locker(&catalogStaticData->mutex);
  d->setupGettextEnv();
  const char *msgstr = dgettext(d->name, msgid);
  d->resetSystemLanguage();
  return QString::fromUtf8(msgstr);
}

QString KCatalog::translate(const char * msgctxt, const char * msgid) const
{
  QMutexLocker locker(&catalogStaticData->mutex);
  d->setupGettextEnv();
  const char *msgstr = dpgettext_expr(d->name, msgctxt, msgid);
  d->resetSystemLanguage();
  return QString::fromUtf8(msgstr);
}

QString KCatalog::translate(const char * msgid, const char * msgid_plural,
                            unsigned long n) const
{
  QMutexLocker locker(&catalogStaticData->mutex);
  d->setupGettextEnv();
  const char *msgstr = dngettext(d->name, msgid, msgid_plural, n);
  d->resetSystemLanguage();
  return QString::fromUtf8(msgstr);
}

QString KCatalog::translate(const char * msgctxt, const char * msgid,
                            const char * msgid_plural, unsigned long n) const
{
  QMutexLocker locker(&catalogStaticData->mutex);
  d->setupGettextEnv();
  const char *msgstr = dnpgettext_expr(d->name, msgctxt, msgid, msgid_plural, n);
  d->resetSystemLanguage();
  return QString::fromUtf8(msgstr);
}

QString KCatalog::translateStrict(const char * msgid) const
{
  QMutexLocker locker(&catalogStaticData->mutex);
  d->setupGettextEnv();
  const char *msgstr = dgettext(d->name, msgid);
  d->resetSystemLanguage();
  return msgstr != msgid ? QString::fromUtf8(msgstr) : QString();
}

QString KCatalog::translateStrict(const char * msgctxt, const char * msgid) const
{
  QMutexLocker locker(&catalogStaticData->mutex);
  d->setupGettextEnv();
  const char *msgstr = dpgettext_expr(d->name, msgctxt, msgid);
  d->resetSystemLanguage();
  return msgstr != msgid ? QString::fromUtf8(msgstr) : QString();
}

QString KCatalog::translateStrict(const char * msgid, const char * msgid_plural,
                                  unsigned long n) const
{
  QMutexLocker locker(&catalogStaticData->mutex);
  d->setupGettextEnv();
  const char *msgstr = dngettext(d->name, msgid, msgid_plural, n);
  d->resetSystemLanguage();
  return msgstr != msgid && msgstr != msgid_plural ? QString::fromUtf8(msgstr) : QString();
}

QString KCatalog::translateStrict(const char * msgctxt, const char * msgid,
                                  const char * msgid_plural, unsigned long n) const
{
  QMutexLocker locker(&catalogStaticData->mutex);
  d->setupGettextEnv();
  const char *msgstr = dnpgettext_expr(d->name, msgctxt, msgid, msgid_plural, n);
  d->resetSystemLanguage();
  return msgstr != msgid && msgstr != msgid_plural ? QString::fromUtf8(msgstr) : QString();
}

