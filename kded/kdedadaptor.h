#ifndef KDEDADAPTOR_H
#define KDEDADAPTOR_H

#include <QtDBus/QtDBus>

class KdedAdaptor: public QDBusAbstractAdaptor
{
   Q_OBJECT
   Q_CLASSINFO("D-Bus Interface", "org.kde.kded")
public:
   KdedAdaptor(QObject *parent);

public Q_SLOTS:
   bool loadModule(const QString &obj);
   QStringList loadedModules();
   bool unloadModule(const QString &obj);
   //bool isWindowRegistered(qlonglong windowId) const;
   void registerWindowId(qlonglong windowId, const QDBusMessage&);
   void unregisterWindowId(qlonglong windowId, const QDBusMessage&);
   void reconfigure();
   void loadSecondPhase();
   void quit();
};

#endif
