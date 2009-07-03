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

#include <kio/kio_export.h>

#include <kbuttongroup.h>
#include <kcombobox.h>
#include <klistwidgetsearchline.h>
#include <kapplication.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kimagefilepreview.h>
#include <ksvgrenderer.h>

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtCore/QTimer>
#include <QtGui/QRadioButton>
#include <QtCore/QFileInfo>
#include <QtGui/QProgressBar>
#include <QtGui/QPainter>
#include <QtGui/QScrollBar>


/**
 * Qt allocates very little horizontal space for the icon name,
 * even if the gridSize width is large.  This delegate allocates
 * the gridSize width (minus some padding) for the icon and icon name.
 */
class KIconCanvasDelegate : public QAbstractItemDelegate
{
public:
    KIconCanvasDelegate(KIconCanvas *parent, QAbstractItemDelegate *defaultDelegate);
    ~KIconCanvasDelegate() {};
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const; 
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
private:
    KIconCanvas *m_iconCanvas;
    QAbstractItemDelegate *m_defaultDelegate;
    static const int HORIZONTAL_EDGE_PAD = 3;
};

KIconCanvasDelegate::KIconCanvasDelegate(KIconCanvas *parent, QAbstractItemDelegate *defaultDelegate)
    : QAbstractItemDelegate(parent)
{
    m_iconCanvas = parent;
    m_defaultDelegate = defaultDelegate;
}

void KIconCanvasDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    const int GRID_WIDTH = m_iconCanvas->gridSize().width();
    QStyleOptionViewItem newOption = option;
    // Manipulate the width available.
    newOption.rect.setX((option.rect.x() / GRID_WIDTH) * GRID_WIDTH + HORIZONTAL_EDGE_PAD);
    newOption.rect.setWidth(GRID_WIDTH - 2 * HORIZONTAL_EDGE_PAD);

    m_defaultDelegate->paint(painter, newOption, index);
}

QSize KIconCanvasDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QSize size = m_defaultDelegate->sizeHint(option, index);
    const int GRID_WIDTH = m_iconCanvas->gridSize().width();
    size.setWidth(GRID_WIDTH - 2 * HORIZONTAL_EDGE_PAD);
    return size;
}

class KIconCanvas::KIconCanvasPrivate
{
  public:
    KIconCanvasPrivate(KIconCanvas *qq) { q = qq; m_bLoading = false; }
    ~KIconCanvasPrivate() {}
    KIconCanvas *q;
    bool m_bLoading;
    QStringList mFiles;
    QTimer *mpTimer;
    KIconCanvasDelegate *mpDelegate;

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
    setGridSize(QSize(100,80));

    d->mpDelegate = new KIconCanvasDelegate(this, itemDelegate());
    setItemDelegate(d->mpDelegate);
}

KIconCanvas::~KIconCanvas()
{
    delete d->mpTimer;
    delete d->mpDelegate;
    delete d;
}

void KIconCanvas::loadFiles(const QStringList& files)
{
    clear();
    d->mFiles = files;
    emit startLoading(d->mFiles.count());
    d->mpTimer->setSingleShot(true);
    d->mpTimer->start(10);
    d->m_bLoading = false;
}

