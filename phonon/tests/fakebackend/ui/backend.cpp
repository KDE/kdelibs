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

#include "backend.h"
#include "videowidget.h"
#include <kgenericfactory.h>

typedef KGenericFactory<Phonon::Fake::UiBackend, Phonon::Fake::UiBackend> FakeUiBackendFactory;
K_EXPORT_COMPONENT_FACTORY( phonon_fakeui, FakeUiBackendFactory( "fakeuibackend" ) )

namespace Phonon
{
namespace Fake
{

UiBackend::UiBackend( QObject* parent, const QStringList& )
	: QObject( parent )
{
}

UiBackend::~UiBackend()
{
}

QObject* UiBackend::createVideoWidget( QWidget* parent )
{
	return new Fake::VideoWidget( parent );
}

}}

#include "backend.moc"
// vim: sw=4 ts=4
