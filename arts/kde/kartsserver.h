// Copyright (c) 2000-2001 Charles Samuels <charles@kde.org>
//                         Neil Stevens <neil@qualityassistant.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIAB\ILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef KARTSSERVER_H
#define KARTSSERVER_H

#include <kmedia2.h>
#include <qobject.h>
#include <soundserver.h>

/**
 * KArtsServer is a wrapper to conveniently get a reference to a SoundServer,
 * and restart artsd when necessary, using the kcontrol settings.
 *
 * Of course we'd rather that artsd never crashed, and that all users start
 * artsd on KDE startup, but at the very least there will always be third
 * party PlayObjects that will crash.  So, this is necessary.
 */
class KArtsServer : public QObject
{
Q_OBJECT

public:
	KArtsServer(QObject *parent = 0, const char *name = 0);
	virtual ~KArtsServer(void);

	/**
	 * Get a verified reference to the SoundServerV2, (re)starting artsd
	 * using the kcontrol-specified settings if necessary.
	 */
	Arts::SoundServerV2 server(void);

signals:
	/**
	 * This is emitted whenever KArtsServer has to start artsd
	 */
	void restartedServer(void);

private:
	struct Data;
	Data *d;
};

#endif
