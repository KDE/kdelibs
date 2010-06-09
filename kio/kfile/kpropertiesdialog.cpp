/* This file is part of the KDE project

   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (c) 1999, 2000 Preston Brown <pbrown@kde.org>
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (c) 2000 David Faure <faure@kde.org>
   Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/*
 * kpropertiesdialog.cpp
 * View/Edit Properties of files, locally or remotely
 *
 * some FilePermissionsPropsPlugin-changes by
 *  Henner Zeller <zeller@think.de>
 * some layout management by
 *  Bertrand Leconte <B.Leconte@mail.dotcom.fr>
 * the rest of the layout management, bug fixes, adaptation to libkio,
 * template feature by
 *  David Faure <faure@kde.org>
 * More layout, cleanups, and fixes by
 *  Preston Brown <pbrown@kde.org>
 * Plugin capability, cleanups and port to KDialog by
 *  Simon Hausmann <hausmann@kde.org>
 * KDesktopPropsPlugin by
 *  Waldo Bastian <bastian@kde.org>
 */

#include "kpropertiesdialog.h"
#include "kpropertiesdialog_p.h"


#include <config.h>
#include <config-acl.h>
extern "C" {
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
}
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <algorithm>
#include <functional>

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>
#include <QtCore/QMutableStringListIterator>
#include <QtCore/QTextIStream>
#include <QtGui/QPainter>
#include <QtGui/QLayout>
#include <QtGui/QStyle>
#include <QtGui/QProgressBar>
#include <QVector>
#include <QFileInfo>

#ifdef HAVE_POSIX_ACL
extern "C" {
#  include <sys/xattr.h>
}
#endif

#include <kauthorized.h>
#include <kdialog.h>
#include <kdirwatch.h>
#include <kdirnotify.h>
#include <kdiskfreespaceinfo.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kicondialog.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kjobuidelegate.h>
#include <kio/job.h>
#include <kio/copyjob.h>
#include <kio/chmodjob.h>
#include <kio/directorysizejob.h>
#include <kio/renamedialog.h>
#include <kio/netaccess.h>
#include <kio/jobuidelegate.h>
#include <kfiledialog.h>
#include <kmimetype.h>
#include <kmountpoint.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <kcombobox.h>
#include <kcompletion.h>
#include <klineedit.h>
#include <kseparator.h>
#include <ksqueezedtextlabel.h>
#include <klibloader.h>
#include <kmimetypetrader.h>
#include <kmetaprops.h>
#include <kpreviewprops.h>
#include <krun.h>
#include <kvbox.h>
#include <kacl.h>
#include <kconfiggroup.h>
#include <kshell.h>
#include <kcapacitybar.h>
#include <kfileitemlistproperties.h>

#ifndef Q_OS_WIN
#include "kfilesharedialog.h"
#endif

#include "ui_kpropertiesdesktopbase.h"
#include "ui_kpropertiesdesktopadvbase.h"
#ifdef HAVE_POSIX_ACL
#include "kacleditwidget.h"
#endif

#include <kbuildsycocaprogressdialog.h>
#include <kmimetypechooser.h>

#ifdef Q_WS_WIN
# include <kkernel_win.h>
#ifdef __GNUC__
# warning TODO: port completely to win32
#endif
#endif

using namespace KDEPrivate;

static QString nameFromFileName(QString nameStr)
{
    if ( nameStr.endsWith(QLatin1String(".desktop")) )
        nameStr.truncate( nameStr.length() - 8 );
    if ( nameStr.endsWith(QLatin1String(".kdelnk")) )
        nameStr.truncate( nameStr.length() - 7 );
    // Make it human-readable (%2F => '/', ...)
    nameStr = KIO::decodeFileName( nameStr );
    return nameStr;
}

mode_t KFilePermissionsPropsPlugin::fperm[3][4] = {
    {S_IRUSR, S_IWUSR, S_IXUSR, S_ISUID},
    {S_IRGRP, S_IWGRP, S_IXGRP, S_ISGID},
    {S_IROTH, S_IWOTH, S_IXOTH, S_ISVTX}
};

class KPropertiesDialog::KPropertiesDialogPrivate
{
public:
    KPropertiesDialogPrivate(KPropertiesDialog *qq)
    {
        q = qq;
        m_aborted = false;
        fileSharePage = 0;
    }
    ~KPropertiesDialogPrivate()
    {
    }

    /**
     * Common initialization for all constructors
     */
    void init();
    /**
     * Inserts all pages in the dialog.
     */
    void insertPages();

    KPropertiesDialog *q;
    bool m_aborted:1;
    QWidget* fileSharePage;
    /**
     * The URL of the props dialog (when shown for only one file)
     */
    KUrl m_singleUrl;
    /**
     * List of items this props dialog is shown for
     */
    KFileItemList m_items;
    /**
     * For templates
     */
    QString m_defaultName;
    KUrl m_currentDir;
    /**
     * List of all plugins inserted ( first one first )
     */
    QList<KPropertiesDialogPlugin*> m_pageList;
};

KPropertiesDialog::KPropertiesDialog (const KFileItem& item,
                                      QWidget* parent)
    : KPageDialog(parent), d(new KPropertiesDialogPrivate(this))
{
    setCaption( i18n( "Properties for %1" , KIO::decodeFileName(item.url().fileName())) );

    assert( !item.isNull() );
    d->m_items.append(item);

    d->m_singleUrl = item.url();
    assert(!d->m_singleUrl.isEmpty());

    d->init();
}

KPropertiesDialog::KPropertiesDialog (const QString& title,
                                      QWidget* parent)
    : KPageDialog(parent), d(new KPropertiesDialogPrivate(this))
{
    setCaption( i18n( "Properties for %1", title ) );

    d->init();
}

KPropertiesDialog::KPropertiesDialog(const KFileItemList& _items,
                                     QWidget* parent)
    : KPageDialog(parent), d(new KPropertiesDialogPrivate(this))
{
    if ( _items.count() > 1 )
        setCaption( i18np( "Properties for 1 item", "Properties for %1 Selected Items", _items.count() ) );
    else
        setCaption( i18n( "Properties for %1" , KIO::decodeFileName(_items.first().url().fileName())) );

    assert( !_items.isEmpty() );
    d->m_singleUrl = _items.first().url();
    assert(!d->m_singleUrl.isEmpty());

    d->m_items = _items;

    d->init();
}

KPropertiesDialog::KPropertiesDialog (const KUrl& _url,
                                      QWidget* parent)
    : KPageDialog(parent), d(new KPropertiesDialogPrivate(this))
{
    setCaption( i18n( "Properties for %1" , KIO::decodeFileName(_url.fileName()))  );

    d->m_singleUrl = _url;

    KIO::UDSEntry entry;
    KIO::NetAccess::stat(_url, entry, parent);

    d->m_items.append(KFileItem(entry, _url));
    d->init();
}

KPropertiesDialog::KPropertiesDialog (const KUrl& _tempUrl, const KUrl& _currentDir,
                                      const QString& _defaultName,
                                      QWidget* parent)
    : KPageDialog(parent), d(new KPropertiesDialogPrivate(this))
{
    setCaption( i18n( "Properties for %1" , KIO::decodeFileName(_tempUrl.fileName()))  );

    d->m_singleUrl = _tempUrl;
    d->m_defaultName = _defaultName;
    d->m_currentDir = _currentDir;
    assert(!d->m_singleUrl.isEmpty());

    // Create the KFileItem for the _template_ file, in order to read from it.
    d->m_items.append(KFileItem(KFileItem::Unknown, KFileItem::Unknown, d->m_singleUrl));
    d->init();
}

bool KPropertiesDialog::showDialog(const KFileItem& item, QWidget* parent,
                                   bool modal)
{
    // TODO: do we really want to show the win32 property dialog?
    // This means we lose metainfo, support for .desktop files, etc. (DF)
#ifdef Q_WS_WIN
    QString localPath = item.localPath();
    if (!localPath.isEmpty())
        return showWin32FilePropertyDialog(localPath);
#endif
    KPropertiesDialog* dlg = new KPropertiesDialog(item, parent);
    if (modal) {
        dlg->exec();
    } else {
        dlg->show();
    }

    return true;
}

bool KPropertiesDialog::showDialog(const KUrl& _url, QWidget* parent,
                                   bool modal)
{
#ifdef Q_WS_WIN
    if (_url.isLocalFile())
        return showWin32FilePropertyDialog( _url.toLocalFile() );
#endif
    KPropertiesDialog* dlg = new KPropertiesDialog(_url, parent);
    if (modal) {
        dlg->exec();
    } else {
        dlg->show();
    }

    return true;
}

bool KPropertiesDialog::showDialog(const KFileItemList& _items, QWidget* parent,
                                   bool modal)
{
    if (_items.count()==1) {
        const KFileItem item = _items.first();
        if (item.entry().count() == 0 && item.localPath().isEmpty()) // this remote item wasn't listed by a slave
            // Let's stat to get more info on the file
            return KPropertiesDialog::showDialog(item.url(), parent, modal);
        else
            return KPropertiesDialog::showDialog(_items.first(), parent, modal);
    }
    KPropertiesDialog* dlg = new KPropertiesDialog(_items, parent);
    if (modal) {
        dlg->exec();
    } else {
        dlg->show();
    }
    return true;
}

void KPropertiesDialog::KPropertiesDialogPrivate::init()
{
    q->setFaceType(KPageDialog::Tabbed);
    q->setButtons(KDialog::Ok | KDialog::Cancel);
    q->setDefaultButton(KDialog::Ok);

    connect(q, SIGNAL(okClicked()), q, SLOT(slotOk()));
    connect(q, SIGNAL(cancelClicked()), q, SLOT(slotCancel()));

    insertPages();

    KConfigGroup group(KGlobal::config(), "KPropertiesDialog");
    q->restoreDialogSize(group);
}

void KPropertiesDialog::showFileSharingPage()
{
    if (d->fileSharePage) {
        // FIXME: this showFileSharingPage thingy looks broken! (tokoe)
        // showPage( pageIndex( d->fileSharePage));
    }
}

void KPropertiesDialog::setFileSharingPage(QWidget* page) {
    d->fileSharePage = page;
}


void KPropertiesDialog::setFileNameReadOnly( bool ro )
{
    foreach(KPropertiesDialogPlugin *it, d->m_pageList) {
        KFilePropsPlugin* plugin = dynamic_cast<KFilePropsPlugin*>(it);
        if ( plugin ) {
            plugin->setFileNameReadOnly( ro );
            break;
        }
    }
}

KPropertiesDialog::~KPropertiesDialog()
{
    qDeleteAll(d->m_pageList);
    delete d;

    KConfigGroup group(KGlobal::config(), "KPropertiesDialog");
    saveDialogSize(group, KConfigBase::Persistent);
}

void KPropertiesDialog::insertPlugin (KPropertiesDialogPlugin* plugin)
{
    connect (plugin, SIGNAL (changed ()),
             plugin, SLOT (setDirty ()));

    d->m_pageList.append(plugin);
}

KUrl KPropertiesDialog::kurl() const
{
    return d->m_singleUrl;
}

KFileItem& KPropertiesDialog::item()
{
    return d->m_items.first();
}

KFileItemList KPropertiesDialog::items() const
{
    return d->m_items;
}

KUrl KPropertiesDialog::currentDir() const
{
    return d->m_currentDir;
}

QString KPropertiesDialog::defaultName() const
{
    return d->m_defaultName;
}

bool KPropertiesDialog::canDisplay( const KFileItemList& _items )
{
    // TODO: cache the result of those calls. Currently we parse .desktop files far too many times
    return KFilePropsPlugin::supports( _items ) ||
            KFilePermissionsPropsPlugin::supports( _items ) ||
            KDesktopPropsPlugin::supports( _items ) ||
            KUrlPropsPlugin::supports( _items ) ||
            KDevicePropsPlugin::supports( _items ) ||
            KFileMetaPropsPlugin::supports( _items ) ||
            KPreviewPropsPlugin::supports( _items );
}

void KPropertiesDialog::slotOk()
{
    QList<KPropertiesDialogPlugin*>::const_iterator pageListIt;
    d->m_aborted = false;

    KFilePropsPlugin * filePropsPlugin = qobject_cast<KFilePropsPlugin*>(d->m_pageList.first());

    // If any page is dirty, then set the main one (KFilePropsPlugin) as
    // dirty too. This is what makes it possible to save changes to a global
    // desktop file into a local one. In other cases, it doesn't hurt.
    for (pageListIt = d->m_pageList.constBegin(); pageListIt != d->m_pageList.constEnd(); ++pageListIt) {
        if ( (*pageListIt)->isDirty() && filePropsPlugin )
        {
            filePropsPlugin->setDirty();
            break;
        }
    }

    // Apply the changes in the _normal_ order of the tabs now
    // This is because in case of renaming a file, KFilePropsPlugin will call
    // KPropertiesDialog::rename, so other tab will be ok with whatever order
    // BUT for file copied from templates, we need to do the renaming first !
    for (pageListIt = d->m_pageList.constBegin(); pageListIt != d->m_pageList.constEnd() && !d->m_aborted; ++pageListIt) {
        if ( (*pageListIt)->isDirty() )
        {
            kDebug( 250 ) << "applying changes for " << (*pageListIt)->metaObject()->className();
            (*pageListIt)->applyChanges();
            // applyChanges may change d->m_aborted.
        }
        else {
            kDebug( 250 ) << "skipping page " << (*pageListIt)->metaObject()->className();
        }
    }

    if ( !d->m_aborted && filePropsPlugin )
        filePropsPlugin->postApplyChanges();

    if ( !d->m_aborted )
    {
        emit applied();
        emit propertiesClosed();
        deleteLater(); // somewhat like Qt::WA_DeleteOnClose would do.
        accept();
    } // else, keep dialog open for user to fix the problem.
}

void KPropertiesDialog::slotCancel()
{
    emit canceled();
    emit propertiesClosed();

    deleteLater();
    done( Rejected );
}

void KPropertiesDialog::KPropertiesDialogPrivate::insertPages()
{
    if (m_items.isEmpty())
        return;

    if ( KFilePropsPlugin::supports( m_items ) ) {
        KPropertiesDialogPlugin *p = new KFilePropsPlugin(q);
        q->insertPlugin(p);
    }

    if ( KFilePermissionsPropsPlugin::supports( m_items ) ) {
        KPropertiesDialogPlugin *p = new KFilePermissionsPropsPlugin(q);
        q->insertPlugin(p);
    }

    if ( KDesktopPropsPlugin::supports( m_items ) ) {
        KPropertiesDialogPlugin *p = new KDesktopPropsPlugin(q);
        q->insertPlugin(p);
    }

    if ( KUrlPropsPlugin::supports( m_items ) ) {
        KPropertiesDialogPlugin *p = new KUrlPropsPlugin(q);
        q->insertPlugin(p);
    }

    if ( KDevicePropsPlugin::supports( m_items ) ) {
        KPropertiesDialogPlugin *p = new KDevicePropsPlugin(q);
        q->insertPlugin(p);
    }

    if ( KFileMetaPropsPlugin::supports( m_items ) ) {
        KPropertiesDialogPlugin *p = new KFileMetaPropsPlugin(q);
        q->insertPlugin(p);
    }

    if ( KPreviewPropsPlugin::supports( m_items ) ) {
        KPropertiesDialogPlugin *p = new KPreviewPropsPlugin(q);
        q->insertPlugin(p);
    }

#ifndef Q_OS_WIN
    if ( KAuthorized::authorizeKAction("sharefile") &&
         KFileSharePropsPlugin::supports( m_items ) ) {
        KPropertiesDialogPlugin *p = new KFileSharePropsPlugin(q);
        q->insertPlugin(p);
    }
#endif

    //plugins

    if ( m_items.count() != 1 )
        return;

    const KFileItem item = m_items.first();
    const QString mimetype = item.mimetype();

    if ( mimetype.isEmpty() )
        return;

    QString query = QString::fromLatin1(
            "((not exist [X-KDE-Protocol]) or "
            " ([X-KDE-Protocol] == '%1'  )   )"
            ).arg(item.url().protocol());

    kDebug( 250 ) << "trader query: " << query;
    const KService::List offers = KMimeTypeTrader::self()->query( mimetype, "KPropertiesDialog/Plugin", query );
    foreach (const KService::Ptr &ptr, offers) {
        KPropertiesDialogPlugin *plugin = ptr->createInstance<KPropertiesDialogPlugin>(q);
        if (!plugin)
            continue;
        plugin->setObjectName(ptr->name());

        q->insertPlugin(plugin);
    }
}

