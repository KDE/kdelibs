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

#include "susehelper.h"
#include "driver.h"

#include <qmap.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdict.h>
#include <qregexp.h>
#include <kdebug.h>
#include <kapplication.h>
#include <ctype.h>

// internal structures
struct SuSEValue
{
	QString id;
	QString text;
	QValueList< QPair<QString,QString> >    options;
	bool    pattern;
};
struct SuSEOption
{
	QString                 id;
	QValueList<SuSEValue>       values;
	QString                 valuespec;
	QString                 pattern;
};
struct SuSEConfig
{
	QString                                 id;
	QValueList< QPair<QString,QString> >    options;
	QValueList< QPair<QString,QString> >    subconfigs;
};
struct SuSEConfigInfo
{
	QString id;
	QString text;
	QString queue;
};
struct SuSEPrinter
{
	QString                 id;
	QValueList<SuSEConfigInfo>  configs;
};
struct SuSEPrinterInfo
{
	QString id;
	QString IEEE;
	QString text;
};
struct SuSEManufacturer
{
	QString                 id;
	QValueList<SuSEPrinterInfo> printers;
};

// parsing code
QString SuSEHelper::nextValue(QFile& f)
{
	int	c(-1);
	QByteArray	arr(1);
	int	len(1), currlen(0), nest(0);
	while (!f.atEnd())
	{
		c = f.getch();
		if (c == '{')
			break;
		else if (c == '#')
			while (!f.atEnd() && f.getch() != '\n') ;
	}
	while (!f.atEnd())
	{
		c = f.getch();
		if (c == '{')
			nest++;
		else if (c == '}')
		{
			if (nest > 0)
				nest--;
			else
				break;
		}
		if (currlen == len-1)
		{
			len += 10;
			arr.resize(len);
			arr[len-1] = (char)0;
		}
		arr[currlen++] = (char)c;
	}
	arr[currlen] = (char)0;
	QString	s(arr);
	s = s.stripWhiteSpace();
	return s;
}

QString SuSEHelper::nextToken(QFile& f)
{
	int	c(-1);
	QByteArray	arr(1);
	uint	len(1), currlen(0);
	while (!f.atEnd())
	{
		c = f.getch();
		if (c == '#')
			while (!f.atEnd() && f.getch() != '\n') ;
		else if (!isspace((char)c))
		{
			f.ungetch(c);
			break;
		}
	}
	while (!f.atEnd())
	{
		c = f.getch();
		if (isspace((char)c))
			break;
		if (currlen == len-1)
		{
			len += 10;
			arr.resize(len);
			arr[len-1] = (char)0;
		}
		arr[currlen++] = (char)c;
	}
	arr[currlen] = (char)0;
	return QString(arr);
}

SuSEValue SuSEHelper::parseValue(QFile& f)
{
	SuSEValue	val;
	val.id = nextValue(f);
	if (nextToken(f) == "Text")
		val.text = nextValue(f);
	if (val.id.isEmpty())
		val.id = "__kdeprint_"+kapp->randomString(4);
	val.pattern = false;
	return val;
}

