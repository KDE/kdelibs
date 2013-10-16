/*  This file is part of the KDE libraries
 *  Copyright 2012 David Faure <faure+bluesystems@kde.org>
 *  Copyright 2013 Aurélien Gâteau <agateau@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#include "kmessagebox_p.h"

#include <QPluginLoader>
#include <QVariant>

namespace KMessageBox {

class KMessageBoxDontAskAgainMemoryStorage : public KMessageBoxDontAskAgainInterface
{
public:
    KMessageBoxDontAskAgainMemoryStorage() {}
    virtual ~KMessageBoxDontAskAgainMemoryStorage() {}

    virtual bool shouldBeShownYesNo(const QString &dontShowAgainName, KMessageBox::ButtonCode &result) {
        KMessageBox::ButtonCode code = m_saved.value(dontShowAgainName, KMessageBox::ButtonCode(0));
        if (code == KMessageBox::Yes || code == KMessageBox::No) {
            result = code;
            return false;
        }
        return true;
    }
    virtual bool shouldBeShownContinue(const QString &dontShowAgainName) {
        KMessageBox::ButtonCode code = m_saved.value(dontShowAgainName, KMessageBox::Yes);
        return code == KMessageBox::Yes;
    }
    virtual void saveDontShowAgainYesNo(const QString &dontShowAgainName, KMessageBox::ButtonCode result) {
        m_saved[dontShowAgainName] = result;
    }
    virtual void saveDontShowAgainContinue(const QString &dontShowAgainName) {
        m_saved[dontShowAgainName] = KMessageBox::No;
    }
    virtual void enableAllMessages() {
        m_saved.clear();
    }
    virtual void enableMessage(const QString& dontShowAgainName) {
        m_saved.remove(dontShowAgainName);
    }
    virtual void setConfig(KConfig *) {}

private:
    QHash<QString, KMessageBox::ButtonCode> m_saved;
};

class KMessageBoxNotifyDummy : public KMessageBoxNotifyInterface
{
public:
    void sendNotification(QMessageBox::Icon /*notificationType*/, const QString &/*message*/, QWidget */*parent*/) {}
};

// TODO should we use QSharedPointer here?
static KMessageBoxDontAskAgainInterface* s_dontAskAgainInterface = 0;
static KMessageBoxNotifyInterface* s_notifyInterface = 0;

static void loadKMessageBoxPlugin()
{
    static bool triedLoadingPlugin = false;
    if (!triedLoadingPlugin) {
        triedLoadingPlugin = true;

        QPluginLoader lib(QStringLiteral("kf5/FrameworkIntegrationPlugin"));
        QObject *rootObj = lib.instance();
        if (rootObj) {
            s_dontAskAgainInterface = rootObj->property(KMESSAGEBOXDONTASKAGAIN_PROPERTY).value<KMessageBoxDontAskAgainInterface *>();
            s_notifyInterface = rootObj->property(KMESSAGEBOXNOTIFY_PROPERTY).value<KMessageBoxNotifyInterface *>();
        }
    }
    // TODO use Qt-5.1's Q_GLOBAL_STATIC
    if (!s_dontAskAgainInterface) {
        s_dontAskAgainInterface = new KMessageBoxDontAskAgainMemoryStorage;
    }
    if (!s_notifyInterface) {
        s_notifyInterface = new KMessageBoxNotifyDummy;
    }
}

KMessageBoxDontAskAgainInterface *dontAskAgainInterface()
{
    if (!s_dontAskAgainInterface) {
        loadKMessageBoxPlugin();
    }
    return s_dontAskAgainInterface;
}

KMessageBoxNotifyInterface *notifyInterface()
{
    if (!s_notifyInterface) {
        loadKMessageBoxPlugin();
    }
    return s_notifyInterface;
}

void setDontShowAgainInterface(KMessageBoxDontAskAgainInterface* dontAskAgainInterface)
{
    Q_ASSERT(dontAskAgainInterface != 0);
    // FIXME should we delete s_dontAskAgainInterface before? Or perhaps use smart pointers to avoid problems?
    s_dontAskAgainInterface = dontAskAgainInterface;
}

void setNotifyInterface(KMessageBoxNotifyInterface *notifyInterface)
{
    Q_ASSERT(notifyInterface != 0);
    s_notifyInterface = notifyInterface;
}

} // KMessageBox namespace
