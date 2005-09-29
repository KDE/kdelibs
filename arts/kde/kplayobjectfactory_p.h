/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#ifndef KPLAYOBJECTFACTORY_P_H
#define KPLAYOBJECTFACTORY_P_H

#include <qobject.h>
#include "kplayobjectfactory.h"

namespace KDE
{

/**
 * @internal
 */
class POFHelper : public QObject
{
	friend class PlayObjectFactory;
	Q_OBJECT
private slots:
	void connectAmanPlay();
private:
	KDE::PlayObject *po;
	KAudioManagerPlay *ap;
};

}

#endif // KPLAYOBJECTFACTORY_P_H
// vim: sw=4 ts=4 noet
