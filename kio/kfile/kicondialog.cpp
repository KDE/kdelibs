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

#include <kio/kio_export.h>

#include "kicondialog.h"
#include <kbuttongroup.h>
#include <klistwidgetsearchline.h>
#include <assert.h>

#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kimagefilepreview.h>

#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <qradiobutton.h>
#include <qfileinfo.h>
#include <qtoolbutton.h>
#include <qprogressbar.h>
#include <ksvgrenderer.h>

#include <QImage>
#include <QPainter>

class KIconCanvas::KIconCanvasPrivate
{
  public:
    KIconCanvasPrivate(KIconCanvas *qq) { q = qq; m_bLoading = false; }
    ~KIconCanvasPrivate() {}
    KIconCanvas *q;
    bool m_bLoading;
    QStringList mFiles;
    QTimer *mpTimer;

    // slots
    void _k_slotLoadFiles();
    void _k_slotCurrentChanged(QListWidgetItem *item);
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
    : KListWidget(parent), d(new KIconCanvasPrivate(this))
{
    setViewMode(IconMode);
    setUniformItemSizes(true);
    setMovement(Static);
    setIconSize(QSize(60, 60));
    d->mpTimer = new QTimer(this);
    connect(d->mpTimer, SIGNAL(timeout()), this, SLOT(_k_slotLoadFiles()));
    connect(this, SIGNAL( currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(_k_slotCurrentChanged(QListWidgetItem *)));
    setGridSize(QSize(80,80));
}

KIconCanvas::~KIconCanvas()
{
    delete d->mpTimer;
    delete d;
}

void KIconCanvas::loadFiles(const QStringList& files)
{
    clear();
    d->mFiles = files;
    emit startLoading(d->mFiles.count());
    d->mpTimer->setSingleShot(true);
    d->mpTimer->start(10); // #86680
    d->m_bLoading = false;
}

void KIconCanvas::KIconCanvasPrivate::_k_slotLoadFiles()
{
    q->setResizeMode(QListWidget::Fixed);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // disable updates to not trigger paint events when adding child items
    q->setUpdatesEnabled(false);

    m_bLoading = true;
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
            emit q->progress(i);
            emitProgress = 0;
        }

        emitProgress++;
//	kapp->processEvents();
        if (!m_bLoading) { // user clicked on a button that will load another set of icons
            break;
        }
	QImage img;

	// Use the extension as the format. Works for XPM and PNG, but not for SVG
	QString path= *it;
	QString ext = path.right(3).toUpper();

	if (ext != "SVG" && ext != "VGZ")
	    img.load(*it);
	else {
            // Special stuff for SVG icons
            img = QImage(60, 60, QImage::Format_ARGB32_Premultiplied);
            img.fill(0);
            KSvgRenderer renderer(*it);
            if (renderer.isValid()) {
                QPainter p(&img);
                renderer.render(&p);
            }
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
	QListWidgetItem *item = new QListWidgetItem(pm, fi.baseName(), q);
	item->setData(Qt::UserRole, *it);
        item->setToolTip(fi.baseName());
    }

    // enable updates since we have to draw the whole view now
    q->setUpdatesEnabled(true);

    QApplication::restoreOverrideCursor();
    m_bLoading = false;
    emit q->finished();
    q->setResizeMode(QListWidget::Adjust);
}

QString KIconCanvas::getCurrent() const
{
    if (!currentItem())
	return QString();
    return currentItem()->data(Qt::UserRole).toString();
}

void KIconCanvas::stopLoading()
{
    d->m_bLoading = false;
}

void KIconCanvas::KIconCanvasPrivate::_k_slotCurrentChanged(QListWidgetItem *item)
{
    emit q->nameChanged((item != 0L) ? item->text() : QString());
}

class KIconDialog::KIconDialogPrivate
{
  public:
    KIconDialogPrivate(KIconDialog *qq) {
        q = qq;
        m_bStrictIconSize = true;
	m_bLockUser = false;
	m_bLockCustomDir = false;
	searchLine = 0;
    }
    ~KIconDialogPrivate() {}

    void init();
    void showIcons();
    void setContext( K3Icon::Context context );

    // slots
    void _k_slotContext(int);
    void _k_slotStartLoading(int);
    void _k_slotProgress(int);
    void _k_slotFinished();
    void _k_slotAcceptIcons();
    void _k_slotBrowse();
    void _k_slotOtherIconClicked();
    void _k_slotSystemIconClicked();

