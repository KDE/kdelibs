/* vi: ts=8 sts=4 sw=4
 *  -*- C++ -*-
 *
 *  Copyright (C) 1997 Christoph Neerfeld
 *  email:  Christoph.Neerfeld@home.ivm.de or chris@kde.org
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.	If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 *  Revamped April 2000 by Geert Jansen <jansen@kde.org>
 */

#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qfileinfo.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kprogress.h>

#include "kiconview.h"
#include "kiconloaderdialog.h"


/*
 * KIconLoaderCanvas: Iconview for the iconloader dialog.
 */

KIconLoaderCanvas::KIconLoaderCanvas(QWidget *parent, const char *name)
    : KIconView(parent, name)
{
    mpLoader = KGlobal::iconLoader();
    mpTimer = new QTimer(this);
    connect(mpTimer, SIGNAL(timeout()), SLOT(slotLoadFiles()));
    connect(this, SIGNAL(currentChanged(QIconViewItem *)),
	    SLOT(slotCurrentChanged(QIconViewItem *)));

    setGridX(65);
    setWordWrapIconText(false);
    setShowToolTips(true);
}

KIconLoaderCanvas::~KIconLoaderCanvas()
{
    delete mpTimer;
}

void KIconLoaderCanvas::loadFiles(QStringList files)
{
    clear();
    mFiles = files;
    mpTimer->start(0, true);
}

void KIconLoaderCanvas::slotLoadFiles()
{
    setResizeMode(Fixed);
    emit startLoading(mFiles.count());
    QApplication::setOverrideCursor(waitCursor);

    int i;
    QStringList::ConstIterator it;
    for (it=mFiles.begin(), i=0; it!=mFiles.end(); it++, i++)
    {
	emit progress(i);
	kapp->processEvents();
	QImage img;
	img.load(*it);
	if (img.isNull())
	    continue;
	if (img.width() > 60 || img.height() > 60) 
	{
	    if (img.width() > img.height())
	    {
		int height = (int) ((60.0 / img.width()) * img.height());
		img = img.smoothScale(60, height);
	    } else 
	    {
		int width = (int) ((60.0 / img.height()) * img.width());
		img = img.smoothScale(width, 60);
	    }
	}
	QPixmap pm;
	pm.convertFromImage(img);
	QFileInfo fi(*it);
	QIconViewItem *item = new QIconViewItem(this, fi.baseName(), pm);
	item->setKey(*it);
	item->setDragEnabled(false);
	item->setDropEnabled(false);
    }

    QApplication::restoreOverrideCursor();
    emit finished();
    setResizeMode(Adjust);
}

QString KIconLoaderCanvas::getCurrent()
{
    if (!currentItem())
	return QString::null;
    return currentItem()->key();
}

void KIconLoaderCanvas::slotCurrentChanged(QIconViewItem *item)
{
    emit nameChanged((item != 0L) ? item->text() : QString::null);
}

/*
 * Ignore escape when having focus (Espen Sand).
 */
void KIconLoaderCanvas::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Key_Escape)
        e->ignore();
    else
        KIconView::keyPressEvent(e);
}


/*
 * KIconLoaderDialog: Dialog for selecting icons. Both system and user
 * specified icons can be chosen.
 */

KIconLoaderDialog::KIconLoaderDialog(QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("Select Icon"), Help|Ok|Cancel, Ok)
{
    mpLoader = KGlobal::iconLoader();
    init();
}

KIconLoaderDialog::KIconLoaderDialog(KIconLoader *loader, QWidget *parent,
	const char *name)
    : KDialogBase(parent, name, true, i18n("Select Icon"), Help|Ok|Cancel, Ok)
{
    mpLoader = loader;
    init();
}

