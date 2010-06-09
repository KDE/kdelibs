// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
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
#include "loader_p.h"
#include "settings_p.h"
#include "client_p.h"
#include "spellerplugin_p.h"

#include <klocale.h>
#include <kservicetypetrader.h>

#include <kconfig.h>
#include <kdebug.h>

#include <QtCore/QHash>
#include <QtCore/QMap>

#define DEFAULT_CONFIG_FILE   "sonnetrc"

namespace Sonnet
{

class Loader::Private
{
public:
    KService::List plugins;
    Settings *settings;

    // <language, Clients with that language >
    QMap<QString, QList<Client*> > languageClients;
    QStringList clients;

    QStringList languagesNameCache;
};

K_GLOBAL_STATIC(Loader, s_loader)

Loader *Loader::openLoader()
{
    if (s_loader.isDestroyed()) {
        return 0;
    }

    return s_loader;
}

Loader::Loader()
    :d(new Private)
{
    d->settings = new Settings(this);
    KConfig config(DEFAULT_CONFIG_FILE);
    d->settings->restore(&config);
    loadPlugins();
}

Loader::~Loader()
{
    //kDebug()<<"Removing loader : "<< this;
    d->plugins.clear();
    delete d->settings; d->settings = 0;
    delete d;
}

SpellerPlugin *Loader::createSpeller(const QString& language,
                                     const QString& clientName) const
{
    QString pclient = clientName;
    QString plang   = language;
    bool ddefault = false;

    if (plang.isEmpty()) {
        plang = d->settings->defaultLanguage();
    }
    if (clientName == d->settings->defaultClient() &&
        plang == d->settings->defaultLanguage()) {
        ddefault = true;
    }

    const QList<Client*> lClients = d->languageClients[plang];

    if (lClients.isEmpty()) {
        kError()<<"No language dictionaries for the language : "
                << plang <<endl;
        return 0;
    }

    QListIterator<Client*> itr(lClients);
    while (itr.hasNext()) {
        Client* item = itr.next();
        if (!pclient.isEmpty()) {
            if (pclient == item->name()) {
                SpellerPlugin *dict = item->createSpeller(plang);
                return dict;
            }
        } else {
            //the first one is the one with the highest
            //reliability
            SpellerPlugin *dict = item->createSpeller(plang);
            return dict;
        }
    }

    return 0;
}

QStringList Loader::clients() const
{
    return d->clients;
}

QStringList Loader::languages() const
{
    return d->languageClients.keys();
}

QString Loader::languageNameForCode(const QString &langCode) const
{
    QString currentDictionary = langCode,   // e.g. en_GB-ize-wo_accents
        lISOName,            // language ISO name
        cISOName,            // country ISO name
        variantName,         // dictionary variant name e.g. w_accents
        localizedLang,       // localized language
        localizedCountry;    // localized country
    QByteArray variantEnglish; // dictionary variant in English

    int underscorePos,     // position of "_" char
        minusPos,          // position of "-" char
        variantCount = 0;  // used to iterate over variantList

    struct variantListType
    {
        const char* variantShortName;
        const char* variantEnglishName;
    };

    const variantListType variantList[] = {
        { "40", I18N_NOOP2("dictionary variant", "40") }, // what does 40 mean?
        { "60", I18N_NOOP2("dictionary variant", "60") }, // what does 60 mean?
        { "80", I18N_NOOP2("dictionary variant", "80") }, // what does 80 mean?
        { "ise", I18N_NOOP2("dictionary variant", "-ise suffixes") },
        { "ize", I18N_NOOP2("dictionary variant", "-ize suffixes") },
        { "ise-w_accents", I18N_NOOP2("dictionary variant", "-ise suffixes and with accents") },
        { "ise-wo_accents", I18N_NOOP2("dictionary variant", "-ise suffixes and without accents") },
        { "ize-w_accents", I18N_NOOP2("dictionary variant", "-ize suffixes and with accents") },
        { "ize-wo_accents", I18N_NOOP2("dictionary variant", "-ize suffixes and without accents") },
        { "lrg", I18N_NOOP2("dictionary variant", "large") },
        { "med", I18N_NOOP2("dictionary variant", "medium") },
        { "sml", I18N_NOOP2("dictionary variant", "small") },
        { "variant_0", I18N_NOOP2("dictionary variant", "variant 0") },
        { "variant_1", I18N_NOOP2("dictionary variant", "variant 1") },
        { "variant_2", I18N_NOOP2("dictionary variant", "variant 2") },
        { "wo_accents", I18N_NOOP2("dictionary variant", "without accents") },
        { "w_accents", I18N_NOOP2("dictionary variant", "with accents") },
        { "ye", I18N_NOOP2("dictionary variant", "with ye") },
        { "yeyo", I18N_NOOP2("dictionary variant", "with yeyo") },
        { "yo", I18N_NOOP2("dictionary variant", "with yo") },
        { "extended", I18N_NOOP2("dictionary variant", "extended") },
        { 0, 0 }
    };

    minusPos = currentDictionary.indexOf("-");
    underscorePos = currentDictionary.indexOf("_");
    if (underscorePos != -1 && underscorePos <= 3) {
        cISOName = currentDictionary.mid(underscorePos + 1, 2);
        lISOName = currentDictionary.left(underscorePos);
        if ( minusPos != -1 )
            variantName = currentDictionary.right(
                                     currentDictionary.length() - minusPos - 1);
    }  else {
        if ( minusPos != -1 ) {
            variantName = currentDictionary.right(
                                     currentDictionary.length() - minusPos - 1);
            lISOName = currentDictionary.left(minusPos);
        }
        else
            lISOName = currentDictionary;
    }
    localizedLang = KGlobal::locale()->languageCodeToName(lISOName);
    if (localizedLang.isEmpty())
        localizedLang = lISOName;
    if (!cISOName.isEmpty()) {
        if (!KGlobal::locale()->countryCodeToName(cISOName).isEmpty())
            localizedCountry = KGlobal::locale()->countryCodeToName(cISOName);
        else
            localizedCountry = cISOName;
    }
    if (!variantName.isEmpty()) {
        while (variantList[variantCount].variantShortName != 0)
            if (variantList[ variantCount ].variantShortName ==
                variantName)
                break;
            else
                variantCount++;
        if (variantList[variantCount].variantShortName != 0)
            variantEnglish = variantList[variantCount].variantEnglishName;
        else
            variantEnglish = variantName.toLatin1();
    }
    if (!cISOName.isEmpty() && !variantName.isEmpty())
        return i18nc(
                    "dictionary name. %1-language, %2-country and %3 variant name",
                    "%1 (%2) [%3]", localizedLang, localizedCountry,
                    i18nc( "dictionary variant", variantEnglish));
    else if (!cISOName.isEmpty())
        return i18nc(
                        "dictionary name. %1-language and %2-country name",
                        "%1 (%2)", localizedLang, localizedCountry);
    else if (!variantName.isEmpty())
        return i18nc(
                            "dictionary name. %1-language and %2-variant name",
                            "%1 [%2]", localizedLang,
                            i18nc("dictionary variant", variantEnglish));
    else
        return localizedLang;
}

QStringList Loader::languageNames() const
{
    /* For whatever reason languages() might change. So,
     * to be in sync with it let's do the following check.
     */
    if (d->languagesNameCache.count() == languages().count() )
        return d->languagesNameCache;

    QStringList allLocalizedDictionaries;
    const QStringList allDictionaries = languages();

    for (QStringList::ConstIterator it = allDictionaries.begin();
         it != allDictionaries.end(); ++it) {
        allLocalizedDictionaries.append(languageNameForCode(*it));
    }
    // cache the list
    d->languagesNameCache = allLocalizedDictionaries;
    return allLocalizedDictionaries;
}

Settings* Loader::settings() const
{
    return d->settings;
}

void Loader::loadPlugins()
{
    d->plugins = KServiceTypeTrader::self()->query("Sonnet/SpellClient");

    for (KService::List::const_iterator itr = d->plugins.constBegin();
         itr != d->plugins.constEnd(); ++itr ) {
        loadPlugin((*itr));
    }
}

void Loader::loadPlugin(const KSharedPtr<KService> &service)
{
    QString error;

    Client *client = service->createInstance<Client>(this,
                                                      QVariantList(),
                                                      &error);

    if (client) {
        const QStringList languages = client->languages();
        d->clients.append(client->name());

        for (QStringList::const_iterator itr = languages.begin();
             itr != languages.end(); ++itr) {
            if (!d->languageClients[*itr].isEmpty() &&
                client->reliability() <
                d->languageClients[*itr].first()->reliability())
                d->languageClients[*itr].append(client);
            else
                d->languageClients[*itr].prepend(client);
        }

        //kDebug() << "Successfully loaded plugin:" << service->entryPath();
    } else {
        kDebug() << error;
    }
}

void Loader::changed()
{
    emit configurationChanged();
}

}

#include "loader_p.moc"
