/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "predicate.h"

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <QtCore/QStringList>
#include <QtCore/QMetaEnum>

namespace Solid
{
    class Predicate::Private
    {
    public:
        enum OperatorType { AtomType, AndType, OrType, IsType };

        Private() : isValid(false), type(AtomType),
                    compOperator(Predicate::Equals),
                    operand1(0), operand2(0) {}

        bool isValid;
        OperatorType type;

        DeviceInterface::Type ifaceType;
        QString property;
        QVariant value;
        Predicate::ComparisonOperator compOperator;

        Predicate *operand1;
        Predicate *operand2;
    };
}


Solid::Predicate::Predicate()
    : d(new Private())
{
}

Solid::Predicate::Predicate(const Predicate &other)
    : d(new Private())
{
    *this = other;
}

Solid::Predicate::Predicate(const DeviceInterface::Type &ifaceType,
                            const QString &property, const QVariant &value,
                            ComparisonOperator compOperator)
    : d(new Private())
{
    d->isValid = true;
    d->ifaceType = ifaceType;
    d->property = property;
    d->value = value;
    d->compOperator = compOperator;
}

Solid::Predicate::Predicate(const QString &ifaceName,
                            const QString &property, const QVariant &value,
                            ComparisonOperator compOperator)
    : d(new Private())
{
    DeviceInterface::Type ifaceType = DeviceInterface::stringToType(ifaceName);

    if (((int)ifaceType)!=-1)
    {
        d->isValid = true;
        d->ifaceType = ifaceType;
        d->property = property;
        d->value = value;
        d->compOperator = compOperator;
    }
}

Solid::Predicate::Predicate(const DeviceInterface::Type &ifaceType)
    : d(new Private())
{
    d->isValid = true;
    d->type = Private::IsType;
    d->ifaceType = ifaceType;
}

Solid::Predicate::Predicate(const QString &ifaceName)
    : d(new Private())
{
    DeviceInterface::Type ifaceType = DeviceInterface::stringToType(ifaceName);

    if (((int)ifaceType)!=-1)
    {
        d->isValid = true;
        d->type = Private::IsType;
        d->ifaceType = ifaceType;
    }
}

Solid::Predicate::~Predicate()
{
    if (d->type!=Private::AtomType && d->type!=Private::IsType) {
        delete d->operand1;
        delete d->operand2;
    }

    delete d;
}

Solid::Predicate &Solid::Predicate::operator=(const Predicate &other)
{
    d->isValid = other.d->isValid;
    d->type = other.d->type;

    if (d->type!=Private::AtomType && d->type!=Private::IsType)
    {
        Predicate* operand1 = new Predicate(*(other.d->operand1));
        delete d->operand1;
        d->operand1 = operand1;
        Predicate* operand2 = new Predicate(*(other.d->operand2));
        delete d->operand2;
        d->operand2 = operand2;
    }
    else
    {
        d->ifaceType = other.d->ifaceType;
        d->property = other.d->property;
        d->value = other.d->value;
        d->compOperator = other.d->compOperator;
    }

    return *this;
}

Solid::Predicate Solid::Predicate::operator &(const Predicate &other)
{
    Predicate result;

    result.d->isValid = true;
    result.d->type = Private::AndType;
    result.d->operand1 = new Predicate(*this);
    result.d->operand2 = new Predicate(other);

    return result;
}

Solid::Predicate &Solid::Predicate::operator &=(const Predicate &other)
{
    *this = *this & other;
    return *this;
}

Solid::Predicate Solid::Predicate::operator|(const Predicate &other)
{
    Predicate result;

    result.d->isValid = true;
    result.d->type = Private::OrType;
    result.d->operand1 = new Predicate(*this);
    result.d->operand2 = new Predicate(other);

    return result;
}

Solid::Predicate &Solid::Predicate::operator |=(const Predicate &other)
{
    *this = *this | other;
    return *this;
}

bool Solid::Predicate::isValid() const
{
    return d->isValid;
}

bool Solid::Predicate::matches(const Device &device) const
{
    if (!d->isValid) return false;

    switch(d->type)
    {
    case Private::OrType:
        return d->operand1->matches(device)
            || d->operand2->matches(device);
    case Private::AndType:
        return d->operand1->matches(device)
            && d->operand2->matches(device);
    case Private::AtomType:
    {
        const DeviceInterface *iface = device.asDeviceInterface(d->ifaceType);

        if (iface!=0)
        {
            QVariant value = iface->property(d->property.toLatin1());
            QVariant expected = d->value;

            int index = iface->metaObject()->indexOfProperty(d->property.toLatin1());
            QMetaProperty metaProp = iface->metaObject()->property(index);

            if (metaProp.isEnumType() && expected.type()==QVariant::String) {
                QMetaEnum metaEnum = metaProp.enumerator();
                expected = QVariant(metaEnum.keysToValue(d->value.toString().toLatin1()));
            }

            if (d->compOperator==Mask) {
                bool v_ok;
                int v = value.toInt(&v_ok);
                bool e_ok;
                int e = expected.toInt(&e_ok);

                return (e_ok && v_ok && (v &e));
            } else {
                return (value == expected);
            }
        }
        break;
    }
    case Private::IsType:
        return device.isDeviceInterface(d->ifaceType);
    }

    return false;
}

QSet<Solid::DeviceInterface::Type> Solid::Predicate::usedTypes() const
{
    QSet<DeviceInterface::Type> res;

    if (d->isValid) {

        switch(d->type)
        {
        case Private::OrType:
        case Private::AndType:
            res+= d->operand1->usedTypes();
            res+= d->operand2->usedTypes();
            break;
        case Private::AtomType:
        case Private::IsType:
            res << d->ifaceType;
            break;
        }

    }

    return res;
}


QString Solid::Predicate::toString() const
{
    if (!d->isValid) return "False";

    if (d->type!=Private::AtomType && d->type!=Private::IsType)
    {
        QString op = " AND ";
        if (d->type==Private::OrType) op = " OR ";

        return '['+d->operand1->toString()+op+d->operand2->toString()+']';
    }
    else
    {
        QString ifaceName = DeviceInterface::typeToString(d->ifaceType);

        if (ifaceName.isEmpty()) ifaceName = "Unknown";

        if (d->type==Private::IsType) {
            return "IS "+ifaceName;
        }

        QString value;

        switch (d->value.type())
        {
        case QVariant::StringList:
        {
            value = "{";

            QStringList list = d->value.toStringList();

            QStringList::ConstIterator it = list.begin();
            QStringList::ConstIterator end = list.end();

            for (; it!=end; ++it)
            {
                value+= '\''+ *it+'\'';

                if (it+1!=end)
                {
                    value+= ", ";
                }
            }

            value+= '}';
            break;
        }
        case QVariant::Bool:
            value = (d->value.toBool()?"true":"false");
            break;
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong:
            value = d->value.toString();
            break;
        default:
            value = '\''+d->value.toString()+'\'';
            break;
        }

        QString str_operator = "==";
        if (d->compOperator!=Equals) str_operator = " &";


        return ifaceName+'.'+d->property+' '+str_operator+' '+value;
    }
}




