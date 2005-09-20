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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "lpdtools.h"
#include "driver.h"
#include "kmprinter.h"

#include <qfile.h>
#include <klocale.h>

static const char *pt_pagesize[] = {
	"ledger", I18N_NOOP("Ledger"),
	"legal", I18N_NOOP("US Legal"),
	"letter", I18N_NOOP("US Letter"),
	"a4", I18N_NOOP("A4"),
	"a3", I18N_NOOP("A3"),
	"b4", I18N_NOOP("B4"),
	"b5", I18N_NOOP("B5"),
	0
};
static int pt_nup[] = { 1, 2, 4, 8, -1 };
static const char *pt_bool[] = {
	"YES", I18N_NOOP("Enabled"),
	"NO", I18N_NOOP("Disabled"),
	0
};

void setupBooleanOption(DrBooleanOption *opt)
{
	int 	i(0);
	while (pt_bool[i])
	{
		DrBase	*ch = new DrBase();
		ch->setName(pt_bool[i++]);
		ch->set("text",pt_bool[i++]);
		opt->addChoice(ch);
	}
}

QString nextWord(const QString& s, int& pos)
{
	int	p1(pos), p2(0);
	while (s[p1].isSpace() && p1 < (int)s.length()) p1++;
	if (s[p1] == '{')
	{
		p1++;
		p2 = s.find('}',p1);
	}
	else
	{
		p2 = p1;
		while (!s[p2].isSpace() && p2 < (int)s.length()) p2++;
	}
	pos = (p2+1);
	return s.mid(p1,p2-p1);
}

//************************************************************************************************

bool PrintcapEntry::readLine(const QString& line)
{
	QStringList	l = QStringList::split(':',line,false);
	if (l.count() > 0)
	{
		m_name = l[0];
		int	p(-1);
		// discard aliases
		if ((p=m_name.find('|')) != -1)
			m_name = m_name.left(p);
		m_args.clear();
		for (uint i=1; i<l.count(); i++)
		{
			int	p = l[i].find('=');
			if (p == -1) p = 2;
			QString	key = l[i].left(p);
			QString	value = l[i].right(l[i].length()-(l[i][p] == '=' ? p+1 : p));
			m_args[key] = value;
		}
		return true;
	}
	return false;
}

void PrintcapEntry::writeEntry(QTextStream& t)
{
	if (m_comment.isEmpty()) t << "# Entry for printer " << m_name << endl;
	else t << m_comment << endl;
	t << m_name << ":";
	for (QMap<QString,QString>::ConstIterator it=m_args.begin(); it!=m_args.end(); ++it)
	{
		t << "\\\n\t:" << it.key();
		if (!it.data().isEmpty())
			t << ((*it)[0] == '#' ? "" : "=") << *it;
		t << ":";
	}
	t << endl << endl;
}

QString PrintcapEntry::comment(int index)
{
	QString	w;
	if (m_comment.startsWith("##PRINTTOOL3##"))
	{
		int	p(0);
		for (int i=0;i<index;i++)
			w = nextWord(m_comment,p);
	}
	return w;
}

KMPrinter* PrintcapEntry::createPrinter()
{
	KMPrinter	*printer = new KMPrinter();
	printer->setName(m_name);
	printer->setPrinterName(m_name);
	printer->setInstanceName(QString::null);
	printer->setState(KMPrinter::Idle);
	printer->setType(KMPrinter::Printer);
	return printer;
}

//************************************************************************************************

QStringList splitPrinttoolLine(const QString& line)
{
	QStringList	l;
	int 		p = line.find(':');
	if (p != -1)
	{
		l.append(line.left(p));
		p = line.find('{',p);
		if (p == -1)
			l.append(line.right(line.length()-l[0].length()-1).trimmed());
		else
		{
			while (p != -1)
			{
				int 	q = line.find('}',p);
				if (q != -1)
				{
					l.append(line.mid(p+1,q-p-1));
					p = line.find('{',q);
				}
				else break;
			}
		}
	}
	return l;
}