void KPropertiesDialog::updateUrl( const KUrl& _newUrl )
{
    Q_ASSERT(d->m_items.count() == 1);
    kDebug(250) << "KPropertiesDialog::updateUrl (pre)" << _newUrl.url();
    KUrl newUrl = _newUrl;
    emit saveAs(d->m_singleUrl, newUrl);
    kDebug(250) << "KPropertiesDialog::updateUrl (post)" << newUrl.url();

    d->m_singleUrl = newUrl;
    d->m_items.first().setUrl(newUrl);
    assert(!d->m_singleUrl.isEmpty());
    // If we have an Desktop page, set it dirty, so that a full file is saved locally
    // Same for a URL page (because of the Name= hack)
    foreach (KPropertiesDialogPlugin *it, d->m_pageList) {
        if ( qobject_cast<KUrlPropsPlugin*>(it) ||
             qobject_cast<KDesktopPropsPlugin*>(it) )
        {
            //kDebug(250) << "Setting page dirty";
            it->setDirty();
            break;
        }
    }
}

void KPropertiesDialog::rename( const QString& _name )
{
    Q_ASSERT(d->m_items.count() == 1);
    kDebug(250) << "KPropertiesDialog::rename " << _name;
    KUrl newUrl;
    // if we're creating from a template : use currentdir
    if (!d->m_currentDir.isEmpty()) {
        newUrl = d->m_currentDir;
        newUrl.addPath(_name);
    } else {
        QString tmpurl = d->m_singleUrl.url();
        if (!tmpurl.isEmpty() && tmpurl.at(tmpurl.length() - 1) == '/') {
            // It's a directory, so strip the trailing slash first
            tmpurl.truncate(tmpurl.length() - 1);
        }

        newUrl = tmpurl;
        newUrl.setFileName(_name);
    }
    updateUrl(newUrl);
}

void KPropertiesDialog::abortApplying()
{
    d->m_aborted = true;
}

class KPropertiesDialogPlugin::KPropertiesDialogPluginPrivate
{
public:
    KPropertiesDialogPluginPrivate()
    {
    }
    ~KPropertiesDialogPluginPrivate()
    {
    }

    bool m_bDirty;
    int fontHeight;
};

KPropertiesDialogPlugin::KPropertiesDialogPlugin( KPropertiesDialog *_props )
    : QObject( _props ),d(new KPropertiesDialogPluginPrivate)
{
    properties = _props;
    d->fontHeight = 2*properties->fontMetrics().height();
    d->m_bDirty = false;
}

KPropertiesDialogPlugin::~KPropertiesDialogPlugin()
{
    delete d;
}

bool KPropertiesDialogPlugin::isDesktopFile( const KFileItem& _item )
{
    return _item.isDesktopFile();
}

void KPropertiesDialogPlugin::setDirty( bool b )
{
    d->m_bDirty = b;
}

void KPropertiesDialogPlugin::setDirty()
{
    d->m_bDirty = true;
}

bool KPropertiesDialogPlugin::isDirty() const
{
    return d->m_bDirty;
}

void KPropertiesDialogPlugin::applyChanges()
{
    kWarning(250) << "applyChanges() not implemented in page !";
}

int KPropertiesDialogPlugin::fontHeight() const
{
    return d->fontHeight;
}

///////////////////////////////////////////////////////////////////////////////

class KFilePropsPlugin::KFilePropsPluginPrivate
{
public:
    KFilePropsPluginPrivate()
    {
        dirSizeJob = 0L;
        dirSizeUpdateTimer = 0L;
        m_lined = 0;
        m_capacityBar = 0;
        m_linkTargetLineEdit = 0;
    }
    ~KFilePropsPluginPrivate()
    {
        if ( dirSizeJob )
            dirSizeJob->kill();
    }

    KIO::DirectorySizeJob * dirSizeJob;
    QTimer *dirSizeUpdateTimer;
    QFrame *m_frame;
    bool bMultiple;
    bool bIconChanged;
    bool bKDesktopMode;
    bool bDesktopFile;
    KCapacityBar *m_capacityBar;
    QString mimeType;
    QString oldFileName;
    KLineEdit* m_lined;

    QWidget *iconArea;
    QWidget *nameArea;

    QLabel *m_sizeLabel;
    QPushButton *m_sizeDetermineButton;
    QPushButton *m_sizeStopButton;
    KLineEdit* m_linkTargetLineEdit;

    QString m_sRelativePath;
    bool m_bFromTemplate;

    /**
   * The initial filename
   */
    QString oldName;
};

KFilePropsPlugin::KFilePropsPlugin( KPropertiesDialog *_props )
    : KPropertiesDialogPlugin( _props ),d(new KFilePropsPluginPrivate)
{
    d->bMultiple = (properties->items().count() > 1);
    d->bIconChanged = false;
    d->bDesktopFile = KDesktopPropsPlugin::supports(properties->items());
    kDebug(250) << "KFilePropsPlugin::KFilePropsPlugin bMultiple=" << d->bMultiple;

    // We set this data from the first item, and we'll
    // check that the other items match against it, resetting when not.
    bool isLocal;
    const KFileItem item = properties->item();
    KUrl url = item.mostLocalUrl( isLocal );
    bool isReallyLocal = item.url().isLocalFile();
    bool bDesktopFile = item.isDesktopFile();
    mode_t mode = item.mode();
    bool hasDirs = item.isDir() && !item.isLink();
    bool hasRoot = url.path() == QLatin1String("/");
    QString iconStr = KMimeType::iconNameForUrl(url, mode);
    QString directory = properties->kurl().directory();
    QString protocol = properties->kurl().protocol();
    d->bKDesktopMode = protocol == QLatin1String("desktop") ||
                properties->currentDir().protocol() == QLatin1String("desktop");
    QString mimeComment = item.mimeComment();
    d->mimeType = item.mimetype();
    KIO::filesize_t totalSize = item.size();
    QString magicMimeComment;
    if ( isLocal ) {
        KMimeType::Ptr magicMimeType = KMimeType::findByFileContent( url.path() );
        if ( magicMimeType->name() != KMimeType::defaultMimeType() )
            magicMimeComment = magicMimeType->comment();
    }
#ifdef Q_WS_WIN
    if ( isReallyLocal ) {
        directory = QDir::toNativeSeparators( directory.mid( 1 ) );
    }
#endif

    // Those things only apply to 'single file' mode
    QString filename;
    bool isTrash = false;
    d->m_bFromTemplate = false;

    // And those only to 'multiple' mode
    uint iDirCount = hasDirs ? 1 : 0;
    uint iFileCount = 1-iDirCount;

    d->m_frame = new QFrame();
    properties->addPage(d->m_frame, i18nc("@title:tab File properties", "&General"));

    QVBoxLayout *vbl = new QVBoxLayout( d->m_frame );
    vbl->setMargin( 0 );
    vbl->setObjectName( QLatin1String( "vbl" ) );
    QGridLayout *grid = new QGridLayout(); // unknown rows
    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 0);
    grid->setColumnStretch(2, 1);
    grid->addItem(new QSpacerItem(KDialog::spacingHint(),0), 0, 1);
    vbl->addLayout(grid);
    int curRow = 0;

    if ( !d->bMultiple )
    {
        QString path;
        if ( !d->m_bFromTemplate ) {
            isTrash = ( properties->kurl().protocol().toLower() == "trash" );
            // Extract the full name, but without file: for local files
            if ( isReallyLocal )
                path = properties->kurl().toLocalFile();
            else
                path = properties->kurl().prettyUrl();
        } else {
            path = properties->currentDir().path(KUrl::AddTrailingSlash) + properties->defaultName();
            directory = properties->currentDir().prettyUrl();
        }

        if (d->bDesktopFile) {
            determineRelativePath( path );
        }

        // Extract the file name only
        filename = properties->defaultName();
        if ( filename.isEmpty() ) { // no template
            filename = item.name(); // this gives support for UDS_NAME, e.g. for kio_trash or kio_system
        } else {
            d->m_bFromTemplate = true;
            setDirty(); // to enforce that the copy happens
        }
        d->oldFileName = filename;

        // Make it human-readable
        filename = nameFromFileName( filename );

        if ( d->bKDesktopMode && d->bDesktopFile ) {
            KDesktopFile config( url.path() );
            if ( config.desktopGroup().hasKey( "Name" ) ) {
                filename = config.readName();
            }
        }

        d->oldName = filename;
    }
    else
    {
        // Multiple items: see what they have in common
        const KFileItemList items = properties->items();
        KFileItemList::const_iterator kit = items.begin();
        const KFileItemList::const_iterator kend = items.end();
        for ( ++kit /*no need to check the first one again*/ ; kit != kend; ++kit )
        {
            const KUrl url = (*kit).url();
            kDebug(250) << "KFilePropsPlugin::KFilePropsPlugin " << url.prettyUrl();
            // The list of things we check here should match the variables defined
            // at the beginning of this method.
            if ( url.isLocalFile() != isLocal )
                isLocal = false; // not all local
            if ( bDesktopFile && (*kit).isDesktopFile() != bDesktopFile )
                bDesktopFile = false; // not all desktop files
            if ( (*kit).mode() != mode )
                mode = (mode_t)0;
            if ( KMimeType::iconNameForUrl(url, mode) != iconStr )
                iconStr = "document-multiple";
            if ( url.directory() != directory )
                directory.clear();
            if ( url.protocol() != protocol )
                protocol.clear();
            if ( !mimeComment.isNull() && (*kit).mimeComment() != mimeComment )
                mimeComment.clear();
            if ( isLocal && !magicMimeComment.isNull() ) {
                KMimeType::Ptr magicMimeType = KMimeType::findByFileContent( url.path() );
                if ( magicMimeType->comment() != magicMimeComment )
                    magicMimeComment.clear();
            }

            if ( isLocal && url.path() == QLatin1String("/") )
                hasRoot = true;
            if ( (*kit).isDir() && !(*kit).isLink() )
            {
                iDirCount++;
                hasDirs = true;
            }
            else
            {
                iFileCount++;
                totalSize += (*kit).size();
            }
        }
    }

    if (!isReallyLocal && !protocol.isEmpty())
    {
        directory += ' ';
        directory += '(';
        directory += protocol;
        directory += ')';
    }

    if (!isTrash && (bDesktopFile || S_ISDIR(mode))
        && !d->bMultiple // not implemented for multiple
        && enableIconButton()) // #56857
    {
        KIconButton *iconButton = new KIconButton( d->m_frame );
        int bsize = 66 + 2 * iconButton->style()->pixelMetric(QStyle::PM_ButtonMargin);
        iconButton->setFixedSize(bsize, bsize);
        iconButton->setIconSize(48);
        iconButton->setStrictIconSize(false);
        // This works for everything except Device icons on unmounted devices
        // So we have to really open .desktop files
        QString iconStr = KMimeType::findByUrl( url, mode )->iconName( url );
        if ( bDesktopFile && isLocal )
        {
            KDesktopFile config( url.path() );
            KConfigGroup group = config.desktopGroup();
            iconStr = group.readEntry( "Icon" );
            if ( config.hasDeviceType() )
                iconButton->setIconType( KIconLoader::Desktop, KIconLoader::Device );
            else
                iconButton->setIconType( KIconLoader::Desktop, KIconLoader::Application );
        } else
            iconButton->setIconType( KIconLoader::Desktop, KIconLoader::Place );
        iconButton->setIcon(iconStr);
        d->iconArea = iconButton;
        connect(iconButton, SIGNAL(iconChanged(QString)),
                this, SLOT(slotIconChanged()));
    } else {
        QLabel *iconLabel = new QLabel( d->m_frame );
        int bsize = 66 + 2 * iconLabel->style()->pixelMetric(QStyle::PM_ButtonMargin);
        iconLabel->setFixedSize(bsize, bsize);
        iconLabel->setPixmap( KIconLoader::global()->loadIcon( iconStr, KIconLoader::Desktop, 48) );
        d->iconArea = iconLabel;
    }
    grid->addWidget(d->iconArea, curRow, 0, Qt::AlignLeft);

    if (d->bMultiple || isTrash || hasRoot)
    {
        QLabel *lab = new QLabel(d->m_frame );
        if ( d->bMultiple )
            lab->setText( KIO::itemsSummaryString( iFileCount + iDirCount, iFileCount, iDirCount, 0, false ) );
        else
            lab->setText( filename );
        d->nameArea = lab;
    } else
    {
        d->m_lined = new KLineEdit( d->m_frame );
        d->m_lined->setText(filename);
        d->nameArea = d->m_lined;
        d->m_lined->setFocus();

        //if we don't have permissions to rename, we need to make "m_lined" read only.
        KFileItemListProperties itemList(KFileItemList()<< item);
        setFileNameReadOnly(!itemList.supportsMoving());

        // Enhanced rename: Don't highlight the file extension.
        QString extension = KMimeType::extractKnownExtension( filename );
        if ( !extension.isEmpty() )
            d->m_lined->setSelection( 0, filename.length() - extension.length() - 1 );
        else
        {
            int lastDot = filename.lastIndexOf('.');
            if (lastDot > 0)
                d->m_lined->setSelection(0, lastDot);
        }

        connect( d->m_lined, SIGNAL( textChanged( const QString & ) ),
                 this, SLOT( nameFileChanged(const QString & ) ) );
    }

    grid->addWidget(d->nameArea, curRow++, 2);

    KSeparator* sep = new KSeparator( Qt::Horizontal, d->m_frame);
    grid->addWidget(sep, curRow, 0, 1, 3);
    ++curRow;

    QLabel *l;
    if (!mimeComment.isEmpty() && !isTrash) {
        l = new QLabel(i18n("Type:"), d->m_frame );

        grid->addWidget(l, curRow, 0, Qt::AlignRight);

        KHBox *box = new KHBox(d->m_frame);
        box->setSpacing(20); // ### why 20?
        l = new QLabel(mimeComment, box );

        QPushButton *button = new QPushButton(box);

        button->setIcon( KIcon(QString::fromLatin1("configure")) );
        const int pixmapSize = button->style()->pixelMetric(QStyle::PM_SmallIconSize);
        button->setFixedSize( pixmapSize+8, pixmapSize+8 );
        if ( d->mimeType == KMimeType::defaultMimeType() )
            button->setToolTip(i18n("Create new file type"));
        else
            button->setToolTip(i18n("Edit file type"));

        connect( button, SIGNAL( clicked() ), SLOT( slotEditFileType() ));

        if (!KAuthorized::authorizeKAction("editfiletype"))
            button->hide();

        grid->addWidget(box, curRow++, 2);
    }

    if ( !magicMimeComment.isEmpty() && magicMimeComment != mimeComment )
    {
        l = new QLabel(i18n("Contents:"), d->m_frame );
        grid->addWidget(l, curRow, 0, Qt::AlignRight);

        l = new QLabel(magicMimeComment, d->m_frame );
        grid->addWidget(l, curRow++, 2);
    }

    if ( !directory.isEmpty() )
    {
        l = new QLabel( i18n("Location:"), d->m_frame );
        grid->addWidget(l, curRow, 0, Qt::AlignRight);

        l = new KSqueezedTextLabel( directory, d->m_frame );
        // force the layout direction to be always LTR
        l->setLayoutDirection(Qt::LeftToRight);
        // but if we are in RTL mode, align the text to the right
        // otherwise the text is on the wrong side of the dialog
        if (properties->layoutDirection() == Qt::RightToLeft)
            l->setAlignment( Qt::AlignRight );
        l->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
        grid->addWidget(l, curRow++, 2);
    }

    l = new QLabel(i18n("Size:"), d->m_frame );
    grid->addWidget(l, curRow, 0, Qt::AlignRight);

    d->m_sizeLabel = new QLabel( d->m_frame );
    grid->addWidget( d->m_sizeLabel, curRow++, 2 );

    if ( !hasDirs ) // Only files [and symlinks]
    {
        d->m_sizeLabel->setText(QString::fromLatin1("%1 (%2)").arg(KIO::convertSize(totalSize))
                                .arg(KGlobal::locale()->formatNumber(totalSize, 0)));
        d->m_sizeDetermineButton = 0L;
        d->m_sizeStopButton = 0L;
    }
    else // Directory
    {
        QHBoxLayout * sizelay = new QHBoxLayout();
        grid->addLayout( sizelay, curRow++, 2 );

        // buttons
        d->m_sizeDetermineButton = new QPushButton( i18n("Calculate"), d->m_frame );
        d->m_sizeStopButton = new QPushButton( i18n("Stop"), d->m_frame );
        connect( d->m_sizeDetermineButton, SIGNAL( clicked() ), this, SLOT( slotSizeDetermine() ) );
        connect( d->m_sizeStopButton, SIGNAL( clicked() ), this, SLOT( slotSizeStop() ) );
        sizelay->addWidget(d->m_sizeDetermineButton, 0);
        sizelay->addWidget(d->m_sizeStopButton, 0);
        sizelay->addStretch(10); // so that the buttons don't grow horizontally

        // auto-launch for local dirs only, and not for '/'
        if ( isLocal && !hasRoot )
        {
            d->m_sizeDetermineButton->setText( i18n("Refresh") );
            slotSizeDetermine();
        }
        else
            d->m_sizeStopButton->setEnabled( false );
    }

    if (!d->bMultiple && item.isLink()) {
        l = new QLabel(i18n("Points to:"), d->m_frame );
        grid->addWidget(l, curRow, 0, Qt::AlignRight);

        d->m_linkTargetLineEdit = new KLineEdit(item.linkDest(), d->m_frame );
        grid->addWidget(d->m_linkTargetLineEdit, curRow++, 2);
        connect(d->m_linkTargetLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setDirty()));
    }

    if (!d->bMultiple) // Dates for multiple don't make much sense...
    {
        KDateTime dt = item.time(KFileItem::CreationTime);
        if ( !dt.isNull() )
        {
            l = new QLabel(i18n("Created:"), d->m_frame );
            grid->addWidget(l, curRow, 0, Qt::AlignRight);

            l = new QLabel(KGlobal::locale()->formatDateTime(dt), d->m_frame );
            grid->addWidget(l, curRow++, 2);
        }

        dt = item.time(KFileItem::ModificationTime);
        if ( !dt.isNull() )
        {
            l = new QLabel(i18n("Modified:"), d->m_frame );
            grid->addWidget(l, curRow, 0, Qt::AlignRight);

            l = new QLabel(KGlobal::locale()->formatDateTime(dt), d->m_frame );
            grid->addWidget(l, curRow++, 2);
        }

        dt = item.time(KFileItem::AccessTime);
        if ( !dt.isNull() )
        {
            l = new QLabel(i18n("Accessed:"), d->m_frame );
            grid->addWidget(l, curRow, 0, Qt::AlignRight);

            l = new QLabel(KGlobal::locale()->formatDateTime(dt), d->m_frame );
            grid->addWidget(l, curRow++, 2);
        }
    }

    if ( isLocal && hasDirs )  // only for directories
    {

        KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByPath( url.path() );
        if (mp) {
            KDiskFreeSpaceInfo info = KDiskFreeSpaceInfo::freeSpaceInfo( mp->mountPoint() );
            if(info.size() != 0 )
            {
                sep = new KSeparator( Qt::Horizontal, d->m_frame);
                grid->addWidget(sep, curRow, 0, 1, 3);
                ++curRow;
                if (mp->mountPoint() != "/")
                {
                    l = new QLabel(i18n("Mounted on:"), d->m_frame );
                    grid->addWidget(l, curRow, 0, Qt::AlignRight);

                    l = new KSqueezedTextLabel( mp->mountPoint(), d->m_frame );
                    l->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
                    grid->addWidget( l, curRow++, 2 );
                }

                l = new QLabel(i18n("Device usage:"), d->m_frame );
                grid->addWidget(l, curRow, 0, Qt::AlignRight);

                d->m_capacityBar = new KCapacityBar( KCapacityBar::DrawTextOutline, d->m_frame );
                grid->addWidget( d->m_capacityBar, curRow++, 2);

                slotFoundMountPoint( info.mountPoint(), info.size()/1024, info.used()/1024, info.available()/1024);
            }
        }
    }

    vbl->addStretch(1);
}