SuSEOption* SuSEHelper::parseOption(QFile& f)
{
	SuSEOption	*opt = new SuSEOption;
	QString	token;
	opt->id = nextValue(f);
	while (!f.atEnd())
	{
		token = nextToken(f);
		if (token == "Value")
		{
			opt->values.append(parseValue(f));
		}
		else if (token == "UseOption")
		{
			QPair<QString,QString>	p;
			p.first = nextValue(f);
			if (nextToken(f) == "As")
			{
				SuSEOption	*otherOpt = options.find(p.first);
				p.first = nextValue(f);
				if (otherOpt && !options.find(p.first))
				{
					SuSEOption	*cloneOpt = new SuSEOption;
					cloneOpt->id = p.first;
					cloneOpt->values = otherOpt->values;
					cloneOpt->valuespec = otherOpt->valuespec;
					cloneOpt->pattern = otherOpt->pattern;
					options.insert(cloneOpt->id, cloneOpt);
				}
			}
			p.second = nextValue(f);
			if (opt->values.count() == 0)
				kdWarning() << "Sub-option '" << p.first << "' not attached to a value: " << opt->id << endl;
			else
				opt->values.last().options.append(p);
		}
		else if (token == "Pattern")
		{
			opt->values.append(parseValue(f));
			opt->values.last().pattern = true;
		}
		else if (token == "ValueText" || token == "ValueInt" || token == "ValueFloat")
		{
			opt->valuespec = nextValue(f);
			opt->pattern = nextValue(f);
		}
		else if (token == "PasteOption")
		{
			QString	id = nextValue(f);
			SuSEOption	*thisOpt = options.find(id);
			if (thisOpt)
			{
				for (QValueList<SuSEValue>::ConstIterator it=thisOpt->values.begin(); it!=thisOpt->values.end(); ++it)
					opt->values.append(*it);
			}
			else
				kdWarning() << "Could not find option for pasting: " << id << endl;
		}
		else if (token == "EndOption")
		{
			QString	v = nextValue(f);
			if (v != opt->id)
				kdWarning() << "Wrong option format detected: " << opt->id << endl;
			break;
		}
		else if (token == "Comment")
			nextValue(f);
		else
		{
			kdWarning() << "Unexpected token '" << token << "' while parsing option " << opt->id << endl;
			nextValue(f);
		}
	}
	return opt;
}

SuSEConfig* SuSEHelper::parseConfig(QFile& f)
{
	SuSEConfig	*conf = new SuSEConfig;
	QString	token;
	conf->id = nextValue(f);
	while (!f.atEnd())
	{
		token = nextToken(f);
		if (token == "EndConfig")
		{
			QString	v = nextValue(f);
			if (v != conf->id)
				kdWarning() << "Wrong config format detected: " << conf->id << endl;
			break;
		}
		else if (token == "UseOption")
		{
			QPair<QString,QString>	p;
			p.first = nextValue(f);
			p.second = nextValue(f);
			conf->options.append(p);
		}
		else if (token == "UseConfig")
		{
			QPair<QString,QString>	p;
			p.first = nextValue(f);
			p.second = nextValue(f);
			conf->subconfigs.append(p);
		}
		else if (token == "Comment")
			nextValue(f);
		else
		{
			kdWarning() << "Unexpected token '" << token << "' while parsing config " << conf->id << endl;
			nextValue(f);
		}
	}
	return conf;
}

SuSEPrinter* SuSEHelper::parsePrinter(QFile& f)
{
	SuSEPrinter	*printer = new SuSEPrinter;
	QString	token;
	printer->id = nextValue(f);
	bool	readToken(true);
	while (!f.atEnd())
	{
		if (readToken)
			token = nextToken(f);
		else
			readToken = true;
		if (token == "EndPrinter")
		{
			QString	v = nextValue(f);
			if (v != printer->id)
				kdWarning() << "Wrong printer format detected: " << printer->id << endl;
			break;
		}
		else if (token == "UseConfig")
		{
			SuSEConfigInfo	info;
			info.id = nextValue(f);
			info.text = nextValue(f);
			QString	ntoken = nextToken(f);
			if (ntoken == "Queue")
				info.queue = nextValue(f);
			else
			{
				readToken = false;
				token = ntoken;
			}
			printer->configs.append(info);
		}
		else if (token == "Comment")
			nextValue(f);
		else
		{
			kdWarning() << "Unexpected token '" << token << "' while parsing printer " << printer->id << endl;
			nextValue(f);
		}
	}
	return printer;
}

