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

#include "phonon/config-phonon.h"

#include "kdepluginfactory.h"
#include "kiomediastream.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
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
#include <kconfiggroup.h>
#include <kstandarddirs.h>

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

KdePlatformPlugin::KdePlatformPlugin()
{
    ensureMainComponentData();
    KGlobal::locale()->insertCatalog(QLatin1String("phonon_kde"));
}

AbstractMediaStream *KdePlatformPlugin::createMediaStream(const QUrl &url, QObject *parent)
{
    return new KioMediaStream(url, parent);
}

QIcon KdePlatformPlugin::icon(const QString &name) const
{
    return KIcon(name);
}

void KdePlatformPlugin::notification(const char *notificationName, const QString &text,
        const QStringList &actions, QObject *receiver,
        const char *actionSlot) const
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

QString KdePlatformPlugin::applicationName() const
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

#undef PHONON_LOAD_BACKEND_GLOBAL

QObject *KdePlatformPlugin::createBackend(KService::Ptr newService)
{
    QString errorReason;
#ifdef PHONON_LOAD_BACKEND_GLOBAL
    KLibFactory *factory = 0;
    // This code is in here temporarily until NMM gets fixed.
    // Currently the NMM backend will fail with undefined symbols if
    // the backend is not loaded with global symbol resolution
    QObject *backend = 0;
    factory = KLibLoader::self()->factory(newService->library(), QLibrary::ExportExternalSymbolsHint);
    if (!factory) {
        errorReason = KLibLoader::self()->lastErrorMessage();
    } else {
        QObject *backend = factory->create<QObject>();
        if (0 == backend) {
            errorReason = i18n("create method returned 0");
        }
    }
#else
    QObject *backend = newService->createInstance<QObject>(0, QVariantList(), &errorReason);
#endif
    if (0 == backend) {
        const QLatin1String suffix("/phonon_backend/");
        const QStringList libFilter(newService->library() + QLatin1String(".*"));
        foreach (QString libPath, QCoreApplication::libraryPaths()) {
            libPath += suffix;
            const QDir dir(libPath);
            foreach (const QString &pluginName, dir.entryList(libFilter, QDir::Files)) {
                QPluginLoader pluginLoader(pluginName);
                backend = pluginLoader.instance();
                if (backend) {
                    break;
                }
            }
            if (backend) {
                break;
            }
        }
    }
    if (0 == backend) {
        kError(600) << "Can not create backend object from factory for " <<
            newService->name() << ", " << newService->library() << ":\n" << errorReason;

        KMessageBox::error(0,
                i18n("<qt>Unable to use the <b>%1</b> Multimedia Backend:<br/>%2</qt>",
                    newService->name(), errorReason));
        return 0;
    }

    kDebug() << "using backend: " << newService->name();
    return backend;
}

QObject *KdePlatformPlugin::createBackend()
{
    // Within this process, display the warning about missing backends
    // only once.
    static bool has_shown = false;
    ensureMainComponentData();
    const KService::List offers = KServiceTypeTrader::self()->query("PhononBackend",
            "Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1");
    if (offers.isEmpty()) {
        if (!has_shown) {
#if defined(HAVE_KDE4_MULTIMEDIA)
            KMessageBox::error(0, i18n("Unable to find a Multimedia Backend"));
#endif
            has_shown = true;
        }
        return 0;
    }
    // Flag the warning as not shown, since if the next time the
    // list of backends is suddenly empty again the user should be
    // told.
    has_shown = false;

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

QObject *KdePlatformPlugin::createBackend(const QString &library, const QString &version)
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

bool KdePlatformPlugin::isMimeTypeAvailable(const QString &mimeType) const
{
    ensureMainComponentData();
    const KService::List offers = KServiceTypeTrader::self()->query("PhononBackend",
            "Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1");
    if (!offers.isEmpty()) {
        return offers.first()->hasMimeType(KMimeType::mimeType(mimeType).data());
    }
    return false;
}

void KdePlatformPlugin::saveVolume(const QString &outputName, qreal volume)
{
    ensureMainComponentData();
    KConfigGroup config(KGlobal::config(), "Phonon::AudioOutput");
    config.writeEntry(outputName + "_Volume", volume);
}

qreal KdePlatformPlugin::loadVolume(const QString &outputName) const
{
    ensureMainComponentData();
    KConfigGroup config(KGlobal::config(), "Phonon::AudioOutput");
    return config.readEntry<qreal>(outputName + "_Volume", 1.0);
}

} // namespace Phonon

Q_EXPORT_PLUGIN2(phonon_platform_kde, Phonon::KdePlatformPlugin)

#include "kdepluginfactory.moc"
// vim: sw=4 sts=4 et tw=100
