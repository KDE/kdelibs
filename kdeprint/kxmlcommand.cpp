/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kxmlcommand.h"
#include "driver.h"
#include "kmfactory.h"
#include "kdeprintcheck.h"
#include "driverview.h"

#include <qfile.h>
#include <qregexp.h>
#include <qdir.h>
#include <qinputdialog.h>
#include <qmap.h>
#include <qvaluelist.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kdialogbase.h>
#include <kdebug.h>
#include <kprocess.h>

static void setOptionText(DrBase *opt, const QString& s)
{
	if (s.isEmpty())
		opt->set("text", opt->name());
	else
		opt->set("text", i18n(s.local8Bit()));
}

class KXmlCommand::KXmlCommandPrivate
{
public:
	QString	m_name;
	QString	m_command;
	DrMain	*m_driver;
	struct
	{
		QString	m_format[2];	// 0 -> file, 1 -> pipe
	}	m_io[2];				// 0 -> input, 1 -> output
	QString	m_description;
	QString	m_outputMime;
	QStringList	m_inputMime;
	QStringList	m_requirements;
	bool	m_loaded[2];	// 0 -> Desktop, 1 -> XML
};

KXmlCommand::KXmlCommand(const QString& xmlId)
: QObject(KXmlCommandManager::self(), "XmlCommand")
{
	init();
	d->m_name = xmlId;
}

KXmlCommand::~KXmlCommand()
{
	kdDebug() << "deleting driver" << endl;
	delete d->m_driver;
	kdDebug() << "deleting private data" << endl;
	delete d;
	kdDebug() << "finished" << endl;
}

void KXmlCommand::init()
{
	d = new KXmlCommandPrivate;
	d->m_driver = 0;
	d->m_loaded[0] = d->m_loaded[1] = false;
}

QString KXmlCommand::name() const
{ return d->m_name; }

void KXmlCommand::setName(const QString& s)
{ d->m_name = s; }

QString KXmlCommand::command()
{
	check(true);
	return d->m_command;
}

void KXmlCommand::setCommand(const QString& s)
{
	d->m_command = s;
}

DrMain* KXmlCommand::driver()
{
	check(true);
	return d->m_driver;
}

DrMain* KXmlCommand::takeDriver()
{
	check(true);
	DrMain	*dr = d->m_driver;
	d->m_driver = 0;
	d->m_loaded[1] = false;
	return dr;
}

void KXmlCommand::setDriver(DrMain *driver)
{
	delete d->m_driver;
	d->m_driver = driver;
}

QString KXmlCommand::io(bool io_input, bool io_pipe)
{
	check(true);
	return d->m_io[(io_input?0:1)].m_format[(io_pipe?1:0)];
}

void KXmlCommand::setIo(const QString& s, bool io_input, bool io_pipe)
{
	d->m_io[(io_input?0:1)].m_format[(io_pipe?1:0)] = s;
}

QString KXmlCommand::description()
{
	check();
	return d->m_description;
}

void KXmlCommand::setDescription(const QString& s)
{
	d->m_description = s;
}

QString KXmlCommand::mimeType()
{
	check();
	return d->m_outputMime;
}

void KXmlCommand::setMimeType(const QString& s)
{
	d->m_outputMime = s;
}

bool KXmlCommand::acceptMimeType(const QString& s)
{
	check();
	return (d->m_inputMime.find(s) != d->m_inputMime.end());
}

QStringList KXmlCommand::inputMimeTypes()
{
	check();
	return d->m_inputMime;
}

void KXmlCommand::setInputMimeTypes(const QStringList& l)
{
	d->m_inputMime = l;
}

QStringList KXmlCommand::requirements()
{
	check();
	return d->m_requirements;
}

void KXmlCommand::setRequirements(const QStringList& l)
{
	d->m_requirements = l;
}

bool KXmlCommand::isValid()
{
	return (!locate("data", "kdeprint/filters/"+name()+".desktop").isEmpty());
}

void KXmlCommand::check(bool use_xml)
{
	if (!d->m_loaded[0])
	{
		loadDesktop();
		d->m_loaded[0] = true;
	}
	if (use_xml && !d->m_loaded[1])
	{
		loadXml();
		d->m_loaded[1] = true;
	}
}

void KXmlCommand::loadDesktop()
{
	KSimpleConfig	conf(locate("data", "kdeprint/filters/"+name()+".desktop"));
	conf.setGroup("KDE Print Filter Entry");
	d->m_description = conf.readEntry("Comment");
	d->m_outputMime = conf.readEntry("MimeTypeOut");
	d->m_inputMime = conf.readListEntry("MimeTypeIn");
	d->m_requirements = conf.readListEntry("Require");
}