    KIconDialog *q;

    int mGroupOrSize;
    K3Icon::Context mContext;

    QStringList mFileList;
    QComboBox *mpCombo;
    QPushButton *mpBrowseBut;
    QRadioButton *mpRb1, *mpRb2;
    QProgressBar *mpProgress;
    KIconLoader *mpLoader;
    KIconCanvas *mpCanvas;
    int mNumContext;
    K3Icon::Context mContextMap[ 12 ]; // must match KIcon::Context size, code has assert

    bool m_bStrictIconSize, m_bLockUser, m_bLockCustomDir;
    QString custom;
    QString customLocation;
    KListWidgetSearchLine *searchLine;
};

/*
 * KIconDialog: Dialog for selecting icons. Both system and user
 * specified icons can be chosen.
 */

KIconDialog::KIconDialog(QWidget *parent)
    : KDialog(parent), d(new KIconDialogPrivate(this))
{
    setObjectName( "icondialog" );
    setModal( true );
    setCaption( i18n("Select Icon") );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );

    d->mpLoader = KIconLoader::global();
    d->init();
}

KIconDialog::KIconDialog(KIconLoader *loader, QWidget *parent)
    : KDialog(parent), d(new KIconDialogPrivate(this))
{
    setObjectName( "icondialog" );
    setModal( true );
    setCaption( i18n("Select Icon") );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );

    d->mpLoader = loader;
    d->init();
}

void KIconDialog::KIconDialogPrivate::init()
{
    mGroupOrSize = K3Icon::Desktop;
    mContext = K3Icon::Any;
    mFileList = KGlobal::dirs()->findAllResources("appicon", QLatin1String("*.png"));

    QWidget *main = new QWidget(q);
    q->setMainWidget(main);

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
    connect(mpRb1, SIGNAL(clicked()), q, SLOT(_k_slotSystemIconClicked()));
    grid->addWidget(mpRb1, 1, 0);
    mpCombo = new QComboBox(bgroup);
    connect(mpCombo, SIGNAL(activated(int)), q, SLOT(_k_slotContext(int)));
    grid->addWidget(mpCombo, 1, 1);
    mpRb2 = new QRadioButton(i18n("O&ther icons:"), bgroup);
    connect(mpRb2, SIGNAL(clicked()), q, SLOT(_k_slotOtherIconClicked()));
    grid->addWidget(mpRb2, 2, 0);
    mpBrowseBut = new QPushButton(i18n("&Browse..."), bgroup);
    connect(mpBrowseBut, SIGNAL(clicked()), q, SLOT(_k_slotBrowse()));
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

    searchLine = new KListWidgetSearchLine(main);
    searchLayout->addWidget(searchLine);
    searchLabel->setBuddy(searchLine);

    QString wtstr = i18n("Search interactively for icon names (e.g. folder).");
    searchLabel->setWhatsThis(wtstr);
    searchLine->setWhatsThis(wtstr);


    mpCanvas = new KIconCanvas(main);
    connect(mpCanvas, SIGNAL(itemActivated(QListWidgetItem *)), q, SLOT(_k_slotAcceptIcons()));
    mpCanvas->setMinimumSize(400, 125);
    top->addWidget(mpCanvas);
    searchLine->setListWidget(mpCanvas);

    mpProgress = new QProgressBar(main);
    top->addWidget(mpProgress);
    connect(mpCanvas, SIGNAL(startLoading(int)), q, SLOT(_k_slotStartLoading(int)));
    connect(mpCanvas, SIGNAL(progress(int)), q, SLOT(_k_slotProgress(int)));
    connect(mpCanvas, SIGNAL(finished()), q, SLOT(_k_slotFinished()));

    // When pressing Ok or Cancel, stop loading icons
    connect(q, SIGNAL(hidden()), mpCanvas, SLOT(stopLoading()));

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
    q->incrementInitialSize(QSize(0,100));
    connect(q, SIGNAL(okClicked()), q, SLOT(slotOk()));
}


KIconDialog::~KIconDialog()
{
    delete d;
}

void KIconDialog::KIconDialogPrivate::_k_slotAcceptIcons()
{
    custom.clear();
    q->slotOk();
}

