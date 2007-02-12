/*  This file is part of the KDE project
    Copyright (C) 2004-2006 Matthias Kretz <kretz@kde.org>

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
#include "base_p.h"

#include <kservicetypetrader.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <QFile>
#include <QList>
#include <klocale.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <QCoreApplication>

#include <QtDBus/QtDBus>
#include "backendinterface.h"
#include "factory_p.h"
#include <kglobal.h>
#include "audiooutput.h"
#include "audiooutput_p.h"

#define PHONON_LOAD_BACKEND_GLOBAL 0

namespace Phonon
{

K_GLOBAL_STATIC(Phonon::FactoryPrivate, globalFactory)

const KComponentData &Factory::componentData()
{
    if (!globalFactory->componentData.isValid()) {
        Q_ASSERT(KGlobal::hasMainComponent());
        globalFactory->componentData = KComponentData("phonon");
    }
    return globalFactory->componentData;
}

void FactoryPrivate::createBackend()
{
    /*if (service) {
        // we already have a valid backend lib opened. force it to unload:
        KLibLoader::self()->unloadLibrary(QFile::encodeName(ptr->library()));
    }*/
    const KService::List offers = KServiceTypeTrader::self()->query("PhononBackend",
            "Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1");
    KService::List::const_iterator it = offers.begin();
    const KService::List::const_iterator end = offers.end();
    QStringList errormsg;
    for (; it != end; ++it) {
        KService::Ptr ptr = *it;
        KLibFactory* factory = 0;
#ifdef PHONON_LOAD_BACKEND_GLOBAL
        // This code is in here temporarily until NMM gets fixed.
        // Currently the NMM backend will fail with undefined symbols if
        // the backend is not loaded with global symbol resolution
        KLibrary* library = KLibLoader::self()->library(QFile::encodeName(ptr->library()), QLibrary::ExportExternalSymbolsHint);
        if (library) {
            factory = library->factory();
        }
#else
        factory = KLibLoader::self()->factory(QFile::encodeName(ptr->library()));
#endif
        if (factory) {
            backend = factory->create();
            if (0 == backend) {
                QString e = i18n("create method returned 0");
                errormsg.append(e);
                kDebug(600) << "Error getting backend from factory for " <<
                    ptr->name() << ", " << ptr->library() << ":\n" << e << endl;
            } else {
                service = ptr;
                kDebug(600) << "using backend: " << ptr->name() << endl;
                break;
            }
        } else {
            QString e = KLibLoader::self()->lastErrorMessage();
            errormsg.append(e);
            kDebug(600) << "Error getting factory for " << ptr->name() <<
                ":\n" << e << endl;
        }
    }
    if (backend) {
        connect(backend, SIGNAL(objectDescriptionChanged(ObjectDescriptionType)),
                SLOT(objectDescriptionChanged(ObjectDescriptionType)));
    } else {
        if (offers.size() == 0) {
            KMessageBox::error(0, i18n("Unable to find a Multimedia Backend"));
        } else {
            QString details = "<qt><table>";
            QStringList::Iterator eit = errormsg.begin();
            QStringList::Iterator eend = errormsg.end();
            KService::List::const_iterator oit = offers.begin();
            const KService::List::const_iterator oend = offers.end();
            for (; eit != eend || oit != oend; ++eit, ++oit)
                details += QString("<tr><td><b>%1</b></td><td>%2</td></tr>")
                    .arg((*oit)->name()).arg(*eit);
            details += "</table></qt>";

            KMessageBox::detailedError(0,
                    i18n("Unable to use any of the available Multimedia Backends"), details);
        }
    }
}

FactoryPrivate::FactoryPrivate()
    : backend(0)
{
    // Add the post routine to make sure that all other global statics (especially the ones from Qt)
    // are still available. If the FactoryPrivate dtor is called too late many bad things can happen
    // as the whole backend might still be alive.
    qAddPostRoutine(globalFactory.destroy);
    qRegisterMetaType<qint64>("qint64");
    qRegisterMetaType<qint32>("qint32");
    QDBusConnection::sessionBus().connect(QString(), QString(), "org.kde.Phonon.Factory",
            "phononBackendChanged", this, SLOT(phononBackendChanged()));
}

FactoryPrivate::~FactoryPrivate()
{
    emit aboutToBeDestroyed();

    foreach(BasePrivate* bp, basePrivateList) {
        bp->deleteIface();
    }
    if (objects.size() > 0) {
        kError(600) << "The backend objects are not deleted as was requested." << endl;
        qDeleteAll(objects);
    }
    delete backend;
}

