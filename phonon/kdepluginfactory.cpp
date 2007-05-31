/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "kdepluginfactory.h"
#include "kiomediastream.h"

#include <QtCore/QtPlugin>
#include <QtCore/QCoreApplication>

#include <kaboutdata.h>
#include <kdebug.h>
#include <kcomponentdata.h>
#include <kglobal.h>
#include <kicon.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <knotification.h>
#include <kservice.h>
#include <kservicetypetrader.h>

namespace Phonon
{

K_GLOBAL_STATIC_WITH_ARGS(KComponentData, mainComponentData, (QCoreApplication::applicationName().toUtf8()))
K_GLOBAL_STATIC_WITH_ARGS(KComponentData, phononComponentData, ("phonon"))

static void ensureMainComponentData()
{
    if (!KGlobal::hasMainComponent()) {
        // a pure Qt application does not have a KComponentData object,
        // we'll give it one.
        *mainComponentData;
        qAddPostRoutine(mainComponentData.destroy);
        Q_ASSERT(KGlobal::hasMainComponent());
    }
}

static const KComponentData &componentData()
{
    ensureMainComponentData();
    return *phononComponentData;
}

AbstractMediaStream *KdePluginFactory::createKioMediaStream(const QUrl &url, QObject *parent)
{
    return new KioMediaStream(url, parent);
}

QIcon KdePluginFactory::icon(const QString &name)
{
    return KIcon(name);
}

void KdePluginFactory::notification(const char *notificationName, const QString &text,
        const QStringList &actions, QObject *receiver,
        const char *actionSlot)
{
    KNotification *notification = new KNotification(notificationName);
    notification->setComponentData(componentData());
    notification->setText(text);
    //notification->setPixmap(...);
    notification->addContext(QLatin1String("Application"), KGlobal::mainComponent().componentName());
    if (!actions.isEmpty() && receiver && actionSlot) {
        notification->setActions(actions);
        QObject::connect(notification, SIGNAL(activated(unsigned int)), receiver, actionSlot);
    }
    notification->sendEvent();
}

QString KdePluginFactory::applicationName() const
{
    ensureMainComponentData();
    const KAboutData *ad = KGlobal::mainComponent().aboutData();
    if (ad) {
        const QString programName = ad->programName();
        if (programName.isEmpty()) {
            return KGlobal::mainComponent().componentName();
        }
        return programName;
    }
    return KGlobal::mainComponent().componentName();
}

#define PHONON_LOAD_BACKEND_GLOBAL 0

QObject *KdePluginFactory::createBackend(KService::Ptr newService)
{
    KLibFactory *factory = 0;
#ifdef PHONON_LOAD_BACKEND_GLOBAL
    // This code is in here temporarily until NMM gets fixed.
    // Currently the NMM backend will fail with undefined symbols if
    // the backend is not loaded with global symbol resolution
    KLibrary *library = KLibLoader::self()->library(newService->library(), QLibrary::ExportExternalSymbolsHint);
    if (library) {
        factory = library->factory();
    }
    //factory = KLibLoader::self()->factory(QFile::encodeName(newService->library()), QLibrary::ExportExternalSymbolsHint);
#else
    factory = KLibLoader::self()->factory(QFile::encodeName(newService->library()));
#endif
    if (!factory) {
        QString errorReason = KLibLoader::self()->lastErrorMessage();
        kError(600) << "Can not create factory for " << newService->name() <<
            ":\n" << errorReason << endl;

        KMessageBox::error(0,
                QLatin1String("<html>")
                + i18n("Unable to use the <b>%1</b> Multimedia Backend:", newService->name())
                + QLatin1Char('\n')
                + errorReason
                + QLatin1String("</html>"));
        return false;
    }

    QObject *backend = factory->create();
    if (0 == backend) {
        QString errorReason = i18n("create method returned 0");
        kError(600) << "Can not create backend object from factory for " <<
            newService->name() << ", " << newService->library() << ":\n" << errorReason << endl;

        KMessageBox::error(0,
                QLatin1String("<qt>")
                + i18n("Unable to use the <b>%1</b> Multimedia Backend:", newService->name())
                + QLatin1Char('\n')
                + errorReason
                + QLatin1String("<qt>"));
        return false;
    }

    kDebug() << "using backend: " << newService->name();
    return backend;
}

QObject *KdePluginFactory::createBackend()
{
    ensureMainComponentData();
    const KService::List offers = KServiceTypeTrader::self()->query("PhononBackend",
            "Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1");
    if (offers.isEmpty()) {
        KMessageBox::error(0, i18n("Unable to find a Multimedia Backend"));
        return 0;
    }

    KService::List::const_iterator it = offers.begin();
    const KService::List::const_iterator end = offers.end();
    while (it != end) {
        QObject *backend = createBackend(*it);
        if (backend) {
            return backend;
        }
        ++it;
    }
    return 0;
}

QObject *KdePluginFactory::createBackend(const QString &library, const QString &version)
{
    ensureMainComponentData();
    QString additionalConstraints = QLatin1String(" and Library == '") + library + QLatin1Char('\'');
    if (!version.isEmpty()) {
        additionalConstraints += QLatin1String(" and [X-KDE-PhononBackendInfo-Version] == '")
            + version + QLatin1Char('\'');
    }
    const KService::List offers = KServiceTypeTrader::self()->query(QLatin1String("PhononBackend"),
            QString("Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1%1")
            .arg(additionalConstraints));
    if (offers.isEmpty()) {
        KMessageBox::error(0, i18n("Unable to find the requested Multimedia Backend"));
        return 0;
    }

    KService::List::const_iterator it = offers.begin();
    const KService::List::const_iterator end = offers.end();
    while (it != end) {
        QObject *backend = createBackend(*it);
        if (backend) {
            return backend;
        }
        ++it;
    }
    return 0;
}

bool KdePluginFactory::isMimeTypeAvailable(const QString &mimeType)
{
    ensureMainComponentData();
    const KService::List offers = KServiceTypeTrader::self()->query("PhononBackend",
            "Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1");
    if (!offers.isEmpty()) {
        return offers.first()->hasMimeType(KMimeType::mimeType(mimeType).data());
    }
    return false;
}

} // namespace Phonon

Q_EXPORT_PLUGIN2(phonon_platform_kde, Phonon::KdePluginFactory)

#include "kdepluginfactory.moc"
// vim: sw=4 sts=4 et tw=100
