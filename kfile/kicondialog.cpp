/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kfile.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *           (C) 2000 Kurt Granroth <granroth@kde.org>
 *           (C) 1997 Christoph Neerfeld <chris@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qsortedlist.h>
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
#include <kiconview.h>
#include <kfiledialog.h>

#include "kicondialog.h"

class KIconCanvas::KIconCanvasPrivate
{
  public:
    KIconCanvasPrivate() { m_bLoading = false; }
    ~KIconCanvasPrivate() {}
    bool m_bLoading;
};

/**
 * Helper class for sorting icon paths by icon name
 */
class IconPath : public QString
{
protected:
 QString m_iconName;

public:
 IconPath(const QString &ip) : QString (ip)
 {
   int n = findRev('/');
   m_iconName = (n==-1) ? static_cast<QString>(*this) : mid(n+1);
 }


 IconPath() : QString ()
 { }

 bool operator== (const IconPath &ip) const
 { return m_iconName == ip.m_iconName; }

 bool operator< (const IconPath &ip) const
 { return m_iconName < ip.m_iconName; }

};

/*
 * KIconCanvas: Iconview for the iconloader dialog.
 */

KIconCanvas::KIconCanvas(QWidget *parent, const char *name)
    : KIconView(parent, name)
{
    d = new KIconCanvasPrivate;
    mpLoader = KGlobal::iconLoader();
    mpTimer = new QTimer(this);
    connect(mpTimer, SIGNAL(timeout()), SLOT(slotLoadFiles()));
    connect(this, SIGNAL(currentChanged(QIconViewItem *)),
	    SLOT(slotCurrentChanged(QIconViewItem *)));
    setGridX(65);
    setWordWrapIconText(false);
    setShowToolTips(true);
}

KIconCanvas::~KIconCanvas()
{
    delete mpTimer;
    delete d;
}

void KIconCanvas::loadFiles(QStringList files)
{
    clear();
    mFiles = files;
    mpTimer->start(0, true);
    d->m_bLoading = false;
}

void KIconCanvas::slotLoadFiles()
{
    setResizeMode(Fixed);
    emit startLoading(mFiles.count());
    QApplication::setOverrideCursor(waitCursor);

    d->m_bLoading = true;
    int i;
    QStringList::ConstIterator it;
    for (it=mFiles.begin(), i=0; it!=mFiles.end(); it++, i++)
    {
	emit progress(i);
	kapp->processEvents();
        if ( !d->m_bLoading ) // user clicked on a button that will load another set of icons
            break;
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
    d->m_bLoading = false;
    emit finished();
    setResizeMode(Adjust);
}

QString KIconCanvas::getCurrent()
{
    if (!currentItem())
	return QString::null;
    return currentItem()->key();
}

void KIconCanvas::stopLoading()
{
    d->m_bLoading = false;
}

void KIconCanvas::slotCurrentChanged(QIconViewItem *item)
{
    emit nameChanged((item != 0L) ? item->text() : QString::null);
}

class KIconDialog::KIconDialogPrivate
{
  public:
    KIconDialogPrivate() { m_bStrictIconSize = true; }
    ~KIconDialogPrivate() {}
    bool m_bStrictIconSize;
    QString custom;
};

/*
 * KIconDialog: Dialog for selecting icons. Both system and user
 * specified icons can be chosen.
 */

KIconDialog::KIconDialog(QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("Select Icon"), Help|Ok|Cancel, Ok)
{
    d = new KIconDialogPrivate;
    mpLoader = KGlobal::iconLoader();
    init();
}

KIconDialog::KIconDialog(KIconLoader *loader, QWidget *parent,
	const char *name)
    : KDialogBase(parent, name, true, i18n("Select Icon"), Help|Ok|Cancel, Ok)
{
    d = new KIconDialogPrivate;
    mpLoader = loader;
    init();
}

void KIconDialog::init()
{
    mGroup = mContext = mType = 0;
    mFileList = KGlobal::dirs()->findAllResources("appicon", QString::fromLatin1("*.png"));

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
    mpCombo = new QComboBox(bgroup);
    connect(mpCombo, SIGNAL(activated(int)), SLOT(slotContext(int)));
    grid->addWidget(mpCombo, 1, 1);
    mpRb2 = new QRadioButton(i18n("Other icons:"), bgroup);
    grid->addWidget(mpRb2, 2, 0);
    mpBrowseBut = new QPushButton(i18n("&Browse..."), bgroup);
    grid->addWidget(mpBrowseBut, 2, 1);

    mpCanvas = new KIconCanvas(main);
    connect(mpCanvas, SIGNAL(executed(QIconViewItem *)), SLOT(slotAcceptIcons()));
    mpCanvas->setMinimumSize(400, 125);
    top->addWidget(mpCanvas);

    mpProgress = new KProgress(main);
    top->addWidget(mpProgress);
    connect(mpCanvas, SIGNAL(startLoading(int)), SLOT(slotStartLoading(int)));
    connect(mpCanvas, SIGNAL(progress(int)), SLOT(slotProgress(int)));
    connect(mpCanvas, SIGNAL(finished()), SLOT(slotFinished()));

    // When pressing Ok or Cancel, stop loading icons
    connect(this, SIGNAL(hidden()), mpCanvas, SLOT(stopLoading()));

    // The order must match the context definitions in KIcon.
    mpCombo->insertItem(i18n("Actions"));
    mpCombo->insertItem(i18n("Applications"));
    mpCombo->insertItem(i18n("Devices"));
    mpCombo->insertItem(i18n("Filesystems"));
    mpCombo->insertItem(i18n("Mimetypes"));
    mpCombo->setFixedSize(mpCombo->sizeHint());
    mpBrowseBut->setFixedSize(QSize(mpCombo->width(), mpCombo->height()+6));

    // Make the dialog a little taller
    incInitialSize(QSize(0,100));
}


KIconDialog::~KIconDialog()
{
    delete d;
}

void KIconDialog::slotAcceptIcons()
{
  d->custom=QString::null;
  accept();
}

void KIconDialog::showIcons()
{
    mpCanvas->clear();
    QStringList filelist;
    if (mType == 0)
	if (d->m_bStrictIconSize)
	  filelist=mpLoader->queryIcons(mGroup, mContext);
        else
	  filelist=mpLoader->queryIconsByContext(mGroup, mContext);
    else
	filelist=mFileList;

    QSortedList <IconPath>iconlist;
    iconlist.setAutoDelete(true);
    QStringList::Iterator it;
    for( it = filelist.begin(); it != filelist.end(); ++it )
       iconlist.append(new IconPath(*it));

    iconlist.sort();
    filelist.clear();

    for ( IconPath *ip=iconlist.first(); ip != 0; ip=iconlist.next() )
       filelist.append(*ip);

    mpCanvas->loadFiles(filelist);
}

void KIconDialog::setStrictIconSize(bool b)
{
    d->m_bStrictIconSize=b;
}

QString KIconDialog::selectIcon(int group, int context, bool user)
{
    mGroup = group;
    mType = user ? 1 : 0;
    mpRb1->setChecked(!user);
    mpRb2->setChecked(user);
    mpCombo->setEnabled(!user);
    mpBrowseBut->setEnabled(user);
    mContext = context;
    mpCombo->setCurrentItem(mContext-1);
    showIcons();
    KDialogBase::exec();

    if (result() == Accepted)
    {
        if (!d->custom.isNull())
            return d->custom;
	QString name = mpCanvas->getCurrent();
	if (name.isEmpty() || (mType == 1))
	    return name;
	QFileInfo fi(name);
	return fi.baseName();
    }
    return QString::null;
}

void KIconDialog::slotButtonClicked(int id)
{
    QString file;

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
	file = KFileDialog::getOpenFileName(QString::null,
		i18n("*.png *.xpm|Icon Files (*.png *.xpm)"), this);
	if (!file.isEmpty())
        {
            d->custom = file;
            accept();
	}
	break;
    }
}

