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

#include "kdxsbutton.h"

#include "knewstuff2/dxs/dxs.h"

#include "knewstuff2/core/entry.h"
#include "knewstuff2/core/category.h"

#include "downloaddialog.h"
#include "kdxsrating.h"
#include "kdxscomment.h"
#include "kdxscomments.h"
#include "kdxschanges.h"
#include "kdxstranslation.h"

#include <QtGui/QLayout>
#include <QtXml/qdom.h>
#include <QtGui/QToolButton>

#include <ktoolinvocation.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcursor.h>
#include <krun.h>

#include <kmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kprocess.h>
#include <kpassworddialog.h>

using namespace KNS;

KDXSButton::KDXSButton(QWidget *parent)
        : QToolButton(parent), d(0)
{
    m_entry = 0;
    m_provider = 0;
    m_dxs = 0;
    m_engine = 0;

    // FIXME KDE4PORT
    //setBackgroundColor(QColor(255, 255, 255));

    m_p = new KMenu(this);
    action_install = m_p->addAction(SmallIcon("get-hot-new-stuff"),
                                    i18n("Install"));
    action_uninstall = m_p->addAction(i18n("Uninstall"));
    action_comments = m_p->addAction(SmallIcon("help-about"),
                                     i18n("Comments"));
    action_changes = m_p->addAction(SmallIcon("help-about"),
                                    i18n("Changelog"));

    m_history = new KMenu(this);
    m_history->setTitle(i18n("Switch version"));

    // FIXME KDE4PORT
    //m_history->insertItem(i18n("(Search...)"), historyinactive);
    //m_history->setItemEnabled(historyinactive, false);

    action_historysub = m_p->addMenu(m_history);

    m_p->addSeparator();
    action_info = m_p->addAction(i18n("Provider information"));

    m_contact = new KMenu(this);
    m_contact->setIcon(SmallIcon("mail-message-new"));
    m_contact->setTitle(i18n("Contact author"));

    KMenu *pcollab = new KMenu(this);
    pcollab->setTitle(i18n("Collaboration"));

    action_collabrating = pcollab->addAction(i18n("Add Rating"));
    action_collabcomment = pcollab->addAction(i18n("Add Comment"));
    action_collabtranslation = pcollab->addAction(i18n("Translate"));
    action_collabsubscribe = pcollab->addAction(i18n("Subscribe"));
    action_collabremoval = pcollab->addAction(i18n("Report bad entry"));
    pcollab->addMenu(m_contact);

    m_p->addSeparator();
    action_collaboratesub = m_p->addMenu(pcollab);

    connect(this, SIGNAL(clicked()), SLOT(slotClicked()));

    connect(m_p, SIGNAL(triggered(QAction*)), SLOT(slotTriggered(QAction*)));

    connect(m_contact, SIGNAL(triggered(QAction*)), SLOT(slotTriggered(QAction*)));
    connect(pcollab, SIGNAL(triggered(QAction*)), SLOT(slotTriggered(QAction*)));

    // FIXME KDE4PORT: dynamic qactions are needed here
    //connect(m_history, SIGNAL(activated(int)), SLOT(slotVersionsActivated(int)));
    //connect(m_history, SIGNAL(highlighted(int)), SLOT(slotVersionsHighlighted(int)));

    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setPopupMode(QToolButton::MenuButtonPopup);
    setMenu(m_p);

    setEnabled(false);
    show();
}

KDXSButton::~KDXSButton()
{
}