void KXmlCommand::saveDesktop()
{
	KSimpleConfig	conf(locateLocal("data", "kdeprint/filters/"+name()+".desktop"));
	conf.setGroup("KDE Print Filter Entry");
	conf.writeEntry("Comment", d->m_description);
	conf.writeEntry("MimeTypeIn", d->m_inputMime);
	conf.writeEntry("MimeTypeOut", d->m_outputMime);
	conf.writeEntry("Require", d->m_requirements);
}

void KXmlCommand::loadXml()
{
	QFile	f(locate("data", "kdeprint/filters/"+name()+".xml"));
	QDomDocument	doc;
	if (f.open(IO_ReadOnly) && doc.setContent(&f) && doc.documentElement().tagName() == "kprintfilter")
	{
		QDomElement	e, docElem = doc.documentElement();
		d->m_name = docElem.attribute("name");

		// command
		e = docElem.namedItem("filtercommand").toElement();
		if (!e.isNull())
			d->m_command = e.attribute("data");

		// arguments
		e = docElem.namedItem("filterargs").toElement();
		if (!e.isNull())
		{
			d->m_driver = new DrMain;
			d->m_driver->setName(d->m_name);
			parseGroup(e, d->m_driver);
			setOptionText(d->m_driver, d->m_name);
		}

		// input/output
		e = docElem.namedItem("filterinput").toElement();
		if (!e.isNull())
			parseIO(e, 0);
		e = docElem.namedItem("filteroutput").toElement();
		if (!e.isNull())
			parseIO(e, 1);
	}
}

void KXmlCommand::parseIO(const QDomElement& e, int n)
{
	QDomElement	elem = e.firstChild().toElement();
	while (!elem.isNull())
	{
		if (elem.tagName() == "filterarg")
		{
			int	format = (elem.attribute("name") == "file" ? 0 : 1);
			d->m_io[n].m_format[format] = elem.attribute("format");
		}
		elem = elem.nextSibling().toElement();
	}
}

DrGroup* KXmlCommand::parseGroup(const QDomElement& e, DrGroup *grp)
{
	if (!grp)
		grp = new DrGroup;
	grp->setName(e.attribute("name"));
	setOptionText(grp, e.attribute("description"));

	QDomElement	elem = e.firstChild().toElement();
	while (!elem.isNull())
	{
		if (elem.tagName() == "filterarg")
		{
			DrBase	*opt = parseArgument(elem);
			if (opt)
				grp->addOption(opt);
		}
		else if (elem.tagName() == "filtergroup")
		{
			DrGroup	*group = parseGroup(elem, 0);
			if (group)
				grp->addGroup(group);
		}
		elem = elem.nextSibling().toElement();
	}

	return grp;
}

DrBase* KXmlCommand::parseArgument(const QDomElement& e)
{
	DrBase	*opt(0);
	QString	type = e.attribute("type");

	if (type == "int" || type == "float")
	{
		if (type == "int")
			opt = new DrIntegerOption;
		else
			opt = new DrFloatOption;
		opt->set("minval", e.attribute("min"));
		opt->set("maxval", e.attribute("max"));
	}
	else if (type == "string")
		opt = new DrStringOption;
	else if (type == "list" || type == "bool")
	{
		if (type == "list")
			opt = new DrListOption;
		else
			opt = new DrBooleanOption;
		DrListOption	*lopt = static_cast<DrListOption*>(opt);
		QDomElement	elem = e.firstChild().toElement();
		while (!elem.isNull())
		{
			if (elem.tagName() == "value")
			{
				DrBase	*choice = new DrBase;
				choice->setName(elem.attribute("name"));
				setOptionText(choice, elem.attribute("description"));
				lopt->addChoice(choice);
			}
			elem = elem.nextSibling().toElement();
		}
	}
	else
		return 0;

	opt->setName("_kde-" + d->m_name + "-" + e.attribute("name"));
	setOptionText(opt, e.attribute("description"));
	opt->set("format", e.attribute("format"));
	opt->set("default", e.attribute("default"));
	opt->setValueText(opt->get("default"));

	return opt;
}

