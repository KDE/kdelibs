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

#include "kprintfilter.h"
#include "driver.h"
#include "driverview.h"

#include <qxml.h>
#include <qstack.h>
#include <kstddirs.h>
#include <kdialogbase.h>

struct KPrintFilter::KPrintFilterIO
{
	QString	m_file;
	QString	m_pipe;
};

//*****************************************************************************************************

class FilterHandler : public QXmlDefaultHandler
{
public:
	FilterHandler(KPrintFilter *filter);
	virtual bool startElement(const QString&, const QString& lname, const QString&, const QXmlAttributes& attrs);
	virtual bool endElement(const QString&, const QString& lname, const QString&);
private:
	enum State { None, Filter, Args, Input, Output };
private:
	QStack<DrGroup>	m_stack;
	DrBase		*m_opt;
	int		m_state;
	KPrintFilter		*m_filter;
};

FilterHandler::FilterHandler(KPrintFilter *filter)
{
	m_stack.setAutoDelete(false);
	m_state = None;
	m_filter = filter;
	m_opt = 0;
}

bool FilterHandler::startElement(const QString&, const QString& lname, const QString&, const QXmlAttributes& attrs)
{
	if (lname == "kprintfilter" && m_state == None)
	{
		m_filter->m_idname = attrs.value("name");
		m_filter->m_description = attrs.value("description");
		m_state = Filter;
	}
	else if (lname == "filtercommand" && m_state == Filter)
	{
		m_filter->m_command = attrs.value("data");
	}
	else if (lname == "filterargs" && m_state == Filter)
	{
		m_filter->m_driver = new DrMain();
		m_filter->m_driver->set("text",m_filter->m_description);
		m_filter->m_driver->setName(m_filter->m_idname);
		m_stack.push(m_filter->m_driver);
		m_state = Args;
	}
	else if (lname == "filterarg" && m_state == Args)
	{
		QString	type = attrs.value("type");
		m_opt = 0;
		if (type == "int" || type == "float")
		{
			if (type == "int") m_opt = new DrIntegerOption();
			else m_opt = new DrFloatOption();
			m_opt->set("minval",attrs.value("min"));
			m_opt->set("maxval",attrs.value("max"));
		}
		else if (type == "string")
		{
			m_opt = new DrStringOption();
		}
		else if (type == "bool")
		{
			m_opt = new DrBooleanOption();
		}
		else if (type == "list")
		{
			m_opt = new DrListOption();
		}
		if (m_opt && m_stack.top() && !attrs.value("name").isEmpty())
		{
			// prepend "kde-<filtername>-" to all options
			m_opt->setName(QString::fromLatin1("_kde-%1-").arg(m_filter->m_idname)+attrs.value("name"));
			m_opt->set("text",attrs.value("description"));
			m_opt->set("format",attrs.value("format"));
			m_opt->set("default",attrs.value("default"));
			m_stack.top()->addOption(m_opt);
		}
		else
		{
			delete m_opt;
			m_opt = 0;
			return false;
		}
	}
	else if (lname == "filterarg" && (m_state == Input || m_state == Output))
	{
		QString	type = attrs.value("name");
		KPrintFilter::KPrintFilterIO	*io = (m_state == Input ? m_filter->m_input : m_filter->m_output);
		if (type == "file") io->m_file = attrs.value("format");
		else if (type == "pipe") io->m_pipe = attrs.value("format");
		else return false;
	}
	else if (lname == "filtergroup" && m_state == Args && m_stack.top())
	{
		DrGroup	*grp = new DrGroup();
		grp->setName(attrs.value("name"));
		grp->set("text",attrs.value("description"));
		m_stack.top()->addGroup(grp);
		m_stack.push(grp);
	}
	else if (lname == "value" && m_state == Args && m_opt)
	{
		DrBase	*ch = new DrBase();
		ch->setName(attrs.value("name"));
		ch->set("text",attrs.value("description"));
		((DrListOption*)m_opt)->addChoice(ch);
	}
	else if (lname == "filterinput" && m_state == Filter)
	{
		m_filter->m_input = new KPrintFilter::KPrintFilterIO();
		m_state = Input;
	}
	else if (lname == "filteroutput" && m_state == Filter)
	{
		m_filter->m_output = new KPrintFilter::KPrintFilterIO();
		m_state = Output;
	}
	else return false;
	return true;
}

