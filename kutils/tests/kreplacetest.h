/*
    Copyright (C) 2002, David Faure <david@mandrakesoft.com>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KREPLACETEST_H
#define KREPLACETEST_H

#include <qobject.h>
#include <qstringlist.h>

class KReplace;

class KReplaceTest : public QObject
{
    Q_OBJECT
public:
    KReplaceTest( const QStringList& text, int button )
        : QObject( 0L ), m_text( text ), m_replace( 0 ), m_button( button ) {}

    void replace( const QString &pattern, const QString &replacement, long options );
    void print();
    const QStringList& textLines() const { return m_text; }

public Q_SLOTS:
    void slotHighlight( const QString &, int, int );
    void slotReplaceNext();
    void slotReplace(const QString &text, int replacementIndex, int replacedLength, int matchedLength);

Q_SIGNALS:
    void exitLoop();

private:
    void enterLoop();

    QStringList::Iterator m_currentPos;
    QStringList m_text;
    KReplace* m_replace;
    bool m_needEventLoop;
    int m_button;
};

#endif
