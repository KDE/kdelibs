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

#include "cupsdsplash.h"

#include <qlabel.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <klocale.h>
#include <kstandarddirs.h>

CupsdSplash::CupsdSplash(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	delete deflabel_;
	deflabel_ = 0;

	header_ = i18n("Welcome to the CUPS server configuration tool");

	QVBoxLayout	*main_ = new QVBoxLayout(this, 10, 10);
	QHBoxLayout	*sub_ = new QHBoxLayout(0, 0, 10);
	main_->addLayout(sub_);

	QLabel	*cupslogo_ = new QLabel(this);
	QString	logopath = locate("data", QString("kdeprint/cups_logo.png"));
	cupslogo_->setPixmap(logopath.isEmpty() ? QPixmap() : QPixmap(logopath));
	cupslogo_->setAlignment(Qt::AlignCenter);
	QLabel	*kupslogo_ = new QLabel(this);
	logopath = locate("data", QString("kdeprint/kde_logo.png"));
	kupslogo_->setPixmap(logopath.isEmpty() ? QPixmap() : QPixmap(logopath));
	kupslogo_->setAlignment(Qt::AlignCenter);

	QLabel	*helptxt_ = new QLabel(this);
	helptxt_->setText(i18n("<p>This tool will help you to configure graphically the server of the CUPS printing system. "
			       "The available options are classified in a hierarchical way "
			       "and can be accessed quickly through the tree view located on the left. "
			       "If this tree view is not visible, simply click (or double-click) on the first item in the view.</p><br>"
			       "<p>Each option has a default value. If you want the server to use this value, "
			       "simply check the box located on the right side of the corresponding option.</p>"));

	sub_->addWidget(cupslogo_);
	sub_->addWidget(kupslogo_);
	main_->addWidget(helptxt_, 1);
}

CupsdSplash::~CupsdSplash()
{
}

bool CupsdSplash::loadConfig(CupsdConf*, QString&)
{
	return true;
}

bool CupsdSplash::saveConfig(CupsdConf*, QString&)
{
	return true;
}

void CupsdSplash::setDefaults()
{
}