bool KFilePropsPlugin::enableIconButton() const
{
    bool iconEnabled = false;
    const KFileItem item = properties->item();
    // If the current item is a directory, check if it's writable,
    // so we can create/update a .directory
    // Current item is a file, same thing: check if it is writable
    if (item.isWritable()) {
        iconEnabled = true;
    }
    return iconEnabled;
}

// QString KFilePropsPlugin::tabName () const
// {
//   return i18n ("&General");
// }

void KFilePropsPlugin::setFileNameReadOnly( bool ro )
{
    if ( d->m_lined && !d->m_bFromTemplate )
    {
        d->m_lined->setReadOnly( ro );
        if (ro)
        {
            // Don't put the initial focus on the line edit when it is ro
            properties->setButtonFocus(KDialog::Ok);
        }
    }
}

void KFilePropsPlugin::slotEditFileType()
{
    QString mime;
    if (d->mimeType == KMimeType::defaultMimeType()) {
        const int pos = d->oldFileName.lastIndexOf('.');
        if (pos != -1)
            mime = '*' + d->oldFileName.mid(pos);
        else
            mime = '*';
    }  else {
        mime = d->mimeType;
    }
    QString keditfiletype = QString::fromLatin1("keditfiletype");
    KRun::runCommand( keditfiletype
#ifdef Q_WS_X11
                      + " --parent " + QString::number( (ulong)properties->window()->winId())
#endif
                      + ' ' + KShell::quoteArg(mime),
                      keditfiletype, keditfiletype /*unused*/, properties->window());
}

void KFilePropsPlugin::slotIconChanged()
{
    d->bIconChanged = true;
    emit changed();
}

void KFilePropsPlugin::nameFileChanged(const QString &text )
{
    properties->enableButtonOk(!text.isEmpty());
    emit changed();
}

void KFilePropsPlugin::determineRelativePath( const QString & path )
{
    // now let's make it relative
    d->m_sRelativePath = KGlobal::dirs()->relativeLocation("apps", path);
    if (d->m_sRelativePath.startsWith('/'))
    {
        d->m_sRelativePath =KGlobal::dirs()->relativeLocation("xdgdata-apps", path);
        if (d->m_sRelativePath.startsWith('/'))
            d->m_sRelativePath.clear();
        else
            d->m_sRelativePath = path;
    }
}

void KFilePropsPlugin::slotFoundMountPoint( const QString&,
                                            quint64 kibSize,
                                            quint64 /*kibUsed*/,
                                            quint64 kibAvail )
{
    d->m_capacityBar->setText(
            i18nc("Available space out of total partition size (percent used)", "%1 free of %2 (%3% used)",
                  KIO::convertSizeFromKiB(kibAvail),
                  KIO::convertSizeFromKiB(kibSize),
                  100 - (int)(100.0 * kibAvail / kibSize) ));

    d->m_capacityBar->setValue(100 - (int)(100.0 * kibAvail / kibSize));
}

void KFilePropsPlugin::slotDirSizeUpdate()
{
    KIO::filesize_t totalSize = d->dirSizeJob->totalSize();
    KIO::filesize_t totalFiles = d->dirSizeJob->totalFiles();
    KIO::filesize_t totalSubdirs = d->dirSizeJob->totalSubdirs();
    d->m_sizeLabel->setText(
            i18n("Calculating... %1 (%2)\n%3, %4",
                 KIO::convertSize(totalSize),
                 totalSize,
                 i18np("1 file", "%1 files", totalFiles),
                 i18np("1 sub-folder", "%1 sub-folders", totalSubdirs)));
}

void KFilePropsPlugin::slotDirSizeFinished( KJob * job )
{
    if (job->error())
        d->m_sizeLabel->setText( job->errorString() );
    else
    {
        KIO::filesize_t totalSize = d->dirSizeJob->totalSize();
        KIO::filesize_t totalFiles = d->dirSizeJob->totalFiles();
        KIO::filesize_t totalSubdirs = d->dirSizeJob->totalSubdirs();
        d->m_sizeLabel->setText( QString::fromLatin1("%1 (%2)\n%3, %4")
                                 .arg(KIO::convertSize(totalSize))
                                 .arg(KGlobal::locale()->formatNumber(totalSize, 0))
                                 .arg(i18np("1 file","%1 files",totalFiles))
                                 .arg(i18np("1 sub-folder","%1 sub-folders",totalSubdirs)));
    }
    d->m_sizeStopButton->setEnabled(false);
    // just in case you change something and try again :)
    d->m_sizeDetermineButton->setText( i18n("Refresh") );
    d->m_sizeDetermineButton->setEnabled(true);
    d->dirSizeJob = 0;
    delete d->dirSizeUpdateTimer;
    d->dirSizeUpdateTimer = 0;
}

void KFilePropsPlugin::slotSizeDetermine()
{
    d->m_sizeLabel->setText( i18n("Calculating...") );
    kDebug(250) << " KFilePropsPlugin::slotSizeDetermine() properties->item()=" <<  properties->item();
    kDebug(250) << " URL=" << properties->item().url().url();

    d->dirSizeJob = KIO::directorySize( properties->items() );
    d->dirSizeUpdateTimer = new QTimer(this);
    connect( d->dirSizeUpdateTimer, SIGNAL( timeout() ),
             SLOT( slotDirSizeUpdate() ) );
    d->dirSizeUpdateTimer->start(500);
    connect( d->dirSizeJob, SIGNAL( result( KJob * ) ),
             SLOT( slotDirSizeFinished( KJob * ) ) );
    d->m_sizeStopButton->setEnabled(true);
    d->m_sizeDetermineButton->setEnabled(false);

    // also update the "Free disk space" display
    if ( d->m_capacityBar )
    {
        bool isLocal;
        const KFileItem item = properties->item();
        KUrl url = item.mostLocalUrl( isLocal );
        KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByPath( url.path() );
        if (mp) {
            KDiskFreeSpaceInfo info = KDiskFreeSpaceInfo::freeSpaceInfo( mp->mountPoint() );
            slotFoundMountPoint( info.mountPoint(), info.size()/1024, info.used()/1024, info.available()/1024);
        }
    }
}

void KFilePropsPlugin::slotSizeStop()
{
    if ( d->dirSizeJob )
    {
        KIO::filesize_t totalSize = d->dirSizeJob->totalSize();
        d->m_sizeLabel->setText(i18n("At least %1",
                                     KIO::convertSize(totalSize)));
        d->dirSizeJob->kill();
        d->dirSizeJob = 0;
    }
    if ( d->dirSizeUpdateTimer )
        d->dirSizeUpdateTimer->stop();

    d->m_sizeStopButton->setEnabled(false);
    d->m_sizeDetermineButton->setEnabled(true);
}

KFilePropsPlugin::~KFilePropsPlugin()
{
    delete d;
}

bool KFilePropsPlugin::supports( const KFileItemList& /*_items*/ )
{
    return true;
}

void KFilePropsPlugin::applyChanges()
{
    if ( d->dirSizeJob )
        slotSizeStop();

    kDebug(250) << "KFilePropsPlugin::applyChanges";

    if (qobject_cast<QLineEdit*>(d->nameArea))
    {
        QString n = ((QLineEdit *) d->nameArea)->text();
        // Remove trailing spaces (#4345)
        while ( ! n.isEmpty() && n[n.length()-1].isSpace() )
            n.truncate( n.length() - 1 );
        if ( n.isEmpty() )
        {
            KMessageBox::sorry( properties, i18n("The new file name is empty."));
            properties->abortApplying();
            return;
        }

        // Do we need to rename the file ?
        kDebug(250) << "oldname = " << d->oldName;
        kDebug(250) << "newname = " << n;
        if ( d->oldName != n || d->m_bFromTemplate ) { // true for any from-template file
            KIO::Job * job = 0L;
            KUrl oldurl = properties->kurl();

            QString newFileName = KIO::encodeFileName(n);
            if (d->bDesktopFile && !newFileName.endsWith(QLatin1String(".desktop")) &&
                !newFileName.endsWith(QLatin1String(".kdelnk")))
                newFileName += ".desktop";

            // Tell properties. Warning, this changes the result of properties->kurl() !
            properties->rename( newFileName );

            // Update also relative path (for apps and mimetypes)
            if ( !d->m_sRelativePath.isEmpty() )
                determineRelativePath( properties->kurl().toLocalFile() );

            kDebug(250) << "New URL = " << properties->kurl().url();
            kDebug(250) << "old = " << oldurl.url();

            // Don't remove the template !!
            if ( !d->m_bFromTemplate ) // (normal renaming)
                job = KIO::move( oldurl, properties->kurl() );
            else // Copying a template
                job = KIO::copy( oldurl, properties->kurl() );

            connect( job, SIGNAL( result( KJob * ) ),
                     SLOT( slotCopyFinished( KJob * ) ) );
            connect( job, SIGNAL( renamed( KIO::Job *, const KUrl &, const KUrl & ) ),
                     SLOT( slotFileRenamed( KIO::Job *, const KUrl &, const KUrl & ) ) );
            // wait for job
            QEventLoop eventLoop;
            connect(this, SIGNAL(leaveModality()),
                    &eventLoop, SLOT(quit()));
            eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
            return;
        }
        properties->updateUrl(properties->kurl());
        // Update also relative path (for apps and mimetypes)
        if ( !d->m_sRelativePath.isEmpty() )
            determineRelativePath( properties->kurl().toLocalFile() );
    }

    // No job, keep going
    slotCopyFinished( 0L );
}

