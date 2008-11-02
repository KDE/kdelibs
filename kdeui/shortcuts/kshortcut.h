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

#include <kdeui_export.h>

#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtGui/QKeySequence>

class KShortcutPrivate;

/**
* @short Represents a keyboard shortcut
*
* The KShortcut class is used to represent a keyboard shortcut to an action.
* A shortcut is normally a single key with modifiers, such as Ctrl+V.
* A KShortcut object may also contain an alternate key sequence which will also
* activate the action it's associated with, as long as no other actions have
* defined that key as their primary key. Ex: Ctrl+V;Shift+Insert.
*
* This can be used to add additional accelerators to a KAction. For example,
* the below code binds the escape key to the close action.
*
* \code
*  KAction *closeAction = KStandardAction::close(this, SLOT( close() ), actionCollection());
*  KShortcut closeShortcut = closeAction->shortcut();
*  closeShortcut.setAlternate(Qt::Key_Escape);
*  closeAction->setShortcut(closeShortcut);
* \endcode
*/
class KDEUI_EXPORT KShortcut
{
public:
    /**
     * An enum about the behavior of operations that treat a KShortcut like a list of QKeySequences.
     */
    enum EmptyHandling {
        ///if a shortcut is or becomes empty, let it stay as a placeholder
        KeepEmpty = 0,
        ///remove empty QKeySequences, possibly changing the positions of QKeySequences due to the ensuing reshuffling.
        RemoveEmpty
    };

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
     * Copy constructor.
     */
    KShortcut(const KShortcut &other);

    /**
     * Creates a new shortcut that contains the key sequences described
     * in @p description. The format of description is the same as
     * used in QKeySequence::fromString(const QString&).
     * Up to two key sequences separated by a semicolon followed by a
     * space "; " may be given.
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

    /**
     * Destructor.
     */
    ~KShortcut();

    /** @name Query methods */
    /** @{ */

    /**
     * Returns the primary key sequence of this shortcut.
     * @return primary key sequence
     */
    QKeySequence primary() const;

    /**
     * Returns the alternate key sequence of this shortcut.
     * @return alternate key sequence
     */
    QKeySequence alternate() const;

    /**
     * Returns whether this shortcut contains any nonempty key sequences.
     * @return whether this shortcut is empty
     */
    bool isEmpty() const;

    /**
     * Returns whether at least one of the key sequences is equal to @p needle.
     * @return whether this shortcut contains @p needle
     */
    bool contains(const QKeySequence &needle) const;

    /**
     * Returns whether at least one of the key sequences conflicts witho @p needle.
     * @return whether this shortcut conflicts with @p needle
     */
    bool conflictsWith(const QKeySequence &needle) const;

   /**
     * Returns a description of the shortcut as a semicolon-separated
     * list of key sequences, as returned by QKeySequence::toString().
     * @return the string represenation of this shortcut
     * @see QKeySequence::toString()
     * @see KShortcut(const QString &description)
     */
    QString toString() const;

    /**
     * Returns a description of the shortcut as a semicolon-separated
     * list of key sequences, as returned by QKeySequence::toString().
     * @return the string represenation of this shortcut
     * @see QKeySequence::toString()
     * @see KShortcut(const QString &description)
     * @since KDE 4.2
     */
    QString toString(QKeySequence::SequenceFormat format) const;

    bool operator==(const KShortcut &other) const;

    bool operator!=(const KShortcut &other) const;

    /**
     * Returns shortcut as QList\<QKeySequence\>, and is equivalent to toList(RemoveEmpty).
     * Be aware that empty shortcuts will not be included in the list;
     * due to this, conversion operations like
     * KShortcut b = (QList\<QKeySequence\>)KShortcut a
     * will not always result in b == a.
     * @return the shortcut converted to a QList\<QKeySequence\>
     */
    operator QList<QKeySequence>() const;

    /**
     * The same as operator QList\<QKeySequence\>()
     * If @p handleEmpty equals RemoveEmpty, empty key sequences will be left out of the result.
     * Otherwise, empy key sequences will be included; you can be sure that
     * shortcut.alternate() == shortcut.toList(KeepEmpty).at(1).
     * @return the shortcut converted to a QList\<QKeySequence\>
     */
    QList<QKeySequence> toList(enum EmptyHandling handleEmpty = RemoveEmpty) const;

    /**
     * Returns shortcut as QVariant.
     */
    operator QVariant() const;

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
     * Remove @p keySeq from this shortcut.
     * If @p handleEmpty equals RemoveEmpty, following key sequences will move up to take the place of
     * @p keySeq. Otherwise, key sequences equal to @p keySeq will be set to empty.
     * @param keySeq remove this key sequence from the shortcut
     */
    void remove(const QKeySequence &keySeq, enum EmptyHandling handleEmpty = RemoveEmpty);

    /**
     * Assignment operator.
     */
    KShortcut &operator=(const KShortcut &other);

private:
    class KShortcutPrivate *const d;
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

Q_DECLARE_METATYPE(KShortcut)

#endif // KSHORTCUT_H
