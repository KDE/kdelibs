/**
 * kspell_aspellclient.cpp
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
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "kspell_aspellclient.h"

#include "kspell_aspelldict.h"

#include <kgenericfactory.h>
#include <kdebug.h>

typedef KGenericFactory<ASpellClient> ASpellClientFactory;
K_EXPORT_COMPONENT_FACTORY( kspell_aspell, ASpellClientFactory( "kspell_aspell" )  )

using namespace KSpell2;

ASpellClient::ASpellClient( QObject *parent, const char *name, const QStringList& /* args */  )
    : Client( parent )
{
    m_config = new_aspell_config();
}

ASpellClient::~ASpellClient()
{
    delete_aspell_config( m_config );
}

Dictionary* ASpellClient::dictionary( const QString& language )
{
    ASpellDict *ad = new ASpellDict( language );
    return ad;
}

QStringList ASpellClient::languages() const
{
    AspellDictInfoList *l = get_aspell_dict_info_list( m_config );
    AspellDictInfoEnumeration *el = aspell_dict_info_list_elements( l );

    QStringList langs;
    const AspellDictInfo *di = 0;
    while ( ( di = aspell_dict_info_enumeration_next( el ) ) ) {
        langs.append( di->name );
    }

    delete_aspell_dict_info_enumeration( el );

    return langs;
}

#include "kspell_aspellclient.moc"