void KIconDialog::slotContext(int id)
{
    mContext = id+1;
    showIcons();
}

void KIconDialog::slotStartLoading(int steps)
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

void KIconDialog::slotProgress(int p)
{
    mpProgress->setValue(p);
}

void KIconDialog::slotFinished()
{
    mpProgress->hide();
}

class KIconButton::KIconButtonPrivate
{
  public:
    KIconButtonPrivate() { m_bStrictIconSize = true; }
    ~KIconButtonPrivate() {}
    bool m_bStrictIconSize;
};


/*
 * KIconButton: A "choose icon" pushbutton.
 */

KIconButton::KIconButton(QWidget *parent, const char *name)
    : QPushButton(parent, name)
{
    d = new KIconButtonPrivate;
    // arbitrary
    mGroup = KIcon::Desktop;
    mContext = KIcon::Application;
    mbUser = false;

    mpLoader = KGlobal::iconLoader();
    mpDialog = 0L;
    connect(this, SIGNAL(clicked()), SLOT(slotChangeIcon()));
}

KIconButton::KIconButton(KIconLoader *loader,
	QWidget *parent, const char *name)
    : QPushButton(parent, name)
{
    d = new KIconButtonPrivate;
    mGroup = KIcon::Desktop;
    mContext = KIcon::Application;
    mbUser = false;

    mpLoader = loader;
    mpDialog = 0L;
    connect(this, SIGNAL(clicked()), SLOT(slotChangeIcon()));
}

KIconButton::~KIconButton()
{
    delete mpDialog;
    delete d;
}

void KIconButton::setStrictIconSize(bool b)
{
    d->m_bStrictIconSize=b;
}

void KIconButton::setIconType(int group, int context, bool user)
{
    mGroup = group;
    mContext = context;
    mbUser = user;
}

void KIconButton::setIcon(QString icon)
{
    mIcon = icon;
    setPixmap(mpLoader->loadIcon(mIcon, mGroup));
}

void KIconButton::resetIcon()
{
    mIcon = QString::null;
    setPixmap(QPixmap());
}

void KIconButton::slotChangeIcon()
{
    if (!mpDialog)
      mpDialog = new KIconDialog(mpLoader, this);
    mpDialog->setStrictIconSize(d->m_bStrictIconSize);
    QString name = mpDialog->selectIcon(mGroup, mContext, mbUser);
    if (name.isNull())
	return;
    QPixmap pm = mpLoader->loadIcon(name, mGroup);
    setPixmap(pm);
    mIcon = name;
    emit iconChanged(name);
}


#include "kicondialog.moc"
