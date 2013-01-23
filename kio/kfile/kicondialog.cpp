/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kfile.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *           (C) 2000 Kurt Granroth <granroth@kde.org>
 *           (C) 1997 Christoph Neerfeld <chris@kde.org>
 *           (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
 *           (C) 2013 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#include "kicondialog.h"
//#include "zoomlevelinfo.h"

#include <kio/kio_export.h>

#include <klistwidgetsearchline.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kimagefilepreview.h>
#ifndef _WIN32_WCE
#include <ksvgrenderer.h>
#endif

#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtCore/QTimer>
#include <QtGui/QRadioButton>
#include <QtCore/QFileInfo>
#include <QtGui/QProgressBar>
#include <QtGui/QPainter>

#include <QFontMetrics>
#include <QScrollBar>

#define HORIZONTAL_EDGE_PAD 3

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
    //const int GRID_HEIGHT = m_iconCanvas->gridSize().height();
    QStyleOptionViewItem newOption = option;
    newOption.displayAlignment = Qt::AlignHCenter | Qt::AlignTop;
    // Manipulate the width available.
    newOption.rect.setX((option.rect.x() / GRID_WIDTH) * GRID_WIDTH + HORIZONTAL_EDGE_PAD);
    newOption.rect.setWidth(GRID_WIDTH - 2 * HORIZONTAL_EDGE_PAD);
    /*newOption.rect.setY((option.rect.y() / GRID_HEIGHT) * GRID_HEIGHT + HORIZONTAL_EDGE_PAD);
    newOption.rect.setHeight(GRID_HEIGHT - 2 * HORIZONTAL_EDGE_PAD);*/
    newOption.font = KGlobalSettings::smallestReadableFont();

    m_defaultDelegate->paint(painter, newOption, index);
}

QSize KIconCanvasDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QSize size = m_defaultDelegate->sizeHint(option, index);
    const int GRID_WIDTH = m_iconCanvas->gridSize().width();
    //const int GRID_HEIGHT = m_iconCanvas->gridSize().height();
    size.setWidth(GRID_WIDTH - 2 * HORIZONTAL_EDGE_PAD);
    //size.setHeight(GRID_HEIGHT - 2 * HORIZONTAL_EDGE_PAD);
    return size;
}

class KIconCanvas::KIconCanvasPrivate
{
  public:
    KIconCanvasPrivate(KIconCanvas *qq) { q = qq; m_isLoading = false; }
    ~KIconCanvasPrivate() {}
    KIconCanvas *q;
    bool m_isLoading;
    QStringList m_files;
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
    setTextElideMode(Qt::ElideNone);
    setWordWrap(true);
    const int desktopIconSize = IconSize(KIconLoader::Desktop);
    setIconSize(QSize(desktopIconSize, desktopIconSize));
    d->mpTimer = new QTimer(this);
    connect(d->mpTimer, SIGNAL(timeout()), this, SLOT(_k_slotLoadFiles()));
    connect(this, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(_k_slotCurrentChanged(QListWidgetItem*)));
    const int gridWidth  = qMax(100, (int)(desktopIconSize * 1.5));
    const int gridHeight = desktopIconSize + 3 * QFontMetrics(KGlobalSettings::smallestReadableFont()).height();
    setGridSize( QSize( gridWidth, gridHeight ) );

    d->mpDelegate = new KIconCanvasDelegate(this, itemDelegate());
    setItemDelegate(d->mpDelegate);
}

KIconCanvas::~KIconCanvas()
{
    delete d->mpTimer;
    delete d->mpDelegate;
    delete d;
}

QSize KIconCanvas::sizeHint() const
{
    return QSize(minimumSize().width(), 3 * gridSize().height());
}

void KIconCanvas::wheelEvent(QWheelEvent* event)
{
    // this is a workaround because scrolling by mouse wheel is broken in Qt list views for big items
    // https://bugreports.qt-project.org/browse/QTBUG-7232
    // Somehow cannot use gridSize() here, so duplicating code
    verticalScrollBar()->setSingleStep(20);
    //IconSize(KIconLoader::Desktop) + QFontMetrics(QWidget::font()).height() + 2 * HORIZONTAL_EDGE_PAD );
    KListWidget::wheelEvent(event);
}

