/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "ksettingswidgetadaptor.h"
#include <QtCore/QString>
#include <kapplication.h>
#include <kdebug.h>

KSettingsWidgetAdaptor::KSettingsWidgetAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
	kDebug() << k_funcinfo << endl;
	setAutoRelaySignals(true);
}
KSettingsWidgetAdaptor::~KSettingsWidgetAdaptor()
{
	kDebug() << k_funcinfo << endl;
}

QString KSettingsWidgetAdaptor::applicationName()
{
	return kapp->caption();
}

#include "ksettingswidgetadaptor.moc"
// vim: sw=4 ts=4 tw=80