bool PrinttoolEntry::readEntry(QTextStream& t)
{
	QString	line;
	QStringList	args;

	m_resolutions.setAutoDelete(true);
	m_depths.setAutoDelete(true);
	m_resolutions.clear();
	m_depths.clear();
	while (!t.atEnd())
	{
		line = getPrintcapLine(t);
		if (line.isEmpty())
			break;
		if (line == "EndEntry")
			return !m_name.isEmpty();
		QStringList	l = splitPrinttoolLine(line);
		if (l.count() > 1)
		{
			if (l[0] == "StartEntry") m_name = l[1];
			else if (l[0] == "GSDriver") m_gsdriver = l[1];
			else if (l[0] == "About") m_about = l[1];
			else if (l[0] == "Description") m_description = l[1];
			else if (l[0] == "Resolution" && l.count() > 2)
			{
				Resolution	*resol = new Resolution;
				bool 		ok(false);
				resol->xdpi = l[1].toInt(&ok);
				if (ok) resol->ydpi = l[2].toInt(&ok);
				if (l.count() > 3)
					resol->comment = l[3];
				if (ok) m_resolutions.append(resol);
				else delete resol;
			}
			else if (l[0] == "BitsPerPixel" && l.count() > 1)
			{
				BitsPerPixel	*dpth = new BitsPerPixel;
				dpth->bpp = l[1];
				if (l.count() > 2)
					dpth->comment = l[2];
				m_depths.append(dpth);
			}
		}
	}
	return false;
}