void KIconCanvas::KIconCanvasPrivate::_k_slotLoadFiles()
{
    q->setResizeMode(QListWidget::Fixed);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // disable updates to not trigger paint events when adding child items,
    // but force an initial paint so that we do not get garbage
    q->repaint();
    q->setUpdatesEnabled(false);

    // Cache these as we will call them frequently.
    const int canvasIconWidth = q->iconSize().width();
    const int canvasIconHeight = q->iconSize().width();
    const bool uniformIconSize = q->uniformItemSizes();

    m_bLoading = true;
    int i;
    QStringList::ConstIterator it;
    uint emitProgress = 10; // so we will emit it once in the beginning
    QStringList::ConstIterator end(mFiles.constEnd());
    for (it=mFiles.constBegin(), i=0; it!=end; ++it, i++)
    {
	if ( emitProgress >= 10 ) {
            emit q->progress(i);
            emitProgress = 0;
        }

        emitProgress++;

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
            img = QImage(canvasIconWidth, canvasIconHeight, QImage::Format_ARGB32_Premultiplied);
            img.fill(0);
            KSvgRenderer renderer(*it);
            if (renderer.isValid()) {
                QPainter p(&img);
                renderer.render(&p);
            }
        }

	if (img.isNull())
	    continue;
	if (img.width() > canvasIconWidth || img.height() > canvasIconHeight)
	{
	    if (img.width() / (float)canvasIconWidth  > img.height() / (float)canvasIconHeight)
	    {
		int height = (int) (((float)canvasIconWidth / img.width()) * img.height());
		img = img.scaled(canvasIconWidth, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	    } else
	    {
		int width = (int) (((float)canvasIconHeight / img.height()) * img.width());
		img = img.scaled(width, canvasIconHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	    }
	}
	
	if (uniformIconSize && (img.width() != canvasIconWidth || img.height() != canvasIconHeight))
	{
	   // Image is smaller than desired.  Draw onto a transparent QImage of the required dimensions.
	   // (Unpleasant glitches occur if we break the uniformIconSizes() contract).
	   QImage paddedImage = QImage(canvasIconWidth, canvasIconHeight, QImage::Format_ARGB32_Premultiplied);
	   paddedImage.fill(0);
	   QPainter painter(&paddedImage);
	   painter.drawImage( (canvasIconWidth - img.width()) / 2, (canvasIconHeight - img.height()) / 2, img);
	   img = paddedImage;
	}
	
	QPixmap pm = QPixmap::fromImage(img);
	QFileInfo fi(*it);
        QListWidgetItem *item = new QListWidgetItem(pm, fi.completeBaseName(), q);
	item->setData(Qt::UserRole, *it);
        item->setToolTip(fi.completeBaseName());
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
        mNumOfSteps = 1;
    }
    ~KIconDialogPrivate() {}

    void init();
    void showIcons();
    void setContext( KIconLoader::Context context );

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
    KIconLoader::Context mContext;

    QStringList mFileList;
    KComboBox *mpCombo;
    QPushButton *mpBrowseBut;
    QRadioButton *mpSystemIcons, *mpOtherIcons;
    QProgressBar *mpProgress;
    int mNumOfSteps;
    KIconLoader *mpLoader;
    KIconCanvas *mpCanvas;
    int mNumContext;
    KIconLoader::Context mContextMap[ 12 ]; // must match KIcon::Context size, code has assert

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
    setModal( true );
    setCaption( i18n("Select Icon") );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );

    d->mpLoader = loader;
    d->init();
}

