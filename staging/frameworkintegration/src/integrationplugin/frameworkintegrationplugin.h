/*  This file is part of the KDE libraries
 *  Copyright 2012 David Faure <faure+bluesystems@kde.org>
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

#include <QObject>
#include <kmessageboxdontaskagaininterface.h>
#include <kmessageboxnotifyinterface.h>

class KConfig;

class KMessageBoxDontAskAgainConfigStorage : public KMessageBoxDontAskAgainInterface
{
public:
    KMessageBoxDontAskAgainConfigStorage() : KMessageBox_againConfig(0) {}
    virtual ~KMessageBoxDontAskAgainConfigStorage() {}

    virtual bool shouldBeShownYesNo(const QString &dontShowAgainName, KMessageBox::ButtonCode &result);
    virtual bool shouldBeShownContinue(const QString &dontShowAgainName);
    virtual void saveDontShowAgainYesNo(const QString &dontShowAgainName, KMessageBox::ButtonCode result);
    virtual void saveDontShowAgainContinue(const QString &dontShowAgainName);
    virtual void enableAllMessages();
    virtual void enableMessage(const QString& dontShowAgainName);
    virtual void setConfig(KConfig *cfg) { KMessageBox_againConfig = cfg; }

private:
    KConfig* KMessageBox_againConfig;
};

class KMessageBoxNotify : public KMessageBoxNotifyInterface
{
public:
    void sendNotification(QMessageBox::Icon notificationType, const QString &message, QWidget *parent) Q_DECL_OVERRIDE;
};

class KFrameworkIntegrationPlugin : public QObject
{
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.kde.FrameworkIntegrationPlugin")
#endif
    Q_OBJECT
public:

    KFrameworkIntegrationPlugin();

private:
    KMessageBoxDontAskAgainConfigStorage m_dontAskAgainConfigStorage;
    KMessageBoxNotify m_notify;
};

