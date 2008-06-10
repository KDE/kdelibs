// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  1999,2000,2001,2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>
                  2003 Clarence Dang <dang@kde.org>

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

#include "kfiledialog.h"
#include "kabstractfilewidget.h"

#include <QtGui/QCheckBox>
#include <QtGui/QKeyEvent>
#include <QtGui/QFileDialog>

#include <kimageio.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <config-kfile.h>
#include <krecentdocument.h>
#include <kimagefilepreview.h>
#include <kpluginloader.h>
#include <kpluginfactory.h>
#include "kabstractfilemodule.h"
#include <kdebug.h>

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

static KAbstractFileModule* s_module = 0;
static KAbstractFileModule* fileModule()
{
    if (!s_module) {
        // TODO fix memleak -- qApp post routine for deleting the module ?
        KPluginLoader loader("libkfilemodule");
        KPluginFactory *factory = loader.factory();
        if (!factory) {
            kWarning() << "KFileDialog wasn't able to find libkfilemodule: " << loader.errorString();
        } else {
            s_module = factory->create<KAbstractFileModule>();
            if (!s_module) {
                kWarning() << "An error occurred while loading libkfilemodule";
            }
        }
    }
    return s_module;
}

class KFileDialogPrivate
{
public:
    KFileDialogPrivate() : w(0)
    {
        cfgGroup = KConfigGroup(KGlobal::config(), ConfigGroup);
    }

    KAbstractFileWidget* w;
    KConfigGroup cfgGroup;
};

KFileDialog::KFileDialog( const KUrl& startDir, const QString& filter,
                          QWidget *parent, QWidget* customWidget)
#ifdef Q_WS_WIN
    : KDialog( parent , Qt::WindowMinMaxButtonsHint),
#else
    : KDialog( parent ),
#endif
      d( new KFileDialogPrivate )

{
    setButtons( KDialog::None );
    restoreDialogSize(d->cfgGroup); // call this before the fileQWidget is set as the main widget.
                                    // otherwise the sizes for the components are not obeyed (ereslibre)

    // Dlopen the file widget from libkfilemodule
    QWidget* fileQWidget = fileModule()->createFileWidget(startDir, this);
    d->w = ::qobject_cast<KAbstractFileWidget *>(fileQWidget);
    d->w->setFilter(filter);
    setMainWidget(fileQWidget);

    d->w->okButton()->show();
    connect(d->w->okButton(), SIGNAL(clicked()), SLOT(slotOk()));
    d->w->cancelButton()->show();
    connect(d->w->cancelButton(), SIGNAL( clicked() ), SLOT( slotCancel() ));

    // Publish signals
    // TODO: Move the relevant signal declarations from KFileWidget to the
    //       KAbstractFileWidget interface?
    //
    //       Else, all of these connects (including "accepted") are not typesafe.
    kDebug (kfile_area) << "KFileDialog connecting signals";
    connect(fileQWidget, SIGNAL(fileSelected(const QString&)),
                         SIGNAL(fileSelected(const QString&)));
    connect(fileQWidget, SIGNAL(fileHighlighted(const QString&)),
                         SIGNAL(fileHighlighted(const QString&)));
    connect(fileQWidget, SIGNAL(selectionChanged()),
                         SIGNAL(selectionChanged()));
    connect(fileQWidget, SIGNAL(filterChanged(const QString&)),
                         SIGNAL(filterChanged(const QString&)));

    connect(fileQWidget, SIGNAL(accepted()), SLOT(accept()));
    //connect(fileQWidget, SIGNAL(canceled()), SLOT(slotCancel()));

    if (customWidget)
     d->w->setCustomWidget(customWidget);
}


KFileDialog::~KFileDialog()
{
    delete d;
}

void KFileDialog::setLocationLabel(const QString& text)
{
    d->w->setLocationLabel(text);
}

void KFileDialog::setFilter(const QString& filter)
{
    d->w->setFilter(filter);
}

QString KFileDialog::currentFilter() const
{
    return d->w->currentFilter();
}

void KFileDialog::setMimeFilter( const QStringList& mimeTypes,
                                 const QString& defaultType )
{
    d->w->setMimeFilter(mimeTypes, defaultType);
}