void KIconLoaderDialog::init()
{
    mGroup = mContext = mType = 0;

    QWidget *main = new QWidget( this );
    setMainWidget(main);

    QVBoxLayout *top = new QVBoxLayout(main, marginHint(), spacingHint());

    QButtonGroup *bgroup = new QButtonGroup(i18n("Icon source"), main);
    top->addWidget(bgroup);
    connect(bgroup, SIGNAL(clicked(int)), SLOT(slotButtonClicked(int)));
    QGridLayout *grid = new QGridLayout(bgroup, 3, 2, marginHint(), spacingHint());
    grid->addRowSpacing(0, 15);
    mpRb1 = new QRadioButton(i18n("System icons:"), bgroup);
    grid->addWidget(mpRb1, 1, 0);
    mpCombo = new QComboBox(main);
    connect(mpCombo, SIGNAL(activated(int)), SLOT(slotContext(int)));
    grid->addWidget(mpCombo, 1, 1);
    mpRb2 = new QRadioButton(i18n("Other icons:"), bgroup);
    grid->addWidget(mpRb2, 2, 0);
    mpBrowseBut = new QPushButton(i18n("Browse"), bgroup);
    grid->addWidget(mpBrowseBut, 2, 1);

    mpCanvas = new KIconLoaderCanvas(main);
    connect(mpCanvas, SIGNAL(executed(QIconViewItem *)), SLOT(accept()));
    mpCanvas->setMinimumSize(400, 125);
    top->addWidget(mpCanvas);

    mpProgress = new KProgress(main);
    top->addWidget(mpProgress);
    connect(mpCanvas, SIGNAL(startLoading(int)), SLOT(slotStartLoading(int)));
    connect(mpCanvas, SIGNAL(progress(int)), SLOT(slotProgress(int)));
    connect(mpCanvas, SIGNAL(finished()), SLOT(slotFinished()));

    // The order must match the context definitions in KIcon.
    mpCombo->insertItem(i18n("Actions"));
    mpCombo->insertItem(i18n("Applications"));
    mpCombo->insertItem(i18n("Devices"));
    mpCombo->insertItem(i18n("Filesystems"));
    mpCombo->insertItem(i18n("Mimetypes"));
    mpCombo->setFixedSize(mpCombo->sizeHint());
    mpBrowseBut->setFixedSize(mpCombo->sizeHint());

    // Make the dialog a little taller
    incInitialSize(QSize(0,100));
}


KIconLoaderDialog::~KIconLoaderDialog()
{
}

void KIconLoaderDialog::showIcons()
{
    mpCanvas->clear();
    if (mType == 0)
	mpCanvas->loadFiles(mpLoader->queryIcons(mGroup, mContext));
    else
	mpCanvas->loadFiles(mFileList);
}
	
void KIconLoaderDialog::slotButtonClicked(int id)
{
    QStringList lst;

    switch (id)
    {
    case 0:
	mType = 0;
	mpBrowseBut->setEnabled(false);
	mpCombo->setEnabled(true);
	showIcons();
	break;

    case 1:
	mType = 1;
	mpBrowseBut->setEnabled(true);
	mpCombo->setEnabled(false);
	showIcons();
	break;

    case 2:
	// TODO: Add file to mFileList
	break;
    }
}
	
void KIconLoaderDialog::slotContext(int id)
{
    mContext = id+1;
    showIcons();
}

void KIconLoaderDialog::slotStartLoading(int steps)
{
    if (steps < 10)
	mpProgress->hide();
    else
    {
        mpProgress->setRange(0, steps);
        mpProgress->setValue(0);
        mpProgress->show();
    }
}

void KIconLoaderDialog::slotProgress(int p)
{
    mpProgress->setValue(p);
}

void KIconLoaderDialog::slotFinished()
{
    mpProgress->hide();
}

QString KIconLoaderDialog::selectIcon(int group, int context)
{
    mGroup = group;
    mType = 0;
    mpRb1->setChecked(true);
    mpRb2->setChecked(false);
    mpCombo->setEnabled(true);
    mpBrowseBut->setEnabled(false);
    mContext = context;
    mpCombo->setCurrentItem(mContext-1);
    showIcons();
    KDialogBase::exec();
    
    if (result() == Accepted)
    {
	QString name = mpCanvas->getCurrent();
	if (name.isEmpty() || (mType == 1))
	    return name;
	QFileInfo fi(name);
	return fi.baseName();
    }
    return QString::null;
}


/*
 * KIconLoaderButton: A "choose icon" pushbutton.
 */

KIconLoaderButton::KIconLoaderButton(QWidget *parent, const char *name)
    : QPushButton(parent, name)
{
    // arbitrary
    mGroup = KIcon::Desktop; 
    mContext = KIcon::Application;

    mpLoader = KGlobal::iconLoader();
    mpDialog = new KIconLoaderDialog(this);
    connect(this, SIGNAL(clicked()), SLOT(slotChangeIcon()));
}

KIconLoaderButton::KIconLoaderButton(KIconLoader *loader,
	QWidget *parent, const char *name)
    : QPushButton(parent, name)
{
    mGroup = KIcon::Desktop; 
    mContext = KIcon::Application;

    mpLoader = loader;
    mpDialog = new KIconLoaderDialog(mpLoader, this);
    connect(this, SIGNAL(clicked()), SLOT(slotChangeIcon()));
}

KIconLoaderButton::~KIconLoaderButton()
{
    delete mpDialog;
}

void KIconLoaderButton::setIconType(int group, int context)
{
    mGroup = group;
    mContext = context;
}

void KIconLoaderButton::setIcon(QString icon)
{
    mIcon = icon;
    setPixmap(mpLoader->loadIcon(mIcon, mGroup));
}

void KIconLoaderButton::slotChangeIcon()
{
    QString name = mpDialog->selectIcon(mGroup, mContext);
    if (name.isNull())
	return;
    QPixmap pm = mpLoader->loadIcon(name, mGroup);
    setPixmap(pm);
    mIcon = name;
    emit iconChanged(name);
}


#include "kiconloaderdialog.moc"
