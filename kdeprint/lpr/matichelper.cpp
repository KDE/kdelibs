/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2002 Michael Goffioul <goffioul@imec.be>
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

#include "matichelper.h"
#include "printcapentry.h"
#include "matic.h"
#include "driver.h"

#include <klocale.h>
#include <kdebug.h>
#include <qstringlist.h>

void loadPageSizes(DrMain*, MaticBlock*);

DrMain* maticToDriver(MaticBlock *blk)
{
	MaticBlock	*varblk = blk->block("$VAR1"), *argblk(0);
	if (varblk)
		argblk = varblk->block("args");
	else
		return NULL;

	DrMain	*driver = new DrMain;
	driver->set("manufacturer", varblk->arg("make"));
	driver->set("model", varblk->arg("model"));
	driver->set("matic_driver", varblk->arg("driver"));
	driver->set("matic_printer", varblk->arg("id"));
	QString	desc = QString::fromLatin1("%1 %2 (%3)").arg(driver->get("manufacturer")).arg(driver->get("model")).arg(varblk->arg("driver"));
	driver->set("text", desc);
	if (!(desc = blk->arg("$postpipe")).isEmpty())
		driver->set("postpipe", desc);
	if (argblk)
	{
		DrGroup	*adjgrp(0), *gengrp(0), *othergrp(0);
		QStringList	genname;
		genname << "PageSize" << "InputSlot" << "Manualfeed" << "Duplex";
		QDictIterator<MaticBlock>	it(argblk->m_blocks);
		for (; it.current(); ++it)
		{
			QString	type = it.current()->arg("type");
			DrBase	*opt(0);
			DrGroup	*grp(0);
			if (type == "float" || type == "int")
			{
				if (!adjgrp)
				{
					adjgrp = new DrGroup;
					adjgrp->set("text", i18n("Adjustments"));
					driver->addGroup(adjgrp);
				}
				grp = adjgrp;
				if (type == "float")
					opt = new DrFloatOption;
				else
					opt = new DrIntegerOption;
				opt->set("minval", it.current()->arg("min"));
				opt->set("maxval", it.current()->arg("max"));
			}
			else if (type == "enum" || type == "bool")
			{
				if (type == "enum")
				{
					MaticBlock	*valblk = it.current()->block("vals_byname");
					if (!valblk)
						continue;
					DrListOption	*lopt = new DrListOption;
					QDictIterator<MaticBlock>	it2(valblk->m_blocks);
					for (; it2.current(); ++it2)
					{
						DrBase	*ch = new DrBase;
						ch->setName(it2.currentKey());
						ch->set("text", it2.current()->arg("comment"));
						lopt->addChoice(ch);
					}
					opt = lopt;
					if (it.currentKey() == "PageSize")
						loadPageSizes(driver, valblk);
				}
				else
				{
					DrBooleanOption	*bopt = new DrBooleanOption;
					DrBase	*ch(0);
					ch = new DrBase;
					ch->setName("0");
					ch->set("text", it.current()->arg("name_false"));
					bopt->addChoice(ch);
					ch = new DrBase;
					ch->setName("1");
					ch->set("text", it.current()->arg("name_true"));
					bopt->addChoice(ch);
					opt = bopt;
				}
				if (genname.find(it.currentKey()) != genname.end())
				{
					if (!gengrp)
					{
						gengrp = new DrGroup;
						gengrp->set("text", i18n("General"));
						driver->addGroup(gengrp);
					}
					grp = gengrp;
				}
				else
				{
					if (!othergrp)
					{
						othergrp = new DrGroup;
						othergrp->set("text", i18n("Others"));
						driver->addGroup(othergrp);
					}
					grp = othergrp;
				}
			}

			if (grp && opt)
			{
				opt->setName(it.current()->arg("name"));
				opt->set("text", it.current()->arg("comment"));
				opt->setValueText(it.current()->arg("default"));
				opt->set("default", it.current()->arg("default"));
				grp->addOption(opt);
			}
		}
	}

	return driver;
}

void loadPageSizes(DrMain *driver, MaticBlock *blk)
{
	QDictIterator<MaticBlock>	it(blk->m_blocks);
	for (; it.current(); ++it)
	{
		QString	vals = it.current()->arg("driverval");
		int	p = vals.find(' ');
		if (p != -1)
		{
			int	w = vals.left(p).toInt(), h = vals.right(vals.length()-p-1).toInt();
			DrPageSize	*ps = new DrPageSize(it.currentKey(), w, h, 36, 24, w-36, h-24);
			driver->addPageSize(ps);
		}
	}
}


QString maticFile(PrintcapEntry *entry)
{
	QString	s(entry->field("af"));
	if (s.isEmpty())
	{
		s = entry->field("filter_options");
		if (!s.isEmpty())
		{
			int	p = s.findRev(' ');
			if (p != -1)
				s = s.mid(p+1);
		}
	}
	return s;
}