void KFileDialog::clearFilter()
{
    d->w->clearFilter();
}

QString KFileDialog::currentMimeFilter() const
{
    return d->w->currentMimeFilter();
}

KMimeType::Ptr KFileDialog::currentFilterMimeType()
{
    return KMimeType::mimeType( currentMimeFilter() );
}

void KFileDialog::setPreviewWidget(KPreviewWidgetBase *w)
{
    d->w->setPreviewWidget(w);
}

QSize KFileDialog::sizeHint() const
{
    return QSize(640, 400);
}

// This slot still exists mostly for compat purposes; for subclasses which reimplement slotOk
void KFileDialog::slotOk()
{
    d->w->slotOk();
}

// This slot still exists mostly for compat purposes; for subclasses which reimplement accept
void KFileDialog::accept()
{
    setResult( QDialog::Accepted ); // keep old behavior; probably not needed though
    d->w->accept();
    KConfigGroup cfgGroup(KGlobal::config(), ConfigGroup);
    KDialog::accept();
    emit okClicked();
}

// This slot still exists mostly for compat purposes; for subclasses which reimplement slotCancel
void KFileDialog::slotCancel()
{
    d->w->slotCancel();
    reject();
}

void KFileDialog::setUrl(const KUrl& url, bool clearforward)
{
    d->w->setUrl(url, clearforward);
}

void KFileDialog::setSelection(const QString& name)
{
    d->w->setSelection(name);
}

QString KFileDialog::getOpenFileName(const KUrl& startDir,
                                     const QString& filter,
                                     QWidget *parent, const QString& caption)
{
    KFileDialog dlg(startDir, filter, parent);
    dlg.setOperationMode( Opening );

    dlg.setMode( KFile::File | KFile::LocalOnly );
    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);

    //dlg.d->ops->clearHistory();
    dlg.exec();

    return dlg.selectedFile();
}

QString KFileDialog::getOpenFileNameWId(const KUrl& startDir,
                                        const QString& filter,
                                        WId parent_id, const QString& caption)
{
    QWidget* parent = QWidget::find( parent_id );
    KFileDialog dlg(startDir, filter, parent);
#ifdef Q_WS_X11
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint( QX11Info::display(), dlg.winId(), parent_id );
#else
    // TODO
#endif

    dlg.setOperationMode( KFileDialog::Opening );

    dlg.setMode( KFile::File | KFile::LocalOnly );
    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);

    //dlg.d->ops->clearHistory();
    dlg.exec();

    return dlg.selectedFile();
}

QStringList KFileDialog::getOpenFileNames(const KUrl& startDir,
                                          const QString& filter,
                                          QWidget *parent,
                                          const QString& caption)
{
    KFileDialog dlg(startDir, filter, parent);
    dlg.setOperationMode( Opening );

    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);
    dlg.setMode(KFile::Files | KFile::LocalOnly);
    //dlg.d->ops->clearHistory();
    dlg.exec();

    return dlg.selectedFiles();
}

KUrl KFileDialog::getOpenUrl(const KUrl& startDir, const QString& filter,
                                QWidget *parent, const QString& caption)
{
    KFileDialog dlg(startDir, filter, parent);
    dlg.setOperationMode( Opening );

    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);
    dlg.setMode( KFile::File );
    //dlg.d->ops->clearHistory();
    dlg.exec();

    return dlg.selectedUrl();
}

KUrl::List KFileDialog::getOpenUrls(const KUrl& startDir,
                                          const QString& filter,
                                          QWidget *parent,
                                          const QString& caption)
{
    KFileDialog dlg(startDir, filter, parent);
    dlg.setOperationMode( Opening );

    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);
    dlg.setMode(KFile::Files);
    //dlg.d->ops->clearHistory();
    dlg.exec();

    return dlg.selectedUrls();
}

KUrl KFileDialog::getExistingDirectoryUrl(const KUrl& startDir,
                                          QWidget *parent,
                                          const QString& caption)
{
    return fileModule()->selectDirectory(startDir, false, parent, caption);
}

