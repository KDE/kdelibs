/*
    Copyright 2013 Patrick von Reth <vonreth@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef WINBATTERY_H
#define WINBATTERY_H


#include <solid/ifaces/battery.h>

#include "wininterface.h"

namespace Solid
{
namespace Backends
{
namespace Win
{
class WinBattery : public WinInterface, virtual public Solid::Ifaces::Battery
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Battery)

public:
    typedef QPair<QString,ulong> Battery;

    WinBattery(WinDevice *device);

    virtual bool isPlugged() const;

    virtual Solid::Battery::BatteryType type() const;

    virtual int chargePercent() const;

    virtual int capacity() const;

    virtual bool isRechargeable() const;

    virtual bool isPowerSupply() const;

    virtual Solid::Battery::ChargeState chargeState() const;

    static QSet<QString> getUdis();

    static const Battery batteryInfoFromUdi(const QString &udi);

    Solid::Battery::Technology technology() const;

    double energy() const;

    double energyRate() const;

    double voltage() const;

signals:
    void chargePercentChanged(int value, const QString &udi) ;
    void capacityChanged(int value, const QString &udi);
    void chargeStateChanged(int newState, const QString &udi);
    void plugStateChanged(bool newState, const QString &udi);
    void powerSupplyStateChanged(bool newState, const QString &udi);
    void energyChanged(double energy, const QString &udi);
    void energyRateChanged(double energyRate, const QString &udi);

private slots:
    void powerChanged();

private:

    static QMap<QString,Battery> m_udiToGDI;
    bool m_pluggedIn;
    Solid::Battery::BatteryType m_type;
    int m_charge;
    int m_capacity;
    bool m_rechargabel;
    bool m_isPowerSupply;
    Solid::Battery::ChargeState m_state;
    Solid::Battery::Technology m_technology;
    double m_energy;
    double m_energyRate;
    double m_voltage;

    

};
}
}
}

#endif // WINBATTERY_H
