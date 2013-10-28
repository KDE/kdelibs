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

#include "frameworkintegrationplugin.h"
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <knotification.h>

#include <qplugin.h>
#include <QDebug>

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(frameworkintegrationplugin, KFrameworkIntegrationPlugin)
#endif

bool KMessageBoxDontAskAgainConfigStorage::shouldBeShownYesNo(const QString &dontShowAgainName,
                                                              KMessageBox::ButtonCode &result)
{
    KConfigGroup cg( KMessageBox_againConfig ? KMessageBox_againConfig : KSharedConfig::openConfig().data(), "Notification Messages" );
    const QString dontAsk = cg.readEntry(dontShowAgainName, QString()).toLower();
    if (dontAsk == QLatin1String("yes") || dontAsk == QLatin1String("true")) {
        result = KMessageBox::Yes;
        return false;
    }
    if (dontAsk == QLatin1String("no") || dontAsk == QLatin1String("false")) {
        result = KMessageBox::No;
        return false;
    }
    return true;
}

bool KMessageBoxDontAskAgainConfigStorage::shouldBeShownContinue(const QString &dontShowAgainName)
{
    KConfigGroup cg( KMessageBox_againConfig ? KMessageBox_againConfig : KSharedConfig::openConfig().data(), "Notification Messages" );
    return cg.readEntry(dontShowAgainName, true);
}

void KMessageBoxDontAskAgainConfigStorage::saveDontShowAgainYesNo(const QString &dontShowAgainName,
                                                                  KMessageBox::ButtonCode result)
{
    KConfigGroup::WriteConfigFlags flags = KConfig::Persistent;
    if (dontShowAgainName[0] == QLatin1Char(':')) {
        flags |= KConfigGroup::Global;
    }
    KConfigGroup cg( KMessageBox_againConfig? KMessageBox_againConfig : KSharedConfig::openConfig().data(), "Notification Messages" );
    cg.writeEntry( dontShowAgainName, result == KMessageBox::Yes, flags );
    cg.sync();
}

void KMessageBoxDontAskAgainConfigStorage::saveDontShowAgainContinue(const QString &dontShowAgainName)
{
    KConfigGroup::WriteConfigFlags flags = KConfigGroup::Persistent;
    if (dontShowAgainName[0] == QLatin1Char(':')) {
        flags |= KConfigGroup::Global;
    }
    KConfigGroup cg( KMessageBox_againConfig? KMessageBox_againConfig: KSharedConfig::openConfig().data(), "Notification Messages" );
    cg.writeEntry( dontShowAgainName, false, flags );
    cg.sync();
}

void KMessageBoxDontAskAgainConfigStorage::enableAllMessages()
{
   KConfig *config = KMessageBox_againConfig ? KMessageBox_againConfig : KSharedConfig::openConfig().data();
   if (!config->hasGroup("Notification Messages")) {
      return;
   }

   KConfigGroup cg(config, "Notification Messages" );

   typedef QMap<QString, QString> configMap;

   const configMap map = cg.entryMap();

   configMap::ConstIterator it;
   for (it = map.begin(); it != map.end(); ++it) {
      cg.deleteEntry( it.key() );
   }
}

void KMessageBoxDontAskAgainConfigStorage::enableMessage(const QString &dontShowAgainName)
{
   KConfig *config = KMessageBox_againConfig ? KMessageBox_againConfig : KSharedConfig::openConfig().data();
   if (!config->hasGroup("Notification Messages")) {
      return;
   }

   KConfigGroup cg( config, "Notification Messages" );

   cg.deleteEntry(dontShowAgainName);
   config->sync();
}

void KMessageBoxNotify::sendNotification(QMessageBox::Icon notificationType, const QString &message, QWidget *parent)
{
    QString messageType;
    switch (notificationType) {
    case QMessageBox::Warning:
        messageType = QStringLiteral("messageWarning");
        break;
    case QMessageBox::Critical:
        messageType = QStringLiteral("messageCritical");
        break;
    case QMessageBox::Question:
        messageType = QStringLiteral("messageQuestion");
        break;
    default:
        messageType = QStringLiteral("messageInformation");
        break;
    }

    KNotification::event(messageType, message, QPixmap(), parent,
        KNotification::DefaultEvent | KNotification::CloseOnTimeout);
}

KFrameworkIntegrationPlugin::KFrameworkIntegrationPlugin()
    : QObject()
{
    setProperty(KMESSAGEBOXDONTASKAGAIN_PROPERTY, QVariant::fromValue<KMessageBoxDontAskAgainInterface *>(&m_dontAskAgainConfigStorage));
    setProperty(KMESSAGEBOXNOTIFY_PROPERTY, QVariant::fromValue<KMessageBoxNotifyInterface *>(&m_notify));
}
