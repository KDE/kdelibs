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
#include <kbuttongroup.h>
#include <k3iconviewsearchline.h>
#include <assert.h>

#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <k3iconview.h>
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

#include <QSvgRenderer>
#include <QImage>
#include <QPainter>

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

KIconCanvas::KIconCanvas(QWidget *parent)
    : K3IconView(parent),d(new KIconCanvasPrivate)
{
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
    mpTimer->setSingleShot(true);
    mpTimer->start(10); // #86680
    d->m_bLoading = false;
}

void KIconCanvas::slotLoadFiles()
{
    setResizeMode(Fixed);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // disable updates to not trigger paint events when adding child items
    setUpdatesEnabled( false );

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
	else {
            // Special stuff for SVG icons
            img = QImage(60, 60, QImage::Format_ARGB32_Premultiplied);
            QPainter p(&img);
            QSvgRenderer renderer(*it);
            if (renderer.isValid())
                renderer.render(&p);
            p.end();
        }

	if (img.isNull())
	    continue;
	if (img.width() > 60 || img.height() > 60)
	{
	    if (img.width() > img.height())
	    {
		int height = (int) ((60.0 / img.width()) * img.height());
		img = img.scaled(60, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	    } else
	    {
		int width = (int) ((60.0 / img.height()) * img.width());
		img = img.scaled(width, 60, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	    }
	}
	QPixmap pm = QPixmap::fromImage(img);
	QFileInfo fi(*it);
	Q3IconViewItem *item = new Q3IconViewItem(this, fi.baseName(), pm);
	item->setKey(*it);
	item->setDragEnabled(false);
	item->setDropEnabled(false);
    }

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
    K3IconViewSearchLine *searchLine;
};

/*
 * KIconDialog: Dialog for selecting icons. Both system and user
 * specified icons can be chosen.
 */

KIconDialog::KIconDialog(QWidget *parent)
    : KDialog( parent ), d(new KIconDialogPrivate)
{
    setObjectName( "icondialog" );
    setModal( true );
    setCaption( i18n("Select Icon") );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );

    mpLoader = KGlobal::iconLoader();
    init();
}

KIconDialog::KIconDialog(KIconLoader *loader, QWidget *parent)
    : KDialog( parent ), d(new KIconDialogPrivate)
{
    setObjectName( "icondialog" );
    setModal( true );
    setCaption( i18n("Select Icon") );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );

    mpLoader = loader;
    init();
}

void KIconDialog::init()
{
    mGroupOrSize = K3Icon::Desktop;
    mContext = K3Icon::Any;
    mFileList = KGlobal::dirs()->findAllResources("appicon", QLatin1String("*.png"));

    QWidget *main = new QWidget( this );
    setMainWidget(main);

    QVBoxLayout *top = new QVBoxLayout(main);

    QGroupBox *bgroup = new QGroupBox(main);
    bgroup->setTitle(i18n("Icon Source"));

    QVBoxLayout *vbox = new QVBoxLayout;
    bgroup->setLayout( vbox );
    top->addWidget(bgroup);

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(KDialog::spacingHint());
    bgroup->layout()->addItem(grid);

    mpRb1 = new QRadioButton(i18n("S&ystem icons:"), bgroup);
    connect( mpRb1, SIGNAL( clicked() ), SLOT( slotSystemIconClicked() ) );
    grid->addWidget(mpRb1, 1, 0);
    mpCombo = new QComboBox(bgroup);
    connect(mpCombo, SIGNAL(activated(int)), SLOT(slotContext(int)));
    grid->addWidget(mpCombo, 1, 1);
    mpRb2 = new QRadioButton(i18n("O&ther icons:"), bgroup);
    connect( mpRb2, SIGNAL( clicked() ), SLOT( slotOtherIconClicked() ) );
    grid->addWidget(mpRb2, 2, 0);
    mpBrowseBut = new QPushButton(i18n("&Browse..."), bgroup);
    connect( mpBrowseBut, SIGNAL(clicked()), this, SLOT( slotBrowse() ) );
    grid->addWidget(mpBrowseBut, 2, 1);

    //
    // ADD SEARCHLINE
    //
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setMargin(0);
    searchLayout->setSpacing(KDialog::spacingHint());
    top->addLayout(searchLayout);

    QLabel *searchLabel = new QLabel(i18n("&Search:"), main);
    searchLayout->addWidget(searchLabel);

    d->searchLine = new K3IconViewSearchLine(main);
    searchLayout->addWidget(d->searchLine);
    searchLabel->setBuddy(d->searchLine);

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

    static const char* const context_text[] = {
        I18N_NOOP( "Actions" ),
        I18N_NOOP( "Animations" ),
        I18N_NOOP( "Applications" ),
        I18N_NOOP( "Categories" ),
        I18N_NOOP( "Devices" ),
        I18N_NOOP( "Emblems" ),
        I18N_NOOP( "Emotes" ),
        I18N_NOOP( "Filesystems" ),
        I18N_NOOP( "International" ),
        I18N_NOOP( "Mimetypes" ),
        I18N_NOOP( "Places" ),
        I18N_NOOP( "Status" ) };
    static const K3Icon::Context context_id[] = {
        K3Icon::Action,
        K3Icon::Animation,
        K3Icon::Application,
        K3Icon::Category,
        K3Icon::Device,
        K3Icon::Emblem,
        K3Icon::Emote,
        K3Icon::FileSystem,
        K3Icon::International,
        K3Icon::MimeType,
        K3Icon::Place,
        K3Icon::StatusIcon };
    mNumContext = 0;
    int cnt = sizeof( context_text ) / sizeof( context_text[ 0 ] );
    // check all 3 arrays have same sizes
    assert( cnt == sizeof( context_id ) / sizeof( context_id[ 0 ] )
            && cnt == sizeof( mContextMap ) / sizeof( mContextMap[ 0 ] ));
    for( int i = 0;
         i < cnt;
         ++i )
    {
        if( mpLoader->hasContext( context_id[ i ] ))
        {
            mpCombo->addItem(i18n( context_text[ i ] ));
            mContextMap[ mNumContext++ ] = context_id[ i ];
        }
    }
    mpCombo->setFixedSize(mpCombo->sizeHint());

    mpBrowseBut->setFixedWidth(mpCombo->width());

    // Make the dialog a little taller
    incrementInitialSize(QSize(0,100));
    connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
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
    if (mpRb1->isChecked())
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
        mGroupOrSize = K3Icon::Desktop; // default Group
    else
        mGroupOrSize = -size; // yes, KIconLoader::queryIconsByContext is weird
}

