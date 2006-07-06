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

#ifndef KCMODULEPROXYADAPTOR_H
#define KCMODULEPROXYADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
class QString;

/*
 * Adaptor class for interface org.kde.internal.KSettingsWidget
 */
class KSettingsWidgetAdaptor: public QDBusAbstractAdaptor
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.internal.KSettingsWidget")
	/*
	Q_CLASSINFO("D-Bus Introspection", ""
			"  <interface name=\"org.kde.internal.KSettingsWidget\" >\n"
			"    <method name=\"applicationName\" >\n"
			"      <arg direction=\"out\" type=\"s\" />\n"
			"    </method>\n"
			"  </interface>\n"
			"")*/
	public:
		KSettingsWidgetAdaptor(QObject *parent);
		~KSettingsWidgetAdaptor();

	public Q_SLOTS:
		QString applicationName();
};

#endif // KCMODULEPROXYADAPTOR_H
// vim: sw=4 ts=4 tw=80