QString KFileDialog::getExistingDirectory(const KUrl& startDir,
                                          QWidget *parent,
                                          const QString& caption)
{
#ifdef Q_WS_WIN
    return QFileDialog::getExistingDirectory(parent, caption,
                                             startDir.path(), QFileDialog::ShowDirsOnly);
#else
    KUrl url = fileModule()->selectDirectory(startDir, true, parent, caption);
    if ( url.isValid() )
        return url.path();
    return QString();
#endif
}

KUrl KFileDialog::getImageOpenUrl( const KUrl& startDir, QWidget *parent,
                                   const QString& caption)
{
    QStringList mimetypes = KImageIO::mimeTypes( KImageIO::Reading );
    KFileDialog dlg(startDir,
                    mimetypes.join(" "),
                    parent);
    dlg.setOperationMode( Opening );
    dlg.setCaption( caption.isEmpty() ? i18n("Open") : caption );
    dlg.setMode( KFile::File );

    KImageFilePreview *ip = new KImageFilePreview( &dlg );
    dlg.setPreviewWidget( ip );
    dlg.exec();

    return dlg.selectedUrl();
}

KUrl KFileDialog::selectedUrl() const
{
    return d->w->selectedUrl();
}

KUrl::List KFileDialog::selectedUrls() const
{
    return d->w->selectedUrls();
}

QString KFileDialog::selectedFile() const
{
    return d->w->selectedFile();
}

QStringList KFileDialog::selectedFiles() const
{
    return d->w->selectedFiles();
}

KUrl KFileDialog::baseUrl() const
{
    return d->w->baseUrl();
}

QString KFileDialog::getSaveFileName(const KUrl& dir, const QString& filter,
                                     QWidget *parent,
                                     const QString& caption)
{
    bool defaultDir = dir.isEmpty();
    bool specialDir = !defaultDir && dir.protocol() == "kfiledialog";
    KFileDialog dlg( specialDir ? dir : KUrl(), filter, parent);
    if ( !specialDir && !defaultDir ) {
        if (!dir.isLocalFile())
            kWarning() << "KFileDialog::getSaveFileName called with non-local start dir " << dir;
        dlg.setSelection( dir.path() ); // may also be a filename
    }

    dlg.setOperationMode( Saving );
    dlg.setMode( KFile::File );
    dlg.setCaption(caption.isEmpty() ? i18n("Save As") : caption);

    dlg.exec();

    QString filename = dlg.selectedFile();
    if (!filename.isEmpty())
        KRecentDocument::add(filename);

    return filename;
}

QString KFileDialog::getSaveFileNameWId(const KUrl& dir, const QString& filter,
                                     WId parent_id,
                                     const QString& caption)
{
    bool defaultDir = dir.isEmpty();
    bool specialDir = !defaultDir && dir.protocol() == "kfiledialog";
    QWidget* parent = QWidget::find( parent_id );
    KFileDialog dlg( specialDir ? dir : KUrl(), filter, parent);
#ifdef Q_WS_X11
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint(QX11Info::display(), dlg.winId(), parent_id);
#else
    // TODO
#endif

    if ( !specialDir && !defaultDir ) {
        if (!dir.isLocalFile())
            kWarning() << "KFileDialog::getSaveFileNameWId called with non-local start dir " << dir;
        dlg.setSelection( dir.path() ); // may also be a filename
    }

    dlg.setOperationMode( Saving );
    dlg.setMode( KFile::File );
    dlg.setCaption(caption.isEmpty() ? i18n("Save As") : caption);

    dlg.exec();

    QString filename = dlg.selectedFile();
    if (!filename.isEmpty())
        KRecentDocument::add(filename);

    return filename;
}

KUrl KFileDialog::getSaveUrl(const KUrl& dir, const QString& filter,
                             QWidget *parent, const QString& caption)
{
    bool defaultDir = dir.isEmpty();
    bool specialDir = !defaultDir && dir.protocol() == "kfiledialog";
    KFileDialog dlg(specialDir ? dir : KUrl(), filter, parent);
    if ( !specialDir )
        dlg.setSelection( dir.url() ); // may also be a filename

    dlg.setCaption(caption.isEmpty() ? i18n("Save As") : caption);
    dlg.setOperationMode( Saving );
    dlg.setMode( KFile::File );

    dlg.exec();

    KUrl url = dlg.selectedUrl();
    if (url.isValid())
        KRecentDocument::add( url );

    return url;
}

