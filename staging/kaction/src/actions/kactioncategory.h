/* This file is part of the KDE libraries

   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

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

#ifndef KACTIONCATEGORY_H
#define KACTIONCATEGORY_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>

#include "kstandardaction.h"
#include "kactioncollection.h"


class KAction;
struct KActionCategoryPrivate;

class QAction;



/**
 * Categorize actions for KShortcutsEditor.
 *
 * KActionCategory provides a second level to organize the actions in
 * KShortcutsEditor.
 *
 * The first possibility is using more than one action collection. Each
 * actions collection becomes a top level node.
 *
 * + action collection 1
 *   + first action
 *   + second action
 *   + third action
 * + action collection 2
 *   + first action
 *   + second action
 *   + third action
 *
 * Using KActionCategory it's possible to group the actions of one collection.
 * + action collection 1
 *   + first action
 *   + first category
 *     + action 1 in category
 *     + action 2 in category
 *   + second action
 *
 * \section Usage
 *
 * The usage is analog to action collections. Just create a category and use
 * it instead of the collection to create the actions.
 *
 * The synchronization between KActionCollection and KActionCategory is done
 * internally. There is for example no need to remove actions from a category.
 * It is done implicitely if the action is removed from the associated
 * collection.
 *
 * \code
 *
 * KActionCategory *file = new KActionCategory(i18n("File"), actionCollection());
 * file->addAction(
 *      KStandardAction::New,   //< see KStandardAction
 *      this,                   //< Receiver
 *      SLOT(fileNew()));       //< SLOT
 *
 * ... more actions added to file ...
 *
 * KActionCategory *edit = new KActionCategory(i18n("Edit"), actionCollection());
 * edit->addAction(
 *      KStandardAction::Copy,  //< see KStandardAction
 *      this,                   //< Receiver
 *      SLOT(fileNew()));       //< SLOT
 *
 * ...
 *
 * \endcode
 */
class KDEUI_EXPORT KActionCategory : public QObject
    {
    Q_OBJECT

    Q_PROPERTY( QString text READ text WRITE setText )

public:

    /**
     * Default constructor
     */
    explicit KActionCategory(const QString &text, KActionCollection *parent=NULL);

    /**
     * Destructor
     */
    virtual ~KActionCategory();

    /**
     * \name Adding Actions
     *
     * Add a action to the category.
     *
     * This methods are provided for your convenience. They call the
     * corresponding method of KActionCollection.
     */
    //@{
    QAction * addAction(const QString &name, QAction *action);

    KAction * addAction(const QString &name, KAction *action);

    KAction * addAction(
            KStandardAction::StandardAction actionType,
            const QObject *receiver = NULL,
            const char *member = NULL);

    KAction * addAction(
            KStandardAction::StandardAction actionType,
            const QString &name,
            const QObject *receiver = NULL,
            const char *member = NULL);

    KAction *addAction(
            const QString &name,
            const QObject *receiver = NULL,
            const char *member = NULL);

    template<class ActionType>
    ActionType *add(
            const QString &name,
            const QObject *receiver = NULL,
            const char *member = NULL)
        {
        ActionType *action = collection()->add<ActionType>(name, receiver, member);
        addAction(action);
        return action;
        }

    //@}

    /**
     * Returns the actions belonging to this category
      */
    const QList<QAction*> actions() const;

    /**
     * The action collection this category is associated with.
     */
    KActionCollection * collection() const;

    /**
     * The action categorys descriptive text
     */
    QString text() const;

    /**
     * Set the action categorys descriptive text.
     */
    void setText(const QString& text);

private:

    /**
     * Remove \action from this category if found.
     */
    void unlistAction(QAction *action);

    /**
     * Add action to category
     */
    void addAction(QAction *action);

    //! KActionCollection needs access to some of our helper methods
    friend class KActionCollectionPrivate;

    //! Implementation details
    KActionCategoryPrivate *const d;
};


#endif /* #ifndef KACTIONCATEGORY_H */