void KIconDialog::KIconDialogPrivate::showIcons()
{
    mpCanvas->clear();
    QStringList filelist;
    if (mpRb1->isChecked())
        if (m_bStrictIconSize)
            filelist=mpLoader->queryIcons(mGroupOrSize, mContext);
        else
            filelist=mpLoader->queryIconsByContext(mGroupOrSize, mContext);
    else if (!customLocation.isNull()) {
        filelist = mpLoader->queryIconsByDir(customLocation);
    }
    else
	filelist=mFileList;

    QList<IconPath> iconlist;
    QStringList::Iterator it;
    foreach (const QString &it, filelist) {
       iconlist.append(IconPath(it));
    }

    qSort(iconlist);
    filelist.clear();

    foreach (const IconPath &ip, iconlist) {
       filelist.append(ip);
    }

    searchLine->clear();
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
    if (size == 0) {
        d->mGroupOrSize = K3Icon::Desktop; // default Group
    } else {
        d->mGroupOrSize = -size; // yes, KIconLoader::queryIconsByContext is weird
    }
}

int KIconDialog::iconSize() const
{
    // 0 or any other value ==> mGroupOrSize is a group, so we return 0
    return (d->mGroupOrSize < 0) ? -d->mGroupOrSize : 0;
}

void KIconDialog::setup(K3Icon::Group group, K3Icon::Context context,
                        bool strictIconSize, int iconSize, bool user )
{
    d->m_bStrictIconSize = strictIconSize;
    d->mGroupOrSize = (iconSize == 0) ? group : -iconSize;
    d->mpRb1->setChecked(!user);
    d->mpRb2->setChecked(user);
    d->mpCombo->setEnabled(!user);
    d->mpBrowseBut->setEnabled(user);
    d->mContext = context;
    d->setContext(context);
}

void KIconDialog::setup(K3Icon::Group group, K3Icon::Context context,
                        bool strictIconSize, int iconSize, bool user,
                        bool lockUser, bool lockCustomDir )
{
    d->m_bStrictIconSize = strictIconSize;
    d->m_bLockUser = lockUser;
    d->m_bLockCustomDir = lockCustomDir;
    d->mGroupOrSize = (iconSize == 0) ? group : -iconSize;
    d->mpRb1->setChecked(!user);
    d->mpRb1->setEnabled(!lockUser || !user);
    d->mpRb2->setChecked(user);
    d->mpRb2->setEnabled(!lockUser || user);
    d->mpCombo->setEnabled(!user);
    d->mpBrowseBut->setEnabled(user && !lockCustomDir);
    d->setContext(context);
}

void KIconDialog::KIconDialogPrivate::setContext(K3Icon::Context context)
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
    d->showIcons();

    if ( exec() == Accepted )
    {
        if (!d->custom.isNull())
            return d->custom;
        QString name = d->mpCanvas->getCurrent();
        if (name.isEmpty() || (d->mpRb2->isChecked())) {
            return name;
        }
	QFileInfo fi(name);
	return fi.baseName();
    }
    return QString();
}

