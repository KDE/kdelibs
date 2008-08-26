/* This file is part of the KDE project

   Copyright (C) 2005 Ivor Hewitt     <ivor@kde.org>
   Copyright (C) 2008 Maksim Orlovich <maksim@kde.org>
   Copyright (C) 2008 Vyacheslav Tokarev <tsjoker@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "khtml_filter_p.h"
#include <QDebug>

// rolling hash parameters
#define HASH_P (2003)
#define HASH_Q (8191)
// HASH_MOD = (HASH_P^7) % HASH_Q
#define HASH_MOD (598)

namespace khtml {

void FilterSet::addFilter(const QString& filterStr)
{
    QString filter = filterStr;
    
    if (filter.startsWith(QLatin1Char('!')))
        return;

    // Strip leading @@
    int first = 0;
    int last  = filter.length() - 1;
    if (filter.startsWith(QLatin1String("@@")))
        first = 2;
        
    // Strip options, we ignore them for now.
    int dollar = filter.lastIndexOf(QLatin1Char('$'));
    if (dollar != -1)
        last = dollar - 1;
    
    // Perhaps nothing left?
    if (first > last)
        return;
        
    filter = filter.mid(first, last - first + 1);
    
    // Is it a regexp filter?
    if (filter.length()>2 && filter.startsWith(QLatin1Char('/')) && filter.endsWith(QLatin1Char('/')))
    {
        QString inside = filter.mid(1, filter.length()-2);
        QRegExp rx(inside);
        reFilters.append(rx);
//         qDebug() << "R:" << inside;
    }
    else
    {
        // Nope, a wildcard one.
        // Note: For these, we also need to handle |.
        
        // Strip wildcards at the ends
        first = 0;
        last  = filter.length() - 1;
        
        while (first < filter.length() && filter[first] == QLatin1Char('*'))
            ++first;
            
        while (last >= 0 && filter[last] == QLatin1Char('*'))
            --last;
            
        if (first > last)
            filter = QLatin1String("*"); // erm... Well, they asked for it.
        else
            filter = filter.mid(first, last - first + 1);
            
        // Now, do we still have any wildcard stuff left?
        if (filter.contains("*") || filter.contains("?")) 
        {
//             qDebug() << "W:" << filter;
            QRegExp rx;

            rx.setPatternSyntax(QRegExp::Wildcard);
            rx.setPattern(filter);
            reFilters.append(rx);        
        }
        else
        {
            // Fast path
            stringFiltersMatcher.addString(filter);
        }
    }
}

bool FilterSet::isUrlMatched(const QString& url)
{
    if (stringFiltersMatcher.isMatched(url))
        return true;

    for (int c = 0; c < reFilters.size(); ++c)
    {
        if (url.contains(reFilters[c]))
            return true;
    }

    return false;
}

void FilterSet::clear()
{
    reFilters.clear();
    stringFiltersMatcher.clear();
}


void StringsMatcher::addString(const QString& pattern)
{
    if (pattern.length() < 8) {
        // hanlde short string differently
        shortStringFilters.append(pattern);
    } else {
        // use modified Rabin-Karp's algorithm with 8-length string hash
        // i.e. store hash of first 8 chars in the HashMap for fast look-up
        stringFilters.append(pattern);
        int ind = stringFilters.size() - 1;
        int current = 0;

        // compute hash using rolling hash
        // hash for string: x0,x1,x2...xn-1 will be:
        // (p^(n-1)*x0 + p^(n-2)*x1 + ... + p * xn-2 + xn-1) % q
        // where p and q some wisely-chosen integers
        for (int k = 0; k < 8; ++k)
            current = (current * HASH_P + pattern[k].unicode()) & HASH_Q;

        // insert computed hash value into HashMap
        WTF::HashMap<int, QVector<int> >::iterator it = stringFiltersHash.find(current + 1);
        if (it == stringFiltersHash.end()) {
            QVector<int> list;
            list.append(ind);
            stringFiltersHash.add(current + 1, list);
        } else {
            it->second.append(ind);
        }
    }
}

bool StringsMatcher::isMatched(const QString& str) const
{
    // check short strings first
    for (int i = 0; i < shortStringFilters.size(); ++i) {
        if (str.contains(shortStringFilters[i]))
            return true;
    }

    int len = str.length();
    int k;

    int current = 0;
    // compute hash for first 8 characters
    for (k = 0; k < 8 && k < len; ++k)
        current = (current * HASH_P + str[k].unicode()) & HASH_Q;

    WTF::HashMap<int, QVector<int> >::const_iterator hashEnd = stringFiltersHash.end();
    // main Rabin-Karp's algorithm loop
    for (k = 7; k < len; ++k) {
        // look-up the hash in the HashMap and check all strings
        WTF::HashMap<int, QVector<int> >::const_iterator it = stringFiltersHash.find(current + 1);

        // check possible strings
        if (it != hashEnd) {
            for (int j = 0; j < it->second.size(); ++j) {
                int index = it->second[j];
                int flen = stringFilters[index].length();
                if (k - 8 + flen < len && stringFilters[index] == str.midRef(k - 7, flen))
                    return true;
            }
        }

        // roll the hash if not at the end
        if (k + 1 < len)
            current = (HASH_P * ((current + HASH_Q + 1 - ((HASH_MOD * str[k - 7].unicode()) & HASH_Q)) & HASH_Q) + str[k + 1].unicode()) & HASH_Q;
    }

    return false;
}

}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
