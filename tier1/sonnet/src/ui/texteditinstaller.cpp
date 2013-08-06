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
#include <QContextMenuEvent>
#include <QMenu>
#include <QTextEdit>

namespace Sonnet
{

class TextEditInstaller::Private
{
public:
    Private(TextEditInstaller *installer, QTextEdit *textEdit)
    : q(installer)
    , m_textEdit(textEdit)
    , m_highlighter(new Highlighter(textEdit))
    {
        // Catch pressing the "menu" key
        m_textEdit->installEventFilter(q);
        // Catch right-click
        m_textEdit->viewport()->installEventFilter(q);
    }

    void onContextMenuEvent(QContextMenuEvent *event)
    {
        QMenu *menu = m_textEdit->createStandardContextMenu(event->globalPos());
        menu->addAction(tr("My Menu Item"));
        menu->exec(event->globalPos());
        delete menu;
    }

    TextEditInstaller *q;
    QTextEdit *m_textEdit;
    Highlighter *m_highlighter;
};

TextEditInstaller::TextEditInstaller(QTextEdit *textEdit)
: QObject(textEdit)
, d(new Private(this, textEdit))
{
}

TextEditInstaller::~TextEditInstaller()
{
    delete d;
}

Highlighter *TextEditInstaller::highlighter() const
{
    return d->m_highlighter;
}

bool TextEditInstaller::eventFilter(QObject * /*obj*/, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu) {
        d->onContextMenuEvent(static_cast<QContextMenuEvent *>(event));
        return true;
    }
    return false;
}

} // namespace

#include <texteditinstaller.moc>