void KIconDialog::showDialog()
{
    setModal(false);
    d->showIcons();
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
        name = d->mpCanvas->getCurrent();
        if (!name.isEmpty() && (d->mpRb2->isChecked())) {
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

void KIconDialog::KIconDialogPrivate::_k_slotBrowse()
{
    // Create a file dialog to select a PNG, XPM or SVG file,
    // with the image previewer shown.
    // KFileDialog::getImageOpenURL doesn't allow svg.
    KUrl emptyUrl;
    KFileDialog dlg(emptyUrl, i18n("*.png *.xpm *.svg *.svgz|Icon Files (*.png *.xpm *.svg *.svgz)"), q);
    dlg.setOperationMode( KFileDialog::Opening );
    dlg.setCaption( i18n("Open") );
    dlg.setMode( KFile::File );

    KImageFilePreview *ip = new KImageFilePreview( &dlg );
    dlg.setPreviewWidget( ip );
    dlg.exec();

    QString file = dlg.selectedFile();
    if (!file.isEmpty())
    {
        custom = file;
        if (mpRb1->isChecked()) {
            customLocation = QFileInfo(file).absolutePath();
        }
        q->slotOk();
    }
}

void KIconDialog::KIconDialogPrivate::_k_slotSystemIconClicked()
{
    mpBrowseBut->setEnabled(false);
    mpCombo->setEnabled(true);
    showIcons();
}

void KIconDialog::KIconDialogPrivate::_k_slotOtherIconClicked()
{
    mpBrowseBut->setEnabled(!m_bLockCustomDir);
    mpCombo->setEnabled(false);
    showIcons();
}

void KIconDialog::KIconDialogPrivate::_k_slotContext(int id)
{
    mContext = static_cast<K3Icon::Context>( mContextMap[ id ] );
    showIcons();
}

void KIconDialog::KIconDialogPrivate::_k_slotStartLoading(int steps)
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

void KIconDialog::KIconDialogPrivate::_k_slotProgress(int p)
{
    mpProgress->setValue(p);
    // commented out the following since setProgress already paints ther
    // progress bar. ->repaint() only makes it flicker
    //mpProgress->repaint();
}

void KIconDialog::KIconDialogPrivate::_k_slotFinished()
{
    mpProgress->hide();
}

class KIconButton::KIconButtonPrivate
{
  public:
    KIconButtonPrivate(KIconButton *qq, KIconLoader *loader);
    ~KIconButtonPrivate();

    // slots
    void _k_slotChangeIcon();
    void _k_newIconName(const QString&);

    KIconButton *q;

    bool m_bStrictIconSize;
    int iconSize;

    bool mbUser;
    K3Icon::Group mGroup;
    K3Icon::Context mContext;

    QString mIcon;
    KIconDialog *mpDialog;
    KIconLoader *mpLoader;
};


/*
 * KIconButton: A "choose icon" pushbutton.
 */

KIconButton::KIconButton(QWidget *parent)
    : QPushButton(parent), d(new KIconButtonPrivate(this, KIconLoader::global()))
{
}

KIconButton::KIconButton(KIconLoader *loader, QWidget *parent)
    : QPushButton(parent), d(new KIconButtonPrivate(this, loader))
{
}

KIconButton::KIconButtonPrivate::KIconButtonPrivate(KIconButton *qq, KIconLoader *loader)
    : q(qq)
{
    m_bStrictIconSize = false;
    iconSize = 0; // let KIconLoader choose the default

    mGroup = K3Icon::Desktop;
    mContext = K3Icon::Application;
    mbUser = false;

    mpLoader = loader;
    mpDialog = 0L;
    connect(q, SIGNAL(clicked()), q, SLOT(_k_slotChangeIcon()));
}

KIconButton::KIconButtonPrivate::~KIconButtonPrivate()
{
    delete mpDialog;
}

KIconButton::~KIconButton()
{
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
    QPushButton::setIconSize(QSize(size, size));
    d->iconSize = size;
}

int KIconButton::iconSize() const
{
    return d->iconSize;
}

void KIconButton::setIconType(K3Icon::Group group, K3Icon::Context context, bool user)
{
    d->mGroup = group;
    d->mContext = context;
    d->mbUser = user;
}

void KIconButton::setIcon(const QString& icon)
{
    d->mIcon = icon;
    setIcon(d->mpLoader->loadIconSet(d->mIcon, d->mGroup, d->iconSize));

    if (!d->mpDialog) {
        d->mpDialog = new KIconDialog(d->mpLoader, this);
        connect(d->mpDialog, SIGNAL(newIconName(const QString&)), this, SLOT(_k_newIconName(const QString&)));
    }

    if (d->mbUser) {
        d->mpDialog->setCustomLocation(QFileInfo(d->mpLoader->iconPath(d->mIcon, d->mGroup, true) ).absolutePath());
    }
}

void KIconButton::setIcon(const QIcon& icon)
{
    QPushButton::setIcon(icon);
}

void KIconButton::resetIcon()
{
    d->mIcon.clear();
    setIcon(QIcon());
}

const QString &KIconButton::icon() const
{
    return d->mIcon;
}

void KIconButton::KIconButtonPrivate::_k_slotChangeIcon()
{
    if (!mpDialog)
    {
        mpDialog = new KIconDialog(mpLoader, q);
        connect(mpDialog, SIGNAL(newIconName(const QString&)), q, SLOT(_k_newIconName(const QString&)));
    }

    mpDialog->setup(mGroup, mContext, m_bStrictIconSize, iconSize, mbUser);
    mpDialog->showDialog();
}

void KIconButton::KIconButtonPrivate::_k_newIconName(const QString& name)
{
    if (name.isEmpty())
        return;

    QIcon iconset = mpLoader->loadIconSet(name, mGroup, iconSize);
    q->setIcon(iconset);
    mIcon = name;

    if (mbUser) {
        mpDialog->setCustomLocation(QFileInfo(mpLoader->iconPath(mIcon, mGroup, true)).absolutePath());
    }

    emit q->iconChanged(name);
}

#include "kicondialog.moc"
