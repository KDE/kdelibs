// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * broker.cpp
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
#include "broker.h"
#include "settings.h"
#include "client.h"
#include "defaultdictionary.h"
#include "dictionary.h"

#include <klocale.h>
#include <kparts/plugin.h>
#include <kparts/componentfactory.h>

#include <kplugininfo.h>
#include <ktrader.h>
#include <kconfig.h>

#include <kdebug.h>

#include <qhash.h>
#include <qmap.h>
#include <Q3PtrList>

#define DEFAULT_CONFIG_FILE   "kspellrc"

namespace KSpell2
{

class Broker::Private
{
public:
    KPluginInfo::List plugins;
    Settings *settings;

    // <language, Clients with that language >
    QMap<QString, QList<Client*> > languageClients;
    QStringList clients;
    DefaultDictionary *defaultDictionary;
};

// TODO KStaticDeleter
static QHash<KSharedConfig*, Broker*> *s_brokers = 0;

Broker::Ptr Broker::openBroker( KSharedConfig *config )
{
    KSharedConfig::Ptr preventDeletion;
    if ( !config ) {
        preventDeletion = KSharedConfig::openConfig( DEFAULT_CONFIG_FILE );
    } else
        preventDeletion = config;

    if ( s_brokers ) {
        Broker *broker = s_brokers->value( preventDeletion.data() );
        if ( broker )
            return Ptr( broker );
    }

    Broker *broker = new Broker( preventDeletion.data() );
    return Ptr( broker );
}

Broker::Broker( KSharedConfig *config )
	:d(new Private)
{
    KSharedConfig::Ptr preventDeletion( config );
    Q_UNUSED( preventDeletion );

    if ( !s_brokers )
        s_brokers = new QHash<KSharedConfig*, Broker*>;
    s_brokers->insert( config, this );

    d->settings = new Settings( this, config );
    loadPlugins();

    d->defaultDictionary = new DefaultDictionary( d->settings->defaultLanguage(),
                                                  this );
}

Broker::~Broker()
{
    kDebug()<<"Removing broker : "<< this << endl;
    s_brokers->remove( d->settings->sharedConfig() );
    KPluginInfo::List::iterator it = d->plugins.begin();
    while ( it != d->plugins.end() ) {
        KPluginInfo *pluginInfo = *it;
        it = d->plugins.erase( it );
        delete pluginInfo;
    }

    delete d->settings; d->settings = 0;
    delete d;
}

DefaultDictionary* Broker::defaultDictionary() const
{
    return d->defaultDictionary;
}

Dictionary* Broker::dictionary( const QString& language, const QString& clientName ) const
{
    QString pclient = clientName;
    QString plang   = language;
    bool ddefault = false;

    if ( plang.isEmpty() ) {
        plang = d->settings->defaultLanguage();
    }
    if ( clientName == d->settings->defaultClient() &&
        plang == d->settings->defaultLanguage() ) {
        ddefault = true;
    }

    QList<Client*> lClients = d->languageClients[ plang ];

    if ( lClients.isEmpty() ) {
        kError()<<"No language dictionaries for the language : "<< plang <<endl;
        return 0;
    }

    QListIterator<Client*> itr( lClients );
    while ( itr.hasNext() ) {
			Client* item = itr.next();
        if ( !pclient.isEmpty() ) {
            if ( pclient == item->name() ) {
                Dictionary *dict = item->dictionary( plang );
                if ( dict ) //remove the if if the assert proves ok
                    dict->m_default = ddefault;
                return dict;
            }
        } else {
            //the first one is the one with the highest
            //reliability
            Dictionary *dict = item->dictionary( plang );
            Q_ASSERT( dict );
            if ( dict ) //remove the if if the assert proves ok
                dict->m_default = ddefault;
            return dict;
        }
    }

    return 0;
}

QStringList Broker::clients() const
{
    return d->clients;
}

QStringList Broker::languages() const
{
    return d->languageClients.keys();
}

QStringList Broker::languagesName() const
{
    /* For whatever reason languages() might change. So,
     * to be in sync with it let's do the following check.
     */
    if (languagesNameCache.count() == languages().count() )
	    return languagesNameCache;

    QStringList allLocalizedDictionaries;
    QStringList allDictionaries = languages();
    QString currentDictionary,   // e.g. en_GB-ize-wo_accents
            lISOName,            // language ISO name
            cISOName,            // country ISO name
            variantName,         // dictionary variant name e.g. w_accents
            localizedLang,       // localized language
            localizedCountry;    // localized country
    const char*  variantEnglish; // dictionary variant in English

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

    for ( QStringList::Iterator it = allDictionaries.begin(); it != allDictionaries.end(); ++it ) {
        currentDictionary = *it;
        minusPos = currentDictionary.find( "-" );
      	underscorePos = currentDictionary.find( "_" );
	if ( underscorePos != -1 && underscorePos <= 3 ) {
           cISOName = currentDictionary.mid( underscorePos + 1, 2 );
           lISOName = currentDictionary.left( underscorePos );
           if ( minusPos != -1 )
              variantName = currentDictionary.right( currentDictionary.length() - minusPos - 1 );
	}  else {
              if ( minusPos != -1 ) {
                  variantName = currentDictionary.right( currentDictionary.length() - minusPos - 1 );
                  lISOName = currentDictionary.left( minusPos );
              }
              else
                 lISOName = currentDictionary;
              }
        localizedLang = KGlobal::locale()->twoAlphaToLanguageName( lISOName );
        if ( localizedLang.isEmpty() )
           localizedLang = lISOName;
	if ( !cISOName.isEmpty() )
           if ( !KGlobal::locale()->twoAlphaToCountryName( cISOName ).isEmpty() )
              localizedCountry = KGlobal::locale()->twoAlphaToCountryName( cISOName );
           else
              localizedCountry = cISOName;
	if ( !variantName.isEmpty() ) {
	   while ( variantList[ variantCount ].variantShortName != 0 )
		  if ( variantList[ variantCount ].variantShortName == variantName )
			 break;
	          else
			 variantCount++;
	   if ( variantList[ variantCount ].variantShortName != 0 )
           	variantEnglish = variantList[ variantCount ].variantEnglishName;
	   else
           	variantEnglish = variantName.latin1();
	}
	if ( !cISOName.isEmpty() && !variantName.isEmpty() )
             allLocalizedDictionaries.append( i18nc(
				    "dictionary name. %1-language, %2-country and %3 variant name",
				    "%1 (%2) [%3]", localizedLang, localizedCountry,
				                    i18nc( "dictionary variant", variantEnglish ) ) );
	else
	     if ( !cISOName.isEmpty() )
                  allLocalizedDictionaries.append( i18nc(
					 "dictionary name. %1-language and %2-country name",
					 "%1 (%2)", localizedLang, localizedCountry ) );
             else
	        if ( !variantName.isEmpty() )
            	     allLocalizedDictionaries.append( i18nc(
					    "dictionary name. %1-language and %2-variant name",
					    "%1 [%2]", localizedLang,
						       i18nc( "dictionary variant", variantEnglish ) ) );
		else
                  allLocalizedDictionaries.append( localizedLang );
	lISOName = cISOName = variantName = "";
	variantCount = 0;
    }
    // cache the list
    languagesNameCache = allLocalizedDictionaries;
    return allLocalizedDictionaries;
}

Settings* Broker::settings() const
{
    return d->settings;
}

void Broker::loadPlugins()
{
    d->plugins = KPluginInfo::fromServices(
        KTrader::self()->query( "KSpell/Client" ) );

    for ( KPluginInfo::List::Iterator itr = d->plugins.begin();
          itr != d->plugins.end(); ++itr ) {
        loadPlugin( ( *itr )->pluginName() );
    }
}

void Broker::loadPlugin( const QString& pluginId )
{
    int error = 0;

    kDebug()<<"Loading plugin " << pluginId << endl;

    Client *client = KParts::ComponentFactory::createInstanceFromQuery<Client>(
        QLatin1String( "KSpell/Client" ),
        QString::fromLatin1( "[X-KDE-PluginInfo-Name]=='%1'" ).arg( pluginId ),
        this, 0, QStringList(), &error );

    if ( client )
    {
        QStringList languages = client->languages();
        d->clients.append( client->name() );

        for ( QStringList::Iterator itr = languages.begin();
              itr != languages.end(); ++itr ) {
            if ( !d->languageClients[ *itr ].isEmpty() &&
                 client->reliability() < d->languageClients[ *itr ].first()->reliability() )
                d->languageClients[ *itr ].append( client );
            else
                d->languageClients[ *itr ].prepend( client );
        }

        kDebug() << k_funcinfo << "Successfully loaded plugin '"
                  << pluginId << "'" << endl;
    }
    else
    {
        switch( error )
        {
        case KParts::ComponentFactory::ErrNoServiceFound:
            kDebug() << k_funcinfo << "No service implementing the given mimetype "
                      << "and fullfilling the given constraint expression can be found."
                      << endl;
            break;
        case KParts::ComponentFactory::ErrServiceProvidesNoLibrary:
            kDebug() << "the specified service provides no shared library." << endl;
            break;
        case KParts::ComponentFactory::ErrNoLibrary:
            kDebug() << "the specified library could not be loaded." << endl;
            break;
        case KParts::ComponentFactory::ErrNoFactory:
            kDebug() << "the library does not export a factory for creating components."
                      << endl;
            break;
        case KParts::ComponentFactory::ErrNoComponent:
            kDebug() << "the factory does not support creating "
                      << "components of the specified type."
                      << endl;
            break;
        }

        kDebug() << k_funcinfo << "Loading plugin '" << pluginId
                  << "' failed, KLibLoader reported error: '" << endl
                  << KLibLoader::self()->lastErrorMessage() << "'" << endl;
    }
}

void Broker::changed()
{
    emit configurationChanged();
}

}

#include "broker.moc"