void KFileDialog::setMode( KFile::Modes m )
{
    d->w->setMode(m);
}

KFile::Modes KFileDialog::mode() const
{
    return d->w->mode();
}

KPushButton * KFileDialog::okButton() const
{
    return d->w->okButton();
}

KPushButton * KFileDialog::cancelButton() const
{
    return d->w->cancelButton();
}

KUrlComboBox* KFileDialog::locationEdit() const
{
    return d->w->locationEdit();
}

KFileFilterCombo* KFileDialog::filterWidget() const
{
    return d->w->filterWidget();
}

KActionCollection * KFileDialog::actionCollection() const
{
    return d->w->actionCollection();
}

void KFileDialog::setKeepLocation( bool keep )
{
    d->w->setKeepLocation(keep);
}

bool KFileDialog::keepsLocation() const
{
    return d->w->keepsLocation();
}

void KFileDialog::setOperationMode( OperationMode mode )
{
    d->w->setOperationMode(static_cast<KAbstractFileWidget::OperationMode>(mode));
}

KFileDialog::OperationMode KFileDialog::operationMode() const
{
    return static_cast<KFileDialog::OperationMode>(d->w->operationMode());
}

void KFileDialog::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Qt::Key_Escape )
    {
        e->accept();
        d->w->cancelButton()->animateClick();
    }
    else
        KDialog::keyPressEvent( e );
}

void KFileDialog::hideEvent( QHideEvent *e )
{
    saveDialogSize(d->cfgGroup, KConfigBase::Persistent);

    KDialog::hideEvent( e );
}

// static
KUrl KFileDialog::getStartUrl( const KUrl& startDir,
                               QString& recentDirClass )
{
    return fileModule()->getStartUrl(startDir, recentDirClass);
}

void KFileDialog::setStartDir( const KUrl& directory )
{
    fileModule()->setStartDir(directory);
}

KToolBar * KFileDialog::toolBar() const
{
    return d->w->toolBar();
}

KAbstractFileWidget* KFileDialog::fileWidget()
{
    return d->w;
}

#ifndef Q_WS_WIN
typedef QString (*_qt_filedialog_existing_directory_hook)(QWidget *parent, const QString &caption,
                                                          const QString &dir,
                                                          QFileDialog::Options options);
extern _qt_filedialog_existing_directory_hook qt_filedialog_existing_directory_hook;
#endif

typedef QString (*_qt_filedialog_open_filename_hook)(QWidget * parent, const QString &caption,
                                                     const QString &dir, const QString &filter,
                                                     QString *selectedFilter,
                                                     QFileDialog::Options options);
extern _qt_filedialog_open_filename_hook qt_filedialog_open_filename_hook;

typedef QStringList (*_qt_filedialog_open_filenames_hook)(QWidget * parent, const QString &caption,
                                                          const QString &dir, const QString &filter,
                                                          QString *selectedFilter,
                                                          QFileDialog::Options options);
extern _qt_filedialog_open_filenames_hook qt_filedialog_open_filenames_hook;

typedef QString (*_qt_filedialog_save_filename_hook)(QWidget * parent, const QString &caption,
                                                     const QString &dir, const QString &filter,
                                                     QString *selectedFilter,
                                                     QFileDialog::Options options);
extern _qt_filedialog_save_filename_hook qt_filedialog_save_filename_hook;

/*
 * This class is used to override Qt's QFileDialog calls with KFileDialog ones.
 * This is necessary because QPrintDialog calls QFileDialog::getSaveFileName() for
 * the print to file function.
 */

struct KFileDialogQtOverride
{
    KFileDialogQtOverride()
    {
#ifndef Q_WS_WIN
        if(!qt_filedialog_existing_directory_hook)
            qt_filedialog_existing_directory_hook=&getExistingDirectory;
#endif
        if(!qt_filedialog_open_filename_hook)
            qt_filedialog_open_filename_hook=&getOpenFileName;
        if(!qt_filedialog_open_filenames_hook)
            qt_filedialog_open_filenames_hook=&getOpenFileNames;
        if(!qt_filedialog_save_filename_hook)
            qt_filedialog_save_filename_hook=&getSaveFileName;
    }