QString KXmlCommand::buildCommand(const QMap<QString,QString>& opts, bool pipein, bool pipeout)
{
	check(true);

	QString		str, cmd = d->m_command;
	QRegExp re( "%value" ), quotedRe( "'%value'" );

	if (d->m_driver)
	{
		QMap<QString,QString>	fopts;

		d->m_driver->setOptions(opts);
		d->m_driver->getOptions(fopts, false);
		for (QMap<QString,QString>::ConstIterator it=fopts.begin(); it!=fopts.end(); ++it)
		{
			DrBase	*dopt = d->m_driver->findOption(it.key());
			if (dopt)
			{
				QString	format = dopt->get("format");
				QString value = dopt->valueText();
				if ( format.find( "'%value'" ) != -1 )
				{
					if ( ( value.right( 1 ) == "'" && value.left( 1 ) == "'" )  ||
					     ( value.right( 1 ) == "\"" && value.left( 1 ) == "\"" ) )
						format.replace( quotedRe, value );
					else
						format.replace( re, value );
				}
				else
				{
					format.replace( re, KShellProcess::quote( dopt->valueText() ) );
				}
				str.append(format).append(" ");
			}
		}
		cmd.replace(QRegExp("%filterargs"), str);
	}

	cmd.replace(QRegExp("%filterinput"), d->m_io[0].m_format[(pipein?1:0)]);
	cmd.replace(QRegExp("%filteroutput"), d->m_io[1].m_format[(pipeout?1:0)]);

	return cmd;
}

void KXmlCommand::setOptions(const QMap<QString,QString>& opts)
{
	if (opts.count() == 0)
		return;
	// force loading the driver if needed
	if (driver())
		d->m_driver->setOptions(opts);
}

void KXmlCommand::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	// force loading the driver
	if (driver())
		d->m_driver->getOptions(opts, incldef);
}

void KXmlCommand::saveXml()
{
	QFile	f(locateLocal("data", "kdeprint/filters/"+name()+".xml"));
	if (!f.open(IO_WriteOnly))
		return;

	QDomDocument	doc("kprintfilter");
	QDomElement	root = doc.createElement("kprintfilter"), elem;

	root.setAttribute("name", d->m_name);
	doc.appendChild(root);

	// command
	elem = doc.createElement("filtercommand");
	elem.setAttribute("data", d->m_command);
	root.appendChild(elem);

	// options
	if (d->m_driver)
	{
		elem = createGroup(doc, d->m_driver);
		elem.setTagName("filterargs");
		root.appendChild(elem);
	}

	// IO
	if (!(elem=createIO(doc, 0, "filterinput")).isNull())
		root.appendChild(elem);
	if (!(elem=createIO(doc, 1, "filteroutput")).isNull())
		root.appendChild(elem);

	// save to file (and close it)
	QTextStream	t(&f);
	t << doc.toString();
	f.close();
}

QDomElement KXmlCommand::createIO(QDomDocument& doc, int n, const QString& tag)
{
	QDomElement	elem = doc.createElement(tag);
	if (d->m_command.find("%"+tag) != -1)
	{
		for (int i=0; i<2; i++)
		{
			QDomElement	io = doc.createElement("filterarg");
			io.setAttribute("name", (i ? "pipe" : "file"));
			io.setAttribute("format", d->m_io[n].m_format[i]);
			elem.appendChild(io);
		}
	}

	return elem;
}

QDomElement KXmlCommand::createGroup(QDomDocument& doc, DrGroup *group)
{
	QDomElement	elem = doc.createElement("filtergroup");
	elem.setAttribute("name", group->name());
	elem.setAttribute("description", group->get("text"));

	QPtrListIterator<DrGroup>	git(group->groups());
	for (; git.current(); ++git)
		elem.appendChild(createGroup(doc, git.current()));

	QPtrListIterator<DrBase>	oit(group->options());
	for (; oit.current(); ++oit)
		elem.appendChild(createElement(doc, oit.current()));

	return elem;
}

QDomElement KXmlCommand::createElement(QDomDocument& doc, DrBase *opt)
{
	QDomElement	elem = doc.createElement("filterarg");
	QString	elemName = opt->name();
	if (elemName.startsWith("_kde-"))
		elemName.replace(0, name().length()+6, "");
	elem.setAttribute("name", elemName);
	elem.setAttribute("format", opt->get("format"));
	elem.setAttribute("description", opt->get("text"));
	elem.setAttribute("default", opt->get("default"));

	switch (opt->type())
	{
		case DrBase::String:
			elem.setAttribute("type", "string");
			break;
		case DrBase::Integer:
		case DrBase::Float:
			elem.setAttribute("type", (opt->type() == DrBase::Integer ? "int" : "float"));
			elem.setAttribute("min", opt->get("minval"));
			elem.setAttribute("max", opt->get("maxval"));
			break;
		case DrBase::Boolean:
		case DrBase::List:
			elem.setAttribute("type", (opt->type() == DrBase::List ? "list" : "bool"));
			{
				QPtrListIterator<DrBase>	it(*(static_cast<DrListOption*>(opt)->choices()));
				for (; it.current(); ++it)
				{
					QDomElement	chElem = doc.createElement("value");
					chElem.setAttribute("name", it.current()->name());
					chElem.setAttribute("description", it.current()->get("text"));
					elem.appendChild(chElem);
				}
			}
			break;
		default:
			break;
	}

	return elem;
}

