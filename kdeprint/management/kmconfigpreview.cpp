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

#include "kmconfigpreview.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kconfig.h>

KMConfigPreview::KMConfigPreview(QWidget *parent, const char *name)
: KMConfigPage(parent, name)
{
	setPageName(i18n("Preview"));
	setPageHeader(i18n("Preview Settings"));
	setPagePixmap("filefind");

	QGroupBox *box = new QGroupBox(0, Qt::Vertical, i18n("Preview Program"), this);

	m_useext = new QCheckBox(i18n("&Use external preview program"), box);
	m_program = new KURLRequester(box);
	QLabel	*lab = new QLabel(box);
	lab->setText(i18n("You can use an external preview program (PS viewer) instead of the "
					  "KDE built-in preview system. Note that if the KDE default PS viewer "
					  "(KGhostView) cannot be found, KDE tries automatically to find another "
					  "external PostScript viewer"));
	lab->setTextFormat(Qt::RichText);

	QVBoxLayout	*l0 = new QVBoxLayout(this, 5, 10);
	l0->addWidget(box);
	l0->addStretch(1);
	QVBoxLayout	*l1 = new QVBoxLayout(box->layout(), 10);
	l1->addWidget(lab);
	l1->addWidget(m_useext);
	l1->addWidget(m_program);

	connect(m_useext, SIGNAL(toggled(bool)), m_program, SLOT(setEnabled(bool)));
	m_program->setEnabled(false);
}

void KMConfigPreview::loadConfig(KConfig *conf)
{
	conf->setGroup("General");
	m_useext->setChecked(conf->readBoolEntry("ExternalPreview", false));
	m_program->setURL(conf->readEntry("PreviewCommand", "gv"));
}

void KMConfigPreview::saveConfig(KConfig *conf)
{
	conf->setGroup("General");
	conf->writeEntry("ExternalPreview", m_useext->isChecked());
	conf->writeEntry("PreviewCommand", m_program->url());
}
