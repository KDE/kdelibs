/*
    This file is part of KNewStuff2.
    Copyright (c) 2005 - 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF2_UI_KDXSBUTTON_H
#define KNEWSTUFF2_UI_KDXSBUTTON_H

#include <QtGui/QToolButton>
#include "knewstuff2/core/entry.h"
class KMenu;
class QAction;
class KUrl;

namespace KNS
{

class Dxs;
class Entry;
class Provider;
class DxsEngine;
class Category;
class Feed;
/**
 * KNewStuff DXS interaction button.
 *
 * This button and its associated popup menu appear beside each item
 * in the download dialog. The user may use it to perform all the actions
 * on the item including installation, uninstallation and collaborative
 * actions.
 *
 * This class is used internally by the DownloadDialog class.
 *
 * @internal
 */
class KDXSButton : public QToolButton
{
    Q_OBJECT
public:
    KDXSButton(QWidget *parent);
    ~KDXSButton();

    void setEntry(KNS::Entry *e);
    void setProvider(const KNS::Provider *provider);
    void setEngine(KNS::DxsEngine *engine);

public Q_SLOTS:
    void slotTriggered(QAction *action);
    void slotVersionsActivated(int id);
    void slotVersionsHighlighted(int id);
    void slotClicked();

    void slotInfo(QString provider, QString server, QString version);
    void slotCategories(QList<KNS::Category*> categories);
    void slotEntries(QList<KNS::Entry*> entries);
    void slotComments(QStringList comments);
    void slotHistory(QStringList entries);
    void slotChanges(QStringList entries);
    void slotRemoval(bool success);
    void slotSubscription(bool success);
    void slotComment(bool success);
    void slotRating(bool success);

    void slotFault();
    void slotError();

    void slotPayloadLoaded(KUrl url);
    void slotPayloadFailed(KNS::Entry *);

private:
    bool authenticate();

    // FIXME KDE4PORT item actions
    /*QAction *Items
    {
        install,
        uninstall,
        comments,
        changes,
        info,

        historysub,

        collabrating,
        collabcomment,
        collaboratesub,
        collabsubscribe,
        collabremoval,
        collabtranslation,

        contactsub,
        contactbymail,
        contactbyjabber
    };*/

    QAction *action_install;
    QAction *action_uninstall;
    QAction *action_comments;
    QAction *action_changes;
    QAction *action_info;

    QAction *action_historysub;

    QAction *action_collabrating;
    QAction *action_collabcomment;
    QAction *action_collaboratesub;
    QAction *action_collabsubscribe;
    QAction *action_collabremoval;
    QAction *action_collabtranslation;

    QAction *action_contactsub;
    QAction *action_contactbymail;
    QAction *action_contactbyjabber;

    enum States {
        historyinactive,
        historydisabled,
        historyslots
    };

    KNS::Dxs *m_dxs;
    KMenu *m_p, *m_history, *m_contact;

    KNS::Entry *m_entry;
    const KNS::Provider *m_provider;
    KNS::DxsEngine *m_engine;

    QString m_username;
    QString m_password;

    class Private;
    Private * const d;

    QWidget * m_parent;
};

}

#endif