void KFilePropsPlugin::slotCopyFinished( KJob * job )
{
    kDebug(250) << "KFilePropsPlugin::slotCopyFinished";
    if (job)
    {
        // allow apply() to return
        emit leaveModality();
        if ( job->error() )
        {
            job->uiDelegate()->showErrorMessage();
            // Didn't work. Revert the URL to the old one
            properties->updateUrl( static_cast<KIO::CopyJob*>(job)->srcUrls().first() );
            properties->abortApplying(); // Don't apply the changes to the wrong file !
            return;
        }
    }

    assert( !properties->item().isNull() );
    assert( !properties->item().url().isEmpty() );

    // Save the file where we can -> usually in ~/.kde/...
    if (d->bDesktopFile && !d->m_sRelativePath.isEmpty())
    {
        kDebug(250) << "KFilePropsPlugin::slotCopyFinished " << d->m_sRelativePath;
        KUrl newURL;
        newURL.setPath( KDesktopFile::locateLocal(d->m_sRelativePath) );
        kDebug(250) << "KFilePropsPlugin::slotCopyFinished path=" << newURL.path();
        properties->updateUrl( newURL );
    }

    if ( d->bKDesktopMode && d->bDesktopFile ) {
        // Renamed? Update Name field
        // Note: The desktop ioslave does this as well, but not when
        //       the file is copied from a template.
        if ( d->m_bFromTemplate ) {
            KIO::UDSEntry entry;
            KIO::NetAccess::stat( properties->kurl(), entry, 0 );
            KFileItem item( entry, properties->kurl() );
            KDesktopFile config( item.localPath() );
            KConfigGroup cg = config.desktopGroup();
            QString nameStr = nameFromFileName(properties->kurl().fileName());
            cg.writeEntry( "Name", nameStr );
            cg.writeEntry( "Name", nameStr, KConfigGroup::Persistent|KConfigGroup::Localized);
        }
    }

    if (d->m_linkTargetLineEdit && !d->bMultiple) {
        const KFileItem item = properties->item();
        const QString newTarget = d->m_linkTargetLineEdit->text();
        if (newTarget != item.linkDest()) {
            kDebug(250) << "Updating target of symlink to" << newTarget;
            KIO::Job* job = KIO::symlink(newTarget, item.url(), KIO::Overwrite);
            job->ui()->setAutoErrorHandlingEnabled(true);
            job->exec();
        }
    }

    // "Link to Application" templates need to be made executable
    // Instead of matching against a filename we check if the destination
    // is an Application now.
    if ( d->m_bFromTemplate ) {
        // destination is not necessarily local, use the src template
        KDesktopFile templateResult ( static_cast<KIO::CopyJob*>(job)->srcUrls().first().toLocalFile() );
        if ( templateResult.hasApplicationType() ) {
            // We can either stat the file and add the +x bit or use the larger chmod() job
            // with a umask designed to only touch u+x.  This is only one KIO job, so let's
            // do that.

            KFileItem appLink ( properties->item() );
            KFileItemList fileItemList;
            fileItemList << appLink;

            // first 0100 adds u+x, second 0100 only allows chmod to change u+x
            KIO::Job* chmodJob = KIO::chmod( fileItemList, 0100, 0100, QString(), QString(), KIO::HideProgressInfo );
            chmodJob->exec();
        }
    }
}

void KFilePropsPlugin::applyIconChanges()
{
    KIconButton *iconButton = qobject_cast<KIconButton*>(d->iconArea);
    if ( !iconButton || !d->bIconChanged )
        return;
    // handle icon changes - only local files (or pseudo-local) for now
    // TODO: Use KTempFile and KIO::file_copy with overwrite = true
    KUrl url = properties->kurl();
    url = KIO::NetAccess::mostLocalUrl( url, properties );
    if ( url.isLocalFile()) {
        QString path;

        if (S_ISDIR(properties->item().mode()))
        {
            path = url.toLocalFile(KUrl::AddTrailingSlash) + QString::fromLatin1(".directory");
            // don't call updateUrl because the other tabs (i.e. permissions)
            // apply to the directory, not the .directory file.
        }
        else
            path = url.toLocalFile();

        // Get the default image
        QString str = KMimeType::findByUrl( url,
                                            properties->item().mode(),
                                            true )->iconName();
        // Is it another one than the default ?
        QString sIcon;
        if ( str != iconButton->icon() )
            sIcon = iconButton->icon();
        // (otherwise write empty value)

        kDebug(250) << "**" << path << "**";

        // If default icon and no .directory file -> don't create one
        if ( !sIcon.isEmpty() || QFile::exists(path) )
        {
            KDesktopFile cfg(path);
            kDebug(250) << "sIcon = " << (sIcon);
            kDebug(250) << "str = " << (str);
            cfg.desktopGroup().writeEntry( "Icon", sIcon );
            cfg.sync();

            cfg.reparseConfiguration();
            if ( cfg.desktopGroup().readEntry("Icon") != sIcon ) {
                KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not "
                                            "have sufficient access to write to <b>%1</b>.</qt>", path));
            }
        }
    }
}

void KFilePropsPlugin::slotFileRenamed( KIO::Job *, const KUrl &, const KUrl & newUrl )
{
    // This is called in case of an existing local file during the copy/move operation,
    // if the user chooses Rename.
    properties->updateUrl( newUrl );
}

void KFilePropsPlugin::postApplyChanges()
{
    // Save the icon only after applying the permissions changes (#46192)
    applyIconChanges();

    const KFileItemList items = properties->items();
    const KUrl::List lst = items.urlList();
    org::kde::KDirNotify::emitFilesChanged( lst.toStringList() );
}

class KFilePermissionsPropsPlugin::KFilePermissionsPropsPluginPrivate
{
public:
    KFilePermissionsPropsPluginPrivate()
    {
    }
    ~KFilePermissionsPropsPluginPrivate()
    {
    }

    QFrame *m_frame;
    QCheckBox *cbRecursive;
    QLabel *explanationLabel;
    KComboBox *ownerPermCombo, *groupPermCombo, *othersPermCombo;
    QCheckBox *extraCheckbox;
    mode_t partialPermissions;
    KFilePermissionsPropsPlugin::PermissionsMode pmode;
    bool canChangePermissions;
    bool isIrregular;
    bool hasExtendedACL;
    KACL extendedACL;
    KACL defaultACL;
    bool fileSystemSupportsACLs;

    KComboBox *grpCombo;

    KLineEdit *usrEdit;
    KLineEdit *grpEdit;

    // Old permissions
    mode_t permissions;
    // Old group
    QString strGroup;
    // Old owner
    QString strOwner;
};

#define UniOwner    (S_IRUSR|S_IWUSR|S_IXUSR)
#define UniGroup    (S_IRGRP|S_IWGRP|S_IXGRP)
#define UniOthers   (S_IROTH|S_IWOTH|S_IXOTH)
#define UniRead     (S_IRUSR|S_IRGRP|S_IROTH)
#define UniWrite    (S_IWUSR|S_IWGRP|S_IWOTH)
#define UniExec     (S_IXUSR|S_IXGRP|S_IXOTH)
#define UniSpecial  (S_ISUID|S_ISGID|S_ISVTX)

// synced with PermissionsTarget
const mode_t KFilePermissionsPropsPlugin::permissionsMasks[3] = {UniOwner, UniGroup, UniOthers};
const mode_t KFilePermissionsPropsPlugin::standardPermissions[4] = { 0, UniRead, UniRead|UniWrite, (mode_t)-1 };

// synced with PermissionsMode and standardPermissions
const char *KFilePermissionsPropsPlugin::permissionsTexts[4][4] = {
    { I18N_NOOP("Forbidden"),
      I18N_NOOP("Can Read"),
      I18N_NOOP("Can Read & Write"),
      0 },
{ I18N_NOOP("Forbidden"),
  I18N_NOOP("Can View Content"),
  I18N_NOOP("Can View & Modify Content"),
  0 },
{ 0, 0, 0, 0}, // no texts for links
{ I18N_NOOP("Forbidden"),
  I18N_NOOP("Can View Content & Read"),
  I18N_NOOP("Can View/Read & Modify/Write"),
  0 }
};


KFilePermissionsPropsPlugin::KFilePermissionsPropsPlugin( KPropertiesDialog *_props )
    : KPropertiesDialogPlugin( _props ),d(new KFilePermissionsPropsPluginPrivate)
{
    d->cbRecursive = 0L;
    d->grpCombo = 0L; d->grpEdit = 0;
    d->usrEdit = 0L;
    QString path = properties->kurl().path(KUrl::RemoveTrailingSlash);
    QString fname = properties->kurl().fileName();
    bool isLocal = properties->kurl().isLocalFile();
    bool isTrash = ( properties->kurl().protocol().toLower() == "trash" );
    bool IamRoot = (geteuid() == 0);

    const KFileItem item = properties->item();
    bool isLink = item.isLink();
    bool isDir = item.isDir(); // all dirs
    bool hasDir = item.isDir(); // at least one dir
    d->permissions = item.permissions(); // common permissions to all files
    d->partialPermissions = d->permissions; // permissions that only some files have (at first we take everything)
    d->isIrregular = isIrregular(d->permissions, isDir, isLink);
    d->strOwner = item.user();
    d->strGroup = item.group();
    d->hasExtendedACL = item.ACL().isExtended() || item.defaultACL().isValid();
    d->extendedACL = item.ACL();
    d->defaultACL = item.defaultACL();
    d->fileSystemSupportsACLs = false;

    if ( properties->items().count() > 1 )
    {
        // Multiple items: see what they have in common
        const KFileItemList items = properties->items();
        KFileItemList::const_iterator it = items.begin();
        const KFileItemList::const_iterator kend = items.end();
        for ( ++it /*no need to check the first one again*/ ; it != kend; ++it )
        {
            const KUrl url = (*it).url();
            if (!d->isIrregular)
                d->isIrregular |= isIrregular((*it).permissions(),
                                              (*it).isDir() == isDir,
                                              (*it).isLink() == isLink);
            d->hasExtendedACL = d->hasExtendedACL || (*it).hasExtendedACL();
            if ( (*it).isLink() != isLink )
                isLink = false;
            if ( (*it).isDir() != isDir )
                isDir = false;
            hasDir |= (*it).isDir();
            if ( (*it).permissions() != d->permissions )
            {
                d->permissions &= (*it).permissions();
                d->partialPermissions |= (*it).permissions();
            }
            if ( (*it).user() != d->strOwner )
                d->strOwner.clear();
            if ( (*it).group() != d->strGroup )
                d->strGroup.clear();
        }
    }

    if (isLink)
        d->pmode = PermissionsOnlyLinks;
    else if (isDir)
        d->pmode = PermissionsOnlyDirs;
    else if (hasDir)
        d->pmode = PermissionsMixed;
    else
        d->pmode = PermissionsOnlyFiles;

    // keep only what's not in the common permissions
    d->partialPermissions = d->partialPermissions & ~d->permissions;

    bool isMyFile = false;

    if (isLocal && !d->strOwner.isEmpty()) { // local files, and all owned by the same person
        struct passwd *myself = getpwuid( geteuid() );
        if ( myself != 0L )
        {
            isMyFile = (d->strOwner == QString::fromLocal8Bit(myself->pw_name));
        } else
            kWarning() << "I don't exist ?! geteuid=" << geteuid();
    } else {
        //We don't know, for remote files, if they are ours or not.
        //So we let the user change permissions, and
        //KIO::chmod will tell, if he had no right to do it.
        isMyFile = true;
    }

    d->canChangePermissions = (isMyFile || IamRoot) && (!isLink);


    // create GUI

    d->m_frame = new QFrame();
    properties->addPage( d->m_frame, i18n("&Permissions") );

    QBoxLayout *box = new QVBoxLayout( d->m_frame );
    box->setMargin( 0 );

    QWidget *l;
    QLabel *lbl;
    QGroupBox *gb;
    QGridLayout *gl;
    QPushButton* pbAdvancedPerm = 0;

    /* Group: Access Permissions */
    gb = new QGroupBox ( i18n("Access Permissions"), d->m_frame );
    box->addWidget (gb);

    gl = new QGridLayout (gb);
    gl->setColumnStretch(1, 1);

    l = d->explanationLabel = new QLabel( "", gb );
    if (isLink)
        d->explanationLabel->setText(i18np("This file is a link and does not have permissions.",
                                           "All files are links and do not have permissions.",
                                           properties->items().count()));
    else if (!d->canChangePermissions)
        d->explanationLabel->setText(i18n("Only the owner can change permissions."));
    gl->addWidget(l, 0, 0, 1, 2);

    lbl = new QLabel( i18n("O&wner:"), gb);
    gl->addWidget(lbl, 1, 0, Qt::AlignRight);
    l = d->ownerPermCombo = new KComboBox(gb);
    lbl->setBuddy(l);
    gl->addWidget(l, 1, 1);
    connect(l, SIGNAL( activated(int) ), this, SIGNAL( changed() ));
    l->setWhatsThis(i18n("Specifies the actions that the owner is allowed to do."));

    lbl = new QLabel( i18n("Gro&up:"), gb);
    gl->addWidget(lbl, 2, 0, Qt::AlignRight);
    l = d->groupPermCombo = new KComboBox(gb);
    lbl->setBuddy(l);
    gl->addWidget(l, 2, 1);
    connect(l, SIGNAL( activated(int) ), this, SIGNAL( changed() ));
    l->setWhatsThis(i18n("Specifies the actions that the members of the group are allowed to do."));

    lbl = new QLabel( i18n("O&thers:"), gb);
    gl->addWidget(lbl, 3, 0, Qt::AlignRight);
    l = d->othersPermCombo = new KComboBox(gb);
    lbl->setBuddy(l);
    gl->addWidget(l, 3, 1);
    connect(l, SIGNAL( activated(int) ), this, SIGNAL( changed() ));
    l->setWhatsThis(i18n("Specifies the actions that all users, who are neither "
                         "owner nor in the group, are allowed to do."));

    if (!isLink) {
        l = d->extraCheckbox = new QCheckBox(hasDir ?
                                             i18n("Only own&er can rename and delete folder content") :
                                             i18n("Is &executable"),
                                             gb );
        connect( d->extraCheckbox, SIGNAL( clicked() ), this, SIGNAL( changed() ) );
        gl->addWidget(l, 4, 1);
        l->setWhatsThis(hasDir ? i18n("Enable this option to allow only the folder's owner to "
                                      "delete or rename the contained files and folders. Other "
                                      "users can only add new files, which requires the 'Modify "
                                      "Content' permission.")
                    : i18n("Enable this option to mark the file as executable. This only makes "
                           "sense for programs and scripts. It is required when you want to "
                           "execute them."));

        QLayoutItem *spacer = new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
        gl->addItem(spacer, 5, 0, 1, 3);

        pbAdvancedPerm = new QPushButton(i18n("A&dvanced Permissions"), gb);
        gl->addWidget(pbAdvancedPerm, 6, 0, 1, 2, Qt::AlignRight);
        connect(pbAdvancedPerm, SIGNAL( clicked() ), this, SLOT( slotShowAdvancedPermissions() ));
    }
    else
        d->extraCheckbox = 0;


    /**** Group: Ownership ****/
    gb = new QGroupBox ( i18n("Ownership"), d->m_frame );
    box->addWidget (gb);

    gl = new QGridLayout (gb);
    gl->addItem(new QSpacerItem(0, 10), 0, 0);

    /*** Set Owner ***/
    l = new QLabel( i18n("User:"), gb );
    gl->addWidget (l, 1, 0, Qt::AlignRight);

    /* GJ: Don't autocomplete more than 1000 users. This is a kind of random
   * value. Huge sites having 10.000+ user have a fair chance of using NIS,
   * (possibly) making this unacceptably slow.
   * OTOH, it is nice to offer this functionality for the standard user.
   */
    int i, maxEntries = 1000;
    struct passwd *user;

    /* File owner: For root, offer a KLineEdit with autocompletion.
   * For a user, who can never chown() a file, offer a QLabel.
   */
    if (IamRoot && isLocal)
    {
        d->usrEdit = new KLineEdit( gb );
        KCompletion *kcom = d->usrEdit->completionObject();
        kcom->setOrder(KCompletion::Sorted);
        setpwent();
        for (i=0; ((user = getpwent()) != 0L) && (i < maxEntries); ++i)
            kcom->addItem(QString::fromLatin1(user->pw_name));
        endpwent();
        d->usrEdit->setCompletionMode((i < maxEntries) ? KGlobalSettings::CompletionAuto :
                                      KGlobalSettings::CompletionNone);
        d->usrEdit->setText(d->strOwner);
        gl->addWidget(d->usrEdit, 1, 1);
        connect( d->usrEdit, SIGNAL( textChanged( const QString & ) ),
                 this, SIGNAL( changed() ) );
    }
    else
    {
        l = new QLabel(d->strOwner, gb);
        gl->addWidget(l, 1, 1);
    }

    /*** Set Group ***/

    QStringList groupList;
    QByteArray strUser;
    user = getpwuid(geteuid());
    if (user != 0L)
        strUser = user->pw_name;

#ifdef HAVE_GETGROUPLIST
    // pick the groups to which the user belongs
    int groupCount = 0;
#ifdef Q_OS_MAC
    QVarLengthArray<int> groups;
#else
    QVarLengthArray<gid_t> groups;
#endif
    if (getgrouplist(strUser, user->pw_gid, NULL, &groupCount) < 0) {
        groups.resize(groupCount);
        if (groups.data())
            getgrouplist(strUser, user->pw_gid, groups.data(), &groupCount);
        else
            groupCount = 0;
    }

    for (i = 0; i < groupCount; i++) {
        struct group *mygroup = getgrgid(groups[i]);
        if (mygroup)
            groupList += QString::fromLocal8Bit(mygroup->gr_name);
    }
#endif // HAVE_GETGROUPLIST

    bool isMyGroup = groupList.contains(d->strGroup);

    /* add the group the file currently belongs to ..
   * .. if it is not there already
   */
    if (!isMyGroup)
        groupList += d->strGroup;

    l = new QLabel( i18n("Group:"), gb );
    gl->addWidget (l, 2, 0, Qt::AlignRight);

    /* Set group: if possible to change:
   * - Offer a KLineEdit for root, since he can change to any group.
   * - Offer a KComboBox for a normal user, since he can change to a fixed
   *   (small) set of groups only.
   * If not changeable: offer a QLabel.
   */
    if (IamRoot && isLocal)
    {
        d->grpEdit = new KLineEdit(gb);
        KCompletion *kcom = new KCompletion;
        kcom->setItems(groupList);
        d->grpEdit->setCompletionObject(kcom, true);
        d->grpEdit->setAutoDeleteCompletionObject( true );
        d->grpEdit->setCompletionMode(KGlobalSettings::CompletionAuto);
        d->grpEdit->setText(d->strGroup);
        gl->addWidget(d->grpEdit, 2, 1);
        connect( d->grpEdit, SIGNAL( textChanged( const QString & ) ),
                 this, SIGNAL( changed() ) );
    }
    else if ((groupList.count() > 1) && isMyFile && isLocal)
    {
        d->grpCombo = new KComboBox(gb);
        d->grpCombo->setObjectName(QLatin1String("combogrouplist"));
        d->grpCombo->addItems(groupList);
        d->grpCombo->setCurrentIndex(groupList.indexOf(d->strGroup));
        gl->addWidget(d->grpCombo, 2, 1);
        connect( d->grpCombo, SIGNAL( activated( int ) ),
                 this, SIGNAL( changed() ) );
    }
    else
    {
        l = new QLabel(d->strGroup, gb);
        gl->addWidget(l, 2, 1);
    }

    gl->setColumnStretch(2, 10);

    // "Apply recursive" checkbox
    if ( hasDir && !isLink && !isTrash  )
    {
        d->cbRecursive = new QCheckBox( i18n("Apply changes to all subfolders and their contents"), d->m_frame );
        connect( d->cbRecursive, SIGNAL( clicked() ), this, SIGNAL( changed() ) );
        box->addWidget( d->cbRecursive );
    }

    updateAccessControls();


    if ( isTrash )
    {
        //don't allow to change properties for file into trash
        enableAccessControls(false);
        if ( pbAdvancedPerm)
            pbAdvancedPerm->setEnabled(false);
    }

    box->addStretch (10);
}

