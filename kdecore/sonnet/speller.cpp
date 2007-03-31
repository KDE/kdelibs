// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * speller.cpp
 *
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
#include "speller.h"

namespace KSpell2
{

class Speller::Private
{
public:
    QString language;
};

Speller::Speller(const QString &lang)
    : d(new Private)
{
    d->language = lang;
}

Speller::~Speller()
{
    delete d; d = 0;
}

QString Speller::language() const
{
    return d->language;
}

bool Speller::isMisspelled(const QString &word) const
{
    return !isCorrect(word);
}

bool Speller::checkAndSuggest(const QString &word,
                              QStringList &suggestions) const
{
    bool c = isCorrect(word);
    if (!c)
        suggestions = suggest(word);
    return c;
}

}
