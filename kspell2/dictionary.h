// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * dictionary.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
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
#ifndef KSPELL_DICTIONARY_H
#define KSPELL_DICTIONARY_H

#include <qstringlist.h>
#include <qstring.h>

namespace KSpell2
{
    /**
     * Class is returned by from Broker. It acts
     * as the actual spellchecker.
     *
     * @author Zack Rusin <zack@kde.org>
     * @short class used for actuall spell checking
     */
    class Dictionary
    {
    public:
        virtual ~Dictionary() {}

        /**
         * Checks the given word.
         * @return false if the word is misspelled. true otherwise
         */
        virtual bool check( const QString& word ) =0;

        /**
         * Fetches suggestions for the word.
         *
         * @return list of all suggestions for the word
         */
        virtual QStringList suggest( const QString& word ) =0;

        /**
         * Checks the word and fetches suggestions for it.
         */
        virtual bool checkAndSuggest( const QString& word,
                                      QStringList& suggestions ) =0;

        /**
         * Stores user defined good replacement for the bad word.
         * @returns true on success
         */
        virtual bool storeReplacement( const QString& bad,
                                       const QString& good ) =0;

        /**
         * Adds word to the list of of personal words.
         * @return true on success
         */
        virtual bool addToPersonal( const QString& word ) =0;

        /**
         * Adds word to the words recognizable in the current session.
         * @return true on success
         */
        virtual bool addToSession( const QString& word ) =0;

        /**
         * Returns language supported by this dictionary.
         */
        QString language() const
        {
            return m_language;
        }

        /**
         * Returns true if this dictionary was constructed from
         * default Settings values
         */
        bool isDefault() const
        {
            return m_default;
        }

    protected:
        Dictionary( const QString& lang, bool def = false )
            : m_language( lang ), m_default( def ) {}
    protected:
        friend class Broker;
        QString m_language;
        bool    m_default;
    private:
        class Private;
        Private* d;
    };
}

#endif
