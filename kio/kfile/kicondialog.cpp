/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kfile.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *           (C) 2000 Kurt Granroth <granroth@kde.org>
 *           (C) 1997 Christoph Neerfeld <chris@kde.org>
 *           (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#include "kicondialog.h"

#include <config.h>

#include <kiconviewsearchline.h>

#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kiconview.h>
#include <kfiledialog.h>
#include <kimagefilepreview.h>

#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <q3sortedlist.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qfileinfo.h>
#include <qtoolbutton.h>
#include <qprogressbar.h>

#ifdef HAVE_LIBAGG
#include <svgicons/ksvgiconengine.h>
#include <svgicons/ksvgiconpainter.h>
#endif

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
   int n = lastIndexOf('/');
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
    : KIconView(parent, name),d(new KIconCanvasPrivate)
{
    mpLoader = KGlobal::iconLoader();
    mpTimer = new QTimer(this);
    connect(mpTimer, SIGNAL(timeout()), SLOT(slotLoadFiles()));
    connect(this, SIGNAL(currentChanged(Q3IconViewItem *)),
	    SLOT(slotCurrentChanged(Q3IconViewItem *)));
    setGridX(80);
    setWordWrapIconText(false);
    setShowToolTips(true);
}

KIconCanvas::~KIconCanvas()
{
    delete mpTimer;
    delete d;
}

void KIconCanvas::loadFiles(const QStringList& files)
{
    clear();
    mFiles = files;
    emit startLoading(mFiles.count());
    mpTimer->start(10, true); // #86680
    d->m_bLoading = false;
}

void KIconCanvas::slotLoadFiles()
{
    setResizeMode(Fixed);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // disable updates to not trigger paint events when adding child items
    setUpdatesEnabled( false );

#ifdef HAVE_LIBAGG
    KSVGIconEngine *svgEngine = new KSVGIconEngine();
#endif

    d->m_bLoading = true;
    int i;
    QStringList::ConstIterator it;
    uint emitProgress = 10; // so we will emit it once in the beginning
    QStringList::ConstIterator end(mFiles.end());
    for (it=mFiles.begin(), i=0; it!=end; ++it, i++)
    {
	// Calling kapp->processEvents() makes the iconview flicker like hell
	// (it's being repainted once for every new item), so we don't do this.
	// Instead, we directly repaint the progress bar without going through
	// the event-loop. We do that just once for every 10th item so that
	// the progress bar doesn't flicker in turn. (pfeiffer)
	if ( emitProgress >= 10 ) {
	    emit progress(i);
            emitProgress = 0;
        }

        emitProgress++;
//	kapp->processEvents();
        if ( !d->m_bLoading ) // user clicked on a button that will load another set of icons
            break;
	QImage img;

	// Use the extension as the format. Works for XPM and PNG, but not for SVG
	QString path= *it;
	QString ext = path.right(3).toUpper();

	if (ext != "SVG" && ext != "VGZ")
	    img.load(*it);
#ifdef HAVE_LIBAGG
	else
	    if (svgEngine->load(60, 60, *it))
		img = *svgEngine->painter()->image();
#endif

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
	Q3IconViewItem *item = new Q3IconViewItem(this, fi.baseName(), pm);
	item->setKey(*it);
	item->setDragEnabled(false);
	item->setDropEnabled(false);
    }

#ifdef HAVE_LIBAGG
    delete svgEngine;
#endif

    // enable updates since we have to draw the whole view now
    setUpdatesEnabled( true );

    QApplication::restoreOverrideCursor();
    d->m_bLoading = false;
    emit finished();
    setResizeMode(Adjust);
}

QString KIconCanvas::getCurrent() const
{
    if (!currentItem())
	return QString();
    return currentItem()->key();
}

void KIconCanvas::stopLoading()
{
    d->m_bLoading = false;
}

void KIconCanvas::slotCurrentChanged(Q3IconViewItem *item)
{
    emit nameChanged((item != 0L) ? item->text() : QString());
}

class KIconDialog::KIconDialogPrivate
{
  public:
    KIconDialogPrivate() {
        m_bStrictIconSize = true;
	m_bLockUser = false;
	m_bLockCustomDir = false;
	searchLine = 0;
    }
    ~KIconDialogPrivate() {}
    bool m_bStrictIconSize, m_bLockUser, m_bLockCustomDir;
    QString custom;
    QString customLocation;
    KIconViewSearchLine *searchLine;
};

/*
 * KIconDialog: Dialog for selecting icons. Both system and user
 * specified icons can be chosen.
 */

KIconDialog::KIconDialog(QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("Select Icon"), Ok|Cancel, Ok),d(new KIconDialogPrivate)
{
    mpLoader = KGlobal::iconLoader();
    init();
}

KIconDialog::KIconDialog(KIconLoader *loader, QWidget *parent,
	const char *name)
    : KDialogBase(parent, name, true, i18n("Select Icon"), Ok|Cancel, Ok),d(new KIconDialogPrivate)
{
    mpLoader = loader;
    init();
}

void KIconDialog::init()
{
    mGroupOrSize = KIcon::Desktop;
    mContext = KIcon::Any;
    mType = 0;
    mFileList = KGlobal::dirs()->findAllResources("appicon", QLatin1String("*.png"));

    QWidget *main = new QWidget( this );
    setMainWidget(main);

    QVBoxLayout *top = new QVBoxLayout(main);
    top->setSpacing( spacingHint() );

    Q3ButtonGroup *bgroup = new Q3ButtonGroup(0, Qt::Vertical, i18n("Icon Source"), main);
    bgroup->layout()->setSpacing(KDialog::spacingHint());
    bgroup->layout()->setMargin(KDialog::marginHint());
    top->addWidget(bgroup);
    connect(bgroup, SIGNAL(clicked(int)), SLOT(slotButtonClicked(int)));
    QGridLayout *grid = new QGridLayout(bgroup->layout(), 3, 2);
    grid->addRowSpacing(0, 15);
    mpRb1 = new QRadioButton(i18n("S&ystem icons:"), bgroup);
    grid->addWidget(mpRb1, 1, 0);
    mpCombo = new QComboBox(bgroup);
    connect(mpCombo, SIGNAL(activated(int)), SLOT(slotContext(int)));
    grid->addWidget(mpCombo, 1, 1);
    mpRb2 = new QRadioButton(i18n("O&ther icons:"), bgroup);
    grid->addWidget(mpRb2, 2, 0);
    mpBrowseBut = new QPushButton(i18n("&Browse..."), bgroup);
    grid->addWidget(mpBrowseBut, 2, 1);

    //
    // ADD SEARCHLINE
    //
    QHBoxLayout *searchLayout = new QHBoxLayout(0, 0, KDialog::spacingHint());
    top->addLayout(searchLayout);

    QToolButton *clearSearch = new QToolButton(main);
    clearSearch->setTextLabel(i18n("Clear Search"), true);
    clearSearch->setIcon(SmallIconSet(QApplication::isRightToLeft() ? "clear_left" :"locationbar_erase"));
    searchLayout->addWidget(clearSearch);

    QLabel *searchLabel = new QLabel(i18n("&Search:"), main);
    searchLayout->addWidget(searchLabel);

    d->searchLine = new KIconViewSearchLine(main);
    searchLayout->addWidget(d->searchLine);
    searchLabel->setBuddy(d->searchLine);


    // signals and slots connections
    connect(clearSearch, SIGNAL(clicked()), d->searchLine, SLOT(clear()));

    QString wtstr = i18n("Search interactively for icon names (e.g. folder).");
    searchLabel->setWhatsThis(wtstr);
    d->searchLine->setWhatsThis(wtstr);


    mpCanvas = new KIconCanvas(main);
    connect(mpCanvas, SIGNAL(executed(Q3IconViewItem *)), SLOT(slotAcceptIcons()));
    connect(mpCanvas, SIGNAL(returnPressed(Q3IconViewItem *)), SLOT(slotAcceptIcons()));
    mpCanvas->setMinimumSize(400, 125);
    top->addWidget(mpCanvas);
    d->searchLine->setIconView(mpCanvas);

    mpProgress = new QProgressBar(main);
    top->addWidget(mpProgress);
    connect(mpCanvas, SIGNAL(startLoading(int)), SLOT(slotStartLoading(int)));
    connect(mpCanvas, SIGNAL(progress(int)), SLOT(slotProgress(int)));
    connect(mpCanvas, SIGNAL(finished()), SLOT(slotFinished()));

    // When pressing Ok or Cancel, stop loading icons
    connect(this, SIGNAL(hidden()), mpCanvas, SLOT(stopLoading()));

    // The order must match the context definitions in KIcon.
    mpCombo->addItem(i18n("Actions"));
    mpCombo->addItem(i18n("Applications"));
    mpCombo->addItem(i18n("Devices"));
    mpCombo->addItem(i18n("Filesystems"));
    mpCombo->addItem(i18n("Mimetypes"));
    mpCombo->setFixedSize(mpCombo->sizeHint());
    mpBrowseBut->setFixedWidth(mpCombo->width());

    // Make the dialog a little taller
    incInitialSize(QSize(0,100));
}


KIconDialog::~KIconDialog()
{
    delete d;
}

void KIconDialog::slotAcceptIcons()
{
  d->custom.clear();
  slotOk();
}

void KIconDialog::showIcons()
{
    mpCanvas->clear();
    QStringList filelist;
    if (mType == 0)
	if (d->m_bStrictIconSize)
            filelist=mpLoader->queryIcons(mGroupOrSize, mContext);
        else
            filelist=mpLoader->queryIconsByContext(mGroupOrSize, mContext);
    else if ( !d->customLocation.isNull() )
	filelist=mpLoader->queryIconsByDir( d->customLocation );
    else
	filelist=mFileList;

    Q3SortedList <IconPath>iconlist;
    iconlist.setAutoDelete(true);
    QStringList::Iterator it;
    for( it = filelist.begin(); it != filelist.end(); ++it )
       iconlist.append(new IconPath(*it));

    iconlist.sort();
    filelist.clear();

    for ( IconPath *ip=iconlist.first(); ip != 0; ip=iconlist.next() )
       filelist.append(*ip);

    d->searchLine->clear();
    mpCanvas->loadFiles(filelist);
}

void KIconDialog::setStrictIconSize(bool b)
{
    d->m_bStrictIconSize=b;
}

bool KIconDialog::strictIconSize() const
{
    return d->m_bStrictIconSize;
}

void KIconDialog::setIconSize( int size )
{
    // see KIconLoader, if you think this is weird
    if ( size == 0 )
        mGroupOrSize = KIcon::Desktop; // default Group
    else
        mGroupOrSize = -size; // yes, KIconLoader::queryIconsByContext is weird
}

int KIconDialog::iconSize() const
{
    // 0 or any other value ==> mGroupOrSize is a group, so we return 0
    return (mGroupOrSize < 0) ? -mGroupOrSize : 0;
}

#ifndef KDE_NO_COMPAT
QString KIconDialog::selectIcon(KIcon::Group group, KIcon::Context context, bool user)
{
    setup( group, context, false, 0, user );
    return openDialog();
}
#endif

void KIconDialog::setup(KIcon::Group group, KIcon::Context context,
                        bool strictIconSize, int iconSize, bool user )
{
    d->m_bStrictIconSize = strictIconSize;
    mGroupOrSize = (iconSize == 0) ? group : -iconSize;
    mType = user ? 1 : 0;
    mpRb1->setChecked(!user);
    mpRb2->setChecked(user);
    mpCombo->setEnabled(!user);
    mpBrowseBut->setEnabled(user);
    mContext = context;
    mpCombo->setCurrentIndex(mContext-1);
}

void KIconDialog::setup(KIcon::Group group, KIcon::Context context,
                        bool strictIconSize, int iconSize, bool user,
                        bool lockUser, bool lockCustomDir )
{
    d->m_bStrictIconSize = strictIconSize;
    d->m_bLockUser = lockUser;
    d->m_bLockCustomDir = lockCustomDir;
    mGroupOrSize = (iconSize == 0) ? group : -iconSize;
    mType = user ? 1 : 0;
    mpRb1->setChecked(!user);
    mpRb1->setEnabled( !lockUser || !user );
    mpRb2->setChecked(user);
    mpRb2->setEnabled( !lockUser || user );
    mpCombo->setEnabled(!user);
    mpBrowseBut->setEnabled( user && !lockCustomDir );
    mContext = context;
    mpCombo->setCurrentIndex(mContext-1);
}

void KIconDialog::setCustomLocation( const QString& location )
{
    d->customLocation = location;
}

QString KIconDialog::openDialog()
{
    showIcons();

    if ( exec() == Accepted )
    {
        if (!d->custom.isNull())
            return d->custom;
	QString name = mpCanvas->getCurrent();
	if (name.isEmpty() || (mType == 1))
	    return name;
	QFileInfo fi(name);
	return fi.baseName();
    }
    return QString();
}

void KIconDialog::showDialog()
{
    setModal(false);
    showIcons();
    show();
}

void KIconDialog::slotOk()
{
    QString name;
    if (!d->custom.isNull())
    {
        name = d->custom;
    }
    else
    {
        name = mpCanvas->getCurrent();
        if (!name.isEmpty() && (mType != 1))
        {
            QFileInfo fi(name);
            name = fi.baseName();
        }
    }

    emit newIconName(name);
    KDialogBase::slotOk();
}

QString KIconDialog::getIcon(KIcon::Group group, KIcon::Context context,
                             bool strictIconSize, int iconSize, bool user,
                             QWidget *parent, const QString &caption)
{
    KIconDialog dlg(parent, "icon dialog");
    dlg.setup( group, context, strictIconSize, iconSize, user );
    if (!caption.isNull())
        dlg.setCaption(caption);

    return dlg.openDialog();
}

void KIconDialog::slotButtonClicked(int id)
{
    QString file;

    switch (id)
    {
    case 0:
        if(mType!=0)
        {
            mType = 0;
            mpBrowseBut->setEnabled(false);
            mpCombo->setEnabled(true);
            showIcons();
        }
	break;

    case 1:
        if(mType!=1)
        {
            mType = 1;
            mpBrowseBut->setEnabled( !d->m_bLockCustomDir );
            mpCombo->setEnabled(false);
            showIcons();
        }
        break;
    case 2:
        {
            // Create a file dialog to select a PNG, XPM or SVG file,
            // with the image previewer shown.
            // KFileDialog::getImageOpenURL doesn't allow svg.
           KFileDialog dlg(QString::null, i18n("*.png *.xpm *.svg *.svgz|Icon Files (*.png *.xpm *.svg *.svgz)"),
                            this);
            dlg.setOperationMode( KFileDialog::Opening );
            dlg.setCaption( i18n("Open") );
            dlg.setMode( KFile::File );

            KImageFilePreview *ip = new KImageFilePreview( &dlg );
            dlg.setPreviewWidget( ip );
            dlg.exec();

            file = dlg.selectedFile();
            if (!file.isEmpty())
            {
                d->custom = file;
                if ( mType == 1 )
                  d->customLocation = QFileInfo( file ).absolutePath();
                slotOk();
            }
        }
        break;
    }
}

void KIconDialog::slotContext(int id)
{
    mContext = static_cast<KIcon::Context>(id+1);
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
    // commented out the following since setProgress already paints ther
    // progress bar. ->repaint() only makes it flicker
    //mpProgress->repaint();
}

void KIconDialog::slotFinished()
{
    mpProgress->hide();
}

class KIconButton::KIconButtonPrivate
{
  public:
    KIconButtonPrivate() {
        m_bStrictIconSize = false;
        iconSize = 0; // let KIconLoader choose the default
    }
    ~KIconButtonPrivate() {}
    bool m_bStrictIconSize;
    int iconSize;
};


/*
 * KIconButton: A "choose icon" pushbutton.
 */

KIconButton::KIconButton(QWidget *parent, const char *name)
    : QPushButton(parent, name),d(new KIconButtonPrivate)
{
    init( KGlobal::iconLoader() );
}

KIconButton::KIconButton(KIconLoader *loader,
	QWidget *parent, const char *name)
    : QPushButton(parent, name),d(new KIconButtonPrivate)
{
    init( loader );
}

void KIconButton::init( KIconLoader *loader )
{
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

bool KIconButton::strictIconSize() const
{
    return d->m_bStrictIconSize;
}

void KIconButton::setIconSize( int size )
{
    d->iconSize = size;
}

int KIconButton::iconSize() const
{
    return d->iconSize;
}

void KIconButton::setIconType(KIcon::Group group, KIcon::Context context, bool user)
{
    mGroup = group;
    mContext = context;
    mbUser = user;
}

void KIconButton::setIcon(const QString& icon)
{
    mIcon = icon;
    setIcon(mpLoader->loadIconSet(mIcon, mGroup, d->iconSize));

    if (!mpDialog)
    {
        mpDialog = new KIconDialog(mpLoader, this);
        connect(mpDialog, SIGNAL(newIconName(const QString&)), SLOT(newIconName(const QString&)));
    }

    if ( mbUser )
      mpDialog->setCustomLocation( QFileInfo( mpLoader->iconPath(mIcon, mGroup, true) ).absolutePath() );
}

void KIconButton::resetIcon()
{
    mIcon.clear();
    setIcon(QIcon());
}

void KIconButton::slotChangeIcon()
{
    if (!mpDialog)
    {
        mpDialog = new KIconDialog(mpLoader, this);
        connect(mpDialog, SIGNAL(newIconName(const QString&)), SLOT(newIconName(const QString&)));
    }

    mpDialog->setup( mGroup, mContext, d->m_bStrictIconSize, d->iconSize, mbUser );
    mpDialog->showDialog();
}

void KIconButton::newIconName(const QString& name)
{
    if (name.isEmpty())
        return;

    QIcon iconset = mpLoader->loadIconSet(name, mGroup, d->iconSize);
    setIcon(iconset);
    mIcon = name;

    if ( mbUser )
      mpDialog->setCustomLocation( QFileInfo( mpLoader->iconPath(mIcon, mGroup, true) ).absolutePath() );

    emit iconChanged(name);
}

void KIconCanvas::virtual_hook( int id, void* data )
{ KIconView::virtual_hook( id, data ); }

void KIconDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "kicondialog.moc"
