#ifndef SOLID_BACKENDS_FAKEHW_FAKEDEVICE_P_H
#define SOLID_BACKENDS_FAKEHW_FAKEDEVICE_P_H

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QObject>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeDevice;
class FakeDevice::Private : public QObject
{
    Q_OBJECT;
public:
    QString udi;
    QMap<QString, QVariant> propertyMap;
    QStringList interfaceList;
    bool locked;
    QString lockReason;
    bool broken;

Q_SIGNALS:
    void propertyChanged(const QMap<QString,int> &changes);
    void conditionRaised(const QString &condition, const QString &reason);

    friend class FakeDevice;
};
}
}
}

#endif
