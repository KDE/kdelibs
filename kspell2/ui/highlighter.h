/**
 * highlighter.h
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
#ifndef KSPELL_HIGHLIGHTER_H
#define KSPELL_HIGHLIGHTER_H

#include "filter.h"

#include <q3syntaxhighlighter.h>

class Q3TextEdit;

namespace KSpell2
{
    class Highlighter : public Q3SyntaxHighlighter
    {
    public:
        Highlighter( Q3TextEdit *textEdit,
                     const QString& configFile = QString::null,
                     Filter *filter = Filter::defaultFilter() );
        ~Highlighter();

        virtual int highlightParagraph( const QString& text,
                                        int endStateOfLastPara );

        Filter *currentFilter() const;
        void setCurrentFilter( Filter *filter );

        QString currentLanguage() const;
        void setCurrentLanguage( const QString& lang );

    protected:
        virtual void setMisspelled( int start, int count );
        virtual void unsetMisspelled( int start,  int count );
    private:
        class Private;
        Private *d;
    };

}

#endif
