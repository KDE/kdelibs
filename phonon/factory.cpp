/*  This file is part of the KDE project
    Copyright (C) 2004-2007 Matthias Kretz <kretz@kde.org>

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

#include "factory.h"
#include "factory_p.h"

#include "backendinterface.h"
#include "base_p.h"
#include "audiooutput.h"
#include "audiooutput_p.h"
#include "pluginfactory.h"

#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QCoreApplication>
#include <QtDBus/QtDBus>
#include <QtCore/QPluginLoader>

#include <klocale.h>
#include <kmimetype.h>
#include "phononnamespace_p.h"
#include <kglobal.h>
#include <kservicetypetrader.h>
#include <klibloader.h>
#include <kmessage.h>
#include <QtCore/QDir>
#include <QtCore/QLibrary>
#include <QtGui/QIcon>

#define PHONON_LOAD_BACKEND_GLOBAL 0

namespace Phonon
{

K_GLOBAL_STATIC(Phonon::FactoryPrivate, globalFactory)

void Factory::createBackend(const QString &library, const QString &version)
{
    Q_ASSERT(!globalFactory->m_backendObject);

    QString additionalConstraints = QLatin1String(" and Library == '") + library + QLatin1Char('\'');
    if (!version.isEmpty()) {
        additionalConstraints += QLatin1String(" and [X-KDE-PhononBackendInfo-Version] == '")
            + version + QLatin1Char('\'');
    }
    const KService::List offers = KServiceTypeTrader::self()->query(QLatin1String("PhononBackend"),
            QString("Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1%1")
            .arg(additionalConstraints));
    if (offers.isEmpty()) {
        KMessage::message(KMessage::Error, i18n("Unable to find the requested Multimedia Backend"));
        return;
    }

    KService::List::const_iterator it = offers.begin();
    const KService::List::const_iterator end = offers.end();
    while (it != end && !globalFactory->createBackend(*it)) {
        ++it;
    }
}

bool FactoryPrivate::createBackend(KService::Ptr newService)
{
    KLibFactory *factory = 0;
#ifdef PHONON_LOAD_BACKEND_GLOBAL
    // This code is in here temporarily until NMM gets fixed.
    // Currently the NMM backend will fail with undefined symbols if
    // the backend is not loaded with global symbol resolution
    KLibrary *library = KLibLoader::self()->library(QFile::encodeName(newService->library()), QLibrary::ExportExternalSymbolsHint);
    if (library) {
        factory = library->factory();
    }
    //factory = KLibLoader::self()->factory(QFile::encodeName(newService->library()), QLibrary::ExportExternalSymbolsHint);
#else
    factory = KLibLoader::self()->factory(QFile::encodeName(newService->library()));
#endif
    if (!factory) {
        QString errorReason = KLibLoader::self()->lastErrorMessage();
        pError() << "Can not create factory for " << newService->name() <<
            ":\n" << errorReason << endl;

        KMessage::message(KMessage::Error,
                QLatin1String("<qt>")
                + i18n("Unable to use the <b>%1</b> Multimedia Backend:", newService->name())
                + QLatin1Char('\n')
                + errorReason
                + QLatin1String("<qt>"));
        return false;
    }

    m_backendObject = factory->create();
    if (0 == m_backendObject) {
        QString errorReason = i18n("create method returned 0");
        pError() << "Can not create backend object from factory for " <<
            newService->name() << ", " << newService->library() << ":\n" << errorReason << endl;

        KMessage::message(KMessage::Error,
                QLatin1String("<qt>")
                + i18n("Unable to use the <b>%1</b> Multimedia Backend:", newService->name())
                + QLatin1Char('\n')
                + errorReason
                + QLatin1String("<qt>"));
        return false;
    }

    service = newService;
    pDebug() << "using backend: " << newService->name();

    connect(m_backendObject, SIGNAL(objectDescriptionChanged(ObjectDescriptionType)),
            SLOT(objectDescriptionChanged(ObjectDescriptionType)));

    return true;
}

void FactoryPrivate::createBackend()
{
    const KService::List offers = KServiceTypeTrader::self()->query("PhononBackend",
            "Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1");
    if (offers.isEmpty()) {
        KMessage::message(KMessage::Error, i18n("Unable to find a Multimedia Backend"));
        return;
    }

    KService::List::const_iterator it = offers.begin();
    const KService::List::const_iterator end = offers.end();
    while (it != end && !createBackend(*it)) {
        ++it;
    }
}

FactoryPrivate::FactoryPrivate()
    : m_backendObject(0),
    m_pluginFactory(0)
{
    // Add the post routine to make sure that all other global statics (especially the ones from Qt)
    // are still available. If the FactoryPrivate dtor is called too late many bad things can happen
    // as the whole backend might still be alive.
    qAddPostRoutine(globalFactory.destroy);
    QDBusConnection::sessionBus().connect(QString(), QString(), "org.kde.Phonon.Factory",
            "phononBackendChanged", this, SLOT(phononBackendChanged()));
}

FactoryPrivate::~FactoryPrivate()
{
    emit aboutToBeDestroyed();

    foreach (BasePrivate *bp, basePrivateList) {
        bp->deleteBackendObject();
    }
    if (objects.size() > 0) {
        pError() << "The backend objects are not deleted as was requested." << endl;
        qDeleteAll(objects);
    }
    delete m_backendObject;
}

void FactoryPrivate::objectDescriptionChanged(ObjectDescriptionType type)
{
#ifdef PHONON_METHODTEST
    Q_UNUSED(type);
#else
    pDebug() << Q_FUNC_INFO << type;
    switch (type) {
    case AudioOutputDeviceType:
        // tell all AudioOutput objects to check their output device preference
        foreach (BasePrivate *obj, globalFactory->basePrivateList) {
            AudioOutputPrivate *output = AudioOutputPrivate::cast(obj);
            if (output) {
                output->deviceListChanged();
            }
        }
        break;
    default:
        break;
    }
    //emit capabilitiesChanged();
#endif // PHONON_METHODTEST
}

Factory::Sender *Factory::sender()
{
    return globalFactory;
}

void Factory::registerFrontendObject(BasePrivate *bp)
{
    globalFactory->basePrivateList.prepend(bp); // inserted last => deleted first
}

void Factory::deregisterFrontendObject(BasePrivate *bp)
{
    // The Factory can already be cleaned up while there are other frontend objects still alive.
    // When those are deleted they'll call deregisterFrontendObject through ~BasePrivate
    if (!globalFactory.isDestroyed()) {
        globalFactory->basePrivateList.removeAll(bp);
    }
}

void FactoryPrivate::phononBackendChanged()
{
    if (m_backendObject) {
        foreach (BasePrivate *bp, basePrivateList) {
            bp->deleteBackendObject();
        }
        if (objects.size() > 0) {
            pDebug() << "WARNING: we were asked to change the backend but the application did\n"
                "not free all references to objects created by the factory. Therefore we can not\n"
                "change the backend without crashing. Now we have to wait for a restart to make\n"
                "backendswitching possible.";
            // in case there were objects deleted give 'em a chance to recreate
            // them now
            foreach (BasePrivate *bp, basePrivateList) {
                bp->createBackendObject();
            }
            return;
        }
        delete m_backendObject;
        m_backendObject = 0;
    }
    createBackend();
    foreach (BasePrivate *bp, basePrivateList) {
        bp->createBackendObject();
    }
    emit backendChanged();
}

//X void Factory::freeSoundcardDevices()
//X {
//X     if (globalFactory->backend) {
//X         globalFactory->backend->freeSoundcardDevices();
//X     }
//X }

void FactoryPrivate::objectDestroyed(QObject * obj)
{
    //pDebug() << Q_FUNC_INFO << obj;
    objects.removeAll(obj);
}

#define FACTORY_IMPL(classname) \
QObject *Factory::create ## classname(QObject *parent) \
{ \
    if (backend()) { \
        return registerQObject(qobject_cast<BackendInterface *>(backend())->createObject(BackendInterface::classname##Class, parent)); \
    } \
    return 0; \
}
#define FACTORY_IMPL_1ARG(classname) \
QObject *Factory::create ## classname(int arg1, QObject *parent) \
{ \
    if (backend()) { \
        return registerQObject(qobject_cast<BackendInterface *>(backend())->createObject(BackendInterface::classname##Class, parent, QList<QVariant>() << arg1)); \
    } \
    return 0; \
}

FACTORY_IMPL(MediaObject)
FACTORY_IMPL(AudioPath)
FACTORY_IMPL_1ARG(AudioEffect)
FACTORY_IMPL(VolumeFaderEffect)
FACTORY_IMPL(AudioOutput)
FACTORY_IMPL(AudioDataOutput)
FACTORY_IMPL(Visualization)
FACTORY_IMPL(VideoPath)
FACTORY_IMPL_1ARG(VideoEffect)
FACTORY_IMPL(BrightnessControl)
FACTORY_IMPL(DeinterlaceFilter)
FACTORY_IMPL(VideoDataOutput)
FACTORY_IMPL(VideoWidget)

#undef FACTORY_IMPL

PluginFactory *FactoryPrivate::pluginFactory()
{
    if (!m_pluginFactory) {
        const QString suffix("/phonon_platform");
        foreach (QString libPath, QCoreApplication::libraryPaths()) {
            libPath += suffix;
            const QDir dir(libPath);
            if (!dir.exists()) {
                pDebug() << Q_FUNC_INFO << dir.canonicalPath() << "does not exist";
                continue;
            }
            QLibrary pluginLib(libPath + QLatin1String("/kde"));
            pluginLib.load();
            pDebug() << Q_FUNC_INFO << "trying to load " << pluginLib.fileName();
            QPluginLoader pluginLoader(pluginLib.fileName());
            Q_ASSERT(pluginLoader.load());
            pDebug() << pluginLoader.instance();
            m_pluginFactory = qobject_cast<PluginFactory *>(pluginLoader.instance());
            pDebug() << m_pluginFactory;
            if (m_pluginFactory) {
                return m_pluginFactory;
            }
        }
        if (!m_pluginFactory) {
            pDebug() << Q_FUNC_INFO << "phonon_platform/kde plugin could not be loaded";
        }
    }
    return m_pluginFactory;
}

AbstractMediaStream *Factory::createKioMediaStream(const QUrl &url, QObject *parent)
{
    PluginFactory *f = globalFactory->pluginFactory();
    if (!f) {
        return 0;
    }
    return f->createKioMediaStream(url, parent);
}

QIcon Factory::icon(const QString &name)
{
    PluginFactory *f = globalFactory->pluginFactory();
    if (!f) {
        return QIcon();
    }
    return f->icon(name);
}

void Factory::notification(const char *notificationName, const QString &text,
        const QStringList &actions, QObject *receiver,
        const char *actionSlot)
{
    PluginFactory *f = globalFactory->pluginFactory();
    if (f) {
        f->notification(notificationName, text, actions, receiver, actionSlot);
    }
}

QString Factory::applicationName()
{
    const PluginFactory *f = globalFactory->pluginFactory();
    if (f) {
        return f->applicationName();
    }
    return QCoreApplication::applicationName();
}

QObject *Factory::backend(bool createWhenNull)
{
    if (createWhenNull && globalFactory->m_backendObject == 0) {
        globalFactory->createBackend();
        // XXX: might create "reentrancy" problems:
        // a method calls this method and is called again because the
        // backendChanged signal is emitted
        emit globalFactory->backendChanged();
    }
    return globalFactory->m_backendObject;
}

QString Factory::identifier()
{
    if (globalFactory->service) {
        return globalFactory->service->library();
    }
    return QString();
}

QString Factory::backendName()
{
    if (globalFactory->service)
        return globalFactory->service->name();
    else
        return QString();
}

QString Factory::backendComment()
{
    if (globalFactory->service)
        return globalFactory->service->comment();
    else
        return QString();
}

QString Factory::backendVersion()
{
    if (globalFactory->service)
        return globalFactory->service->property("X-KDE-PhononBackendInfo-Version").toString();
    else
        return QString();
}

QString Factory::backendIcon()
{
    if (globalFactory->service)
        return globalFactory->service->icon();
    else
        return QString();
}

QString Factory::backendWebsite()
{
    if (globalFactory->service)
        return globalFactory->service->property("X-KDE-PhononBackendInfo-Website").toString();
    else
        return QString();
}

QObject *Factory::registerQObject(QObject *o)
{
    if (o) {
        QObject::connect(o, SIGNAL(destroyed(QObject *)), globalFactory, SLOT(objectDestroyed(QObject *)), Qt::DirectConnection);
        globalFactory->objects.append(o);
    }
    return o;
}

} //namespace Phonon

#include "factory.moc"
#include "factory_p.moc"

// vim: sw=4 ts=4