#ifdef HAVE_POSIX_ACL
static bool fileSystemSupportsACL( const QByteArray& path )
{
    bool fileSystemSupportsACLs = false;
#ifdef Q_OS_FREEBSD
    struct statfs buf;
    fileSystemSupportsACLs = ( statfs( path.data(), &buf ) == 0 ) && ( buf.f_flags & MNT_ACLS );
#else
    fileSystemSupportsACLs =
            getxattr( path.data(), "system.posix_acl_access", NULL, 0 ) >= 0 || errno == ENODATA;
#endif
    return fileSystemSupportsACLs;
}
#endif


void KFilePermissionsPropsPlugin::slotShowAdvancedPermissions() {

    bool isDir = (d->pmode == PermissionsOnlyDirs) || (d->pmode == PermissionsMixed);
    KDialog dlg( properties );
    dlg.setModal( true );
    dlg.setCaption( i18n("Advanced Permissions") );
    dlg.setButtons( KDialog::Ok | KDialog::Cancel );

    QLabel *l, *cl[3];
    QGroupBox *gb;
    QGridLayout *gl;

    QWidget *mainw = new QWidget( &dlg );
    QVBoxLayout *vbox = new QVBoxLayout(mainw);
    // Group: Access Permissions
    gb = new QGroupBox ( i18n("Access Permissions"), mainw );
    vbox->addWidget(gb);

    gl = new QGridLayout (gb);
    gl->addItem(new QSpacerItem(0, 10), 0, 0);

    QVector<QWidget*> theNotSpecials;

    l = new QLabel(i18n("Class"), gb );
    gl->addWidget(l, 1, 0);
    theNotSpecials.append( l );

    if (isDir)
        l = new QLabel( i18n("Show\nEntries"), gb );
    else
        l = new QLabel( i18n("Read"), gb );
    gl->addWidget (l, 1, 1);
    theNotSpecials.append( l );
    QString readWhatsThis;
    if (isDir)
        readWhatsThis = i18n("This flag allows viewing the content of the folder.");
    else
        readWhatsThis = i18n("The Read flag allows viewing the content of the file.");
    l->setWhatsThis(readWhatsThis);

    if (isDir)
        l = new QLabel( i18n("Write\nEntries"), gb );
    else
        l = new QLabel( i18n("Write"), gb );
    gl->addWidget (l, 1, 2);
    theNotSpecials.append( l );
    QString writeWhatsThis;
    if (isDir)
        writeWhatsThis = i18n("This flag allows adding, renaming and deleting of files. "
                              "Note that deleting and renaming can be limited using the Sticky flag.");
    else
        writeWhatsThis = i18n("The Write flag allows modifying the content of the file.");
    l->setWhatsThis(writeWhatsThis);

    QString execWhatsThis;
    if (isDir) {
        l = new QLabel( i18nc("Enter folder", "Enter"), gb );
        execWhatsThis = i18n("Enable this flag to allow entering the folder.");
    }
    else {
        l = new QLabel( i18n("Exec"), gb );
        execWhatsThis = i18n("Enable this flag to allow executing the file as a program.");
    }
    l->setWhatsThis(execWhatsThis);
    theNotSpecials.append( l );
    // GJ: Add space between normal and special modes
    QSize size = l->sizeHint();
    size.setWidth(size.width() + 15);
    l->setFixedSize(size);
    gl->addWidget (l, 1, 3);

    l = new QLabel( i18n("Special"), gb );
    gl->addWidget(l, 1, 4, 1, 2);
    QString specialWhatsThis;
    if (isDir)
        specialWhatsThis = i18n("Special flag. Valid for the whole folder, the exact "
                                "meaning of the flag can be seen in the right hand column.");
    else
        specialWhatsThis = i18n("Special flag. The exact meaning of the flag can be seen "
                                "in the right hand column.");
    l->setWhatsThis(specialWhatsThis);

    cl[0] = new QLabel( i18n("User"), gb );
    gl->addWidget (cl[0], 2, 0);
    theNotSpecials.append( cl[0] );

    cl[1] = new QLabel( i18n("Group"), gb );
    gl->addWidget (cl[1], 3, 0);
    theNotSpecials.append( cl[1] );

    cl[2] = new QLabel( i18n("Others"), gb );
    gl->addWidget (cl[2], 4, 0);
    theNotSpecials.append( cl[2] );

    l = new QLabel(i18n("Set UID"), gb);
    gl->addWidget(l, 2, 5);
    QString setUidWhatsThis;
    if (isDir)
        setUidWhatsThis = i18n("If this flag is set, the owner of this folder will be "
                               "the owner of all new files.");
    else
        setUidWhatsThis = i18n("If this file is an executable and the flag is set, it will "
                               "be executed with the permissions of the owner.");
    l->setWhatsThis(setUidWhatsThis);

    l = new QLabel(i18n("Set GID"), gb);
    gl->addWidget(l, 3, 5);
    QString setGidWhatsThis;
    if (isDir)
        setGidWhatsThis = i18n("If this flag is set, the group of this folder will be "
                               "set for all new files.");
    else
        setGidWhatsThis = i18n("If this file is an executable and the flag is set, it will "
                               "be executed with the permissions of the group.");
    l->setWhatsThis(setGidWhatsThis);

    l = new QLabel(i18nc("File permission", "Sticky"), gb);
    gl->addWidget(l, 4, 5);
    QString stickyWhatsThis;
    if (isDir)
        stickyWhatsThis = i18n("If the Sticky flag is set on a folder, only the owner "
                               "and root can delete or rename files. Otherwise everybody "
                               "with write permissions can do this.");
    else
        stickyWhatsThis = i18n("The Sticky flag on a file is ignored on Linux, but may "
                               "be used on some systems");
    l->setWhatsThis(stickyWhatsThis);

    mode_t aPermissions, aPartialPermissions;
    mode_t dummy1, dummy2;

    if (!d->isIrregular) {
        switch (d->pmode) {
        case PermissionsOnlyFiles:
            getPermissionMasks(aPartialPermissions,
                               dummy1,
                               aPermissions,
                               dummy2);
            break;
        case PermissionsOnlyDirs:
        case PermissionsMixed:
            getPermissionMasks(dummy1,
                               aPartialPermissions,
                               dummy2,
                               aPermissions);
            break;
        case PermissionsOnlyLinks:
            aPermissions = UniRead | UniWrite | UniExec | UniSpecial;
            aPartialPermissions = 0;
            break;
        }
    }
    else {
        aPermissions = d->permissions;
        aPartialPermissions = d->partialPermissions;
    }

    // Draw Checkboxes
    QCheckBox *cba[3][4];
    for (int row = 0; row < 3 ; ++row) {
        for (int col = 0; col < 4; ++col) {
            QCheckBox *cb = new QCheckBox(gb);
            if ( col != 3 ) theNotSpecials.append( cb );
            cba[row][col] = cb;
            cb->setChecked(aPermissions & fperm[row][col]);
            if ( aPartialPermissions & fperm[row][col] )
            {
                cb->setTristate();
                cb->setCheckState(Qt::PartiallyChecked);
            }
            else if (d->cbRecursive && d->cbRecursive->isChecked())
                cb->setTristate();

            cb->setEnabled( d->canChangePermissions );
            gl->addWidget (cb, row+2, col+1);
            switch(col) {
            case 0:
                cb->setWhatsThis(readWhatsThis);
                break;
            case 1:
                cb->setWhatsThis(writeWhatsThis);
                break;
            case 2:
                cb->setWhatsThis(execWhatsThis);
                break;
            case 3:
                switch(row) {
                case 0:
                    cb->setWhatsThis(setUidWhatsThis);
                    break;
                case 1:
                    cb->setWhatsThis(setGidWhatsThis);
                    break;
                case 2:
                    cb->setWhatsThis(stickyWhatsThis);
                    break;
                }
                break;
            }
        }
    }
    gl->setColumnStretch(6, 10);

#ifdef HAVE_POSIX_ACL
    KACLEditWidget *extendedACLs = 0;

    // FIXME make it work with partial entries
    if ( properties->items().count() == 1 ) {
        QByteArray path = QFile::encodeName( properties->item().url().toLocalFile() );
        d->fileSystemSupportsACLs = fileSystemSupportsACL( path );
    }
    if ( d->fileSystemSupportsACLs  ) {
        std::for_each( theNotSpecials.begin(), theNotSpecials.end(), std::mem_fun( &QWidget::hide ) );
        extendedACLs = new KACLEditWidget( mainw );
        vbox->addWidget(extendedACLs);
        if ( d->extendedACL.isValid() && d->extendedACL.isExtended() )
            extendedACLs->setACL( d->extendedACL );
        else
            extendedACLs->setACL( KACL( aPermissions ) );

        if ( d->defaultACL.isValid() )
            extendedACLs->setDefaultACL( d->defaultACL );

        if ( properties->items().first().isDir() )
            extendedACLs->setAllowDefaults( true );
    }
#endif
    dlg.setMainWidget( mainw );
    if (dlg.exec() != KDialog::Accepted)
        return;

    mode_t andPermissions = mode_t(~0);
    mode_t orPermissions = 0;
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 4; ++col) {
        switch (cba[row][col]->checkState())
        {
        case Qt::Checked:
            orPermissions |= fperm[row][col];
            //fall through
        case Qt::Unchecked:
            andPermissions &= ~fperm[row][col];
            break;
        default: // NoChange
            break;
        }
    }

    d->isIrregular = false;
    const KFileItemList items = properties->items();
    KFileItemList::const_iterator it = items.begin();
    const KFileItemList::const_iterator kend = items.end();
    for ( ; it != kend; ++it ) {
        if (isIrregular(((*it).permissions() & andPermissions) | orPermissions,
                        (*it).isDir(), (*it).isLink())) {
            d->isIrregular = true;
            break;
        }
    }

    d->permissions = orPermissions;
    d->partialPermissions = andPermissions;

#ifdef HAVE_POSIX_ACL
    // override with the acls, if present
    if ( extendedACLs ) {
        d->extendedACL = extendedACLs->getACL();
        d->defaultACL = extendedACLs->getDefaultACL();
        d->hasExtendedACL = d->extendedACL.isExtended() || d->defaultACL.isValid();
        d->permissions = d->extendedACL.basePermissions();
        d->permissions |= ( andPermissions | orPermissions ) & ( S_ISUID|S_ISGID|S_ISVTX );
    }
#endif

    updateAccessControls();
    emit changed();
}

// QString KFilePermissionsPropsPlugin::tabName () const
// {
//   return i18n ("&Permissions");
// }

KFilePermissionsPropsPlugin::~KFilePermissionsPropsPlugin()
{
    delete d;
}

bool KFilePermissionsPropsPlugin::supports( const KFileItemList& /*_items*/ )
{
    return true;
}

