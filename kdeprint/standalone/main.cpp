/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : jeu jun 15 09:52:49 CEST 2000
    copyright            : (C) 2000 by Michael Goffioul
    email                : gofioul@emic.ucl.ac.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <unistd.h>
#include <signal.h>

#include <qstring.h>
#include <qstringlist.h>
#include <stdlib.h>
#include <qmessagebox.h>
#include <qfile.h>

#include <kapp.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kio/netaccess.h>
#include <kurl.h>

#include <kprinter.h>
#include <optionset.h>

void signal_handler(int);
char tempFile[1024];
bool fromStdin = false;
char job_output = 0;	// 0: dialog, 1: console, 2: none

void showmsgdialog(const QString &msg, int type = 0)
{
	switch (type)
	{
	   case 0: QMessageBox::information(NULL,i18n("Print information"),msg,QMessageBox::Ok|QMessageBox::Default,0); break;
	   case 1: QMessageBox::warning(NULL,i18n("Print warning"),msg,QMessageBox::Ok|QMessageBox::Default,0); break;
	   case 2: QMessageBox::critical(NULL,i18n("Print error"),msg,QMessageBox::Ok|QMessageBox::Default,0); break;
	}
}

void showmsgconsole(const QString &msg, int type = 0)
{
	qDebug(type == 0 ? "Print info: %s" : (type == 1 ? "Print warning: %s" : "Print error: %s"),msg.local8Bit().data());
}

void showmsg(const QString &msg, int type = 0)
{
	switch (job_output) {
	   case 0: showmsgdialog(msg,type); break;
	   case 1: showmsgconsole(msg,type); break;
	   default: break;
	}
}

void errormsg(const QString &msg)
{
	showmsg(msg,2);
	exit(1);
}

static KCmdLineOptions options[] =
{
	{ "d <printer>",      I18N_NOOP("Printer/destination to print on"),      0},
	{ "t <title>",        I18N_NOOP("Title for the print job"),              0},
	{ "o <option=value>", I18N_NOOP("Printer option"),                       0},
	{ "j <mode>",         I18N_NOOP("Job output mode (gui, console, none)"), "gui"},
	{ "system <printsys>",I18N_NOOP("Print system to use (LPD, CUPS, PDQ)"), 0},
	{ "nostdin",          I18N_NOOP("Forbid impression from STDIN"),         0},
	{ "+file(s)",	      I18N_NOOP("Files to load"),                        0},
	{ 0,                  0,                                      0}
};

int main(int argc, char *argv[])
{
	KCmdLineArgs::init(argc,argv,"kprinter",I18N_NOOP("A printer tool for KDE"),"0.0.1");
	KCmdLineArgs::addCmdLineOptions(options);
	KApplication	app;
	KCmdLineArgs	*args = KCmdLineArgs::parsedArgs();

#if defined(HAVE_SIGACTION) && !defined(HAVE_SIGSET)
	struct sigaction action;
#endif /* HAVE_SIGACTION && !HAVE_SIGSET*/

	// read variables from command line
	QString	printer = args->getOption("d");
	QString	title = args->getOption("t");
	QString	job_mode = args->getOption("j");
	QString	system = args->getOption("system");
	QCStringList	optlist = args->getOptionList("o");
	OptionSet	opts;
	KURL::List	files;
	QStringList	filestoprint;
	bool	nostdin = !(args->isSet("stdin"));

	// parse options
	for (QCStringList::ConstIterator it=optlist.begin(); it!=optlist.end(); ++it)
	{
		QStringList	l = QStringList::split('=',QString(*it),false);
		if (l.count() >= 1) opts[l[0]] = (l.count() == 2 ? l[1] : QString::fromLatin1(""));
	}
qDebug("parse options:");
opts.dump();

	// read file list
	for (int i=0; i<args->count(); i++)
		files.append(args->url(i));

	// some clean-up
	args->clear();

	// set default values if necessary
	if (title.isEmpty()) title = "KPrinter";
	if (job_mode == "console") job_output = 1;
	else if (job_mode == "none") job_output = 2;
	else job_output = 0;

	// check file existence / download file, and show messages if necessary
	for (KURL::List::ConstIterator it=files.begin(); it!=files.end(); ++it)
	{
		QString	target;
		if (!KIO::NetAccess::download(*it,target))
		{
       			QString	msg = i18n("\"%1\": file not found").arg((*it).prettyURL());
       			showmsg(msg,2);
		}
		else
			filestoprint.append(target);
	}

	// do nothing if we have only inexistent files
	if (files.count() > 0 && filestoprint.count() == 0)
		exit(1);

	if (filestoprint.count() == 0 && nostdin)
		errormsg(i18n("Can't print from STDIN (do not use '--nostdin' option)"));

	if (system == "LPD") KPrinterFactory::self()->setPrintSystem(KPrinterFactory::LPD);
	else if (system == "CUPS") KPrinterFactory::self()->setPrintSystem(KPrinterFactory::CUPS);
	else if (system == "PDQ") KPrinterFactory::self()->setPrintSystem(KPrinterFactory::PDQ);

	KPrinter	*kprinter = KPrinterFactory::self()->printer();
	if (!printer.isEmpty()) kprinter->setSearchName(printer);
	kprinter->setApplicationType(KPrinter::StandAlone);
	kprinter->setDocName(title);
	// override default settings
	kprinter->removeDialogFlags(KPrinter::Preview|KPrinter::OutputToFile);
	kprinter->removeStandardPage(KPrinter::SizeColorPage);
	kprinter->initOptions(opts);

	if (kprinter->setup(NULL))
	{
		if (filestoprint.count() == 0)
		{
			// print from stdin

#  if defined(HAVE_SIGSET)
			sigset(SIGHUP, signal_handler);
			sigset(SIGINT, signal_handler);
			sigset(SIGTERM, signal_handler);
#  elif defined(HAVE_SIGACTION)
			memset(&action, 0, sizeof(action));
			action.sa_handler = signal_handler;

			sigaction(SIGHUP, &action, NULL);
			sigaction(SIGINT, &action, NULL);
			sigaction(SIGTERM, &action, NULL);
#  else
			signal(SIGHUP, signal_handler);
			signal(SIGINT, signal_handler);
			signal(SIGTERM, signal_handler);
#  endif

			filestoprint.append(locateLocal("tmp",QString::fromLatin1("kprinter_%1").arg(getpid())));
			fromStdin = true;
			FILE	*fout = fopen(QFile::encodeName(filestoprint[0]),"w");
			if (!fout) errormsg(i18n("Unable to open temporary file"));
			char	buffer[8192];
			int	s;

			// read stdin and write to temporary file
			while ((s=fread(buffer,1,sizeof(buffer),stdin)) > 0)
				fwrite(buffer,1,s,fout);

			s = ftell(fout);
			fclose(fout);
			if (s <= 0) errormsg(i18n("Stdin is empty, no job sent"));
		}

		// print all files
		bool ok = kprinter->printFiles(filestoprint);

		if (!ok) errormsg(i18n("Error while printing files"));
		else
		{
			QString	msg = i18n("File(s) sent to printer %1").arg(kprinter->printerName());
			showmsg(msg,0);
		}

		// if printing from stdin, remove temporary file
		if (fromStdin) ::unlink(QFile::encodeName(filestoprint[0]));
	}

	return (0);
}

void signal_handler(int s)
{
	unlink(tempFile);
	exit(s);
}