SuSEManufacturer* SuSEHelper::parseManufacturer(QFile& f)
{
	SuSEManufacturer	*manu = new SuSEManufacturer;
	QString	token;
	manu->id = nextValue(f);
	while (!f.atEnd())
	{
		token = nextToken(f);
		if (token == "EndManufacturer")
		{
			QString	v = nextValue(f);
			if (v != manu->id)
				kdWarning() << "Wrong manufacturer format detected: " << manu->id << endl;
			break;
		}
		else if (token == "UsePrinter")
		{
			SuSEPrinterInfo	info;
			info.id = nextValue(f);
			if (nextToken(f) == "IEEE")
				info.IEEE = nextValue(f);
			info.text = nextValue(f);
			manu->printers.append(info);
		}
		else if (token == "Comment")
			nextValue(f);
		else
		{
			kdWarning() << "Unexpected token '" << token << "' while parsing manufacturer " << manu->id << endl;
			nextValue(f);
		}
	}
	return manu;
}

void SuSEHelper::load()
{
	if (loaded)
		return;

	QFile	f(yastdir + "/lib/printerdb/suse.prdb");
	if (f.open(IO_ReadOnly))
	{
		QString	token;
		while (!f.atEnd())
		{
			token = nextToken(f);
			if (token == "Manufacturer")
			{
				SuSEManufacturer	*manu = parseManufacturer(f);
				manufacturers.insert(manu->id, manu);
			}
			else if (token == "Printer")
			{
				SuSEPrinter	*printer = parsePrinter(f);
				printers.insert(printer->id, printer);
			}
			else if (token == "Config")
			{
				SuSEConfig	*conf = parseConfig(f);
				configs.insert(conf->id, conf);
			}
			else if (token == "Option")
			{
				SuSEOption	*opt = parseOption(f);
				options.insert(opt->id, opt);
			}
			else if (token == "UseManufacturer")
			{
				// just skip this
				nextValue(f);
				if (nextToken(f) == "IEEE")
					nextValue(f);
				nextValue(f);
			}
			else
			{
				if (!token.isEmpty())
				{
					kdWarning() << "Unexpected token '"<< token << "' at top level" << endl;
					nextValue(f);
				}
				break;
			}
		}
		f.close();
		loaded = true;
	}
	else
		kdWarning() << "Unable to open printer DB file: " << f.name() << endl;
}

// class code
SuSEHelper::SuSEHelper(const QString& d)
{
	yastdir = d;
	manufacturers.setAutoDelete(true);
	printers.setAutoDelete(true);
	options.setAutoDelete(true);
	configs.setAutoDelete(true);
	loaded = false;
}

SuSEHelper::~SuSEHelper()
{
	manufacturers.clear();
	printers.clear();
	configs.clear();
	options.clear();
}

DrBase* SuSEHelper::createItem(SuSEOption *opt, const QString& itemName)
{
	//if (opt->values.count() > 1 || (opt->values.count() == 1 && opt->values[0].options.count() == 0))
	if (opt->values.count() > 0)
	{
		DrListOption	*o = new DrListOption;
		o->setName(opt->id);
		o->set("text", itemName);
		// seems to a be a normal list option, parse all values,
		// creating ChoiceGroup is needed
		QValueList<SuSEValue>::ConstIterator	it;
		int	valindex(0);
		for (it=opt->values.begin(); it!=opt->values.end(); ++it, valindex++)
		{
			DrBase	*ch(0);
			if ((*it).options.count() == 0 && !((*it).pattern))
			{
				// normal value
				ch = new DrBase;
			}
			else
			{
				ch = new DrChoiceGroup;
				QValueList< QPair<QString,QString> >::ConstIterator	it2;
				for (it2=(*it).options.begin(); it2!=(*it).options.end(); ++it2)
				{
					SuSEOption	*subopt = options.find((*it2).first);
					if (subopt)
					{
						DrBase	*subitem = createItem(subopt, (*it2).second);
						if (subitem)
						{
							subitem->setName((*it2).first);
							static_cast<DrChoiceGroup*>(ch)->addObject(subitem);
						}
					}
				}
			}
			if (ch)
			{
				//ch->setName((*it).id);
				ch->setName(QString::number(valindex));
				ch->set("text", (*it).text);
				o->addChoice(ch);
			}
		}
		o->setValueText(o->choices()->first()->name());
		o->set("default", o->valueText());
		return o;
	}
	/*
	else if (opt->values.count() == 1 && opt->values[0].options.count() > 0)
	{
		DrGroup	*grp = new DrGroup;
		grp->setName(opt->id);
		grp->set("text", itemName);
		QValueList< QPair<QString,QString> >::ConstIterator	it;
		for (it=opt->values[0].options.begin(); it!=opt->values[0].options.end(); ++it)
		{
			SuSEOption	*subopt = options.find((*it).first);
			if (subopt)
			{
				DrBase	*item = createItem(subopt, (*it).second);
				if (item)
					grp->addObject(item);
			}
		}
		return grp;
	}
	*/
	else if (!opt->valuespec.isEmpty())
	{
		DrStringOption	*o = new DrStringOption;
		o->setName(opt->id);
		o->set("text", itemName);
		return o;
	}
	return NULL;
}

