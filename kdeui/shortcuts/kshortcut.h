/*  This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2006 Andreas Hartmetz <ahartmetz@gmail.com>

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

/**
 * @file kshortcut.h
 * Defines platform-independent classes for keyboard shortcut handling.
 */

#ifndef KSHORTCUT_H
#define KSHORTCUT_H

#include "kdelibs_export.h"

#include <QtCore/QList>
#include <QtGui/QKeySequence>

class KShortcutPrivate;

/**
* @short Represents a keyboard shortcut
*
* The KShortcut class is used to represent a keyboard shortcut to an action.
* A shortcut is normally a single key with modifiers, such as Ctrl+V.
* A KShortcut object may also contain an alternate key sequence which will also
* activate the action it's associated to, as long as no other actions have
* defined that key as their primary key. Ex: Ctrl+V;Shift+Insert.
*
* This can be used to add additional accelerators to a KAction. For example,
* the below code binds the escape key to the close action.
*
* \code
*  KAction *closeAction = KStdAction::close(this, SLOT( close() ), actionCollection());
*  KShortcut closeShortcut = closeAction->shortcut();
*  closeShortcut.setAlternate(Qt::Key_Escape);
*  closeAction->setShortcut(closeShortcut);
* \endcode
*/
class KDEUI_EXPORT KShortcut : public QList<QKeySequence>
{
public:
    /**
     * Creates a new empty shortcut.
     * @see isEmpty()
     * @see clear()
     */
    KShortcut();

    /**
     * Creates a new shortcut that contains the given Qt key
     * sequence as primary shortcut.
     * @param primary Qt key sequence to add
     */
    explicit KShortcut(const QKeySequence &primary);

    /**
     * Creates a new shortcut with the given Qt key sequences
     * as primary and secondary shortcuts.
     * @param primary Qt keycode of primary shortcut
     * @param alternate Qt keycode of alternate shortcut
     * @see Qt::Key
     */
    KShortcut(const QKeySequence &primary, const QKeySequence &alternate);

    /**
     * Creates a new shortcut with the given Qt key codes
     * as primary and secondary shortcuts.
     * You can only assign single-key shortcuts this way.
     * @param keyQtPri Qt keycode of primary shortcut
     * @param keyQtAlt Qt keycode of alternate shortcut
     * @see Qt::Key
     */
    explicit KShortcut(int keyQtPri, int keyQtAlt = 0);

    /**
     * Creates a new shortcut that contains the key sequences described
     * in @p description. The format of description is the same as
     * used in QKeySequence::fromString(const QString&).
     * Up to two key sequences separated by a semicolon ";" may be given.
     * @param description the description of key sequence(s)
     * @see QKeySequence::fromString(const QString&, SequenceFormat)
     */
    explicit KShortcut(const QString &description);

    /**
     * Creates a new shortcut with the given Qt key sequences.
     * The first sequence in the list is considered to be the primary
     * sequence, the second one the alternate.
     * @param seqs List of key sequeces.
     */
    explicit KShortcut(const QList<QKeySequence> &seqs);

    /** @name Query methods */
    /** @{ */

    /**
     * Returns the primary key sequence of this shortcut.
     * @return primary key sequence
     */
    inline QKeySequence primary() const
    { return value(0); }

    /**
     * Returns the alternate key sequence of this shortcut.
     * @return alternate key sequence
     */
    inline QKeySequence alternate() const
    { return value(1); }

    /** @} */
    /** @name Mutator methods */
    /** @{ */

    /**
     * Set the primary key sequence of this shortcut to the given key sequence.
     * @param keySeq set primary key sequence to this
     */
    void setPrimary(const QKeySequence &keySeq);

    /**
     * Set the alternate key sequence of this shortcut to the given key sequence.
     * @param keySeq set alternate key sequence to this
     */
    void setAlternate(const QKeySequence &keySeq);

    /**
     * Returns a description of the shortcut as a semicolon-separated
     * list of key sequences, as returned by QKeySequence::toString().
     * @return the string represenation of this shortcut
     * @see QKeySequence::toString()
     * @see KShortcut(const QString &description)
     */
    QString toString() const;
};

uint qHash(int);
inline uint qHash(const KShortcut &key)
{
    return qHash(key.primary()[0]) + qHash(key.primary()[1]);
}

inline uint qHash(const QKeySequence &key)
{
    uint hash = 0;
    for(uint i = 0; i < key.count(); i++)
       hash += qHash(key[i]);
    return hash;
}

#endif // KSHORTCUT_H
