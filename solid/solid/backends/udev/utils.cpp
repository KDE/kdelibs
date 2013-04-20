/*************************************************************************************
 *  Copyright (C) 2013 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This library is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU Library General Public                      *
 *  License as published by the Free Software Foundation; either                     *
 *  version 2 of the License, or (at your option) any later version.                 *
 *                                                                                   *
 *  This library is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                *
 *  Library General Public License for more details.                                 *
 *                                                                                   *
 *  You should have received a copy of the GNU Library General Public License        *
 *  along with this library; see the file COPYING.LIB.  If not, write to             *
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,             *
 *  Boston, MA 02110-1301, USA.                                                      *
 *************************************************************************************/

#include "utils.h"

#include <QtCore/QList>


int Solid::Backends::UDev::input_str_to_bitmask(const QByteArray& value, long int* bitmask, size_t max_size)
{
    int i, j;
    int num_bits_set = 0;

    memset (bitmask, 0, max_size);
    QList<QByteArray> bits = value.split(' ');
    for (i = bits.length() - 1, j = 0; i >= 0; i--, j++) {
        unsigned long val;

        val = bits[i].toLong(0, 16);
        bitmask[j] = val;

        while (val != 0) {
            num_bits_set++;
            val &= (val - 1);
        }
    }

    return num_bits_set;
}