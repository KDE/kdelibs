/*
 *
 *
 * Implementation of KRestrictedLine
 *
 * Copyright (C) 1997 Michael Wiedmann, <mw@miwie.in-berlin.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "krestrictedline.h"
#include <kdebug.h>

#include <QtGui/QKeyEvent>

class KRestrictedLinePrivate
{
public:
  /// QString of valid characters for this line
  QString qsValidChars;
};

KRestrictedLine::KRestrictedLine( QWidget *parent )
  : KLineEdit( parent )
  , d( new KRestrictedLinePrivate )
{
}

KRestrictedLine::~KRestrictedLine()
{
    delete d;
}


void KRestrictedLine::keyPressEvent( QKeyEvent *e )
{
    // let KLineEdit process "special" keys and return/enter
    // so that we still can use the default key binding
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return || e->key() == Qt::Key_Delete
        || e->key() == Qt::Key_Backspace
        || (e->modifiers() & (Qt::ControlModifier | Qt::AltModifier
                              | Qt::MetaModifier | Qt::GroupSwitchModifier)))
    {
        KLineEdit::keyPressEvent(e);
        return;
    }

    // do we have a list of valid chars &&
    // is the pressed key in the list of valid chars?
    if (!d->qsValidChars.isEmpty() && !d->qsValidChars.contains(e->text())) {
        // invalid char, emit signal and return
        emit invalidChar(e->key());
    } else {
        // valid char: let KLineEdit process this key as usual
        KLineEdit::keyPressEvent(e);
    }
}

void KRestrictedLine::inputMethodEvent(QInputMethodEvent *e)
{
    const QString str = e->commitString();
    if (!d->qsValidChars.isEmpty() && !str.isEmpty()) {
        bool allOK = true;
        Q_FOREACH(QChar ch, str) {
            if (!d->qsValidChars.contains(ch)) {
                emit invalidChar(ch.unicode());
                allOK = false;
            }
        }
        // ## we can't remove invalid chars from the string, however.
        // we really need a validator (with a different signal like invalidChar(QChar)
        // or invalidCharacters(QString) maybe.

        if (!allOK)
            return;
    }

    KLineEdit::inputMethodEvent(e);
}

void KRestrictedLine::setValidChars( const QString& valid)
{
  d->qsValidChars = valid;
}

QString KRestrictedLine::validChars() const
{
  return d->qsValidChars;
}

#include "krestrictedline.moc"
