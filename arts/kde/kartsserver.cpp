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

#include <flowsystem.h>
#include <ksimpleconfig.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <qdir.h>
#include <qfile.h>
#include "kartsserver.h"

struct KArtsServer::Data
{
	Arts::SoundServerV2 server;
};

KArtsServer::KArtsServer(QObject *parent, const char *name)
	: QObject(parent, name)
	, d(new Data)
{
	d->server = Arts::SoundServerV2::null();
}

KArtsServer::~KArtsServer(void)
{
	d->server = Arts::SoundServerV2::null();
	delete d;
}

Arts::SoundServerV2 KArtsServer::server(void)
{
	if(d->server.isNull())
		d->server = Arts::Reference("global:Arts_SoundServerV2");

	if(!d->server.isNull() && !d->server.error())
		return d->server;

	// aRts seems not to be running, let's try to run it
	// First, let's read the configuration as in kcmarts
	KConfig config("kcmartsrc", false /*bReadOnly*/, false /*bUseKDEGlobals*/);
	KProcess proc;

	config.setGroup("Arts");

	bool rt = config.readBoolEntry("StartRealtime", false);
	bool x11Comm = config.readBoolEntry("X11GlobalComm", false);

	// put the value of x11Comm into .mcoprc
	KSimpleConfig X11CommConfig(QDir::homeDirPath()+"/.mcoprc");

	if(x11Comm)
		X11CommConfig.writeEntry("GlobalComm", "Arts::X11GlobalComm");
	else
		X11CommConfig.writeEntry("GlobalComm", "Arts::TmpGlobalComm");

	X11CommConfig.sync();
	
	proc << QFile::encodeName(KStandardDirs::findExe(QString::fromLatin1("kdeinit_wrapper")));

	if(rt)
		proc << QFile::encodeName(KStandardDirs::findExe(QString::fromLatin1("artswrapper")));
	else
		proc << QFile::encodeName(KStandardDirs::findExe(QString::fromLatin1("artsd")));

	proc << config.readEntry("Arguments", "-F 10 -S 4096 -s 60 -m artsmessage -l 3 -f");

	if(proc.start(KProcess::Block) && proc.normalExit())
	{
		// We could have a race-condition here.
		// The correct way to do it is to make artsd fork-and-exit
		// after starting to listen to connections (and running artsd
		// directly instead of using kdeinit), but this is better
		// than nothing.
		int time = 0;
		do
		{
			sleep(1);
			d->server = Arts::Reference("global:Arts_SoundServerV2");
		} while(++time < 5 && (d->server.isNull()));

		emit restartedServer();
	}
	// TODO else what?

	return d->server;
}

#include "kartsserver.moc"
