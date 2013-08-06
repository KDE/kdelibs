/*
 * texteditinstaller.h
 *
 * Copyright (C)  2013  Aurélien Gâteau <agateau@kde.org>
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
#include <texteditinstaller.h>

// Local
#include <highlighter.h>

// Qt
#include <QTextEdit>

namespace Sonnet
{

class TextEditInstaller::Private
{
public:
    void init(QTextEdit *textEdit)
    {
        m_highlighter = new Highlighter(textEdit);
    }

    Highlighter *m_highlighter;
};

TextEditInstaller::TextEditInstaller(QTextEdit *textEdit)
: QObject(textEdit)
, d(new Private)
{
    d->init(textEdit);
}

TextEditInstaller::~TextEditInstaller()
{
    delete d;
}

Highlighter *TextEditInstaller::highlighter() const
{
    return d->m_highlighter;
}

} // namespace

#include <texteditinstaller.moc>
