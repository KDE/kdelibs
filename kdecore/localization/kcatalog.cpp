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

#include <kdebug.h>

#include <stdlib.h>
#include <locale.h>
#include "gettext.h"

// not defined on win32 :(
#ifdef _WIN32
# ifndef LC_MESSAGES
#  define LC_MESSAGES 42
# endif
#endif

class KCatalogPrivate
{
public:
  QByteArray language;
  QByteArray name;
  QByteArray localeDir;

  QByteArray systemLanguage;

  static int localeSet;
  static QByteArray currentLanguage;

  void setupGettextEnv ();
  void resetSystemLanguage ();
};

QDebug operator<<(QDebug debug, const KCatalog &c)
{
  return debug << c.d->language << " " << c.d->name << " " << c.d->localeDir;
}

int KCatalogPrivate::localeSet = 0;
QByteArray KCatalogPrivate::currentLanguage;

KCatalog::KCatalog(const QString & name, const QString & language )
  : d( new KCatalogPrivate )
{
  // Set locales only once.
  if (! KCatalogPrivate::localeSet) {
    setlocale(LC_ALL, "");
    KCatalogPrivate::localeSet = 1;
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
}

KCatalog::KCatalog(const KCatalog & rhs)
  : d( new KCatalogPrivate )
{
  *this = rhs;
}

KCatalog & KCatalog::operator=(const KCatalog & rhs)
{
  *d = *rhs.d;

  // Update Gettext environment.
  // (Sometimes Gettext picks up wrong locale directory if bindings are not
  // updated here. No idea why that happens.)
  d->setupGettextEnv();
  d->resetSystemLanguage();

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
    qputenv("LANGUAGE", language);
  }

  // Rebind text domain if language actually changed from the last time,
  // as locale directories may differ for different languages of same catalog.
  if (language != currentLanguage) {

    currentLanguage = language;

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
    qputenv("LANGUAGE", systemLanguage);
  }
}

QString KCatalog::translate(const char * msgid) const
{
  d->setupGettextEnv();
  const char *msgstr = dgettext(d->name, msgid);
  d->resetSystemLanguage();
  return QString::fromUtf8(msgstr);
}

QString KCatalog::translate(const char * msgctxt, const char * msgid) const
{
  d->setupGettextEnv();
  const char *msgstr = dpgettext_expr(d->name, msgctxt, msgid);
  d->resetSystemLanguage();
  return QString::fromUtf8(msgstr);
}

QString KCatalog::translate(const char * msgid, const char * msgid_plural,
                            unsigned long n) const
{
  d->setupGettextEnv();
  const char *msgstr = dngettext(d->name, msgid, msgid_plural, n);
  d->resetSystemLanguage();
  return QString::fromUtf8(msgstr);
}

QString KCatalog::translate(const char * msgctxt, const char * msgid,
                            const char * msgid_plural, unsigned long n) const
{
  d->setupGettextEnv();
  const char *msgstr = dnpgettext_expr(d->name, msgctxt, msgid, msgid_plural, n);
  d->resetSystemLanguage();
  return QString::fromUtf8(msgstr);
}

QString KCatalog::translateStrict(const char * msgid) const
{
  d->setupGettextEnv();
  const char *msgstr = dgettext(d->name, msgid);
  d->resetSystemLanguage();
  return msgstr != msgid ? QString::fromUtf8(msgstr) : QString();
}

QString KCatalog::translateStrict(const char * msgctxt, const char * msgid) const
{
  d->setupGettextEnv();
  const char *msgstr = dpgettext_expr(d->name, msgctxt, msgid);
  d->resetSystemLanguage();
  return msgstr != msgid ? QString::fromUtf8(msgstr) : QString();
}

QString KCatalog::translateStrict(const char * msgid, const char * msgid_plural,
                                  unsigned long n) const
{
  d->setupGettextEnv();
  const char *msgstr = dngettext(d->name, msgid, msgid_plural, n);
  d->resetSystemLanguage();
  return msgstr != msgid && msgstr != msgid_plural ? QString::fromUtf8(msgstr) : QString();
}

QString KCatalog::translateStrict(const char * msgctxt, const char * msgid,
                                  const char * msgid_plural, unsigned long n) const
{
  d->setupGettextEnv();
  const char *msgstr = dnpgettext_expr(d->name, msgctxt, msgid, msgid_plural, n);
  d->resetSystemLanguage();
  return msgstr != msgid && msgstr != msgid_plural ? QString::fromUtf8(msgstr) : QString();
}

