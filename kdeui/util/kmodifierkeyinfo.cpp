/*
    Copyright 2009  Michael Leupold <lemma@confuego.org>

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
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kmodifierkeyinfo.h"
#include "kmodifierkeyinfoprovider_p.h"

KModifierKeyInfo::KModifierKeyInfo(QObject *parent)
    : QObject(parent), p(new KModifierKeyInfoProvider)
{
    connect(p, SIGNAL(keyPressed(Qt::Key, bool)), this, SIGNAL(keyPressed(Qt::Key, bool)));
    connect(p, SIGNAL(keyLatched(Qt::Key, bool)), this, SIGNAL(keyLatched(Qt::Key, bool)));
    connect(p, SIGNAL(keyLocked(Qt::Key, bool)), this, SIGNAL(keyLocked(Qt::Key, bool)));
    connect(p, SIGNAL(buttonPressed(Qt::MouseButton, bool)),
            this, SIGNAL(buttonPressed(Qt::MouseButton, bool)));
    connect(p, SIGNAL(keyAdded(Qt::Key)), this, SIGNAL(keyAdded(Qt::Key)));
    connect(p, SIGNAL(keyRemoved(Qt::Key)), this, SIGNAL(keyRemoved(Qt::Key)));
}

KModifierKeyInfo::~KModifierKeyInfo()
{
    delete p;
}

bool KModifierKeyInfo::knowsKey(Qt::Key key) const
{
    return p->knowsKey(key);
}

const QList<Qt::Key> KModifierKeyInfo::knownKeys() const
{
    return p->knownKeys();
}

bool KModifierKeyInfo::isKeyPressed(Qt::Key key) const
{
    return p->isKeyPressed(key);
}

bool KModifierKeyInfo::isKeyLatched(Qt::Key key) const
{
    return p->isKeyLatched(key);
}

bool KModifierKeyInfo::setKeyLatched(Qt::Key key, bool latched)
{
    return p->setKeyLatched(key, latched);
}

bool KModifierKeyInfo::isKeyLocked(Qt::Key key) const
{
    return p->isKeyLocked(key);
}

bool KModifierKeyInfo::setKeyLocked(Qt::Key key, bool locked)
{
    return p->setKeyLocked(key, locked);
}

bool KModifierKeyInfo::isButtonPressed(Qt::MouseButton button) const
{
    return p->isButtonPressed(button);
}

#include "kmodifierkeyinfo.moc"