void KIconDialog::KIconDialogPrivate::init()
{
    mGroupOrSize = KIconLoader::Desktop;
    mContext = KIconLoader::Any;
    mFileList = KGlobal::dirs()->findAllResources("appicon", QLatin1String("*.png"));

    QWidget *main = new QWidget(q);
    q->setMainWidget(main);

    QVBoxLayout *top = new QVBoxLayout(main);
    top->setMargin(0);

    QGroupBox *bgroup = new QGroupBox(main);
    bgroup->setTitle(i18n("Icon Source"));

    QVBoxLayout *vbox = new QVBoxLayout;
    bgroup->setLayout( vbox );
    top->addWidget(bgroup);

    QGridLayout *grid = new QGridLayout();
    vbox->addLayout(grid);

    mpSystemIcons = new QRadioButton(i18n("S&ystem icons:"), bgroup);
    connect(mpSystemIcons, SIGNAL(clicked()), q, SLOT(_k_slotSystemIconClicked()));
    grid->addWidget(mpSystemIcons, 1, 0);
    mpCombo = new KComboBox(bgroup);
    mpCombo->setMaxVisibleItems(12);
    connect(mpCombo, SIGNAL(activated(int)), q, SLOT(_k_slotContext(int)));
    grid->addWidget(mpCombo, 1, 1);
    mpOtherIcons = new QRadioButton(i18n("O&ther icons:"), bgroup);
    connect(mpOtherIcons, SIGNAL(clicked()), q, SLOT(_k_slotOtherIconClicked()));
    grid->addWidget(mpOtherIcons, 2, 0);
    mpBrowseBut = new QPushButton(i18n("&Browse..."), bgroup);
    connect(mpBrowseBut, SIGNAL(clicked()), q, SLOT(_k_slotBrowse()));
    grid->addWidget(mpBrowseBut, 2, 1);

    //
    // ADD SEARCHLINE
    //
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setMargin(0);
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
    top->addWidget(mpCanvas);
    searchLine->setListWidget(mpCanvas);

    // Compute width of canvas with 4 icons displayed in a row
    QStyleOption opt;
    opt.initFrom(mpCanvas);
    int width = 4 * mpCanvas->gridSize().width() + 1;
    width += mpCanvas->verticalScrollBar()->sizeHint().width();
    width += 2 * mpCanvas->frameWidth();
    if (mpCanvas->style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents, &opt, mpCanvas)) {
        width += mpCanvas->style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarSpacing, &opt, mpCanvas);
    }
    mpCanvas->setMinimumSize(width, 125);

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
    static const KIconLoader::Context context_id[] = {
        KIconLoader::Action,
        KIconLoader::Animation,
        KIconLoader::Application,
        KIconLoader::Category,
        KIconLoader::Device,
        KIconLoader::Emblem,
        KIconLoader::Emote,
        KIconLoader::FileSystem,
        KIconLoader::International,
        KIconLoader::MimeType,
        KIconLoader::Place,
        KIconLoader::StatusIcon };
    mNumContext = 0;
    int cnt = sizeof( context_text ) / sizeof( context_text[ 0 ] );
    // check all 3 arrays have same sizes
    Q_ASSERT( cnt == sizeof( context_id ) / sizeof( context_id[ 0 ] )
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
    if (mpSystemIcons->isChecked())
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
    QStringList::const_iterator it;
    foreach (const QString &it, filelist) {
       iconlist.append(IconPath(it));
    }

    qSort(iconlist);
    filelist.clear();

    foreach (const IconPath &ip, iconlist) {
       filelist.append(ip);
    }

    searchLine->clear();

    // The KIconCanvas has uniformItemSizes set which really expects
    // all added icons to be the same size, otherwise weirdness ensues :)
    // Ensure all SVGs are scaled to the desired size and that as few icons
    // need to be padded as possible by specifying a sensible size.
    if (mGroupOrSize < -1) // mGroupOrSize can be -1 if NoGroup is chosen.
    {
        // Explicit size.
        mpCanvas->setIconSize(QSize(-mGroupOrSize, -mGroupOrSize));
    }
    else
    {
        // Icon group.
        int groupSize = mpLoader->currentSize((KIconLoader::Group)mGroupOrSize);
        mpCanvas->setIconSize(QSize(groupSize, groupSize));
    }

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
        d->mGroupOrSize = KIconLoader::Desktop; // default Group
    } else {
        d->mGroupOrSize = -size; // yes, KIconLoader::queryIconsByContext is weird
    }
}

int KIconDialog::iconSize() const
{
    // 0 or any other value ==> mGroupOrSize is a group, so we return 0
    return (d->mGroupOrSize < 0) ? -d->mGroupOrSize : 0;
}

void KIconDialog::setup(KIconLoader::Group group, KIconLoader::Context context,
                        bool strictIconSize, int iconSize, bool user,
                        bool lockUser, bool lockCustomDir )
{
    d->m_bStrictIconSize = strictIconSize;
    d->m_bLockUser = lockUser;
    d->m_bLockCustomDir = lockCustomDir;
    if (iconSize == 0)
    {
        if (group == KIconLoader::NoGroup)
        {
            // NoGroup has numeric value -1, which should
            // not really be used with KIconLoader::queryIcons*(...);
            // pick a proper group.
            d->mGroupOrSize = KIconLoader::Small;
        }
        else
        {
            d->mGroupOrSize = group;
        }
    }
    else
    {
        d->mGroupOrSize = -iconSize;
    }
    
    d->mpSystemIcons->setChecked(!user);
    d->mpSystemIcons->setEnabled(!lockUser || !user);
    d->mpOtherIcons->setChecked(user);
    d->mpOtherIcons->setEnabled(!lockUser || user);
    d->mpCombo->setEnabled(!user);
    d->mpBrowseBut->setEnabled(user && !lockCustomDir);
    d->setContext(context);
}

