/**
 * kspell_aspellclient.h
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
#ifndef KSPELL_ASPELLCLIENT_H
#define KSPELL_ASPELLCLIENT_H

#include "client.h"
#include <qobject.h>

#include "aspell.h"

namespace KSpell2 {
    class Speller;
}
using KSpell2::Speller;

class ASpellClient : public KSpell2::Client
{
    Q_OBJECT
public:
    ASpellClient( QObject *parent, const QStringList & /* args */  );
    ~ASpellClient();

    virtual int reliability() const {
        return 20;
    }

    virtual Speller* createSpeller(const QString &language);

    virtual QStringList languages() const;

    virtual QString name() const {
        return "ASpell";
    }
private:
    AspellConfig *m_config;

};

#endif