int KIconDialog::iconSize() const
{
    // 0 or any other value ==> mGroupOrSize is a group, so we return 0
    return (mGroupOrSize < 0) ? -mGroupOrSize : 0;
}

void KIconDialog::setup(K3Icon::Group group, K3Icon::Context context,
                        bool strictIconSize, int iconSize, bool user )
{
    d->m_bStrictIconSize = strictIconSize;
    mGroupOrSize = (iconSize == 0) ? group : -iconSize;
    mpRb1->setChecked(!user);
    mpRb2->setChecked(user);
    mpCombo->setEnabled(!user);
    mpBrowseBut->setEnabled(user);
    mContext = context;
    setContext( context );
}

void KIconDialog::setup(K3Icon::Group group, K3Icon::Context context,
                        bool strictIconSize, int iconSize, bool user,
                        bool lockUser, bool lockCustomDir )
{
    d->m_bStrictIconSize = strictIconSize;
    d->m_bLockUser = lockUser;
    d->m_bLockCustomDir = lockCustomDir;
    mGroupOrSize = (iconSize == 0) ? group : -iconSize;
    mpRb1->setChecked(!user);
    mpRb1->setEnabled( !lockUser || !user );
    mpRb2->setChecked(user);
    mpRb2->setEnabled( !lockUser || user );
    mpCombo->setEnabled(!user);
    mpBrowseBut->setEnabled( user && !lockCustomDir );
    setContext( context );
}

void KIconDialog::setContext( K3Icon::Context context )
{
    mContext = context;
    for( int i = 0;
         i < mNumContext;
         ++i )
        if( mContextMap[ i ] == context )
        {
            mpCombo->setCurrentIndex( i );
            return;
        }
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
	if (name.isEmpty() || (mpRb2->isChecked()))
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
        if (!name.isEmpty() && (mpRb2->isChecked()))
        {
            QFileInfo fi(name);
            name = fi.baseName();
        }
    }

    emit newIconName(name);
    KDialog::accept();
}

QString KIconDialog::getIcon(K3Icon::Group group, K3Icon::Context context,
                             bool strictIconSize, int iconSize, bool user,
                             QWidget *parent, const QString &caption)
{
    KIconDialog dlg(parent);
    dlg.setObjectName(QLatin1String("icon dialog"));
    dlg.setup( group, context, strictIconSize, iconSize, user );
    if (!caption.isNull())
        dlg.setCaption(caption);

    return dlg.openDialog();
}

void KIconDialog::slotBrowse()
{
    // Create a file dialog to select a PNG, XPM or SVG file,
    // with the image previewer shown.
    // KFileDialog::getImageOpenURL doesn't allow svg.
    KUrl emptyUrl;
    KFileDialog dlg( emptyUrl, i18n("*.png *.xpm *.svg *.svgz|Icon Files (*.png *.xpm *.svg *.svgz)"),
                     this);
    dlg.setOperationMode( KFileDialog::Opening );
    dlg.setCaption( i18n("Open") );
    dlg.setMode( KFile::File );

    KImageFilePreview *ip = new KImageFilePreview( &dlg );
    dlg.setPreviewWidget( ip );
    dlg.exec();

    QString file = dlg.selectedFile();
    if (!file.isEmpty())
    {
        d->custom = file;
        if ( mpRb1->isChecked() )
            d->customLocation = QFileInfo( file ).absolutePath();
        slotOk();
    }
}

void KIconDialog::slotSystemIconClicked()
{
    mpBrowseBut->setEnabled(false);
    mpCombo->setEnabled(true);
    showIcons();
}

void KIconDialog::slotOtherIconClicked()
{
    mpBrowseBut->setEnabled( !d->m_bLockCustomDir );
    mpCombo->setEnabled(false);
    showIcons();
}

void KIconDialog::slotContext(int id)
{
    mContext = static_cast<K3Icon::Context>( mContextMap[ id ] );
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

KIconButton::KIconButton(QWidget *parent)
    : QPushButton(parent),d(new KIconButtonPrivate)
{
    init( KGlobal::iconLoader() );
}

KIconButton::KIconButton(KIconLoader *loader, QWidget *parent)
    : QPushButton(parent),d(new KIconButtonPrivate)
{
    init( loader );
}

void KIconButton::init( KIconLoader *loader )
{
    mGroup = K3Icon::Desktop;
    mContext = K3Icon::Application;
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

void KIconButton::setIconType(K3Icon::Group group, K3Icon::Context context, bool user)
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

#include "kicondialog.moc"
