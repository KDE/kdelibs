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
	the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA 02111-1307, USA.

	*/

#ifndef KARTSDISPATCHER_H
#define KARTSDISPATCHER_H

#include <qobject.h>

namespace Arts
{
	class QIOManager;
	class Dispatcher;
};

/**
 * KArtsDispatcher ensures that an instance of Arts::Dispatcher using an
 * Arts::QIOManager exists. When the last KArtsDispatcher is deleted, the
 * Arts::Dispatcher is released as well.
 *
 * Using KArtsDispatcher is especially useful in setups where another plugin
 * might also already use an Arts::Dispatcher, for instance in konqueror.
 *
 * <pre>
 * {
 *   // old code
 *   Arts::QIOManager qiomanager;
 *   Arts::Dispatcher dispatcher(&qiomanager);
 *
 *   Arts::SoundServer server = Arts::Reference("global:Arts_SoundServer");
 *   server.play("/usr/share/sounds/pop.wav");
 * }
 * </pre>
 *
 * <pre>
 * {
 *   // new code
 *   KArtsDispatcher dispatcher;
 *
 *   Arts::SoundServer server = Arts::Reference("global:Arts_SoundServer");
 *   server.play("/usr/share/sounds/pop.wav");
 * }
 * </pre>
 */
class KArtsDispatcher : public QObject
{
Q_OBJECT
public:
	/**
	 * Constructor.
	 *
	 * @param parent  the parent Qt object
	 * @param name    the Qt object name of this object
	 */
	KArtsDispatcher(QObject *parent = 0, const char *name = 0);

	/**
	 * Destructor
	 */
	~KArtsDispatcher();
	
private:
	static int m_refCount;
	static Arts::Dispatcher *artsDispatcher;
	static Arts::QIOManager *artsQIOManager;
};

#endif