DrMain* SuSEHelper::generateDriver(const QString& configname)
{
	// check loaded state
	load();

	SuSEConfig	*conf = configs.find(configname);
	if (!conf)
		return NULL;
	DrMain	*driver = new DrMain;
	driver->set("text", QString::fromLatin1("SuSE Driver (%1)").arg(configname));
	QValueList< QPair<QString,QString> >::ConstIterator	it;
	for (it=conf->options.begin(); it!=conf->options.end(); ++it)
	{
		SuSEOption	*opt = options.find((*it).first);
		if (opt)
		{
			DrBase	*item = createItem(opt, (*it).second);
			if (item)
				driver->addObject(item);
		}
	}
	return driver;
}

QString SuSEHelper::generateGsOption(const QString& key, const QString& val, const QMap<QString,QString>& opts)
{
	load();

	SuSEOption	*opt = options.find(key);
	if (!opt)
		return QString::null;
	int	index = val.toInt();
	QString	s = opt->values[index].id;
	if (s.startsWith("__kdeprint_"))
		s = QString::null;
	if (opt->values[index].pattern)
	{
		SuSEOption	*subopt = options.find(opt->values[index].options[0].first);
		if (subopt)
		{
			QString	subpatt = subopt->pattern;
			QString	subval = opts[subopt->id];
			subpatt.replace(QRegExp("%"), subval);
			s.replace(QRegExp("%"), subpatt);
		}
	}
	return s;
}

QString SuSEHelper::generateYaST2Option(const QMap<QString,QString>& opts)
{
	load();

	QString	s;
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
	{
		if (it.data() == "0")
			continue;
		s += ("\"" + it.key() + "\":");
		SuSEOption	*o = options.find(it.key());
		if (o && !o->valuespec.isEmpty())
			s += ("\"" + it.data() + "\"");
		else
			s += it.data();
		s += ", ";
	}
	if (!s.isEmpty())
	{
		s.truncate(s.length()-2);
		s.prepend("$[").append("]");
	}
	return s;
}

bool SuSEHelper::writeUppFile(const QString& filename, const QMap<QString,QString>& opts)
{
	QFile	f(filename);
	if (f.open(IO_WriteOnly))
	{
		QTextStream	t(&f);
		for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
		{
			QString	line = generateGsOption(it.key(), it.data(), opts);
			if (!line.isEmpty())
				t << line << endl;
		}
		f.close();
		return true;
	}
	return false;
}

bool SuSEHelper::writeYaST2File(const QString& filename, const QMap<QString,QString>& opts)
{
	QFile	f(filename);
	if (f.open(IO_WriteOnly))
	{
		QTextStream	t(&f);
		for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
		{
			t << '"' << it.key() << "\":";
			if (it.data()[0] == '$' || it.data() == "false" || it.data() == "true" || it.data() == "nil")
				t << it.data() << ',' << endl;
			else
				t << '"' << it.data() << "\"," << endl;
		}
		f.close();
		return true;
	}
	else
		return false;
}