void KIconDialog::KIconDialogPrivate::setContext(KIconLoader::Context context)
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
    d->searchLine->setFocus();

    if ( exec() == Accepted )
    {
        if (!d->custom.isNull())
            return d->custom;
        QString name = d->mpCanvas->getCurrent();
        if (name.isEmpty() || d->mpOtherIcons->isChecked()) {
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
    d->searchLine->setFocus();
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
        if (!name.isEmpty() && d->mpSystemIcons->isChecked()) {
            QFileInfo fi(name);
            name = fi.baseName();
        }
    }

    emit newIconName(name);
    KDialog::accept();
}

QString KIconDialog::getIcon(KIconLoader::Group group, KIconLoader::Context context,
                             bool strictIconSize, int iconSize, bool user,
                             QWidget *parent, const QString &caption)
{
    KIconDialog dlg(parent);
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
        if (mpSystemIcons->isChecked()) {
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
    mContext = static_cast<KIconLoader::Context>( mContextMap[ id ] );
    showIcons();
}

void KIconDialog::KIconDialogPrivate::_k_slotStartLoading(int steps)
{
    if (steps < 10)
	mpProgress->hide();
    else
    {
        mNumOfSteps = steps;
        mpProgress->setValue(0);
        mpProgress->show();
    }
}

void KIconDialog::KIconDialogPrivate::_k_slotProgress(int p)
{
    mpProgress->setValue(static_cast<int>(100.0 * (double)p / (double)mNumOfSteps));
}

void KIconDialog::KIconDialogPrivate::_k_slotFinished()
{
    mNumOfSteps = 1;
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

    int iconSize;
    int buttonIconSize;
    bool m_bStrictIconSize;

    bool mbUser;
    KIconLoader::Group mGroup;
    KIconLoader::Context mContext;

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
    QPushButton::setIconSize(QSize(48, 48));
}

KIconButton::KIconButton(KIconLoader *loader, QWidget *parent)
    : QPushButton(parent), d(new KIconButtonPrivate(this, loader))
{
    QPushButton::setIconSize(QSize(48, 48));
}

KIconButton::KIconButtonPrivate::KIconButtonPrivate(KIconButton *qq, KIconLoader *loader)
    : q(qq)
{
    m_bStrictIconSize = false;
    iconSize = 0; // let KIconLoader choose the default
    buttonIconSize = -1; //When buttonIconSize is -1, iconSize will be used for the button

    mGroup = KIconLoader::Desktop;
    mContext = KIconLoader::Application;
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
    if (d->buttonIconSize == -1) {
        QPushButton::setIconSize(QSize(size, size));
    }

    d->iconSize = size;
}

int KIconButton::iconSize() const
{
    return d->iconSize;
}

void KIconButton::setButtonIconSize( int size )
{
    QPushButton::setIconSize(QSize(size, size));
    d->buttonIconSize = size;
}

int KIconButton::buttonIconSize() const
{
    return QPushButton::iconSize().height();
}

void KIconButton::setIconType(KIconLoader::Group group, KIconLoader::Context context, bool user)
{
    d->mGroup = group;
    d->mContext = context;
    d->mbUser = user;
}

void KIconButton::setIcon(const QString& icon)
{
    d->mIcon = icon;
    setIcon(KIcon(d->mIcon));

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

    q->setIcon(KIcon(name));
    mIcon = name;

    if (mbUser) {
        mpDialog->setCustomLocation(QFileInfo(mpLoader->iconPath(mIcon, mGroup, true)).absolutePath());
    }

    emit q->iconChanged(name);
}

#include "kicondialog.moc"