DrMain* PrinttoolEntry::createDriver()
{
	// create driver
	DrMain	*dr = new DrMain();
	dr->setName(m_name);
	dr->set("description",m_description);
	dr->set("text",m_description);
	dr->set("drtype","printtool");

	DrGroup		*gr(0);
	DrListOption	*lopt(0);
	DrStringOption	*sopt(0);
	DrBooleanOption	*bopt(0);
	DrBase		*ch(0);

	if (m_gsdriver != "TEXT")
	{
		// create GS group
		gr = new DrGroup();
		gr->set("text",i18n("GhostScript settings"));
		dr->addGroup(gr);

		// Pseudo option to have access to GS driver
		lopt = new DrListOption();
		lopt->setName("GSDEVICE");
		lopt->set("text",i18n("Driver"));
		lopt->set("default",m_gsdriver);
		gr->addOption(lopt);
		ch = new DrBase();
		ch->setName(m_gsdriver);
		ch->set("text",m_gsdriver);
		lopt->addChoice(ch);
		lopt->setValueText(m_gsdriver);


		// Resolutions
		if (m_resolutions.count() > 0)
		{
			lopt = new DrListOption();
			lopt->setName("RESOLUTION");
			lopt->set("text",i18n("Resolution"));
			gr->addOption(lopt);
			Q3PtrListIterator<Resolution>	it(m_resolutions);
			for (int i=0;it.current();++it,i++)
			{
				ch = new DrBase;
				ch->setName(QString::fromLatin1("%1x%2").arg(it.current()->xdpi).arg(it.current()->ydpi));
				if (it.current()->comment.isEmpty())
					ch->set("text",QString::fromLatin1("%1x%2 DPI").arg(it.current()->xdpi).arg(it.current()->ydpi));
				else
					ch->set("text",QString::fromLatin1("%2x%3 DPI (%1)").arg(it.current()->comment).arg(it.current()->xdpi).arg(it.current()->ydpi));
				lopt->addChoice(ch);
			}
			QString	defval = lopt->choices()->first()->name();
			lopt->set("default",defval);
			lopt->setValueText(defval);
		}

		// BitsPerPixels
		if (m_depths.count() > 0)
		{
			lopt = new DrListOption();
			lopt->setName("COLOR");
			lopt->set("text",i18n("Color depth"));
			gr->addOption(lopt);
			Q3PtrListIterator<BitsPerPixel>	it(m_depths);
			for (int i=0;it.current();++it,i++)
			{
				ch = new DrBase;
				if (m_gsdriver != "uniprint")
					ch->setName(QString::fromLatin1("-dBitsPerPixel=%1").arg(it.current()->bpp));
				else
					ch->setName(it.current()->bpp);
				if (it.current()->comment.isEmpty())
					ch->set("text",it.current()->bpp);
				else
					ch->set("text",QString::fromLatin1("%1 - %2").arg(it.current()->bpp).arg(it.current()->comment));
				lopt->addChoice(ch);
			}
			QString	defval = lopt->choices()->first()->name();
			lopt->set("default",defval);
			lopt->setValueText(defval);
		}

		// additional GS options
		sopt = new DrStringOption;
		sopt->setName("EXTRA_GS_OPTIONS");
		sopt->set("text",i18n("Additional GS options"));
		gr->addOption(sopt);
	}

	// General group
	gr = new DrGroup();
	gr->set("text",i18n("General"));
	dr->addGroup(gr);

	// Page size
	lopt = new DrListOption();
	lopt->setName("PAPERSIZE");
	lopt->set("text",i18n("Page size"));
	lopt->set("default","letter");
	gr->addOption(lopt);
	int 	i(0);
	while (pt_pagesize[i])
	{
		ch = new DrBase();
		ch->setName(pt_pagesize[i++]);
		ch->set("text",i18n(pt_pagesize[i++]));
		lopt->addChoice(ch);
	}
	lopt->setValueText("letter");

	// Nup
	lopt = new DrListOption();
	lopt->setName("NUP");
	lopt->set("text",i18n("Pages per sheet"));
	lopt->set("default","1");
	gr->addOption(lopt);
	i = 0;
	while (pt_nup[i] != -1)
	{
		ch = new DrBase();
		ch->setName(QString::number(pt_nup[i++]));
		ch->set("text",ch->name());
		lopt->addChoice(ch);
	}
	lopt->setValueText("1");

	// Margins
	sopt = new DrStringOption();
	sopt->setName("RTLFTMAR");
	sopt->set("text",i18n("Left/right margin (1/72 in)"));
	sopt->setValueText("18");
	gr->addOption(sopt);
	sopt = new DrStringOption();
	sopt->setName("TOPBOTMAR");
	sopt->set("text",i18n("Top/bottom margin (1/72 in)"));
	sopt->setValueText("18");
	gr->addOption(sopt);

	// Text group
	gr = new DrGroup();
	gr->set("text",i18n("Text options"));
	dr->addGroup(gr);

	// Send EOF
	bopt = new DrBooleanOption();
	bopt->setName("TEXT_SEND_EOF");
	bopt->set("text",i18n("Send EOF after job to eject page"));
	gr->addOption(bopt);
	setupBooleanOption(bopt);
	bopt->setValueText("NO");

	// Fix stair-stepping
	bopt = new DrBooleanOption();
	bopt->setName("CRLFTRANS");
	bopt->set("text",i18n("Fix stair-stepping text"));
	gr->addOption(bopt);
	setupBooleanOption(bopt);
	bopt->choices()->first()->setName("1");
	bopt->choices()->last()->setName("0");
	bopt->setValueText("0");

	if (m_gsdriver != "POSTSCRIPT")
	{
		// Fast text printing
		bopt = new DrBooleanOption();
		bopt->setName("ASCII_TO_PS");
		bopt->set("text",i18n("Fast text printing (non-PS printers only)"));
		gr->addOption(bopt);
		setupBooleanOption(bopt);
		bopt->choices()->first()->setName("NO");
		bopt->choices()->last()->setName("YES");
		bopt->setValueText("NO");
	}

	return dr;
}

//************************************************************************************************

QString getPrintcapLine(QTextStream& t, QString *lastcomment)
{
	QString	line, buffer, comm;
	while (!t.atEnd())
	{
		buffer = t.readLine().trimmed();
		if (buffer.isEmpty() || buffer[0] == '#')
		{
			comm = buffer;
			continue;
		}
		line.append(buffer);
		if (line.right(1) == "\\")
		{
			line.truncate(line.length()-1);
			line = line.trimmed();
		}
		else break;
	}
	if (lastcomment)
		*lastcomment = comm;
	return line;
}
