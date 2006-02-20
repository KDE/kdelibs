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

#include "kmdriverdbwidget.h"
#include "kmdriverdb.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "driver.h"

#include <klistbox.h>
#include <kpushbutton.h>
#include <qcheckbox.h>
#include <kcursor.h>
#include <qapplication.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <q3strlist.h>

#include <klocale.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kguiitem.h>
#include <kio/netaccess.h>

KMDriverDbWidget::KMDriverDbWidget(QWidget *parent)
    : QWidget(parent)
{
	m_external.clear();
	m_valid = false;

	// build widget
	m_manu = new KListBox(this);
	m_model = new KListBox(this);
	m_postscript = new QCheckBox(i18n("&PostScript printer"),this);
	m_raw = new QCheckBox(i18n("&Raw printer (no driver needed)"),this);
	m_postscript->setCursor(KCursor::handCursor());
	m_raw->setCursor(KCursor::handCursor());
	m_other = new KPushButton(KGuiItem(i18n("&Other..."), "fileopen"), this);
	QLabel	*l1 = new QLabel(i18n("&Manufacturer:"), this);
	QLabel	*l2 = new QLabel(i18n("Mo&del:"), this);
	l1->setBuddy(m_manu);
	l2->setBuddy(m_model);

	// build layout
	QVBoxLayout	*main_ = new QVBoxLayout(this);
  main_->setMargin(0);
  main_->setSpacing(10);
	QGridLayout	*sub1_ = new QGridLayout(0);
  sub1_->setMargin(0);
  sub1_->setSpacing(0);
	QHBoxLayout	*sub2_ = new QHBoxLayout(0);
  sub2_->setMargin(0);
  sub2_->setSpacing(10);
	main_->addLayout(sub1_);
	main_->addLayout(sub2_);
	main_->addWidget(m_raw);
	sub1_->addWidget(l1,0,0);
	sub1_->addWidget(l2,0,2);
	sub1_->addWidget(m_manu,1,0);
	sub1_->addWidget(m_model,1,2);
	sub2_->addWidget(m_postscript,1);
	sub2_->addWidget(m_other,0);

	// build connections
	connect(KMDriverDB::self(),SIGNAL(dbLoaded(bool)),SLOT(slotDbLoaded(bool)));
	connect(KMDriverDB::self(), SIGNAL(error(const QString&)), SLOT(slotError(const QString&)));
	connect(m_manu,SIGNAL(highlighted(const QString&)),SLOT(slotManufacturerSelected(const QString&)));
	connect(m_raw,SIGNAL(toggled(bool)),m_manu,SLOT(setDisabled(bool)));
	connect(m_raw,SIGNAL(toggled(bool)),m_model,SLOT(setDisabled(bool)));
	connect(m_raw,SIGNAL(toggled(bool)),m_other,SLOT(setDisabled(bool)));
	connect(m_raw,SIGNAL(toggled(bool)),m_postscript,SLOT(setDisabled(bool)));
	connect(m_postscript,SIGNAL(toggled(bool)),m_manu,SLOT(setDisabled(bool)));
	connect(m_postscript,SIGNAL(toggled(bool)),m_model,SLOT(setDisabled(bool)));
	connect(m_postscript,SIGNAL(toggled(bool)),m_other,SLOT(setDisabled(bool)));
	connect(m_postscript,SIGNAL(toggled(bool)),m_raw,SLOT(setDisabled(bool)));
	connect(m_postscript,SIGNAL(toggled(bool)),SLOT(slotPostscriptToggled(bool)));
	connect(m_other,SIGNAL(clicked()),SLOT(slotOtherClicked()));
}

KMDriverDbWidget::~KMDriverDbWidget()
{
}

void KMDriverDbWidget::setDriver(const QString& manu, const QString& model)
{
	Q3ListBoxItem	*item = m_manu->findItem(manu);
	QString		model_(model);
	if (item)
	{
		m_manu->setCurrentItem(item);
		item = m_model->findItem(model_);
		if (!item)
			// try by stripping the manufacturer name from
			// the beginning of the model string. This is
			// often the case with PPD files
			item = m_model->findItem(model_.replace(0,manu.length()+1,QLatin1String("")));
		if (item)
			m_model->setCurrentItem(item);
	}
}

void KMDriverDbWidget::setHaveRaw(bool on)
{
	if (on)
		m_raw->show();
	else
		m_raw->hide();
}

void KMDriverDbWidget::setHaveOther(bool on)
{
	if (on)
		m_other->show();
	else
		m_other->hide();
}

