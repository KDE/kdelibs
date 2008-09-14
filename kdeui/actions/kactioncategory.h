#ifndef KACTIONCATEGORY_H
#define KACTIONCATEGORY_H
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

#include <QtCore/QObject>
#include <QtCore/QString>

#include "kstandardaction.h"
#include "kactioncollection.h"


class KAction;
class KActionCategoryPrivate;

class QAction;



/**
 * TODO
 */
class KDEUI_EXPORT KActionCategory : public QObject
    {
    Q_OBJECT

    Q_PROPERTY( QString text READ text WRITE setText )

public:

    /**
     * Default constructor
     */
    KActionCategory(const QString &text, KActionCollection *parent=NULL);

    /**
     * Destructor
     */
    virtual ~KActionCategory();

    /**
     * \group Adding Actions
     *
     * Add a actions to the category.
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

    //! Implementation details
    KActionCategoryPrivate *d;
};


#endif /* #ifndef KACTIONCATEGORY_H */