void KDXSButton::setEntry(Entry *e)
{
    m_entry = e;

    if (m_engine) setEnabled(true);

    Entry::Status status = e->status();
    switch (status) {
    case Entry::Installed:
        setText(i18n("Uninstall"));
        action_install->setVisible(false);
        action_uninstall->setVisible(true);
        break;
    case Entry::Updateable:
        setText(i18n("Update"));
        action_uninstall->setVisible(false);
        action_install->setVisible(true);
        break;
    case Entry::Deleted:
        /// @todo Set different button text when string freeze is over? "Install again"
        setText(i18n("Install"));
        action_uninstall->setVisible(false);
        action_install->setVisible(true);
        break;
    default:
        setText(i18n("Install"));
        action_uninstall->setVisible(false);
        action_install->setVisible(true);
    }

    Author author = e->author();
    if (!author.email().isEmpty()) {
        m_contact->setEnabled(true);
        action_contactbymail = m_contact->addAction(SmallIcon("mail-message-new"),
                               i18n("Send Mail"));
    } else
        m_contact->setEnabled(false);
    if (!author.jabber().isEmpty()) {
        action_contactbyjabber = m_contact->addAction(i18n("Contact on Jabber"));
    }
}

void KDXSButton::setProvider(const KNS::Provider *provider)
{
    m_provider = provider;

    if (!provider) return;

    // FIXME: make it possible to query DxsEngine's DxsPolicy and react here?
    // FIXME: handle switch-version and collab menus as well
    if (provider->webService().isValid()) {
        // the web service url is valid, so enable all the actions
        action_collabrating->setEnabled(true);
        action_collabcomment->setEnabled(true);
        action_collabtranslation->setEnabled(true);
        action_collabsubscribe->setEnabled(true);
        action_collabremoval->setEnabled(true);

        action_comments->setEnabled(true);
        action_changes->setEnabled(true);

        m_history->setEnabled(true);
    } else {
        action_collabrating->setEnabled(false);
        action_collabcomment->setEnabled(false);
        action_collabtranslation->setEnabled(false);
        action_collabsubscribe->setEnabled(false);
        action_collabremoval->setEnabled(false);

        action_comments->setEnabled(false);
        action_changes->setEnabled(false);

        m_history->setEnabled(false);
    }
}

void KDXSButton::setEngine(DxsEngine *engine)
{
    m_engine = engine;

    if (m_entry) setEnabled(true);

    m_dxs = new KNS::Dxs(m_engine, NULL);
    m_dxs->setEndpoint(KUrl("http://new.kstuff.org/cgi-bin/hotstuff-dxs"));
    // FIXME: use real endpoint as soon as provider is loaded
    // FIXME: actually we would need a setProvider() here as well
    // FIXME: another thing: shouldn't dxsengine own the dxs object?

    connect(m_dxs,
            SIGNAL(signalInfo(QString, QString, QString)),
            SLOT(slotInfo(QString, QString, QString)));
    connect(m_dxs,
            SIGNAL(signalCategories(QList<KNS::Category*>)),
            SLOT(slotCategories(QList<KNS::Category*>)));
    connect(m_dxs,
            SIGNAL(signalEntries(KNS::Entry::List, Feed *)),
            SLOT(slotEntries(KNS::Entry::List, Feed *)));
    connect(m_dxs,
            SIGNAL(signalComments(QStringList)),
            SLOT(slotComments(QStringList)));
    connect(m_dxs,
            SIGNAL(signalChanges(QStringList)),
            SLOT(slotChanges(QStringList)));
    connect(m_dxs,
            SIGNAL(signalHistory(QStringList)),
            SLOT(slotHistory(QStringList)));
    connect(m_dxs,
            SIGNAL(signalRemoval(bool)),
            SLOT(slotRemoval(bool)));
    connect(m_dxs,
            SIGNAL(signalSubscription(bool)),
            SLOT(slotSubscription(bool)));
    connect(m_dxs,
            SIGNAL(signalComment(bool)),
            SLOT(slotComment(bool)));
    connect(m_dxs,
            SIGNAL(signalRating(bool)),
            SLOT(slotRating(bool)));
    connect(m_dxs,
            SIGNAL(signalFault()),
            SLOT(slotFault()));
    connect(m_dxs,
            SIGNAL(signalError()),
            SLOT(slotError()));
}

void KDXSButton::slotInfo(QString provider, QString server, QString version)
{
    QString infostring = i18n("Server: %1", server);
    infostring += '\n' + i18n("Provider: %1", provider);
    infostring += '\n' + i18n("Version: %1", version);

    KMessageBox::information(this,
                             infostring,
                             i18n("Provider information"));
}

