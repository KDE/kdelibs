// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 * Copyright (C)  2006  Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "settings_p.h"

#include "loader_p.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <QtCore/QMap>
#include <QtCore/QMutableStringListIterator>

namespace Sonnet
{
class Settings::Private
{
public:
    Loader*  loader; //can't be a Ptr since we don't want to hold a ref on it
    bool     modified;

    QString defaultLanguage;
    QString defaultClient;

    bool checkUppercase;
    bool skipRunTogether;
    bool backgroundCheckerEnabled;
    bool checkerEnabledByDefault;

    int disablePercentage;
    int disableWordCount;

    QMap<QString, bool> ignore;
};

Settings::Settings(Loader *loader)
	:d(new Private)
{
    d->loader = loader;

    d->modified = false;
    d->checkerEnabledByDefault = false;
}

Settings::~Settings()
{
    delete d;
}

void Settings::setDefaultLanguage(const QString &lang)
{
    const QStringList cs = d->loader->languages();
    if (cs.indexOf(lang) != -1 &&
        d->defaultLanguage != lang) {
        d->defaultLanguage = lang;
        //readIgnoreList();
        d->modified = true;
        d->loader->changed();
    }
}

QString Settings::defaultLanguage() const
{
    return d->defaultLanguage;
}

void Settings::setDefaultClient(const QString &client)
{
    //Different from setDefaultLanguage because
    //the number of clients can't be even close
    //as big as the number of languages
    if (d->loader->clients().contains(client)) {
        d->defaultClient = client;
        d->modified = true;
        d->loader->changed();
    }
}

QString Settings::defaultClient() const
{
    return d->defaultClient;
}

void Settings::setCheckUppercase(bool check)
{
    if (d->checkUppercase != check) {
        d->modified = true;
        d->checkUppercase = check;
    }
}

bool Settings::checkUppercase() const
{
    return d->checkUppercase;
}

void Settings::setSkipRunTogether(bool skip)
{
    if (d->skipRunTogether != skip) {
        d->modified = true;
        d->skipRunTogether = skip;
    }
}

bool Settings::skipRunTogether() const
{
    return d->skipRunTogether;
}

void Settings::setCheckerEnabledByDefault(bool check)
{
    if (d->checkerEnabledByDefault != check) {
        d->modified = true;
        d->checkerEnabledByDefault = check;
    }
}

bool Settings::checkerEnabledByDefault() const
{
    return d->checkerEnabledByDefault;
}

void Settings::setBackgroundCheckerEnabled(bool enable)
{
    if (d->backgroundCheckerEnabled != enable) {
        d->modified = true;
        d->backgroundCheckerEnabled = enable;
    }
}

bool Settings::backgroundCheckerEnabled() const
{
    return d->backgroundCheckerEnabled;
}

void Settings::setCurrentIgnoreList(const QStringList &ignores)
{
    setQuietIgnoreList(ignores);
    d->modified = true;
}

void Settings::setQuietIgnoreList(const QStringList &ignores)
{
    d->ignore = QMap<QString, bool>();//clear out
    for (QStringList::const_iterator itr = ignores.begin();
         itr != ignores.end(); ++itr) {
        d->ignore.insert(*itr, true);
    }
}

QStringList Settings::currentIgnoreList() const
{
    return d->ignore.keys();
}

void Settings::addWordToIgnore(const QString &word)
{
    if (!d->ignore.contains(word)) {
        d->modified = true;
        d->ignore.insert( word, true );
    }
}

bool Settings::ignore( const QString& word )
{
    return d->ignore.contains( word );
}

void Settings::readIgnoreList(KConfig *config)
{
    const KConfigGroup conf(config, "Spelling");
    const QString ignoreEntry = QString( "ignore_%1" ).arg(d->defaultLanguage);
    const QStringList ignores = conf.readEntry(ignoreEntry, QStringList());
    setQuietIgnoreList(ignores);
}

int Settings::disablePercentageWordError() const
{
    return d->disablePercentage;
}

int Settings::disableWordErrorCount() const
{
    return d->disableWordCount;
}

void Settings::save(KConfig *config)
{
    KConfigGroup conf(config, "Spelling");
    conf.writeEntry("defaultClient", d->defaultClient);
    conf.writeEntry("defaultLanguage", d->defaultLanguage);
    conf.writeEntry("checkUppercase", d->checkUppercase);
    conf.writeEntry("skipRunTogether", d->skipRunTogether);
    conf.writeEntry("backgroundCheckerEnabled", d->backgroundCheckerEnabled);
    conf.writeEntry("checkerEnabledByDefault", d->checkerEnabledByDefault);
    QString defaultLanguage = QString( "ignore_%1" ).arg(d->defaultLanguage);
    if(conf.hasKey(defaultLanguage) && d->ignore.isEmpty())
      conf.deleteEntry(defaultLanguage);
    else if(!d->ignore.isEmpty())
      conf.writeEntry(QString( "ignore_%1" ).arg(d->defaultLanguage),
                    d->ignore.keys() );

    conf.sync();
}

void Settings::restore(KConfig *config)
{
    KConfigGroup conf(config, "Spelling");
    d->defaultClient = conf.readEntry("defaultClient",
                                      QString());
    d->defaultLanguage = conf.readEntry(
        "defaultLanguage", KGlobal::locale()->language());

    //same defaults are in the default filter (filter.cpp)
    d->checkUppercase = conf.readEntry(
        "checkUppercase", true);

    d->skipRunTogether = conf.readEntry(
        "skipRunTogether", true);

    d->backgroundCheckerEnabled = conf.readEntry(
        "backgroundCheckerEnabled", true);

    d->checkerEnabledByDefault = conf.readEntry(
        "checkerEnabledByDefault", false);

    d->disablePercentage = conf.readEntry("Sonnet_AsYouTypeDisablePercentage", 42);
    d->disableWordCount = conf.readEntry("Sonnet_AsYouTypeDisableWordCount", 100);

    readIgnoreList(config);
}


bool Settings::modified() const
{
    return d->modified;
}

void Settings::setModified(bool modified)
{
    d->modified = modified;
}

}

