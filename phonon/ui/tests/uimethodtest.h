/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#ifndef PHONON_UIMETHODTEST_H
#define PHONON_UIMETHODTEST_H

#include <QObject>
#include "../../tests/loadfakebackend.h"

class UiMethodTest : public QObject
{
	Q_OBJECT
	private Q_SLOTS:
        void initTestCase() { Phonon::loadFakeBackend(); }
		void checkUiBackendMethods_data();
		void checkUiBackendMethods();
		void checkVideoWidgetMethods_data();
		void checkVideoWidgetMethods();

	private:
		void addColumns();
		void addMethod( const char* returnType, const char* signature, bool optional = false );
                void addSignal(const char *signature);
		void checkMethods( QObject* backendObject );
		QObject* backendObject;
		const QMetaObject* meta;
};
#endif // PHONON_UIMETHODTEST_H
// vim: ts=4