// sets a combo box in the Access Control frame
void KFilePermissionsPropsPlugin::setComboContent(QComboBox *combo, PermissionsTarget target,
                                                  mode_t permissions, mode_t partial) {
    combo->clear();
    if (d->isIrregular) //#176876
        return;

    if (d->pmode == PermissionsOnlyLinks) {
        combo->addItem(i18n("Link"));
        combo->setCurrentIndex(0);
        return;
    }

    mode_t tMask = permissionsMasks[target];
    int textIndex;
    for (textIndex = 0; standardPermissions[textIndex] != (mode_t)-1; textIndex++) {
        if ((standardPermissions[textIndex]&tMask) == (permissions&tMask&(UniRead|UniWrite)))
            break;
    }
    Q_ASSERT(standardPermissions[textIndex] != (mode_t)-1); // must not happen, would be irreglar

    for (int i = 0; permissionsTexts[(int)d->pmode][i]; i++)
        combo->addItem(i18n(permissionsTexts[(int)d->pmode][i]));

    if (partial & tMask & ~UniExec) {
        combo->addItem(i18n("Varying (No Change)"));
        combo->setCurrentIndex(3);
    }
    else {
        combo->setCurrentIndex(textIndex);
    }
}

// permissions are irregular if they cant be displayed in a combo box.
bool KFilePermissionsPropsPlugin::isIrregular(mode_t permissions, bool isDir, bool isLink) {
    if (isLink)                             // links are always ok
        return false;

    mode_t p = permissions;
    if (p & (S_ISUID | S_ISGID))  // setuid/setgid -> irregular
        return true;
    if (isDir) {
        p &= ~S_ISVTX;          // ignore sticky on dirs

        // check supported flag combinations
        mode_t p0 = p & UniOwner;
        if ((p0 != 0) && (p0 != (S_IRUSR | S_IXUSR)) && (p0 != UniOwner))
            return true;
        p0 = p & UniGroup;
        if ((p0 != 0) && (p0 != (S_IRGRP | S_IXGRP)) && (p0 != UniGroup))
            return true;
        p0 = p & UniOthers;
        if ((p0 != 0) && (p0 != (S_IROTH | S_IXOTH)) && (p0 != UniOthers))
            return true;
        return false;
    }
    if (p & S_ISVTX) // sticky on file -> irregular
        return true;

    // check supported flag combinations
    mode_t p0 = p & UniOwner;
    bool usrXPossible = !p0; // true if this file could be an executable
    if (p0 & S_IXUSR) {
        if ((p0 == S_IXUSR) || (p0 == (S_IWUSR | S_IXUSR)))
            return true;
        usrXPossible = true;
    }
    else if (p0 == S_IWUSR)
        return true;

    p0 = p & UniGroup;
    bool grpXPossible = !p0; // true if this file could be an executable
    if (p0 & S_IXGRP) {
        if ((p0 == S_IXGRP) || (p0 == (S_IWGRP | S_IXGRP)))
            return true;
        grpXPossible = true;
    }
    else if (p0 == S_IWGRP)
        return true;
    if (p0 == 0)
        grpXPossible = true;

    p0 = p & UniOthers;
    bool othXPossible = !p0; // true if this file could be an executable
    if (p0 & S_IXOTH) {
        if ((p0 == S_IXOTH) || (p0 == (S_IWOTH | S_IXOTH)))
            return true;
        othXPossible = true;
    }
    else if (p0 == S_IWOTH)
        return true;

    // check that there either all targets are executable-compatible, or none
    return (p & UniExec) && !(usrXPossible && grpXPossible && othXPossible);
}

// enables/disabled the widgets in the Access Control frame
void KFilePermissionsPropsPlugin::enableAccessControls(bool enable) {
    d->ownerPermCombo->setEnabled(enable);
    d->groupPermCombo->setEnabled(enable);
    d->othersPermCombo->setEnabled(enable);
    if (d->extraCheckbox)
        d->extraCheckbox->setEnabled(enable);
    if ( d->cbRecursive )
        d->cbRecursive->setEnabled(enable);
}

// updates all widgets in the Access Control frame
void KFilePermissionsPropsPlugin::updateAccessControls() {
    setComboContent(d->ownerPermCombo, PermissionsOwner,
                    d->permissions, d->partialPermissions);
    setComboContent(d->groupPermCombo, PermissionsGroup,
                    d->permissions, d->partialPermissions);
    setComboContent(d->othersPermCombo, PermissionsOthers,
                    d->permissions, d->partialPermissions);

    switch(d->pmode) {
    case PermissionsOnlyLinks:
        enableAccessControls(false);
        break;
    case PermissionsOnlyFiles:
        enableAccessControls(d->canChangePermissions && !d->isIrregular && !d->hasExtendedACL);
        if (d->canChangePermissions)
            d->explanationLabel->setText(d->isIrregular || d->hasExtendedACL ?
                                         i18np("This file uses advanced permissions",
                                               "These files use advanced permissions.",
                                               properties->items().count()) : "");
        if (d->partialPermissions & UniExec) {
            d->extraCheckbox->setTristate();
            d->extraCheckbox->setCheckState(Qt::PartiallyChecked);
        }
        else {
            d->extraCheckbox->setTristate(false);
            d->extraCheckbox->setChecked(d->permissions & UniExec);
        }
        break;
    case PermissionsOnlyDirs:
        enableAccessControls(d->canChangePermissions && !d->isIrregular && !d->hasExtendedACL);
        // if this is a dir, and we can change permissions, don't dis-allow
        // recursive, we can do that for ACL setting.
        if ( d->cbRecursive )
            d->cbRecursive->setEnabled( d->canChangePermissions && !d->isIrregular );

        if (d->canChangePermissions)
            d->explanationLabel->setText(d->isIrregular || d->hasExtendedACL ?
                                         i18np("This folder uses advanced permissions.",
                                               "These folders use advanced permissions.",
                                               properties->items().count()) : "");
        if (d->partialPermissions & S_ISVTX) {
            d->extraCheckbox->setTristate();
            d->extraCheckbox->setCheckState(Qt::PartiallyChecked);
        }
        else {
            d->extraCheckbox->setTristate(false);
            d->extraCheckbox->setChecked(d->permissions & S_ISVTX);
        }
        break;
    case PermissionsMixed:
        enableAccessControls(d->canChangePermissions && !d->isIrregular && !d->hasExtendedACL);
        if (d->canChangePermissions)
            d->explanationLabel->setText(d->isIrregular || d->hasExtendedACL ?
                                         i18n("These files use advanced permissions.") : "");
        break;
        if (d->partialPermissions & S_ISVTX) {
            d->extraCheckbox->setTristate();
            d->extraCheckbox->setCheckState(Qt::PartiallyChecked);
        }
        else {
            d->extraCheckbox->setTristate(false);
            d->extraCheckbox->setChecked(d->permissions & S_ISVTX);
        }
        break;
    }
}

// gets masks for files and dirs from the Access Control frame widgets
void KFilePermissionsPropsPlugin::getPermissionMasks(mode_t &andFilePermissions,
                                                     mode_t &andDirPermissions,
                                                     mode_t &orFilePermissions,
                                                     mode_t &orDirPermissions) {
    andFilePermissions = mode_t(~UniSpecial);
    andDirPermissions = mode_t(~(S_ISUID|S_ISGID));
    orFilePermissions = 0;
    orDirPermissions = 0;
    if (d->isIrregular)
        return;

    mode_t m = standardPermissions[d->ownerPermCombo->currentIndex()];
    if (m != (mode_t) -1) {
        orFilePermissions |= m & UniOwner;
        if ((m & UniOwner) &&
            ((d->pmode == PermissionsMixed) ||
             ((d->pmode == PermissionsOnlyFiles) && (d->extraCheckbox->checkState() == Qt::PartiallyChecked))))
            andFilePermissions &= ~(S_IRUSR | S_IWUSR);
        else {
            andFilePermissions &= ~(S_IRUSR | S_IWUSR | S_IXUSR);
            if ((m & S_IRUSR) && (d->extraCheckbox->checkState() == Qt::Checked))
                orFilePermissions |= S_IXUSR;
        }

        orDirPermissions |= m & UniOwner;
        if (m & S_IRUSR)
            orDirPermissions |= S_IXUSR;
        andDirPermissions &= ~(S_IRUSR | S_IWUSR | S_IXUSR);
    }

    m = standardPermissions[d->groupPermCombo->currentIndex()];
    if (m != (mode_t) -1) {
        orFilePermissions |= m & UniGroup;
        if ((m & UniGroup) &&
            ((d->pmode == PermissionsMixed) ||
             ((d->pmode == PermissionsOnlyFiles) && (d->extraCheckbox->checkState() == Qt::PartiallyChecked))))
            andFilePermissions &= ~(S_IRGRP | S_IWGRP);
        else {
            andFilePermissions &= ~(S_IRGRP | S_IWGRP | S_IXGRP);
            if ((m & S_IRGRP) && (d->extraCheckbox->checkState() == Qt::Checked))
                orFilePermissions |= S_IXGRP;
        }

        orDirPermissions |= m & UniGroup;
        if (m & S_IRGRP)
            orDirPermissions |= S_IXGRP;
        andDirPermissions &= ~(S_IRGRP | S_IWGRP | S_IXGRP);
    }

    m = d->othersPermCombo->currentIndex() >= 0 ? standardPermissions[d->othersPermCombo->currentIndex()] : (mode_t)-1;
    if (m != (mode_t) -1) {
        orFilePermissions |= m & UniOthers;
        if ((m & UniOthers) &&
            ((d->pmode == PermissionsMixed) ||
             ((d->pmode == PermissionsOnlyFiles) && (d->extraCheckbox->checkState() == Qt::PartiallyChecked))))
            andFilePermissions &= ~(S_IROTH | S_IWOTH);
        else {
            andFilePermissions &= ~(S_IROTH | S_IWOTH | S_IXOTH);
            if ((m & S_IROTH) && (d->extraCheckbox->checkState() == Qt::Checked))
                orFilePermissions |= S_IXOTH;
        }

        orDirPermissions |= m & UniOthers;
        if (m & S_IROTH)
            orDirPermissions |= S_IXOTH;
        andDirPermissions &= ~(S_IROTH | S_IWOTH | S_IXOTH);
    }

    if (((d->pmode == PermissionsMixed) || (d->pmode == PermissionsOnlyDirs)) &&
        (d->extraCheckbox->checkState() != Qt::PartiallyChecked)) {
        andDirPermissions &= ~S_ISVTX;
        if (d->extraCheckbox->checkState() == Qt::Checked)
            orDirPermissions |= S_ISVTX;
    }
}

void KFilePermissionsPropsPlugin::applyChanges()
{
    mode_t orFilePermissions;
    mode_t orDirPermissions;
    mode_t andFilePermissions;
    mode_t andDirPermissions;

    if (!d->canChangePermissions)
        return;

    if (!d->isIrregular)
        getPermissionMasks(andFilePermissions,
                           andDirPermissions,
                           orFilePermissions,
                           orDirPermissions);
    else {
        orFilePermissions = d->permissions;
        andFilePermissions = d->partialPermissions;
        orDirPermissions = d->permissions;
        andDirPermissions = d->partialPermissions;
    }

    QString owner, group;
    if (d->usrEdit)
        owner = d->usrEdit->text();
    if (d->grpEdit)
        group = d->grpEdit->text();
    else if (d->grpCombo)
        group = d->grpCombo->currentText();

    if (owner == d->strOwner)
        owner.clear(); // no change

    if (group == d->strGroup)
        group.clear();

    bool recursive = d->cbRecursive && d->cbRecursive->isChecked();
    bool permissionChange = false;

    KFileItemList files, dirs;
    const KFileItemList items = properties->items();
    KFileItemList::const_iterator it = items.begin();
    const KFileItemList::const_iterator kend = items.end();
    for ( ; it != kend; ++it ) {
        if ((*it).isDir()) {
            dirs.append(*it);
            if ((*it).permissions() != (((*it).permissions() & andDirPermissions) | orDirPermissions))
                permissionChange = true;
        }
        else if ((*it).isFile()) {
            files.append(*it);
            if ((*it).permissions() != (((*it).permissions() & andFilePermissions) | orFilePermissions))
                permissionChange = true;
        }
    }

    const bool ACLChange = ( d->extendedACL !=  properties->item().ACL() );
    const bool defaultACLChange = ( d->defaultACL != properties->item().defaultACL() );

    if (owner.isEmpty() && group.isEmpty() && !recursive
        && !permissionChange && !ACLChange && !defaultACLChange)
        return;

    KIO::Job * job;
    if (files.count() > 0) {
        job = KIO::chmod( files, orFilePermissions, ~andFilePermissions,
                          owner, group, false );
        if ( ACLChange && d->fileSystemSupportsACLs )
            job->addMetaData( "ACL_STRING", d->extendedACL.isValid()?d->extendedACL.asString():"ACL_DELETE" );
        if ( defaultACLChange && d->fileSystemSupportsACLs )
            job->addMetaData( "DEFAULT_ACL_STRING", d->defaultACL.isValid()?d->defaultACL.asString():"ACL_DELETE" );

        connect( job, SIGNAL( result( KJob * ) ),
                 SLOT( slotChmodResult( KJob * ) ) );
        QEventLoop eventLoop;
        connect(this, SIGNAL(leaveModality()),
                &eventLoop, SLOT(quit()));
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    }
    if (dirs.count() > 0) {
        job = KIO::chmod( dirs, orDirPermissions, ~andDirPermissions,
                          owner, group, recursive );
        if ( ACLChange && d->fileSystemSupportsACLs )
            job->addMetaData( "ACL_STRING", d->extendedACL.isValid()?d->extendedACL.asString():"ACL_DELETE" );
        if ( defaultACLChange && d->fileSystemSupportsACLs )
            job->addMetaData( "DEFAULT_ACL_STRING", d->defaultACL.isValid()?d->defaultACL.asString():"ACL_DELETE" );

        connect( job, SIGNAL( result( KJob * ) ),
                 SLOT( slotChmodResult( KJob * ) ) );
        QEventLoop eventLoop;
        connect(this, SIGNAL(leaveModality()),
                &eventLoop, SLOT(quit()));
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    }
}

void KFilePermissionsPropsPlugin::slotChmodResult( KJob * job )
{
    kDebug(250) << "KFilePermissionsPropsPlugin::slotChmodResult";
    if (job->error())
        job->uiDelegate()->showErrorMessage();
    // allow apply() to return
    emit leaveModality();
}




class KUrlPropsPlugin::KUrlPropsPluginPrivate
{
public:
    KUrlPropsPluginPrivate()
    {
    }
    ~KUrlPropsPluginPrivate()
    {
    }

    QFrame *m_frame;
    KUrlRequester *URLEdit;
    QString URLStr;
};