bool FilterHandler::endElement(const QString&, const QString& lname, const QString&)
{
	if (lname == "filterarg")
	{
		if (m_opt)
			m_opt->setValueText(m_opt->get("default"));
		m_opt = 0;
	}
	else if (lname == "filtergroup")
	{
		m_stack.pop();
	}
	else if (lname == "filterargs")
	{
		m_stack.pop();
		if (m_stack.count() > 0)
			return false;
		m_state = Filter;
	}
	else if (lname == "filterinput" || lname == "filteroutput")
	{
		m_state = Filter;
	}
	else if (lname == "kprintfilter")
	{
		m_state = None;
	}
	return true;
}

//*****************************************************************************************************

KPrintFilter::KPrintFilter(const QString& idname, QObject *parent, const char *name)
: QObject(parent,name)
{
	m_driver = 0;
	m_input = m_output = 0;
	m_read = false;
	m_idname = idname;
}

KPrintFilter::~KPrintFilter()
{
	clean();
}

void KPrintFilter::clean()
{
	delete m_driver;
	delete m_input;
	delete m_output;
	m_input = m_output = 0;
	m_driver = 0;
}

bool KPrintFilter::readXmlTemplate(const QString& filename)
{
	if (m_read && filename == m_idname)
		return true;
	QString	cpath = locate("data",QString::fromLatin1("kdeprint/filters/%1.xml").arg(filename));
	QFile	f(cpath);
	if (!f.exists())
		return false;
	QXmlSimpleReader	reader;
	QXmlInputSource		source(f);
	FilterHandler		handler(this);
	reader.setContentHandler(&handler);
	clean();
	return (m_read=(reader.parse(source) && m_driver && m_input && m_output));
}

QString KPrintFilter::buildCommand(const QMap<QString,QString>& options, bool pipein, bool pipeout)
{
	QString	cmd;
	if (readXmlTemplate(m_idname))
	{
		QString		str;
		QMap<QString,QString>	fopts;
		cmd = m_command;
		// command arguments
		m_driver->setOptions(options);
		m_driver->getOptions(fopts,false);
		for (QMap<QString,QString>::ConstIterator it=fopts.begin(); it!=fopts.end(); ++it)
		{
			DrBase	*dopt = m_driver->findOption(it.key());
			if (dopt)
			{
				QString	format = dopt->get("format");
				format.replace(QRegExp("%value"),dopt->valueText());
				str.append(format).append(" ");
			}
		}
		cmd.replace(QRegExp("%filterargs"),str);
		// command input
		str = (pipein ? m_input->m_pipe : m_input->m_file);
		cmd.replace(QRegExp("%filterinput"),str);
		// command output
		str = (pipeout ? m_output->m_pipe : m_output->m_file);
		cmd.replace(QRegExp("%filteroutput"),str);
	}
	return cmd;
}

void KPrintFilter::setOptions(const QMap<QString,QString>& opts)
{
	if (readXmlTemplate(m_idname))
		m_driver->setOptions(opts);
}

void KPrintFilter::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	if (readXmlTemplate(m_idname))
		m_driver->getOptions(opts, incldef);
}

bool KPrintFilter::configure(QWidget *parent)
{
	if (readXmlTemplate(m_idname))
	{
		KDialogBase	dlg(parent,0,true,m_description,KDialogBase::Ok);
		DriverView	view(&dlg);
		dlg.setMainWidget(&view);
		view.setDriver(m_driver);
		dlg.resize(350,400);
		dlg.exec();
		return true;
	}
	return false;
}