void KIconCanvas::loadFiles(const QStringList& files)
{
    clear();
    d->m_files = files;
    emit startLoading(d->m_files.count());
    d->mpTimer->setSingleShot(true);
    d->mpTimer->start(10);
    d->m_isLoading = false;
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

    m_isLoading = true;
    int i;
    QStringList::ConstIterator it;
    uint emitProgress = 10; // so we will emit it once in the beginning
    QStringList::ConstIterator end(m_files.constEnd());
    for (it=m_files.constBegin(), i=0; it!=end; ++it, i++) {
        if ( emitProgress >= 10 ) {
            emit q->progress(i);
            emitProgress = 0;
        }

        emitProgress++;

        if (!m_isLoading) { // user clicked on a button that will load another set of icons
            break;
        }

        QImage img;

        // Use the extension as the format. Works for XPM and PNG, but not for SVG
        QString path= *it;
        QString ext = path.right(3).toUpper();

        if (ext != "SVG" && ext != "VGZ") {
            img.load(*it);
        } else {
#ifndef _WIN32_WCE
            // Special stuff for SVG icons
            img = QImage(canvasIconWidth, canvasIconHeight, QImage::Format_ARGB32_Premultiplied);
            img.fill(0);
            QSvgRenderer renderer(*it);
            if (renderer.isValid()) {
                QPainter p(&img);
                renderer.render(&p);
            }
#endif
        }

        if (img.isNull()) {
            continue;
        }

        if (img.width() > canvasIconWidth || img.height() > canvasIconHeight) {
            if (img.width() / (float)canvasIconWidth  > img.height() / (float)canvasIconHeight) {
                int height = (int) (((float)canvasIconWidth / img.width()) * img.height());
                img = img.scaled(canvasIconWidth, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            } else {
                int width = (int) (((float)canvasIconHeight / img.height()) * img.width());
                img = img.scaled(width, canvasIconHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }
        }

        if (uniformIconSize && (img.width() != canvasIconWidth || img.height() != canvasIconHeight)) {
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
    m_isLoading = false;
    emit q->finished();
    q->setResizeMode(QListWidget::Adjust);
}

QString KIconCanvas::getCurrent() const
{
    if (!currentItem()) {
      return QString();
    }
    return currentItem()->data(Qt::UserRole).toString();
}

void KIconCanvas::stopLoading()
{
    d->m_isLoading = false;
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
        m_strictIconSize = true;
        m_lockUser = false;
        m_lockCustomDir = false;
        mNumOfSteps = 1;
    }
    ~KIconDialogPrivate() {}

    void init();
    void showIcons();
    void setContext(KIconLoader::Context context);
    void setCustomIcon();

    // slots
    void _k_slotContext(int);
    void _k_slotStartLoading(int);
    void _k_slotProgress(int);
    void _k_slotFinished();
    void _k_slotAcceptIcons();
    void _k_slotBrowse();

    KIconDialog *q;

    int m_groupOrSize;
    KIconLoader::Context m_context;
    bool m_showOtherIcons;

    QComboBox *m_contextCombo;
    QSlider *m_sizeSlider;
    KListWidgetSearchLine *m_searchLine;

    QStringList m_fileList;
    QProgressBar *mpProgress;
    int mNumOfSteps;
    KIconLoader *mpLoader;
    KIconCanvas *mpCanvas;
    int m_contextCount;
    KIconLoader::Context m_contextMap[ 11 ]; // must match KIcon::Context size, code has assert

    bool m_strictIconSize;
    bool m_lockUser;
    bool m_lockCustomDir;
    QString custom;
    QString customLocation;
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
    setButtons( Ok | Cancel | Reset );

    // We'll abuse the Reset button as our Browse button
    setButtonText(Reset, i18n("Browse..."));
    setButtonIcon(Reset, KIcon("folder-open"));

    setDefaultButton( Ok );

    d->mpLoader = KIconLoader::global();
    d->init();
}

KIconDialog::KIconDialog(KIconLoader *loader, QWidget *parent)
    : KDialog(parent), d(new KIconDialogPrivate(this))
{
    setModal( true );
    setCaption( i18n("Select Icon") );
    setButtons( Ok | Cancel | Reset );

    // We'll abuse the Reset button as our Browse button
    setButtonText(Reset, i18n("Browse..."));
    setButtonIcon(Reset, KIcon("folder-open"));

    setDefaultButton( Ok );

    d->mpLoader = loader;
    d->init();
}

void KIconDialog::KIconDialogPrivate::init()
{
    m_groupOrSize = KIconLoader::Desktop;
    m_context = KIconLoader::Any;
    m_fileList = KGlobal::dirs()->findAllResources("appicon", QLatin1String("*.png"));

    QWidget *main = new QWidget(q);
    q->setMainWidget(main);

    QVBoxLayout *top = new QVBoxLayout(main);
    top->setMargin(0);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_contextCombo = new QComboBox(main);
    m_contextCombo->setAccessibleName(i18n("Icon Group"));
    m_contextCombo->setAccessibleDescription(i18nc("Description for the icon group combobox (accessibility)", "Choose which icon category to show."));
    m_contextCombo->setMaxVisibleItems(13);
    // In case the user is forced to use custom icon, just hide the combo
    if (m_showOtherIcons && m_lockUser) {
        m_contextCombo->hide();
    }
    connect(m_contextCombo, SIGNAL(activated(int)), q, SLOT(_k_slotContext(int)));
    buttonLayout->addWidget(m_contextCombo);

    // We'll abuse the Reset button as our Browse button
    connect(q, SIGNAL(resetClicked()), q, SLOT(_k_slotBrowse()));

    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonLayout->addItem(horizontalSpacer);

    m_sizeSlider = new QSlider(Qt::Horizontal, main);
    m_sizeSlider->setAccessibleName(i18n("Icon Size"));
    m_sizeSlider->setAccessibleDescription(i18nc("Description for icon size slider (accessibility)", "Sets the size in which the icons are displayed."));
    m_sizeSlider->setPageStep(1);
    //m_sizeSlider->setRange(ZoomLevelInfo::minimumLevel(), ZoomLevelInfo::maximumLevel());
    //connect(m_zoomSlider, SIGNAL(valueChanged(int)), this, SIGNAL(zoomLevelChanged(int)));
    //connect(m_zoomSlider, SIGNAL(sliderMoved(int)), this, SLOT(showZoomSliderToolTip(int)));
    buttonLayout->addWidget(m_sizeSlider);

    horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonLayout->addItem(horizontalSpacer);

    m_searchLine = new KListWidgetSearchLine(main);
    m_searchLine->setPlaceholderText(i18n("Search"));
    m_searchLine->setAccessibleName(i18n("Search icons"));
    m_searchLine->setAccessibleDescription(i18nc("Description for the search line edit (accessibility)", "Shows icons that match the term entered into the search box."));
    buttonLayout->addWidget(m_searchLine);

    top->addLayout(buttonLayout);

    // Add the canvas that shows the icons
    mpCanvas = new KIconCanvas(main);
    connect(mpCanvas, SIGNAL(itemActivated(QListWidgetItem*)), q, SLOT(_k_slotAcceptIcons()));
    top->addWidget(mpCanvas);

    // Compute minimum size of canvas with 6 icons in a row and 1 rows minimum
    QStyleOption opt;
    opt.initFrom(mpCanvas);
    int width = 6 * mpCanvas->gridSize().width();
    width += mpCanvas->frameWidth() * 2;
    width += mpCanvas->verticalScrollBar()->sizeHint().width();
    width += 2 * mpCanvas->frameWidth();
    if (mpCanvas->style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents, &opt, mpCanvas)) {
        width += mpCanvas->style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarSpacing, &opt, mpCanvas);
    }
    int height = mpCanvas->gridSize().height();
    height += 2 * mpCanvas->frameWidth();
    mpCanvas->setMinimumSize(width, height);
    m_searchLine->setListWidget(mpCanvas);

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
        I18N_NOOP( "International" ),
        I18N_NOOP( "Mimetypes" ),
        I18N_NOOP( "Places" ),
        I18N_NOOP( "Status" )
    };
    static const KIconLoader::Context context_id[] = {
        KIconLoader::Action,
        KIconLoader::Animation,
        KIconLoader::Application,
        KIconLoader::Category,
        KIconLoader::Device,
        KIconLoader::Emblem,
        KIconLoader::Emote,
        KIconLoader::International,
        KIconLoader::MimeType,
        KIconLoader::Place,
        KIconLoader::StatusIcon,
    };
    m_contextCount = 0;
    int cnt = sizeof( context_text ) / sizeof( context_text[ 0 ] );
    // check all 3 arrays have same sizes
    Q_ASSERT( cnt == sizeof( context_id ) / sizeof( context_id[ 0 ] )
            && cnt == sizeof( m_contextMap ) / sizeof( m_contextMap[ 0 ] ));
    m_contextCombo->addItem(i18n("All Icons"));
    m_contextCount++;
    for( int i = 0; i < cnt; ++i ) {
        if (mpLoader->hasContext( context_id[ i ] )) {
            m_contextCombo->addItem(i18n(context_text[i]));
            m_contextMap[ m_contextCount++ ] = context_id[ i ];
        }
    }
    if (!m_lockUser) {
      m_contextCombo->addItem(i18n("Other Icons"));

      if (m_showOtherIcons) {
        m_contextCombo->setCurrentIndex(m_contextCount);
      }
    }

    /*
    // We basically have the minimumHeight of the canvas which is 1 icon's height,
    // then we add another 2, so we get 3 rows of icons shown by default. Then we
    // add the height of the combobox twice (one for the combobox and one for the
    // dialog buttons, we're lazy here) and then we add the layout spacing 4x as
    // we have two "gaps" in the layout and a gab between window borders
    opt.initFrom(q);
    const int initialHeight = height + 2 * mpCanvas->gridSize().height()
                                     + 2 * ( m_contextCombo->size().height() + q->style()->pixelMetric(QStyle::PM_ComboBoxFrameWidth, &opt, q) * 2 )
                                     + q->style()->pixelMetric(QStyle::PM_LayoutTopMargin, &opt, q)
                                     + 2 * q->style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing, &opt, q)
                                     + q->style()->pixelMetric(QStyle::PM_LayoutBottomMargin, &opt, q);
    q->setInitialSize(QSize(width,initialHeight));*/
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
    QStringList fileList;
    if (!m_showOtherIcons) {
        if (m_strictIconSize) {
            fileList = mpLoader->queryIcons(m_groupOrSize, m_context);
        } else {
            fileList = mpLoader->queryIconsByContext(m_groupOrSize, m_context);
        }
    } else if (!customLocation.isNull()) {
        fileList = mpLoader->queryIconsByDir(customLocation);
    } else {
        fileList = m_fileList;
    }

    QList<IconPath> iconList;
    QStringList::const_iterator it;
    foreach (const QString &it, fileList) {
       iconList.append(IconPath(it));
    }

    qSort(iconList);
    fileList.clear();

    foreach (const IconPath &ip, iconList) {
       fileList.append(ip);
    }

    m_searchLine->clear();

    // The KIconCanvas has uniformItemSizes set which really expects
    // all added icons to be the same size, otherwise weirdness ensues :)
    // Ensure all SVGs are scaled to the desired size and that as few icons
    // need to be padded as possible by specifying a sensible size.
    if (m_groupOrSize < -1) { // m_groupOrSize can be -1 if NoGroup is chosen.
        // Explicit size.
        mpCanvas->setIconSize(QSize(-m_groupOrSize, -m_groupOrSize));
    } else {
        // Icon group.
        int groupSize = mpLoader->currentSize((KIconLoader::Group)m_groupOrSize);
        mpCanvas->setIconSize(QSize(groupSize, groupSize));
    }

    mpCanvas->loadFiles(fileList);
}

