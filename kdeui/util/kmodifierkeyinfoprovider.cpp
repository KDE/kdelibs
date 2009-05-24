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

#include "kmodifierkeyinfoprovider_p.h"

#include <kglobal.h>

bool KModifierKeyInfoProvider::isKeyPressed(Qt::Key key) const
{
    if (m_modifierStates.contains(key)) {
        return m_modifierStates[key] & Pressed;
    }
    return false;
}

bool KModifierKeyInfoProvider::isKeyLatched(Qt::Key key) const
{
    if (m_modifierStates.contains(key)) {
        return m_modifierStates[key] & Latched;
    }
    return false;
}

bool KModifierKeyInfoProvider::isKeyLocked(Qt::Key key) const
{
    if (m_modifierStates.contains(key)) {
        return m_modifierStates[key] & Locked;
    }
    return false;
}

bool KModifierKeyInfoProvider::isButtonPressed(Qt::MouseButton button) const
{
    if (m_buttonStates.contains(button)) {
        return m_buttonStates[button];
    }
    return false;
}

bool KModifierKeyInfoProvider::knowsKey(Qt::Key key) const
{
    return m_modifierStates.contains(key);
}

const QList<Qt::Key> KModifierKeyInfoProvider::knownKeys() const
{
    return m_modifierStates.keys();
}

#include "kmodifierkeyinfoprovider_p.moc"
