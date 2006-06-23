/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KSEPARATORACTION_H
#define KSEPARATORACTION_H

#include <kaction.h>

/**
 * \short An action which acts as a separator.
 *
 * This is a small convenience class which
 * acts as an action separator when it is plugged into a widget.
 *
 * \note unlike KSeparatorAction in KDE3, this action \e cannot be
 *       inserted into a widget more than once, so you will need multiple
 *       separator actions to have multiple separators.
 */
class KDEUI_EXPORT KSeparatorAction : public KAction
{
    Q_OBJECT

public:
    /**
     * Constructs a new separator.
     */
    explicit KSeparatorAction( KActionCollection* parent = 0, const QString& name = QString() );
};

#endif