KUrlPropsPlugin::KUrlPropsPlugin( KPropertiesDialog *_props )
    : KPropertiesDialogPlugin( _props ),d(new KUrlPropsPluginPrivate)
{
    d->m_frame = new QFrame();
    properties->addPage(d->m_frame, i18n("U&RL"));
    QVBoxLayout *layout = new QVBoxLayout(d->m_frame);
    layout->setMargin(0);

    QLabel *l;
    l = new QLabel( d->m_frame );
    l->setObjectName( QLatin1String( "Label_1" ) );
    l->setText( i18n("URL:") );
    layout->addWidget(l, Qt::AlignRight);

    d->URLEdit = new KUrlRequester( d->m_frame );
    layout->addWidget(d->URLEdit);

    KUrl url = KIO::NetAccess::mostLocalUrl( properties->kurl(), properties );
    if (url.isLocalFile()) {
        QString path = url.toLocalFile();

        QFile f( path );
        if ( !f.open( QIODevice::ReadOnly ) ) {
            return;
        }
        f.close();

        KDesktopFile config( path );
        const KConfigGroup dg = config.desktopGroup();
        d->URLStr = dg.readPathEntry( "URL", QString() );

        if (!d->URLStr.isEmpty()) {
            d->URLEdit->setUrl( KUrl(d->URLStr) );
        }
    }

    connect( d->URLEdit, SIGNAL( textChanged( const QString & ) ),
             this, SIGNAL( changed() ) );

    layout->addStretch (1);
}

KUrlPropsPlugin::~KUrlPropsPlugin()
{
    delete d;
}

// QString KUrlPropsPlugin::tabName () const
// {
//   return i18n ("U&RL");
// }

bool KUrlPropsPlugin::supports( const KFileItemList& _items )
{
    if ( _items.count() != 1 )
        return false;
    const KFileItem item = _items.first();
    // check if desktop file
    if (!item.isDesktopFile())
        return false;

    // open file and check type
    bool isLocal;
    KUrl url = item.mostLocalUrl(isLocal);
    if (!isLocal) {
        return false;
    }

    KDesktopFile config( url.path() );
    return config.hasLinkType();
}

void KUrlPropsPlugin::applyChanges()
{
    KUrl url = KIO::NetAccess::mostLocalUrl( properties->kurl(), properties );
    if (!url.isLocalFile()) {
        //FIXME: 4.2 add this: KMessageBox::sorry(0, i18n("Could not save properties. Only entries on local file systems are supported."));
        return;
    }

    QString path = url.path();

    QFile f( path );
    if ( !f.open( QIODevice::ReadWrite ) ) {
        KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have "
                                    "sufficient access to write to <b>%1</b>.</qt>", path));
        return;
    }
    f.close();

    KDesktopFile config( path );
    KConfigGroup dg = config.desktopGroup();
    dg.writeEntry( "Type", QString::fromLatin1("Link"));
    dg.writePathEntry( "URL", d->URLEdit->url().url() );
    // Users can't create a Link .desktop file with a Name field,
    // but distributions can. Update the Name field in that case.
    if ( dg.hasKey("Name") )
    {
        QString nameStr = nameFromFileName(properties->kurl().fileName());
        dg.writeEntry( "Name", nameStr );
        dg.writeEntry( "Name", nameStr, KConfigBase::Persistent|KConfigBase::Localized );

    }
}


/* ----------------------------------------------------
 *
 * KDevicePropsPlugin
 *
 * -------------------------------------------------- */

class KDevicePropsPlugin::KDevicePropsPluginPrivate
{
public:
    KDevicePropsPluginPrivate()
    {
    }
    ~KDevicePropsPluginPrivate()
    {
    }

    QFrame *m_frame;
    QStringList mountpointlist;
    QLabel *m_freeSpaceText;
    QLabel *m_freeSpaceLabel;
    QProgressBar *m_freeSpaceBar;

    KComboBox* device;
    QLabel* mountpoint;
    QCheckBox* readonly;
    KIconButton* unmounted;

    QStringList m_devicelist;
};

KDevicePropsPlugin::KDevicePropsPlugin( KPropertiesDialog *_props ) : KPropertiesDialogPlugin( _props ),d(new KDevicePropsPluginPrivate)
{
    d->m_frame = new QFrame();
    properties->addPage(d->m_frame, i18n("De&vice"));

    QStringList devices;
    const KMountPoint::List mountPoints = KMountPoint::possibleMountPoints();

    for(KMountPoint::List::ConstIterator it = mountPoints.begin();
    it != mountPoints.end(); ++it)
    {
        const KMountPoint::Ptr mp = (*it);
        QString mountPoint = mp->mountPoint();
        QString device = mp->mountedFrom();
        kDebug()<<"mountPoint :"<<mountPoint<<" device :"<<device<<" mp->mountType() :"<<mp->mountType();

        if ((mountPoint != "-") && (mountPoint != "none") && !mountPoint.isEmpty()
            && device != "none")
            {
            devices.append( device + QString::fromLatin1(" (")
                            + mountPoint + QString::fromLatin1(")") );
            d->m_devicelist.append(device);
            d->mountpointlist.append(mountPoint);
        }
    }

    QGridLayout *layout = new QGridLayout( d->m_frame );

    layout->setMargin(0);
    layout->setColumnStretch(1, 1);

    QLabel* label;
    label = new QLabel( d->m_frame );
    label->setText( devices.count() == 0 ?
                    i18n("Device (/dev/fd0):") : // old style
                    i18n("Device:") ); // new style (combobox)
    layout->addWidget(label, 0, 0, Qt::AlignRight);

    d->device = new KComboBox( d->m_frame );
    d->device->setObjectName( QLatin1String( "ComboBox_device" ) );
    d->device->setEditable( true );
    d->device->addItems( devices );
    layout->addWidget(d->device, 0, 1);
    connect( d->device, SIGNAL( activated( int ) ),
             this, SLOT( slotActivated( int ) ) );

    d->readonly = new QCheckBox( d->m_frame );
    d->readonly->setObjectName( QLatin1String( "CheckBox_readonly" ) );
    d->readonly->setText(  i18n("Read only") );
    layout->addWidget(d->readonly, 1, 1);

    label = new QLabel( d->m_frame );
    label->setText( i18n("File system:") );
    layout->addWidget(label, 2, 0, Qt::AlignRight);

    QLabel *fileSystem = new QLabel( d->m_frame );
    layout->addWidget(fileSystem, 2, 1);

    label = new QLabel( d->m_frame );
    label->setText( devices.count()==0 ?
                    i18n("Mount point (/mnt/floppy):") : // old style
                    i18n("Mount point:")); // new style (combobox)
    layout->addWidget(label, 3, 0, Qt::AlignRight);

    d->mountpoint = new QLabel( d->m_frame );
    d->mountpoint->setObjectName( QLatin1String( "LineEdit_mountpoint" ) );

    layout->addWidget(d->mountpoint, 3, 1);

    // show disk free
    d->m_freeSpaceText = new QLabel(i18n("Device usage:"), d->m_frame );
    layout->addWidget(d->m_freeSpaceText, 4, 0, Qt::AlignRight);

    d->m_freeSpaceLabel = new QLabel( d->m_frame );
    layout->addWidget( d->m_freeSpaceLabel, 4, 1 );

    d->m_freeSpaceBar = new QProgressBar( d->m_frame );
    d->m_freeSpaceBar->setObjectName( "freeSpaceBar" );
    layout->addWidget(d->m_freeSpaceBar, 5, 0, 1, 2);

    // we show it in the slot when we know the values
    d->m_freeSpaceText->hide();
    d->m_freeSpaceLabel->hide();
    d->m_freeSpaceBar->hide();

    KSeparator* sep = new KSeparator( Qt::Horizontal, d->m_frame);
    layout->addWidget(sep, 6, 0, 1, 2);

    d->unmounted = new KIconButton( d->m_frame );
    int bsize = 66 + 2 * d->unmounted->style()->pixelMetric(QStyle::PM_ButtonMargin);
    d->unmounted->setFixedSize(bsize, bsize);
    d->unmounted->setIconType(KIconLoader::Desktop, KIconLoader::Device);
    layout->addWidget(d->unmounted, 7, 0);

    label = new QLabel( i18n("Unmounted Icon"),  d->m_frame );
    layout->addWidget(label, 7, 1);

    layout->setRowStretch(8, 1);

    KUrl url = KIO::NetAccess::mostLocalUrl( _props->kurl(), _props );
    if (!url.isLocalFile()) {
        return;
    }
    QString path = url.toLocalFile();

    QFile f( path );
    if ( !f.open( QIODevice::ReadOnly ) )
        return;
    f.close();

    const KDesktopFile _config( path );
    const KConfigGroup config = _config.desktopGroup();
    QString deviceStr = config.readEntry( "Dev" );
    QString mountPointStr = config.readEntry( "MountPoint" );
    bool ro = config.readEntry( "ReadOnly", false );
    QString unmountedStr = config.readEntry( "UnmountIcon" );

    fileSystem->setText(config.readEntry("FSType"));

    d->device->setEditText( deviceStr );
    if ( !deviceStr.isEmpty() ) {
        // Set default options for this device (first matching entry)
        int index = d->m_devicelist.indexOf(deviceStr);
        if (index != -1)
        {
            //kDebug(250) << "found it" << index;
            slotActivated( index );
        }
    }

    if ( !mountPointStr.isEmpty() )
    {
        d->mountpoint->setText( mountPointStr );
        updateInfo();
    }

    d->readonly->setChecked( ro );

    if ( unmountedStr.isEmpty() )
        unmountedStr = KMimeType::defaultMimeTypePtr()->iconName(); // default icon

    d->unmounted->setIcon( unmountedStr );

    connect( d->device, SIGNAL( activated( int ) ),
             this, SIGNAL( changed() ) );
    connect( d->device, SIGNAL( textChanged( const QString & ) ),
             this, SIGNAL( changed() ) );
    connect( d->readonly, SIGNAL( toggled( bool ) ),
             this, SIGNAL( changed() ) );
    connect( d->unmounted, SIGNAL( iconChanged( const QString& ) ),
             this, SIGNAL( changed() ) );

    connect( d->device, SIGNAL( textChanged( const QString & ) ),
             this, SLOT( slotDeviceChanged() ) );
}

KDevicePropsPlugin::~KDevicePropsPlugin()
{
    delete d;
}

// QString KDevicePropsPlugin::tabName () const
// {
//   return i18n ("De&vice");
// }

void KDevicePropsPlugin::updateInfo()
{
    // we show it in the slot when we know the values
    d->m_freeSpaceText->hide();
    d->m_freeSpaceLabel->hide();
    d->m_freeSpaceBar->hide();

    if ( !d->mountpoint->text().isEmpty() )
    {
        KDiskFreeSpaceInfo info = KDiskFreeSpaceInfo::freeSpaceInfo( d->mountpoint->text() );
        slotFoundMountPoint( info.mountPoint(), info.size()/1024, info.used()/1024, info.available()/1024);
    }
}

void KDevicePropsPlugin::slotActivated( int index )
{
    // index can be more than the number of known devices, when the user types
    // a "custom" device.
    if (index < d->m_devicelist.count()) {
        // Update mountpoint so that it matches the device that was selected in the combo
        d->device->setEditText(d->m_devicelist[index]);
        d->mountpoint->setText(d->mountpointlist[index]);
    }

    updateInfo();
}

void KDevicePropsPlugin::slotDeviceChanged()
{
    // Update mountpoint so that it matches the typed device
    int index = d->m_devicelist.indexOf( d->device->currentText() );
    if ( index != -1 )
        d->mountpoint->setText( d->mountpointlist[index] );
    else
        d->mountpoint->setText( QString() );

    updateInfo();
}

void KDevicePropsPlugin::slotFoundMountPoint( const QString&,
                                              quint64 kibSize,
                                              quint64 /*kibUsed*/,
                                              quint64 kibAvail )
{
    d->m_freeSpaceText->show();
    d->m_freeSpaceLabel->show();

    int percUsed = 100 - (int)(100.0 * kibAvail / kibSize);

    d->m_freeSpaceLabel->setText(
            i18nc("Available space out of total partition size (percent used)", "%1 out of %2 (%3% used)",
                  KIO::convertSizeFromKiB(kibAvail),
                  KIO::convertSizeFromKiB(kibSize),
                  100 - (int)(100.0 * kibAvail / kibSize) ));

    d->m_freeSpaceBar->setRange(0, 100);
    d->m_freeSpaceBar->setValue(percUsed);
    d->m_freeSpaceBar->show();
}

bool KDevicePropsPlugin::supports( const KFileItemList& _items )
{
    if ( _items.count() != 1 )
        return false;
    const KFileItem item = _items.first();
    // check if desktop file
    if (!item.isDesktopFile())
        return false;

    // open file and check type
    bool isLocal;
    KUrl url = item.mostLocalUrl(isLocal);
    if (!isLocal) {
        return false;
    }

    KDesktopFile config( url.path() );
    return config.hasDeviceType();
}

void KDevicePropsPlugin::applyChanges()
{
    KUrl url = KIO::NetAccess::mostLocalUrl( properties->kurl(), properties );
    if ( !url.isLocalFile() )
        return;
    QString path = url.toLocalFile();

    QFile f( path );
    if ( !f.open( QIODevice::ReadWrite ) )
    {
        KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have sufficient "
                                    "access to write to <b>%1</b>.</qt>", path));
        return;
    }
    f.close();

    KDesktopFile _config( path );
    KConfigGroup config = _config.desktopGroup();
    config.writeEntry( "Type", QString::fromLatin1("FSDevice") );

    config.writeEntry( "Dev", d->device->currentText() );
    config.writeEntry( "MountPoint", d->mountpoint->text() );

    config.writeEntry( "UnmountIcon", d->unmounted->icon() );
    kDebug(250) << "d->unmounted->icon() = " << d->unmounted->icon();

    config.writeEntry( "ReadOnly", d->readonly->isChecked() );

    config.sync();
}


/* ----------------------------------------------------
 *
 * KDesktopPropsPlugin
 *
 * -------------------------------------------------- */

class KDesktopPropsPlugin::KDesktopPropsPluginPrivate
{
public:
    KDesktopPropsPluginPrivate()
        : w( new Ui_KPropertiesDesktopBase )
        , m_frame( new QFrame() )
    {
    }
    ~KDesktopPropsPluginPrivate()
    {
        delete w;
    }
    Ui_KPropertiesDesktopBase* w;
    QWidget *m_frame;

    QString m_origCommandStr;
    QString m_terminalOptionStr;
    QString m_suidUserStr;
    QString m_dbusStartupType;
    QString m_dbusServiceName;
    bool m_terminalBool;
    bool m_suidBool;
    bool m_startupBool;
    bool m_systrayBool;
};