//---------------------------------------------------------------------------------------------------

class KXmlCommandManager::KXmlCommandManagerPrivate
{
public:
	QStringList	m_cmdlist;
	QMap<QString, QValueList<KXmlCommand*> >	m_mimemap;
	QMap<QString, KXmlCommand*>	m_cmdmap;
};

KXmlCommandManager* KXmlCommandManager::m_self = 0;

KXmlCommandManager* KXmlCommandManager::self()
{
	if (!m_self)
	{
		m_self = new KXmlCommandManager;
		Q_CHECK_PTR(m_self);
	}
	return m_self;
}

KXmlCommandManager::KXmlCommandManager()
: QObject(KMFactory::self(), "XmlCommandManager")
{
	d = new KXmlCommandManagerPrivate;
}

KXmlCommandManager::~KXmlCommandManager()
{
	cleanUp();
	delete d;
}

KXmlCommand* KXmlCommandManager::loadCommand(const QString& xmlId, bool check)
{
	if (check)
	{
		QString	desktopFile = locate("data", "kdeprint/filters/"+xmlId+".desktop");
		if (desktopFile.isEmpty())
			return 0;
	}
	return new KXmlCommand(xmlId);
}

void KXmlCommandManager::saveCommand(KXmlCommand *xmlCmd)
{
	xmlCmd->saveDesktop();
	xmlCmd->saveXml();

	cleanUp();
}

void KXmlCommandManager::cleanUp()
{
	for (QMap<QString, KXmlCommand*>::ConstIterator it=d->m_cmdmap.begin(); it!=d->m_cmdmap.end(); ++it)
		delete (*it);
	d->m_cmdmap.clear();
	d->m_mimemap.clear();
	d->m_cmdlist.clear();
}

void KXmlCommandManager::preload()
{
	if (d->m_cmdmap.count() == 0)
	{
		commandList();
		for (QStringList::Iterator it=d->m_cmdlist.begin(); it!=d->m_cmdlist.end(); ++it)
		{
			KXmlCommand	*xmlCmd = loadCommand(*it);
			QStringList	inputMime = xmlCmd->inputMimeTypes();
			for (QStringList::ConstIterator mime=inputMime.begin(); mime!=inputMime.end(); ++mime)
			{
				d->m_mimemap[*mime].append(xmlCmd);
				d->m_cmdmap[*it] = xmlCmd;
			}
		}
	}
}

QStringList KXmlCommandManager::commandList()
{
	if (d->m_cmdlist.isEmpty())
	{
		QStringList	dirs = KGlobal::dirs()->findDirs("data", "kdeprint/filters/");

		for (QStringList::ConstIterator it=dirs.begin(); it!=dirs.end(); ++it)
		{
			QStringList	list = QDir(*it).entryList("*.desktop", QDir::Files, QDir::Unsorted);
			for (QStringList::ConstIterator it2=list.begin(); it2!=list.end(); ++it2)
			{
				QString	cmdName = (*it2).left((*it2).length()-8);
				if (d->m_cmdlist.find(cmdName) == d->m_cmdlist.end())
					d->m_cmdlist.append(cmdName);
			}
		}

		d->m_cmdlist.sort();
	}

	return d->m_cmdlist;
}

QStringList KXmlCommandManager::commandListWithDescription()
{
	preload();
	QStringList	l;
	for (QMap<QString,KXmlCommand*>::ConstIterator it=d->m_cmdmap.begin(); it!=d->m_cmdmap.end(); ++it)
		l << (*it)->name() << (*it)->description();

	return l;
}

QString KXmlCommandManager::selectCommand(QWidget *parent)
{
	QStringList	cmdList = commandList();
	bool	ok(false);

	QString	cmd = QInputDialog::getItem(i18n("Select Command"), i18n("Select the command to use:"), cmdList, 0, false, &ok, parent);
	return (ok ? cmd : QString::null);
}

KXmlCommand* KXmlCommandManager::command(const QString& xmlId) const
{
	return (d->m_cmdmap.contains(xmlId) ? d->m_cmdmap[xmlId] : 0);
}

