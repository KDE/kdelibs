/*
   This file is part of the KDE libraries
   Copyright (C) 2008 Benjamin Reed <rangerrick@befunk.com>

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

#include "kkernel_mac.h"

#include <config.h>

#ifdef Q_OS_MACX

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <crt_externs.h>
#include <mach-o/dyld.h>

#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURL.h>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QStringList>
#include <QtCore/qvarlengtharray.h>
#include <kstandarddirs.h>
#include <ksharedconfig.h>
#include <kconfig.h>
#include <kdebug.h>

int timeout = 3000; // msec

bool dbus_initialized = false;

/**
  qAppFileName() is not public in qt4/mac, so we need to redo it here
*/

static QString convert_CFString_to_QString(CFStringRef str) {
	CFIndex length = CFStringGetLength(str);
	const UniChar *chars = CFStringGetCharactersPtr(str);
	if (chars)
		return QString(reinterpret_cast<const QChar *>(chars), length);

	QVarLengthArray<UniChar> buffer(length);
	CFStringGetCharacters(str, CFRangeMake(0, length), buffer.data());
	return QString(reinterpret_cast<const QChar *>(buffer.constData()), length);
}

/**
 Calling CoreFoundation APIs (which is unavoidable in Qt/Mac) has always had issues
 on Mac OS X, but as of 10.5 is explicitly disallowed with an exception.  As a
 result, in the case where we would normally fork and then dlopen code, or continue
 to run other code, we must now fork-and-exec.
 
 See "CoreFoundation and fork()" at http://developer.apple.com/releasenotes/CoreFoundation/CoreFoundation.html
*/

void
mac_fork_and_reexec_self()
{
	int argc = *_NSGetArgc();
	char ** argv = *_NSGetArgv();
	char * newargv[argc+2];
	char progname[PATH_MAX];
	uint32_t buflen = PATH_MAX;
	_NSGetExecutablePath(progname, &buflen);
	bool found_psn = false;

	for (int i = 0; i < argc; i++) {
		newargv[i] = argv[i];
	}

	newargv[argc] = "--nofork";
	newargv[argc+1] = NULL;

	int x_fork_result = fork();
	switch(x_fork_result) {

		case -1:
#ifndef NDEBUG
			fprintf(stderr, "Mac OS X workaround fork() failed!\n");
#endif
			::_exit(255);
			break;

		case 0:
			// Child
			execvp(progname, newargv);
			break;

		default:
			// Parent
			_exit(0);
			break;

	}
}

/**
 Set the D-Bus environment based on session bus socket
*/

bool mac_set_dbus_address(QString value)
{
	if (!value.isEmpty() && QFile::exists(value) && (QFile::permissions(value) & QFile::WriteUser)) {
		value = "unix:path=" + value;
		::setenv("DBUS_SESSION_BUS_ADDRESS", value.toLocal8Bit(), 1);
		kDebug() << "set session bus address to" << value;
		return true;
	}
	return false;
}

/**
 Make sure D-Bus is initialized, by any means necessary.
*/

void mac_initialize_dbus()
{
	if (dbus_initialized)
		return;

	QString dbusVar = qgetenv("DBUS_SESSION_BUS_ADDRESS");
	if (!dbusVar.isEmpty()) {
		dbus_initialized = true;
		return;
	}

	dbusVar = QFile::decodeName(qgetenv("DBUS_LAUNCHD_SESSION_BUS_SOCKET"));
	if (mac_set_dbus_address(dbusVar)) {
		dbus_initialized = true;
		return;
	}

	QString externalProc;
	QStringList path = QFile::decodeName(qgetenv("KDEDIRS")).split(':').replaceInStrings(QRegExp("$"), "/bin");
	path << QFile::decodeName(qgetenv("PATH")).split(':') << "/usr/local/bin";

	for (int i = 0; i < path.size(); ++i) {
		QString testLaunchctl = QString(path.at(i)).append("/launchctl");
		if (QFile(testLaunchctl).exists()) {
			externalProc = testLaunchctl;
			break;
		}
	}

	if (!externalProc.isEmpty()) {
                QProcess qp;
                qp.setTextModeEnabled(true);

		qp.start(externalProc, QStringList() << "getenv" << "DBUS_LAUNCHD_SESSION_BUS_SOCKET");
                if (!qp.waitForFinished(timeout)) {
                    kDebug() << "error running" << externalProc << qp.errorString();
                    return;
                }
                if (qp.exitCode() != 0) {
                    kDebug() << externalProc << "unsuccessful:" << qp.readAllStandardError();
                    return;
                }

                QString line = qp.readLine().trimmed(); // read the first line
                if (mac_set_dbus_address(line))
                    dbus_initialized = true; // hooray
	}

	if (dbus_initialized == false) {
		kDebug() << "warning: unable to initialize D-Bus environment!";
	}

}

QString mac_app_filename() {
	static QString appFileName;
	if (appFileName.isEmpty()) {
		CFURLRef bundleURL = NULL;
		CFBundleRef bundle = NULL;
		CFStringRef bundlePath = NULL;

		bundle = CFBundleGetMainBundle();
		if (bundle) {
			bundleURL = CFBundleCopyBundleURL(bundle);
			bundlePath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);

			if (bundleURL) {
				CFRelease(bundleURL);
			}

			if (bundlePath) {
				appFileName = convert_CFString_to_QString(bundlePath);
				CFRelease(bundlePath);
			}
		}
	}
	return appFileName;
}

#endif
