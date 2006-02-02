	/*

	Copyright (C) 2001 Nikolas Zimmermann <wildfox@kde.org>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
  
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public License
	along with this library; see the file COPYING.LIB.  If not, write to
	the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.

	*/

#include <qiomanager.h>
#include <dispatcher.h>

#include <kdebug.h>

#include "kartsdispatcher.moc"

int KArtsDispatcher::m_refCount = 0;
Arts::Dispatcher *KArtsDispatcher::artsDispatcher = 0;
Arts::QIOManager *KArtsDispatcher::artsQIOManager = 0;

KArtsDispatcher::KArtsDispatcher(QObject *parent, const char *name)
	: QObject(parent, name)
{
	m_refCount++;
	if(artsDispatcher == 0)
	{
		if (!Arts::Dispatcher::the()) // only if no Arts::Dispatcher is created yet
		{
			artsQIOManager = new Arts::QIOManager();
			artsDispatcher = new Arts::Dispatcher(artsQIOManager);
		}
		else
			kWarning(400) << "An Arts::Dispatcher() instance exists already while trying to instantiate KArtsDispatcher!" << endl;
	}
}

KArtsDispatcher::~KArtsDispatcher()
{
	m_refCount--;
	if(m_refCount == 0)
	{
		delete KArtsDispatcher::artsDispatcher;
		artsDispatcher = 0;

		delete KArtsDispatcher::artsQIOManager;
		artsQIOManager = 0;
	}
}