    /*
     * Map a Qt filter string into a KDE one.
     */
    static QString qt2KdeFilter(const QString &f)
    {
        QString               filter;
        QTextStream           str(&filter, QIODevice::WriteOnly);
        QStringList           list(f.split(";;"));
        QStringList::Iterator it(list.begin()),
                              end(list.end());
        bool                  first=true;

        for(; it!=end; ++it)
        {
            int ob=(*it).lastIndexOf('('),
                cb=(*it).lastIndexOf(')');

            if(-1!=cb && ob<cb)
            {
                if(first)
                    first=false;
                else
                    str << '\n';
                str << (*it).mid(ob+1, (cb-ob)-1) << '|' << (*it).mid(0, ob);
            }
        }

        return filter;
    }

    /*
     * Map a KDE filter string into a Qt one.
     */
    static void kde2QtFilter(const QString &orig, const QString &kde, QString *sel)
    {
        if(sel)
        {
            QStringList           list(orig.split(";;"));
            QStringList::Iterator it(list.begin()),
                                  end(list.end());
            int                   pos;

            for(; it!=end; ++it)
                if(-1!=(pos=(*it).indexOf(kde)) && pos>0 &&
                   ('('==(*it)[pos-1] || ' '==(*it)[pos-1]) &&
                   (*it).length()>=kde.length()+pos &&
                   (')'==(*it)[pos+kde.length()] || ' '==(*it)[pos+kde.length()]))
                {
                    *sel=*it;
                    return;
                }
        }
    }

#ifndef Q_WS_WIN
    static QString getExistingDirectory(QWidget *parent, const QString &caption, const QString &dir,
                                        QFileDialog::Options)
    {
        KUrl url(KFileDialog::getExistingDirectory(KUrl(dir), parent, caption));

        if(url.isLocalFile())
            return url.pathOrUrl();
        else
            return QString();
    }
#endif

    static QString getOpenFileName(QWidget *parent, const QString &caption, const QString &dir,
                                   const QString &filter, QString *selectedFilter,
                                   QFileDialog::Options)
    {
        KFileDialog dlg(KUrl(dir), qt2KdeFilter(filter), parent);

        dlg.setOperationMode(KFileDialog::Opening);
        dlg.setMode(KFile::File|KFile::LocalOnly);
        dlg.setCaption(caption);
        dlg.exec();

        QString rv(dlg.selectedFile());

        if(!rv.isEmpty())
            kde2QtFilter(filter, dlg.currentFilter(), selectedFilter);

        return rv;
    }

    static QStringList getOpenFileNames(QWidget *parent, const QString &caption, const QString &dir,
                                        const QString &filter, QString *selectedFilter,
                                        QFileDialog::Options)
    {
        KFileDialog dlg(KUrl(dir), qt2KdeFilter(filter), parent);

        dlg.setOperationMode(KFileDialog::Opening);
        dlg.setMode(KFile::Files|KFile::LocalOnly);
        dlg.setCaption(caption);
        dlg.exec();

        QStringList rv(dlg.selectedFiles());

        if(rv.count())
            kde2QtFilter(filter, dlg.currentFilter(), selectedFilter);

        return rv;
    }

    static QString getSaveFileName(QWidget *parent, const QString &caption, const QString &dir,
                                   const QString &filter, QString *selectedFilter,
                                   QFileDialog::Options)
    {
        KFileDialog dlg(KUrl(dir), qt2KdeFilter(filter), parent);

        dlg.setOperationMode(KFileDialog::Saving);
        dlg.setMode(KFile::File|KFile::LocalOnly);
        dlg.setCaption(caption);
        dlg.exec();

        QString rv(dlg.selectedFile());

        if(!rv.isEmpty())
            kde2QtFilter(filter, dlg.currentFilter(), selectedFilter);

        return rv;
    }

};

static KFileDialogQtOverride qtOverride;

#include "kfiledialog.moc"