void KIconDialog::setStrictIconSize(bool b)
{
    d->m_strictIconSize = b;
}

bool KIconDialog::strictIconSize() const
{
    return d->m_strictIconSize;
}

void KIconDialog::setIconSize( int size )
{
    // see KIconLoader, if you think this is weird
    if (size == 0) {
        d->m_groupOrSize = KIconLoader::Desktop; // default Group
    } else {
        d->m_groupOrSize = -size; // yes, KIconLoader::queryIconsByContext is weird
    }
}

int KIconDialog::iconSize() const
{
    // 0 or any other value ==> m_groupOrSize is a group, so we return 0
    return (d->m_groupOrSize < 0) ? -d->m_groupOrSize : 0;
}

void KIconDialog::setup(KIconLoader::Group group, KIconLoader::Context context,
                        bool strictIconSize, int iconSize, bool user,
                        bool lockUser, bool lockCustomDir )
{
    d->m_strictIconSize = strictIconSize;
    d->m_lockUser = lockUser;
    d->m_lockCustomDir = lockCustomDir;
    d->m_showOtherIcons = user;

    if (iconSize == 0) {
        if (group == KIconLoader::NoGroup) {
            // NoGroup has numeric value -1, which should
            // not really be used with KIconLoader::queryIcons*(...);
            // pick a proper group.
            d->m_groupOrSize = KIconLoader::Small;
        } else {
            d->m_groupOrSize = group;
        }
    } else {
        d->m_groupOrSize = -iconSize;
    }

    d->setContext(context);

    // We need to remove the Browse button afterwards if custom dir is locked
    if (lockCustomDir) {
        setButtons( Ok | Cancel);
    }

}

