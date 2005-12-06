/**
 * kspell_hspellclient.h
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 * Copyright (C)  2005  Mashrab Kuvatov <kmashrab@uni-bremen.de>
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
#ifndef KSPELL_HSPELLCLIENT_H
#define KSPELL_HSPELLCLIENT_H

#include "client.h"
#include <qobject.h>

/* libhspell is a C library and it does not have #ifdef __cplusplus */
extern "C" {
#include "hspell.h"
}

namespace KSpell2 {
    class Dictionary;
}
using KSpell2::Dictionary;

class HSpellClient : public KSpell2::Client
{
    Q_OBJECT
public:
    HSpellClient( QObject *parent, const char *name, const QStringList & /* args */  );
    ~HSpellClient();

    virtual int reliability() const {
        return 20;
    }

    virtual Dictionary* dictionary( const QString& language );

    virtual QStringList languages() const;

    virtual QString name() const {
        return "HSpell";
    }
private:

};

#endif
