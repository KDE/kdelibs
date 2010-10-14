/* This file is part of the KDE libraries
 *
 * Copyright (c) 2010 Aurélien Gâteau <agateau@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef KDUALACTION_P_H
#define KDUALACTION_P_H

class KDualActionPrivate
{
public:
    KDualAction *q;

    KGuiItem items[2];
    bool autoToggle;
    bool isActive;

    void init(KDualAction *q_ptr);
    void updateFromCurrentState();
    KGuiItem& item(bool active) { return active ? items[1] : items[0]; }
    KGuiItem& item(KDualAction::State state) { return state == KDualAction::ActiveState ? items[1] : items[0]; }
    void slotTriggered();
};

#endif /* KDUALACTION_P_H */