QString KMDriverDbWidget::manufacturer()
{
	return m_manu->currentText();
}

QString KMDriverDbWidget::model()
{
	return m_model->currentText();
}

KMDBEntryList* KMDriverDbWidget::drivers()
{
	return KMDriverDB::self()->findEntry(manufacturer(),model());
}

bool KMDriverDbWidget::isRaw()
{
	return m_raw->isChecked();
}

void KMDriverDbWidget::init()
{
	if (!m_valid)
	{
		QApplication::setOverrideCursor(KCursor::waitCursor());
		m_manu->clear();
		m_model->clear();
		m_manu->insertItem(i18n("Loading..."));
		KMDriverDB::self()->init(this);
	}
}

void KMDriverDbWidget::slotDbLoaded(bool reloaded)
{
	QApplication::restoreOverrideCursor();
	m_valid = true;
	if (reloaded || m_manu->count() == 0 || (m_manu->count() == 1 && m_manu->text(0) == i18n("Loading...")))
	{ // do something only if DB reloaded
		m_manu->clear();
		m_model->clear();
		Q3DictIterator< Q3Dict<KMDBEntryList> >	it(KMDriverDB::self()->manufacturers());
		for (;it.current();++it)
			m_manu->insertItem(it.currentKey());
		m_manu->sort();
		m_manu->setCurrentItem(0);
	}
}

void KMDriverDbWidget::slotError(const QString& msg)
{
	QApplication::restoreOverrideCursor();
	m_valid = false;
	m_manu->clear();
	KMessageBox::error(this, "<qt>"+msg+"</qt>");
}

void KMDriverDbWidget::slotManufacturerSelected(const QString& name)
{
	m_model->clear();
	Q3Dict<KMDBEntryList>	*models = KMDriverDB::self()->findModels(name);
	if (models)
	{
		QStringList	ilist;
		Q3DictIterator<KMDBEntryList>	it(*models);
		for (;it.current();++it)
			ilist.append(QString( it.currentKey().toLatin1() ).toUpper());
		ilist.sort();
		m_model->insertStringList(ilist);
		m_model->setCurrentItem(0);
	}
}

void KMDriverDbWidget::slotPostscriptToggled(bool on)
{
	if (on)
	{
		Q3ListBoxItem	*item = m_manu->findItem("GENERIC");
		if (item)
		{
			m_manu->setCurrentItem(item);
			item = m_model->findItem( "POSTSCRIPT PRINTER" );
			if ( item )
			{
				m_model->setCurrentItem( item );
				return;
			}
		}
		KMessageBox::error(this,i18n("Unable to find the PostScript driver."));
		m_postscript->setChecked(false);
	}
}

void KMDriverDbWidget::slotOtherClicked()
{
	if (m_external.isEmpty())
	{
           KFileDialog dlg( QString::null, QString::null, this);
		KUrl url;

		dlg.setMode( KFile::File );
		dlg.setCaption( i18n( "Select Driver" ) );
		if ( dlg.exec() )
			url = dlg.selectedURL();

		if ( !url.isEmpty() )
		{
			QString filename;
			if ( KIO::NetAccess::download( url, filename, this ) )
			{
				DrMain	*driver = KMFactory::self()->manager()->loadFileDriver(filename);
				if (driver)
				{
					m_external = filename;
					disconnect(m_manu,SIGNAL(highlighted(const QString&)),this,SLOT(slotManufacturerSelected(const QString&)));
					m_manu->clear();
					m_model->clear();
					QString	s = driver->get("manufacturer");
					m_manu->insertItem((s.isEmpty() ? i18n("<Unknown>") : s));
					s = driver->get("model");
					m_model->insertItem((s.isEmpty() ? i18n("<Unknown>") : s));
					m_manu->setCurrentItem(0);
					m_model->setCurrentItem(0);
					m_other->setText(i18n("Database"));
					m_desc = driver->get("description");
					delete driver;
				}
				else
				{
					KIO::NetAccess::removeTempFile( filename );
					KMessageBox::error(this,"<qt>"+i18n("Wrong driver format.")+"<p>"+KMManager::self()->errorMsg()+"</p></qt>");
				}
			}
		}
	}
	else
	{
		m_external.clear();
		connect(m_manu,SIGNAL(highlighted(const QString&)),this,SLOT(slotManufacturerSelected(const QString&)));
		m_other->setText(i18n("Other"));
		m_desc.clear();
		slotDbLoaded(true);
	}
}
#include "kmdriverdbwidget.moc"
