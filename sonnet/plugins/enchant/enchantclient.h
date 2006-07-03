// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * Copyright 2006  Zack Rusin <zack@kde.org>
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
#ifndef QSPELL_ENCHANTCLIENT_H
#define QSPELL_ENCHANTCLIENT_H

#include "speller.h"
#include "client.h"

#include <qobject.h>
#include <qset.h>
#include <enchant.h>

class QSpellEnchantClient : public KSpell2::Client
{
    Q_OBJECT
    Q_INTERFACES(QSpell::Client)
public:
    QSpellEnchantClient(QObject *parent=0);
    ~QSpellEnchantClient();

    virtual int reliability() const {
        return 30;
    }

    virtual QSpell::Speller *createSpeller(const QString &language);

    virtual QStringList languages() const;

    virtual QString name() const {
        return QString::fromLatin1("Enchant");
    }

    void addLanguage(const QString &lang);

private:
    EnchantBroker *m_broker;
    QSet<QString>  m_languages;
};

#endif
