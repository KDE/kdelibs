#include "kdedadaptor.h"
#include "kded.h"
#include <kglobal.h>
#include <kconfig.h>
#include <QCoreApplication>

KdedAdaptor::KdedAdaptor(QObject *parent)
   : QDBusAbstractAdaptor(parent)
{
}

bool KdedAdaptor::loadModule(const QString &module)
{
   return Kded::self()->loadModule(module, false) != 0;
}

bool KdedAdaptor::unloadModule(const QString &module)
{
   return Kded::self()->unloadModule(module);
}

void KdedAdaptor::registerWindowId(qlonglong windowId, const QDBusMessage &msg)
{
   Kded::self()->registerWindowId(windowId, msg.service());
}

void KdedAdaptor::unregisterWindowId(qlonglong windowId, const QDBusMessage &msg)
{
   Kded::self()->unregisterWindowId(windowId, msg.service());
}

QStringList KdedAdaptor::loadedModules()
{
   return Kded::self()->loadedModules();
}

void KdedAdaptor::reconfigure()
{
   KGlobal::config()->reparseConfiguration();
   Kded::self()->initModules();
}

void KdedAdaptor::loadSecondPhase()
{
   Kded::self()->loadSecondPhase();
}

void KdedAdaptor::quit()
{
   QCoreApplication::instance()->quit();
}

#include "kdedadaptor.moc"