KDesktopPropsPlugin::KDesktopPropsPlugin( KPropertiesDialog *_props )
    : KPropertiesDialogPlugin( _props ), d( new KDesktopPropsPluginPrivate )
{
    d->w->setupUi(d->m_frame);

    properties->addPage(d->m_frame, i18n("&Application"));

    bool bKDesktopMode = properties->kurl().protocol() == QLatin1String("desktop") ||
                    properties->currentDir().protocol() == QLatin1String("desktop");

    if (bKDesktopMode)
    {
        // Hide Name entry
        d->w->nameEdit->hide();
        d->w->nameLabel->hide();
    }

    d->w->pathEdit->setMode(KFile::Directory | KFile::LocalOnly);
    d->w->pathEdit->lineEdit()->setAcceptDrops(false);

    connect( d->w->nameEdit, SIGNAL( textChanged( const QString & ) ), this, SIGNAL( changed() ) );
    connect( d->w->genNameEdit, SIGNAL( textChanged( const QString & ) ), this, SIGNAL( changed() ) );
    connect( d->w->commentEdit, SIGNAL( textChanged( const QString & ) ), this, SIGNAL( changed() ) );
    connect( d->w->commandEdit, SIGNAL( textChanged( const QString & ) ), this, SIGNAL( changed() ) );
    connect( d->w->pathEdit, SIGNAL( textChanged( const QString & ) ), this, SIGNAL( changed() ) );

    connect( d->w->browseButton, SIGNAL( clicked() ), this, SLOT( slotBrowseExec() ) );
    connect( d->w->addFiletypeButton, SIGNAL( clicked() ), this, SLOT( slotAddFiletype() ) );
    connect( d->w->delFiletypeButton, SIGNAL( clicked() ), this, SLOT( slotDelFiletype() ) );
    connect( d->w->advancedButton, SIGNAL( clicked() ), this, SLOT( slotAdvanced() ) );

    // now populate the page

    KUrl url = KIO::NetAccess::mostLocalUrl( _props->kurl(), _props );
    if (!url.isLocalFile()) {
        return;
    }
    QString path = url.toLocalFile();

    QFile f( path );
    if ( !f.open( QIODevice::ReadOnly ) )
        return;
    f.close();

    KDesktopFile  _config( path );
    KConfigGroup config = _config.desktopGroup();
    QString nameStr = _config.readName();
    QString genNameStr = _config.readGenericName();
    QString commentStr = _config.readComment();
    QString commandStr = config.readEntry( "Exec", QString() );
    if (commandStr.startsWith(QLatin1String("ksystraycmd ")))
    {
        commandStr.remove(0, 12);
        d->m_systrayBool = true;
    }
    else
        d->m_systrayBool = false;

    d->m_origCommandStr = commandStr;
    QString pathStr = config.readEntry( "Path", QString() ); // not readPathEntry, see kservice.cpp
    d->m_terminalBool = config.readEntry( "Terminal", false );
    d->m_terminalOptionStr = config.readEntry( "TerminalOptions" );
    d->m_suidBool = config.readEntry( "X-KDE-SubstituteUID", false );
    d->m_suidUserStr = config.readEntry( "X-KDE-Username" );
    if( config.hasKey( "StartupNotify" ))
        d->m_startupBool = config.readEntry( "StartupNotify", true );
    else
        d->m_startupBool = config.readEntry( "X-KDE-StartupNotify", true );
    d->m_dbusStartupType = config.readEntry("X-DBUS-StartupType").toLower();
    // ### should there be a GUI for this setting?
    // At least we're copying it over to the local file, to avoid side effects (#157853)
    d->m_dbusServiceName = config.readEntry("X-DBUS-ServiceName");

    const QStringList mimeTypes = config.readXdgListEntry( "MimeType" );

    if ( nameStr.isEmpty() || bKDesktopMode ) {
        // We'll use the file name if no name is specified
        // because we _need_ a Name for a valid file.
        // But let's do it in apply, not here, so that we pick up the right name.
        setDirty();
    }
    if ( !bKDesktopMode )
        d->w->nameEdit->setText(nameStr);

    d->w->genNameEdit->setText( genNameStr );
    d->w->commentEdit->setText( commentStr );
    d->w->commandEdit->setText( commandStr );
    d->w->pathEdit->lineEdit()->setText( pathStr );

    // was: d->w->filetypeList->setFullWidth(true);
    //  d->w->filetypeList->header()->setStretchEnabled(true, d->w->filetypeList->columns()-1);

    KMimeType::Ptr defaultMimetype = KMimeType::defaultMimeTypePtr();
    for(QStringList::ConstIterator it = mimeTypes.begin();
    it != mimeTypes.end(); )
    {
        KMimeType::Ptr p = KMimeType::mimeType(*it, KMimeType::ResolveAliases);
        ++it;
        QString preference;
        if (it != mimeTypes.end())
        {
            bool numeric;
            (*it).toInt(&numeric);
            if (numeric)
            {
                preference = *it;
                ++it;
            }
        }
        if (p)
        {
            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(0, p->name());
            item->setText(1, p->comment());
            item->setText(2, preference);
            d->w->filetypeList->addTopLevelItem(item);
        }
    }
    d->w->filetypeList->resizeColumnToContents(0);

}

KDesktopPropsPlugin::~KDesktopPropsPlugin()
{
    delete d;
}

void KDesktopPropsPlugin::slotAddFiletype()
{
    KMimeTypeChooserDialog dlg( i18n("Add File Type for %1", properties->kurl().fileName()),
                                i18n("Select one or more file types to add:"),
                                QStringList(), // no preselected mimetypes
                                QString(),
                                QStringList(),
                                KMimeTypeChooser::Comments|KMimeTypeChooser::Patterns,
                                d->m_frame );

    if (dlg.exec() == KDialog::Accepted)
    {
        foreach(const QString &mimetype, dlg.chooser()->mimeTypes())
        {
            KMimeType::Ptr p = KMimeType::mimeType(mimetype);
            if (!p)
                continue;

            bool found = false;
            int count = d->w->filetypeList->topLevelItemCount();
            for (int i = 0; !found && i < count; ++i) {
                if (d->w->filetypeList->topLevelItem(i)->text(0) == mimetype) {
                    found = true;
                }
            }
            if (!found) {
                QTreeWidgetItem *item = new QTreeWidgetItem();
                item->setText(0, p->name());
                item->setText(1, p->comment());
                d->w->filetypeList->addTopLevelItem(item);
            }
            d->w->filetypeList->resizeColumnToContents(0);
        }
    }
    emit changed();
}

void KDesktopPropsPlugin::slotDelFiletype()
{
    QTreeWidgetItem *cur = d->w->filetypeList->currentItem();
    if (cur) {
        delete cur;
        emit changed();
    }
}

void KDesktopPropsPlugin::checkCommandChanged()
{
    if (KRun::binaryName(d->w->commandEdit->text(), true) !=
        KRun::binaryName(d->m_origCommandStr, true))
    {
        d->m_origCommandStr = d->w->commandEdit->text();
        d->m_dbusStartupType.clear(); // Reset
        d->m_dbusServiceName.clear();
    }
}

void KDesktopPropsPlugin::applyChanges()
{
    kDebug(250) << "KDesktopPropsPlugin::applyChanges";

    KUrl url = KIO::NetAccess::mostLocalUrl( properties->kurl(), properties );
    if (!url.isLocalFile()) {
        //FIXME: 4.2 add this: KMessageBox::sorry(0, i18n("Could not save properties. Only entries on local file systems are supported."));
        return;
    }
    QString path = url.toLocalFile();

    QFile f( path );

    if ( !f.open( QIODevice::ReadWrite ) ) {
        KMessageBox::sorry( 0, i18n("<qt>Could not save properties. You do not have "
                                    "sufficient access to write to <b>%1</b>.</qt>", path));
        return;
    }
    f.close();

    // If the command is changed we reset certain settings that are strongly
    // coupled to the command.
    checkCommandChanged();

    KDesktopFile _config( path );
    KConfigGroup config = _config.desktopGroup();
    config.writeEntry( "Type", QString::fromLatin1("Application"));
    config.writeEntry( "Comment", d->w->commentEdit->text() );
    config.writeEntry( "Comment", d->w->commentEdit->text(), KConfigGroup::Persistent|KConfigGroup::Localized ); // for compat
    config.writeEntry( "GenericName", d->w->genNameEdit->text() );
    config.writeEntry( "GenericName", d->w->genNameEdit->text(), KConfigGroup::Persistent|KConfigGroup::Localized ); // for compat

    if (d->m_systrayBool)
        config.writeEntry( "Exec", d->w->commandEdit->text().prepend("ksystraycmd ") );
    else
        config.writeEntry( "Exec", d->w->commandEdit->text() );
    config.writeEntry( "Path", d->w->pathEdit->lineEdit()->text() ); // not writePathEntry, see kservice.cpp

    // Write mimeTypes
    QStringList mimeTypes;
    int count = d->w->filetypeList->topLevelItemCount();
    for (int i = 0; i < count; ++i) {
        QTreeWidgetItem *item = d->w->filetypeList->topLevelItem(i);
        QString preference = item->text(2);
        mimeTypes.append(item->text(0));
        if (!preference.isEmpty())
            mimeTypes.append(preference);
    }

    kDebug() << mimeTypes;
    config.writeXdgListEntry( "MimeType", mimeTypes );

    if ( !d->w->nameEdit->isHidden() ) {
        QString nameStr = d->w->nameEdit->text();
        config.writeEntry( "Name", nameStr );
        config.writeEntry( "Name", nameStr, KConfigGroup::Persistent|KConfigGroup::Localized );
    }

    config.writeEntry("Terminal", d->m_terminalBool);
    config.writeEntry("TerminalOptions", d->m_terminalOptionStr);
    config.writeEntry("X-KDE-SubstituteUID", d->m_suidBool);
    config.writeEntry("X-KDE-Username", d->m_suidUserStr);
    config.writeEntry("StartupNotify", d->m_startupBool);
    config.writeEntry("X-DBUS-StartupType", d->m_dbusStartupType);
    config.writeEntry("X-DBUS-ServiceName", d->m_dbusServiceName);
    config.sync();

    // KSycoca update needed?
    QString sycocaPath = KGlobal::dirs()->relativeLocation("apps", path);
    bool updateNeeded = !sycocaPath.startsWith('/');
    if (!updateNeeded)
    {
        sycocaPath = KGlobal::dirs()->relativeLocation("xdgdata-apps", path);
        updateNeeded = !sycocaPath.startsWith('/');
    }
    if (updateNeeded)
        KBuildSycocaProgressDialog::rebuildKSycoca(d->m_frame);
}


void KDesktopPropsPlugin::slotBrowseExec()
{
    KUrl f = KFileDialog::getOpenUrl( KUrl(),
                                      QString(), d->m_frame );
    if ( f.isEmpty() )
        return;

    if ( !f.isLocalFile()) {
        KMessageBox::sorry(d->m_frame, i18n("Only executables on local file systems are supported."));
        return;
    }

    QString path = f.toLocalFile();
    path = KShell::quoteArg( path );
    d->w->commandEdit->setText( path );
}

void KDesktopPropsPlugin::slotAdvanced()
{
    KDialog dlg( d->m_frame );
    dlg.setObjectName( "KPropertiesDesktopAdv" );
    dlg.setModal( true );
    dlg.setCaption( i18n("Advanced Options for %1", properties->kurl().fileName()) );
    dlg.setButtons( KDialog::Ok | KDialog::Cancel );
    dlg.setDefaultButton( KDialog::Ok );
    Ui_KPropertiesDesktopAdvBase w;
    w.setupUi(dlg.mainWidget());

    // If the command is changed we reset certain settings that are strongly
    // coupled to the command.
    checkCommandChanged();

    // check to see if we use konsole if not do not add the nocloseonexit
    // because we don't know how to do this on other terminal applications
    KConfigGroup confGroup( KGlobal::config(), QString::fromLatin1("General") );
    QString preferredTerminal = confGroup.readPathEntry("TerminalApplication",
                                                        QString::fromLatin1("konsole"));

    bool terminalCloseBool = false;

    if (preferredTerminal == "konsole")
    {
        terminalCloseBool = (d->m_terminalOptionStr.contains( "--noclose" ) > 0);
        w.terminalCloseCheck->setChecked(terminalCloseBool);
        d->m_terminalOptionStr.remove( "--noclose");
    }
    else
    {
        w.terminalCloseCheck->hide();
    }

    w.terminalCheck->setChecked(d->m_terminalBool);
    w.terminalEdit->setText(d->m_terminalOptionStr);
    w.terminalCloseCheck->setEnabled(d->m_terminalBool);
    w.terminalEdit->setEnabled(d->m_terminalBool);
    w.terminalEditLabel->setEnabled(d->m_terminalBool);

    w.suidCheck->setChecked(d->m_suidBool);
    w.suidEdit->setText(d->m_suidUserStr);
    w.suidEdit->setEnabled(d->m_suidBool);
    w.suidEditLabel->setEnabled(d->m_suidBool);

    w.startupInfoCheck->setChecked(d->m_startupBool);
    w.systrayCheck->setChecked(d->m_systrayBool);

    if (d->m_dbusStartupType == "unique")
        w.dbusCombo->setCurrentIndex(2);
    else if (d->m_dbusStartupType == "multi")
        w.dbusCombo->setCurrentIndex(1);
    else if (d->m_dbusStartupType == "wait")
        w.dbusCombo->setCurrentIndex(3);
    else
        w.dbusCombo->setCurrentIndex(0);

    // Provide username completion up to 1000 users.
    KCompletion *kcom = new KCompletion;
    kcom->setOrder(KCompletion::Sorted);
    struct passwd *pw;
    int i, maxEntries = 1000;
    setpwent();
    for (i=0; ((pw = getpwent()) != 0L) && (i < maxEntries); i++)
        kcom->addItem(QString::fromLatin1(pw->pw_name));
    endpwent();
    if (i < maxEntries)
    {
        w.suidEdit->setCompletionObject(kcom, true);
        w.suidEdit->setAutoDeleteCompletionObject( true );
        w.suidEdit->setCompletionMode(KGlobalSettings::CompletionAuto);
    }
    else
    {
        delete kcom;
    }

    connect( w.terminalEdit, SIGNAL( textChanged( const QString & ) ),
             this, SIGNAL( changed() ) );
    connect( w.terminalCloseCheck, SIGNAL( toggled( bool ) ),
             this, SIGNAL( changed() ) );
    connect( w.terminalCheck, SIGNAL( toggled( bool ) ),
             this, SIGNAL( changed() ) );
    connect( w.suidCheck, SIGNAL( toggled( bool ) ),
             this, SIGNAL( changed() ) );
    connect( w.suidEdit, SIGNAL( textChanged( const QString & ) ),
             this, SIGNAL( changed() ) );
    connect( w.startupInfoCheck, SIGNAL( toggled( bool ) ),
             this, SIGNAL( changed() ) );
    connect( w.systrayCheck, SIGNAL( toggled( bool ) ),
             this, SIGNAL( changed() ) );
    connect( w.dbusCombo, SIGNAL( activated( int ) ),
             this, SIGNAL( changed() ) );

    if ( dlg.exec() == QDialog::Accepted )
    {
        d->m_terminalOptionStr = w.terminalEdit->text().trimmed();
        d->m_terminalBool = w.terminalCheck->isChecked();
        d->m_suidBool = w.suidCheck->isChecked();
        d->m_suidUserStr = w.suidEdit->text().trimmed();
        d->m_startupBool = w.startupInfoCheck->isChecked();
        d->m_systrayBool = w.systrayCheck->isChecked();

        if (w.terminalCloseCheck->isChecked())
        {
            d->m_terminalOptionStr.append(" --noclose");
        }

        switch(w.dbusCombo->currentIndex())
        {
        case 1:  d->m_dbusStartupType = "multi"; break;
        case 2:  d->m_dbusStartupType = "unique"; break;
        case 3:  d->m_dbusStartupType = "wait"; break;
        default: d->m_dbusStartupType = "none"; break;
        }
    }
}

bool KDesktopPropsPlugin::supports( const KFileItemList& _items )
{
    if ( _items.count() != 1 ) {
        return false;
    }

    const KFileItem item = _items.first();

    // check if desktop file
    if (!item.isDesktopFile()) {
        return false;
    }

    // open file and check type
    bool isLocal;
    KUrl url = item.mostLocalUrl( isLocal );
    if (!isLocal) {
        return false;
    }

    KDesktopFile config( url.path() );
    return config.hasApplicationType() &&
            KAuthorized::authorize("run_desktop_files") &&
            KAuthorized::authorize("shell_access");
}

#include "kpropertiesdialog.moc"
#include "kpropertiesdialog_p.moc"