void KIconDialog::KIconDialogPrivate::setContext(KIconLoader::Context context)
{
    m_showOtherIcons = false;
    m_context = context;
    for( int i = 0; i < m_contextCount; ++i ) {
        if (m_contextMap[ i ] == context) {
            m_contextCombo->setCurrentIndex(i);
            return;
        }
    }
}

void KIconDialog::setCustomLocation(const QString& location)
{
    d->customLocation = location;
    d->m_showOtherIcons = true;
}

QString KIconDialog::openDialog()
{
    d->showIcons();
    d->m_searchLine->setFocus();

    if ( exec() == Accepted ) {
        if (!d->custom.isNull()) {
            return d->custom;
        }
        QString name = d->mpCanvas->getCurrent();
        if (name.isEmpty() || d->m_showOtherIcons) {
            return name;
        }
        QFileInfo fi(name);
        return fi.completeBaseName();
    }
    return QString();
}

void KIconDialog::showDialog()
{
    setModal(false);
    d->showIcons();
    d->m_searchLine->setFocus();
    show();
}

void KIconDialog::slotOk()
{
    QString name;
    if (!d->custom.isNull()) {
        name = d->custom;
    } else {
        name = d->mpCanvas->getCurrent();
        if (!name.isEmpty() && !d->m_showOtherIcons) {
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
    if (!caption.isNull()) {
        dlg.setCaption(caption);
    }

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
    if (!file.isEmpty()) {
        custom = file;
        m_contextCombo->setCurrentIndex(m_contextCount);
        customLocation = QFileInfo(file).absolutePath();
        q->slotOk();
    }
}

void KIconDialog::KIconDialogPrivate::_k_slotContext(int id)
{
    if (id == m_contextCount) {
      m_showOtherIcons = true;
    } else {
      m_showOtherIcons = false;
      m_context = static_cast<KIconLoader::Context>( m_contextMap[ id ] );
    }
    showIcons();
}

void KIconDialog::KIconDialogPrivate::_k_slotStartLoading(int steps)
{
    if (steps < 10) {
      mpProgress->hide();
    } else {
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
    bool m_strictIconSize;

    bool mbUser;
    KIconLoader::Group mGroup;
    KIconLoader::Context m_context;

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
    QPushButton::setIconSize(QSize(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop)));
}

KIconButton::KIconButton(KIconLoader *loader, QWidget *parent)
    : QPushButton(parent), d(new KIconButtonPrivate(this, loader))
{
    QPushButton::setIconSize(QSize(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop)));
}

KIconButton::KIconButtonPrivate::KIconButtonPrivate(KIconButton *qq, KIconLoader *loader)
    : q(qq)
{
    m_strictIconSize = false;
    iconSize = 0; // let KIconLoader choose the default
    buttonIconSize = -1; //When buttonIconSize is -1, iconSize will be used for the button

    mGroup = KIconLoader::Desktop;
    m_context = KIconLoader::Application;
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
    d->m_strictIconSize = b;
}

bool KIconButton::strictIconSize() const
{
    return d->m_strictIconSize;
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
    d->m_context = context;
    d->mbUser = user;
}

void KIconButton::setIcon(const QString& icon)
{
    d->mIcon = icon;
    setIcon(KIcon(d->mIcon));

    if (!d->mpDialog) {
        d->mpDialog = new KIconDialog(d->mpLoader, this);
        connect(d->mpDialog, SIGNAL(newIconName(QString)), this, SLOT(_k_newIconName(QString)));
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
    if (!mpDialog) {
        mpDialog = new KIconDialog(mpLoader, q);
        connect(mpDialog, SIGNAL(newIconName(QString)), q, SLOT(_k_newIconName(QString)));
    }

    mpDialog->setup(mGroup, m_context, m_strictIconSize, iconSize, mbUser);
    mpDialog->showDialog();
}

void KIconButton::KIconButtonPrivate::_k_newIconName(const QString& name)
{
    if (name.isEmpty()) {
        return;
    }

    q->setIcon(KIcon(name));
    mIcon = name;

    if (mbUser) {
        mpDialog->setCustomLocation(QFileInfo(mpLoader->iconPath(mIcon, mGroup, true)).absolutePath());
    }

    emit q->iconChanged(name);
}

#include "kicondialog.moc"
