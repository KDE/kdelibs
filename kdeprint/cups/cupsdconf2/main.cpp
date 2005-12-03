/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "cupsddialog.h"

#include <qfile.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kurl.h>

static KCmdLineOptions options[] =
{
	{ "+[file]", I18N_NOOP("Configuration file to load"), 0},
	KCmdLineLastOption
};

extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
	KCmdLineArgs::init(argc,argv,"cupsdconf",
			   I18N_NOOP("A CUPS configuration tool"),
			   I18N_NOOP("A CUPS configuration tool"),"0.0.1");
	KCmdLineArgs::addCmdLineOptions(options);
	KApplication	app;
	KCmdLineArgs	*args = KCmdLineArgs::parsedArgs();

	KURL	configfile;
	if (args->count() > 0)
		CupsdDialog::configure(args->url(0).path());
	else
		CupsdDialog::configure();
	return (0);
}