void KDXSButton::slotCategories(QList<KNS::Category*> categories)
{
    for (QList<KNS::Category*>::Iterator it = categories.begin(); it != categories.end(); ++it) {
        KNS::Category *category = (*it);
        //kDebug() << "Category: " << category->name().representation();
    }
}

void KDXSButton::slotEntries(KNS::Entry::List entries, Feed * feed)
{
    for (KNS::Entry::List::Iterator it = entries.begin(); it != entries.end(); ++it) {
        KNS::Entry *entry = (*it);
        //kDebug() << "Entry: " << entry->name().representation();
    }
}

void KDXSButton::slotComments(QStringList comments)
{
    KDXSComments commentsdlg(this);

    for (QStringList::const_iterator it = comments.begin(); it != comments.end(); ++it) {
        //kDebug() << "Comment: " << (*it);
        commentsdlg.addComment("foo", (*it));
    }

    commentsdlg.exec();
}

void KDXSButton::slotChanges(QStringList changes)
{
    KDXSChanges changesdlg(this);

    for (QStringList::const_iterator it = changes.begin(); it != changes.end(); ++it) {
        //kDebug() << "Changelog: " << (*it);
        changesdlg.addChangelog("v???", (*it));
    }

    changesdlg.exec();
}

void KDXSButton::slotHistory(QStringList entries)
{
    m_history->clear();

    int i = 0;
    for (QStringList::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        //kDebug() << (*it);

        // FIXME KDE4PORT
        //m_history->insertItem(SmallIcon("view-history"),
        // i18n((*it)), historyslots + i);
        i++;
    }

    if (entries.size() == 0) {
        // FIXME KDE4PORT
        //m_history->insertItem(i18n("(No history found)"), historydisabled);
        //m_history->setItemEnabled(historydisabled, false);
    }

    m_history->setCursor(Qt::ArrowCursor);
}

void KDXSButton::slotRemoval(bool success)
{
    if (success) {
        KMessageBox::information(this,
                                 i18n("The removal request was successfully registered."),
                                 i18n("Removal of entry"));
    } else {
        KMessageBox::error(this,
                           i18n("The removal request failed."),
                           i18n("Removal of entry"));
    }
}

void KDXSButton::slotSubscription(bool success)
{
    if (success) {
        KMessageBox::information(this,
                                 i18n("The subscription was successfully completed."),
                                 i18n("Subscription to entry"));
    } else {
        KMessageBox::error(this,
                           i18n("The subscription request failed."),
                           i18n("Subscription to entry"));
    }
}

void KDXSButton::slotRating(bool success)
{
    if (success) {
        KMessageBox::information(this,
                                 i18n("The rating was submitted successfully."),
                                 i18n("Rating for entry"));
    } else {
        KMessageBox::error(this,
                           i18n("The rating could not be submitted."),
                           i18n("Rating for entry"));
    }
}

void KDXSButton::slotComment(bool success)
{
    if (success) {
        KMessageBox::information(this,
                                 i18n("The comment was submitted successfully."),
                                 i18n("Comment on entry"));
    } else {
        KMessageBox::error(this,
                           i18n("The comment could not be submitted."),
                           i18n("Comment on entry"));
    }
}

void KDXSButton::slotFault()
{
    KMessageBox::error(this,
                       i18n("A protocol fault has occurred. The request has failed."),
                       i18n("Desktop Exchange Service"));
}

void KDXSButton::slotError()
{
    KMessageBox::error(this,
                       i18n("A network error has occurred. The request has failed."),
                       i18n("Desktop Exchange Service"));
}

void KDXSButton::slotVersionsActivated(int id)
{
    int version = id - historyslots;

    Q_UNUSED(version);
    // and now???
}