void FactoryPrivate::objectDescriptionChanged(ObjectDescriptionType type)
{
    kDebug(600) << k_funcinfo << type << endl;
    switch (type) {
        case AudioOutputDeviceType:
            // tell all AudioOutput objects to check their output device preference
            foreach (BasePrivate *obj, globalFactory->basePrivateList) {
                AudioOutputPrivate *output = dynamic_cast<AudioOutputPrivate *>(obj);
                if (output) {
                    output->deviceListChanged();
                }
            }
            break;
        default:
            break;
    }
    //emit capabilitiesChanged();
}

Factory::Sender *Factory::sender()
{
    return globalFactory;
}

void Factory::registerFrontendObject(BasePrivate* bp)
{
    globalFactory->basePrivateList.prepend(bp); // inserted last => deleted first
}

void Factory::deregisterFrontendObject(BasePrivate* bp)
{
    // The Factory can already be cleaned up while there are other frontend objects still alive.
    // When those are deleted they'll call deregisterFrontendObject through ~BasePrivate
    if (!globalFactory.isDestroyed()) {
        globalFactory->basePrivateList.removeAll(bp);
    }
}

void FactoryPrivate::phononBackendChanged()
{
    if (backend) {
        foreach(BasePrivate* bp, basePrivateList) {
            bp->deleteIface();
        }
        if (objects.size() > 0) {
            kWarning(600) << "we were asked to change the backend but the application did\n"
                "not free all references to objects created by the factory. Therefore we can not\n"
                "change the backend without crashing. Now we have to wait for a restart to make\n"
                "backendswitching possible." << endl;
            // in case there were objects deleted give 'em a chance to recreate
            // them now
            foreach(BasePrivate* bp, basePrivateList) {
                bp->createIface();
            }
            return;
        }
        delete backend;
        backend = 0;
    }
    createBackend();
    foreach(BasePrivate* bp, basePrivateList) {
        bp->createIface();
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
    //kDebug(600) << k_funcinfo << obj << endl;
    objects.removeAll(obj);
}

#define FACTORY_IMPL(classname) \
QObject* Factory::create ## classname(QObject* parent) \
{ \
    if (backend()) { \
        return registerQObject(qobject_cast<BackendInterface*>(backend())->createObject0(BackendInterface::classname##Class, parent)); \
    } \
    return 0; \
}
#define FACTORY_IMPL_1ARG(classname) \
QObject* Factory::create ## classname(int arg1, QObject* parent) \
{ \
    if (backend()) { \
        return registerQObject(qobject_cast<BackendInterface*>(backend())->createObject1(BackendInterface::classname##Class, parent, arg1)); \
    } \
    return 0; \
}

FACTORY_IMPL(MediaObject)
FACTORY_IMPL(MediaQueue)
FACTORY_IMPL(AvCapture)
FACTORY_IMPL(ByteStream)
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

#undef FACTORY_IMPL

QObject* Factory::backend(bool createWhenNull)
{
    if (createWhenNull && globalFactory->backend == 0) {
        globalFactory->createBackend();
        // XXX: might create "reentrancy" problems:
        // a method calls this method and is called again because the
        // backendChanged signal is emitted
        emit globalFactory->backendChanged();
    }
    return globalFactory->backend;
}

const char* Factory::uiLibrary()
{
    if (!backend()) {
        return 0;
    }
    const char* ret = 0;
    QMetaObject::invokeMethod(globalFactory->backend, "uiLibrary", Qt::DirectConnection, Q_RETURN_ARG(const char*, ret));
    return ret;
}

const char* Factory::uiSymbol()
{
	if (!backend())
		return 0;
	const char* ret = 0;
	// the backend doesn't have to implement the symbol - the default factory
	// symbol will be used then
	if (QMetaObject::invokeMethod(globalFactory->backend, "uiSymbol", Qt::DirectConnection, Q_RETURN_ARG(const char*, ret)))
		return ret;
	return 0;
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

QObject* Factory::registerQObject(QObject* o)
{
    if (o) {
        QObject::connect(o, SIGNAL(destroyed(QObject*)), globalFactory, SLOT(objectDestroyed(QObject*)), Qt::DirectConnection);
        globalFactory->objects.append(o);
    }
    return o;
}

} //namespace Phonon

#include "factory.moc"
#include "factory_p.moc"

// vim: sw=4 ts=4