int KXmlCommandManager::insertCommand(QStringList& list, const QString& filtername, bool defaultToStart)
{
	preload();

	int	pos(0);
	KXmlCommand	*f1 = command(filtername), *f2(0);
	if (f1 && f1->inputMimeTypes().count() > 0)
	{
		QString	mimetype = f1->inputMimeTypes()[0];
		for (QStringList::Iterator it=list.begin(); it!=list.end(); ++it, pos++)
		{
			f2 = command(*it);
			if (f2->acceptMimeType(f1->mimeType()) && f1->acceptMimeType(mimetype))
			{
				list.insert(it, filtername);
				break;
			}
			else
			{
				mimetype = f2->mimeType();
				f2 = 0;
			}
		}
		if (pos == (int)(list.count()))
		{
			if (list.count() == 0 || f1->acceptMimeType(mimetype))
				list.append(filtername);
			else if (defaultToStart)
			{
				pos = 0;
				list.prepend(filtername);
			}
			else
				pos = -1;
		}
	}
	return pos;
}

QStringList KXmlCommandManager::autoConvert(const QString& mimesrc, const QString& mimedest)
{
	QStringList	chain;
	uint		score(0);

	preload();

	if (d->m_mimemap.contains(mimesrc))
	{
		const QValueList<KXmlCommand*>	l = d->m_mimemap[mimesrc];
		for (QValueList<KXmlCommand*>::ConstIterator it=l.begin(); it!=l.end(); ++it)
		{
			// direct filter: shortest path => return immediately
			if ((*it)->mimeType() == mimedest)
			{
				chain = QStringList((*it)->name());
				break;
			}
			// non direct filter: find the shortest way between
			// its output and mimedest (do not consider cyling filters)
			else if ((*it)->mimeType() != mimesrc)
			{
				QStringList	subchain = autoConvert((*it)->mimeType(), mimedest);
				// If chain length is 0, then there's no possible filter between those 2
				// mime types. Just discard it. If the subchain contains also the current
				// considered filter, then discard it: it denotes of a cycle in filter
				// chain.
				if (subchain.count() > 0 && subchain.findIndex((*it)->name()) == -1)
				{
					subchain.prepend((*it)->name());
					if (subchain.count() < score || score == 0)
					{
						chain = subchain;
						score = subchain.count();
					}
				}
			}
		}
	}
	// At this point, either we have the shortest path, or empty
	// list if nothing could be found
	return chain;
}

bool KXmlCommandManager::checkCommand(const QString& xmlId, int inputCheck, int outputCheck, QString *msg)
{
	KXmlCommand	*xmlCmd = command(xmlId);
	QString	errmsg;
	bool	needDestroy(false);
	kdDebug() << "checking command: " << xmlId << " (" << (xmlCmd != NULL) << ")" << endl;
	if (!xmlCmd)
	{
		xmlCmd = loadCommand(xmlId, true);
		needDestroy = (xmlCmd != 0);
	}

	bool	status(true);
	if (xmlCmd)
	{
		status = (xmlCmd->isValid() && KdeprintChecker::check(xmlCmd->requirements()));
		if (!status)
			errmsg = i18n("One of the command object's requirements is not met.");
	}
	QString	cmd = (xmlCmd ? xmlCmd->command() : xmlId);
	if (status && !cmd.isEmpty() && (inputCheck > None || outputCheck > None))
	{
		if (inputCheck > None && (cmd.find("%in") == -1 || inputCheck == Advanced) && cmd.find("%filterinput") == -1)
		{
			status = false;
			errmsg = i18n("The command does not contain the required tag %1.").arg(inputCheck == Advanced ? "%filterinput" : "{%in,%filterinput}");
		}
		if (status && outputCheck > None && (cmd.find("%out") == -1 || outputCheck == Advanced) && cmd.find("filteroutput") == -1)
		{
			status = false;
			errmsg = i18n("The command does not contain the required tag %1.").arg(outputCheck == Advanced ? "%filteroutput" : "{%out,%filteroutput}");
		}
	}

	if (needDestroy)
		delete xmlCmd;

	if (msg)
		*msg = errmsg;

	return status;
}

bool KXmlCommandManager::configure(KXmlCommand *xmlCmd, QWidget *parent)
{
	if (xmlCmd->driver())
	{
		KDialogBase	dlg(parent, 0, true, xmlCmd->description(), KDialogBase::Ok);
		DriverView	view(&dlg);

		dlg.setMainWidget(&view);
		view.setDriver(xmlCmd->driver());
		dlg.resize(350,400);
		dlg.exec();

		return true;
	}
	return false;
}