void KDXSButton::slotTriggered(QAction *action)
{
    int ret;

    if (action == action_info) {
        // FIXME: consider engine's DxsPolicy
        if (m_provider->webService().isValid()) {
            m_dxs->call_info();
        } else {
            slotInfo(m_provider->name().representation(),
                     QString(),
                     QString());
        }
    }
    if (action == action_comments) {
        m_dxs->call_comments(0);
    }
    if (action == action_changes) {
        m_dxs->call_changes(2);
    }
    if (action == action_contactbymail) {
        QString address = m_entry->author().email();
        KToolInvocation::invokeMailer(address, i18n("KNewStuff contributions"), "");
    }
    if (action == action_contactbyjabber) {
        new KRun(KUrl(QLatin1String("xmpp:") + m_entry->author().jabber()), this);
    }
    if (action == action_collabtranslation) {
        if (!authenticate())
            return;
        KDXSTranslation translation(this);
        ret = translation.exec();
        if (ret == QDialog::Accepted) {
            //QString s = comment.comment();
            //if(!s.isEmpty())
            //{
            // m_dxs->call_comment(0, s);
            //}
        }
    }
    if (action == action_collabremoval) {
        if (authenticate())
            m_dxs->call_removal(0);
    }
    if (action == action_collabsubscribe) {
        if (authenticate())
            m_dxs->call_subscription(0, true);
    }
    if (action == action_uninstall) {
        if (m_engine->uninstall(m_entry)) {
            setText(i18n("Install"));
            action_uninstall->setVisible(false);
            action_install->setVisible(true);
        }
    }
    if (action == action_install) {
        connect(m_engine,
                SIGNAL(signalPayloadLoaded(KUrl)),
                SLOT(slotPayloadLoaded(KUrl)));
        connect(m_engine,
                SIGNAL(signalPayloadFailed(KNS::Entry *)),
                SLOT(slotPayloadFailed(KNS::Entry *)));

        m_engine->downloadPayload(m_entry);
    }
    if (action == action_collabcomment) {
        if (!authenticate())
            return;
        KDXSComment comment(this);
        ret = comment.exec();
        if (ret == QDialog::Accepted) {
            QString s = comment.comment();
            if (!s.isEmpty()) {
                m_dxs->call_comment(0, s);
            }
        }
    }
    if (action == action_collabrating) {
        if (!authenticate())
            return;
        KDXSRating rating(this);
        ret = rating.exec();
        if (ret == QDialog::Accepted) {
            int r = rating.rating();
            if (r >= 0) {
                m_dxs->call_rating(0, r);
            }
        }
    }
}

void KDXSButton::slotVersionsHighlighted(int id)
{
    //kDebug() << "highlighted!";

    if (id == historyinactive) {
        //m_history->setItemEnabled(historyinactive, true);
        m_history->setCursor(QCursor(Qt::WaitCursor));
        //kDebug() << "hourglass!";

        m_dxs->call_history(0);
        // .....
    }
}

void KDXSButton::slotClicked()
{
    if (action_install->isVisible())
        slotTriggered(action_install);
    else
        slotTriggered(action_uninstall);
}

bool KDXSButton::authenticate()
{
    if ((!m_username.isEmpty()) && (!m_password.isEmpty())) return true;

    KPasswordDialog dlg(this);
    dlg.setPrompt(i18n("This operation requires authentication."));
    int ret = dlg.exec();
    if (ret == QDialog::Accepted) {
        m_username = dlg.username();
        m_password = dlg.password();

        return true;
    }

    return false;
}

void KDXSButton::slotPayloadLoaded(KUrl url)
{
    //kDebug() << "PAYLOAD: success; try to install";

    Entry::Status status = m_entry->status();
    if (status == Entry::Installed) {
        setText(i18n("Uninstall"));
        action_install->setVisible(false);
        action_uninstall->setVisible(true);
    } else {
        setText(i18n("Install"));
        action_uninstall->setVisible(false);
        action_install->setVisible(true);
    }

    m_engine->install(url.pathOrUrl());
}

void KDXSButton::slotPayloadFailed(KNS::Entry *)
{
    //kDebug() << "PAYLOAD: failed";
}

#include "kdxsbutton.moc"
