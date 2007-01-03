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

#include <config.h>

#include <qfile.h>

#include <kdebug.h>

#include "kcatalog.h"
#include "kstandarddirs.h"

#include <stdlib.h>
#include <locale.h>
#include "gettext.h"

static const QByteArray GLUE = GETTEXT_CONTEXT_GLUE;

class KCatalogPrivate
{
public:
  QByteArray language;
  QByteArray name;
  QByteArray localeDir;

  static int localeSet;
  static QByteArray currentLanguage;

  void changeBindings () const;
};

int KCatalogPrivate::localeSet = 0;
QByteArray KCatalogPrivate::currentLanguage;

KCatalog::KCatalog(const QString & name, const QString & language )
  : d( new KCatalogPrivate )
{
  // Set locales only once.
  if (! KCatalogPrivate::localeSet)
  {
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
  d->name      = rhs.d->name;
  d->language  = rhs.d->language;
  d->localeDir = rhs.d->localeDir;

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

void KCatalogPrivate::changeBindings () const
{
  if (language != currentLanguage)
  {
    currentLanguage = language;

    // Point Gettext to new language.
    setenv("LANGUAGE", language, 1);

    // Locale directories may differ between languages.
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

QString KCatalog::translate(const char * msgid) const
{
  d->changeBindings();
  return QString::fromUtf8(dgettext(d->name, msgid));
}

QString KCatalog::translate(const char * msgctxt, const char * msgid) const
{
  d->changeBindings();
  // dpgettext is a macro which needs string literals for msgctxt and msgid
  // so until and unless that is changed, we cannot use it this way.
  //return QString::fromUtf8(dpgettext(d->name, msgctxt, msgid));
  QByteArray tmpstr;
  tmpstr.append(msgctxt).append(GLUE).append(msgid);
  QString r = QString::fromUtf8(dgettext(d->name, tmpstr));
  return r.mid(r.indexOf(GLUE) + 1); // because we may get tmpstr back
}

QString KCatalog::translate(const char * msgid, const char * msgid_plural,
                            unsigned long n) const
{
  d->changeBindings();
  return QString::fromUtf8(dngettext(d->name, msgid, msgid_plural, n));
}

QString KCatalog::translate(const char * msgctxt, const char * msgid,
                            const char * msgid_plural, unsigned long n) const
{
  d->changeBindings();
  // dnpgettext is a macro which needs string literals for msgctxt and msgid
  // so until and unless that is changed, we cannot use it this way.
  //return QString::fromUtf8(dnpgettext(d->name, msgctxt, msgid, msgid_plural, n));
  QByteArray tmpstr;
  tmpstr.append(msgctxt).append(GLUE).append(msgid);
  QString r = QString::fromUtf8(dngettext(d->name, tmpstr, msgid_plural, n));
  return r.mid(r.indexOf(GLUE) + 1); // because we may get tmpstr back
}
