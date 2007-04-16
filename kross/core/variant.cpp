/***************************************************************************
 * variant.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "variant.h"

#include <QObject>
#include <QColor>
#include <QBrush>
#include <QFont>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QMetaObject>
#include <QMetaEnum>

using namespace Kross;

/************************************************************************
 * Color
 */

Color::Color(QObject* parent, const QColor& color)
    : QObject(parent)
    , m_color(color)
{
}

Color::~Color()
{
}

/************************************************************************
 * Brush
 */

Brush::Brush(QObject* parent, const QBrush& brush)
    : QObject(parent)
    , m_brush(brush)
{
}

Brush::~Brush()
{
}

/************************************************************************
 * Font
 */

Font::Font(QObject* parent, const QFont& font)
    : QObject(parent)
    , m_font(font)
{
}

Font::~Font()
{
}

/************************************************************************
 * DateTime
 */

DateTime::DateTime(QObject* parent, const QDateTime& datetime)
    : QObject(parent)
    , m_datetime(datetime)
{
}

DateTime::~DateTime()
{
}

#include "variant.moc"
