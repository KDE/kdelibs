// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  1999,2000,2001,2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>
                  2003 Clarence Dang <dang@kde.org>
                  2008 Jarosław Staniek <staniek@kde.org>
                  2009 David Jarvie <djarvie@kde.org>

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

#include <QtGui/QCheckBox>
#include <QtGui/QKeyEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

#include <kimageio.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <config-kfile.h>
#include <krecentdocument.h>
#include <kimagefilepreview.h>
#include <kpluginloader.h>
#include <kpluginfactory.h>
#include <kdebug.h>
#include <kwindowsystem.h>
#include "kabstractfilewidget.h"
#include "kabstractfilemodule.h"
#include "krecentdirs.h"
#include "kservice.h"

/** File dialogs are native by default on Windows. */
#if defined(Q_WS_WIN) || defined(Q_WS_MAEMO_5)
const bool NATIVE_FILEDIALOGS_BY_DEFAULT = true;
#else
const bool NATIVE_FILEDIALOGS_BY_DEFAULT = false;
#endif

static QStringList mime2KdeFilter( const QStringList &mimeTypes, QString *allExtensions = 0 )
{
  const KUrl emptyUrl;
  QStringList kdeFilter;
  QStringList allExt;
  foreach( const QString& mimeType, mimeTypes ) {
    KMimeType::Ptr mime( KMimeType::mimeType(mimeType) );
    if (mime) {
      allExt += mime->patterns();
      kdeFilter.append(mime->patterns().join(QLatin1String(" ")) +
                       QLatin1Char('|') +
                       mime->comment(emptyUrl));
    }
  }
  if (allExtensions) {
      allExt.sort();
      *allExtensions = allExt.join(QLatin1String(" "));
  }
  return kdeFilter;
}
/** @return File dialog filter in Qt format for @a filters
 *          or "All files (*)" for empty list.
 */
static QString qtFilter(const QStringList& _filters)
{
    QString converted;
    const QStringList filters = _filters;

    foreach (const QString& current, filters) {
        QString new_f; //filter part
        QString new_name; //filter name part
        int p = current.indexOf('|');
        if (p==-1) {
            new_f = current;
            new_name = current; // nothing better found
        }
        else {
            new_f = current.left(p);
            new_name = current.mid(p+1);
        }
	//Qt filters assume anything in () is the file extension list
	new_name = new_name.replace('(', '[').replace(')',']').trimmed();

        //convert everything to lower case and remove dupes (doesn't matter on win32)
        QStringList allfiltersUnique;
        const QStringList origList( new_f.split(' ', QString::SkipEmptyParts) );
        foreach (const QString& origFilter, origList) {
	if (!allfiltersUnique.contains(origFilter, Qt::CaseInsensitive))
                allfiltersUnique += origFilter.toLower();
        }

        if (!converted.isEmpty())
            converted += ";;";

        converted += (new_name + " (" + allfiltersUnique.join(" ") + QLatin1Char(')'));
    }

    // Strip escape characters from escaped '/' characters.
    converted.replace("\\/","/");

    return converted;
}

/** @return File dialog filter in Qt format for @a filter in KDE format
 *          or "All files (*)" for empty filter.
 */
static QString qtFilter(const QString& filter)
{
    // Qt format: "some text (*.first *.second)" or "All files (*)" separated by ;;
    // KDE format: "*.first *.second|Description" or "*|Description", separated by \n (Description is optional)
    QStringList filters;
    if (filter.isEmpty())
        filters += i18n("*|All files");
    else {
      // check if it's a mimefilter
      int pos = filter.indexOf('/');
      if (pos > 0 && filter[pos - 1] != '\\')
          filters = mime2KdeFilter(filter.split(QLatin1Char(' '), QString::SkipEmptyParts));
      else
          filters = filter.split('\n', QString::SkipEmptyParts);
    }
    return qtFilter(filters);
}

static KAbstractFileModule* s_module = 0;
static KAbstractFileModule* loadFileModule( const QString& moduleName )
{
    KService::Ptr fileModuleService = KService::serviceByDesktopName(moduleName);
    if(fileModuleService)
        return fileModuleService->createInstance<KAbstractFileModule>();
    else
        return 0;
}

static const char s_defaultFileModuleName[] = "kfilemodule";
static KAbstractFileModule* fileModule()
{
    if(!s_module) {
        QString moduleName = KConfigGroup(KGlobal::config(), ConfigGroup).readEntry("file module", s_defaultFileModuleName);
        if(!(s_module = loadFileModule(moduleName))) {
            kDebug() << "Failed to load configured file module" << moduleName;
            if(moduleName != s_defaultFileModuleName) {
                kDebug() << "Falling back to default file module.";
                s_module = loadFileModule(s_defaultFileModuleName);
            }
        }
    }
    return s_module;
}

class KFileDialogPrivate
{
public:
    /** Data used for native mode. */
    class Native {
    public:
        Native()
          : mode(KFile::File),
            operationMode(KAbstractFileWidget::Opening)
        {
        }
        /** @return previously set (global) start dir or the first url
         selected using setSelection() or setUrl() if the start dir is empty. */
        KUrl startDir() const
        {
            if (!s_startDir.isEmpty())
                return s_startDir;
            if (!selectedUrls.isEmpty())
                return selectedUrls.first();
            return KUrl();
        }
        /** @return previously set (global) start dir or @p defaultDir
         if the start dir is empty. */
        static KUrl staticStartDir( const KUrl& defaultDir )
        {
            if ( s_startDir.isEmpty() )
              return defaultDir;
            return s_startDir;
        }
        static KUrl s_startDir;
        static bool s_allowNative;  // as fallback when we can't use native dialog
        QString filter;
        QString selectedFilter;
        QStringList mimeTypes;
        KUrl::List selectedUrls;
        KFile::Modes mode;
        KAbstractFileWidget::OperationMode operationMode;
    };

    KFileDialogPrivate()
      : native(0),
        w(0),
        cfgGroup(KGlobal::config(), ConfigGroup)
    {
        if (cfgGroup.readEntry("Native", NATIVE_FILEDIALOGS_BY_DEFAULT) &&
            KFileDialogPrivate::Native::s_allowNative)
            native = new Native;
    }

    static bool isNative()
    {
        if(!KFileDialogPrivate::Native::s_allowNative)
            return false;
        KConfigGroup cfgGroup(KGlobal::config(), ConfigGroup);
        return cfgGroup.readEntry("Native", NATIVE_FILEDIALOGS_BY_DEFAULT);
    }
    
    static QString getOpenFileName(const KUrl& startDir, const QString& filter,
                                   QWidget *parent, const QString& caption,
                                   QString *selectedFilter);
    static KUrl getOpenUrl(const KUrl& startDir, const QString& filter,
                           QWidget *parent, const QString& caption, 
                           QString *selectedFilter);
    static QStringList getOpenFileNames(const KUrl& startDir, const QString& filter,
                                        QWidget *parent, const QString& caption,
                                        QString *selectedFilter);
    static KUrl::List getOpenUrls(const KUrl& startDir, const QString& filter,
                                  QWidget *parent, const QString& caption,
                                  QString *selectedFilter);
    static QString getSaveFileName(const KUrl& dir, const QString& filter,
                                   QWidget *parent, const QString& caption, 
                                   KFileDialog::Options options, QString *selectedFilter);
    static KUrl getSaveUrl(const KUrl& dir, const QString& filter,
                           QWidget *parent, const QString& caption, 
                           KFileDialog::Options options, QString *selectedFilter);

    ~KFileDialogPrivate()
    {
        delete native;
    }

    Native* native;
    KAbstractFileWidget* w;
    KConfigGroup cfgGroup;
};

KUrl KFileDialogPrivate::Native::s_startDir;
bool KFileDialogPrivate::Native::s_allowNative = true;

KFileDialog::KFileDialog( const KUrl& startDir, const QString& filter,
                          QWidget *parent, QWidget* customWidget)
#ifdef Q_WS_WIN
    : KDialog( parent , Qt::WindowMinMaxButtonsHint),
#else
    : KDialog( parent ),
#endif
      d( new KFileDialogPrivate )

{
    // It would be nice to have this behind d->native but it doesn't work
    // because of derived classes like KEncodingDialog...
    // Dlopen the file widget from libkfilemodule
    QWidget* fileQWidget = fileModule()->createFileWidget(startDir, this);
    d->w = ::qobject_cast<KAbstractFileWidget *>(fileQWidget);

    if (d->native) {
        KFileDialogPrivate::Native::s_startDir = startDir;
        // check if it's a mimefilter
        int pos = filter.indexOf('/');
        if (pos > 0 && filter[pos - 1] != '\\')
          setMimeFilter(filter.split(QLatin1Char(' '), QString::SkipEmptyParts));
        else
          setFilter(filter);
        return;
    }

    setButtons( KDialog::None );
    restoreDialogSize(d->cfgGroup); // call this before the fileQWidget is set as the main widget.
                                   // otherwise the sizes for the components are not obeyed (ereslibre)

    d->w->setFilter(filter);
    setMainWidget(fileQWidget);

    d->w->okButton()->show();
    connect(d->w->okButton(), SIGNAL(clicked()), SLOT(slotOk()));
    d->w->cancelButton()->show();
    connect(d->w->cancelButton(), SIGNAL( clicked() ), SLOT( slotCancel() ));

    // Publish signals
    // TODO: Move the relevant signal declarations from KFileWidget to the
    //       KAbstractFileWidget interface?
    //       Else, all of these connects (including "accepted") are not typesafe.
    // Answer: you cannot define signals in a non-qobject base class (DF).
    //         I simply documentde them in kabstractfilewidget.h now.
    kDebug (kfile_area) << "KFileDialog connecting signals";
    connect(fileQWidget, SIGNAL(fileSelected(KUrl)),
                         SIGNAL(fileSelected(KUrl)));
    connect(fileQWidget, SIGNAL(fileHighlighted(KUrl)),
                         SIGNAL(fileHighlighted(KUrl)));
    connect(fileQWidget, SIGNAL(fileSelected(QString)),
                         SIGNAL(fileSelected(QString)));
    connect(fileQWidget, SIGNAL(fileHighlighted(QString)),
                         SIGNAL(fileHighlighted(QString)));
    connect(fileQWidget, SIGNAL(selectionChanged()),
                         SIGNAL(selectionChanged()));
    connect(fileQWidget, SIGNAL(filterChanged(QString)),
                         SIGNAL(filterChanged(QString)));

    connect(fileQWidget, SIGNAL(accepted()), SLOT(accept()));
    //connect(fileQWidget, SIGNAL(canceled()), SLOT(slotCancel()));

    if (customWidget)
     d->w->setCustomWidget(QString(), customWidget);
}


KFileDialog::~KFileDialog()
{
    delete d;
}

void KFileDialog::setLocationLabel(const QString& text)
{
    if (d->native)
        return; // not available
    d->w->setLocationLabel(text);
}

void KFileDialog::setFilter(const QString& filter)
{
    if (d->native) {
        d->native->filter = filter;
        return;
    }
    d->w->setFilter(filter);
}

QString KFileDialog::currentFilter() const
{
    if (d->native)
        return QString(); // not available
    return d->w->currentFilter();
}

void KFileDialog::setMimeFilter( const QStringList& mimeTypes,
                                 const QString& defaultType )
{
    d->w->setMimeFilter(mimeTypes, defaultType);

    if (d->native) {
        QString allExtensions;
        QStringList filters = mime2KdeFilter(mimeTypes, &allExtensions);
        if (defaultType.isEmpty() && (mimeTypes.count() > 1)) {
            filters.prepend(allExtensions + QLatin1Char('|') + i18n("All Supported Files"));
        }
        d->native->filter = filters.join(QLatin1String("\n"));
    }
}

void KFileDialog::clearFilter()
{
    if (d->native) {
        d->native->filter.clear();
        return;
    }
    d->w->clearFilter();
}

QString KFileDialog::currentMimeFilter() const
{
    if (d->native) {
        // adapted from qt2KdeFilter
        QString filter = d->native->selectedFilter.split(";;").replaceInStrings("/", "\\/")[0];
        filter = filter.mid(filter.indexOf('(') + 1, filter.indexOf(')') - filter.indexOf('(') - 1);
        QString mimetype = KMimeType::findByPath("test" + filter.mid(1).split(' ')[0])->name();
        return mimetype;
    }
    return d->w->currentMimeFilter();
}

KMimeType::Ptr KFileDialog::currentFilterMimeType()
{
    return KMimeType::mimeType( currentMimeFilter() );
}

void KFileDialog::setPreviewWidget(KPreviewWidgetBase *w)
{
    if (d->native)
      return;
    d->w->setPreviewWidget(w);
}

void KFileDialog::setInlinePreviewShown(bool show)
{
    if (d->native) {
        return;
    }
    d->w->setInlinePreviewShown(show);
}

// This is only used for the initial size when no configuration has been saved
QSize KFileDialog::sizeHint() const
{
    int fontSize = fontMetrics().height();
    QSize goodSize(48 * fontSize, 30 * fontSize);
    QSize screenSize = QApplication::desktop()->availableGeometry(this).size();
    QSize minSize(screenSize / 2);
    QSize maxSize(screenSize * qreal(0.9));
    return (goodSize.expandedTo(minSize).boundedTo(maxSize));
}

// This slot still exists mostly for compat purposes; for subclasses which reimplement slotOk
void KFileDialog::slotOk()
{
    if (d->native)
        return;
    d->w->slotOk();
}

// This slot still exists mostly for compat purposes; for subclasses which reimplement accept
void KFileDialog::accept()
{
    if (d->native)
        return;
    setResult( QDialog::Accepted ); // keep old behavior; probably not needed though
    d->w->accept();
    KConfigGroup cfgGroup(KGlobal::config(), ConfigGroup);
    KDialog::accept();
    emit okClicked();
}

// This slot still exists mostly for compat purposes; for subclasses which reimplement slotCancel
void KFileDialog::slotCancel()
{
    if (d->native)
        return;
    d->w->slotCancel();
    reject();
}

void KFileDialog::setUrl(const KUrl& url, bool clearforward)
{
    if (d->native) {
         d->native->selectedUrls.clear();
         d->native->selectedUrls.append(url);
        return;
    }
    d->w->setUrl(url, clearforward);
}

void KFileDialog::setSelection(const QString& name)
{
    if (d->native) {
         d->native->selectedUrls.clear();
         d->native->selectedUrls.append( KUrl(name) );
         return;
    }
    d->w->setSelection(name);
}

QString KFileDialog::getOpenFileName(const KUrl& startDir,
                                     const QString& filter,
                                     QWidget *parent, const QString& caption)
{
    return KFileDialogPrivate::getOpenFileName(startDir, filter, parent, caption, 0);
}

QString KFileDialogPrivate::getOpenFileName(const KUrl& startDir,
                                            const QString& filter,
                                            QWidget *parent, 
                                            const QString& caption,
                                            QString *selectedFilter)
{
    if (KFileDialogPrivate::isNative() && (!startDir.isValid() || startDir.isLocalFile())) {
        return QFileDialog::getOpenFileName(
            parent,
            caption.isEmpty() ? i18n("Open") : caption,
            KFileDialogPrivate::Native::staticStartDir( startDir ).toLocalFile(),
            qtFilter(filter),
            selectedFilter );
// TODO use extra args?     QString * selectedFilter = 0, Options options = 0
    }
    KFileDialog dlg(startDir, filter, parent);

    dlg.setOperationMode( KFileDialog::Opening );
    dlg.setMode( KFile::File | KFile::LocalOnly | KFile::ExistingOnly );
    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);

    dlg.exec();
    if(selectedFilter) *selectedFilter = dlg.currentMimeFilter();
    return dlg.selectedFile();
}

QString KFileDialog::getOpenFileNameWId(const KUrl& startDir,
                                        const QString& filter,
                                        WId parent_id, const QString& caption)
{
    if (KFileDialogPrivate::isNative() && (!startDir.isValid() || startDir.isLocalFile()))
        return KFileDialog::getOpenFileName(startDir, filter, 0, caption); // everything we can do...
    QWidget* parent = QWidget::find( parent_id );
    KFileDialogPrivate::Native::s_allowNative = false;
    KFileDialog dlg(startDir, filter, parent);
    if( parent == NULL && parent_id != 0 )
        KWindowSystem::setMainWindow( &dlg, parent_id );

    dlg.setOperationMode( KFileDialog::Opening );
    dlg.setMode( KFile::File | KFile::LocalOnly | KFile::ExistingOnly );
    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);

    dlg.exec();

    return dlg.selectedFile();
}

QStringList KFileDialog::getOpenFileNames(const KUrl& startDir,
                                          const QString& filter,
                                          QWidget *parent,
                                          const QString& caption)
{
    return KFileDialogPrivate::getOpenFileNames(startDir, filter, parent, caption, 0);
}

QStringList KFileDialogPrivate::getOpenFileNames(const KUrl& startDir,
                                                 const QString& filter,
                                                 QWidget *parent,
                                                 const QString& caption,
                                                 QString *selectedFilter)
{
    if (KFileDialogPrivate::isNative() && (!startDir.isValid() || startDir.isLocalFile())) {
        return QFileDialog::getOpenFileNames(
            parent,
            caption.isEmpty() ? i18n("Open") : caption,
            KFileDialogPrivate::Native::staticStartDir( startDir ).toLocalFile(),
            qtFilter( filter ), selectedFilter );
// TODO use extra args?  QString * selectedFilter = 0, Options options = 0
    }
    KFileDialogPrivate::Native::s_allowNative = false;
    KFileDialog dlg(startDir, filter, parent);

    dlg.setOperationMode( KFileDialog::Opening );
    dlg.setMode(KFile::Files | KFile::LocalOnly | KFile::ExistingOnly);
    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);

    dlg.exec();
    if(selectedFilter) *selectedFilter = dlg.currentMimeFilter();
    return dlg.selectedFiles();
}

KUrl KFileDialog::getOpenUrl(const KUrl& startDir, const QString& filter,
                             QWidget *parent, const QString& caption)
{
    return KFileDialogPrivate::getOpenUrl(startDir, filter, parent, caption, 0);
}
KUrl KFileDialogPrivate::getOpenUrl(const KUrl& startDir, const QString& filter,
                                    QWidget *parent, const QString& caption, 
                                    QString *selectedFilter)
{
    if (KFileDialogPrivate::isNative() && (!startDir.isValid() || startDir.isLocalFile())) {
        const QString fileName( KFileDialogPrivate::getOpenFileName(
            startDir, filter, parent, caption, selectedFilter) );
        return fileName.isEmpty() ? KUrl() : KUrl::fromPath(fileName);
    }
    KFileDialogPrivate::Native::s_allowNative = false;
    KFileDialog dlg(startDir, filter, parent);

    dlg.setOperationMode( KFileDialog::Opening );
    dlg.setMode( KFile::File | KFile::ExistingOnly );
    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);

    dlg.exec();
    if(selectedFilter) *selectedFilter = dlg.currentMimeFilter();
    return dlg.selectedUrl();
}

KUrl::List KFileDialog::getOpenUrls(const KUrl& startDir,
                                    const QString& filter,
                                    QWidget *parent,
                                    const QString& caption)
{
    return KFileDialogPrivate::getOpenUrls(startDir, filter, parent, caption, 0);
}

KUrl::List KFileDialogPrivate::getOpenUrls(const KUrl& startDir,
                                           const QString& filter,
                                           QWidget *parent,
                                           const QString& caption,
                                           QString *selectedFilter)
{
    if (KFileDialogPrivate::isNative() && (!startDir.isValid() || startDir.isLocalFile())) {
        const QStringList fileNames( KFileDialogPrivate::getOpenFileNames(
            startDir, filter, parent, caption, selectedFilter) );
        return KUrl::List(fileNames);
    }
    KFileDialogPrivate::Native::s_allowNative = false;

    KFileDialog dlg(startDir, filter, parent);

    dlg.setOperationMode( KFileDialog::Opening );
    dlg.setMode( KFile::Files | KFile::ExistingOnly );
    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);

    dlg.exec();
    if(selectedFilter) *selectedFilter = dlg.currentMimeFilter();
    return dlg.selectedUrls();
}

void KFileDialog::setConfirmOverwrite(bool enable)
{
    if (operationMode() == KFileDialog::Saving) {
        d->w->setConfirmOverwrite(enable);
    }
}

KUrl KFileDialog::getExistingDirectoryUrl(const KUrl& startDir,
                                          QWidget *parent,
                                          const QString& caption)
{
    if (KFileDialogPrivate::isNative() && (!startDir.isValid() || startDir.isLocalFile())) {
        QString result( QFileDialog::getExistingDirectory(parent, caption,
            KFileDialogPrivate::Native::staticStartDir( startDir ).toLocalFile(),
            QFileDialog::ShowDirsOnly) );
        return result.isEmpty() ? KUrl() : KUrl::fromPath(result);
    }
    return fileModule()->selectDirectory(startDir, false, parent, caption);
}

QString KFileDialog::getExistingDirectory(const KUrl& startDir,
                                          QWidget *parent,
                                          const QString& caption)
{
    if (KFileDialogPrivate::isNative() && (!startDir.isValid() || startDir.isLocalFile())) {
        return QFileDialog::getExistingDirectory(parent, caption,
            KFileDialogPrivate::Native::staticStartDir( startDir ).toLocalFile(),
            QFileDialog::ShowDirsOnly);
    }
    KUrl url = fileModule()->selectDirectory(startDir, true, parent, caption);
    if ( url.isValid() )
        return url.path();
    return QString();
}

KUrl KFileDialog::getImageOpenUrl( const KUrl& startDir, QWidget *parent,
                                   const QString& caption)
{
    if (KFileDialogPrivate::isNative() && (!startDir.isValid() || startDir.isLocalFile())) { // everything we can do...
        const QStringList mimetypes( KImageIO::mimeTypes( KImageIO::Reading ) );
        return KFileDialog::getOpenUrl(startDir, mimetypes.join(" "), parent, caption);
    }
    const QStringList mimetypes = KImageIO::mimeTypes( KImageIO::Reading );
    KFileDialogPrivate::Native::s_allowNative = false;
    KFileDialog dlg(startDir, mimetypes.join(" "), parent);

    dlg.setOperationMode( KFileDialog::Opening );
    dlg.setMode( KFile::File | KFile::ExistingOnly );
    dlg.setCaption( caption.isEmpty() ? i18n("Open") : caption );
    dlg.setInlinePreviewShown( true );

    dlg.exec();

    return dlg.selectedUrl();
}

KUrl KFileDialog::selectedUrl() const
{
    if (d->native)
        return d->native->selectedUrls.isEmpty() ? KUrl() : d->native->selectedUrls.first();
    return d->w->selectedUrl();
}

KUrl::List KFileDialog::selectedUrls() const
{
    if (d->native)
        return d->native->selectedUrls;
    return d->w->selectedUrls();
}

QString KFileDialog::selectedFile() const
{
    if (d->native)
        return selectedUrl().toLocalFile();
    return d->w->selectedFile();
}

QStringList KFileDialog::selectedFiles() const
{
    if (d->native)
        return selectedUrls().toStringList();
    return d->w->selectedFiles();
}

KUrl KFileDialog::baseUrl() const
{
    if (d->native)
        return selectedUrl().isEmpty() ? KUrl() : KUrl::fromPath(selectedUrl().path());
    return d->w->baseUrl();
}

QString KFileDialog::getSaveFileName(const KUrl& dir, const QString& filter,
                                     QWidget *parent,
                                     const QString& caption)
{
    //TODO KDE5: replace this method by the method below (with default parameter values in declaration)
    // Set no confirm-overwrite mode for backwards compatibility
    return KFileDialogPrivate::getSaveFileName(dir, filter, parent, caption, Options(0), 0);
}

QString KFileDialog::getSaveFileName(const KUrl& dir, const QString& filter,
                                     QWidget *parent,
                                     const QString& caption, Options options)
{
    return KFileDialogPrivate::getSaveFileName(dir, filter, parent, caption, options, 0);
}

QString KFileDialogPrivate::getSaveFileName(const KUrl& dir, const QString& filter,
                                            QWidget *parent, const QString& caption, 
                                            KFileDialog::Options options, QString *selectedFilter)
{
    if (KFileDialogPrivate::isNative()) {
        bool defaultDir = dir.isEmpty();
        bool specialDir = !defaultDir && dir.protocol() == "kfiledialog";
        KUrl startDir;
        QString recentDirClass;
        if (specialDir) {
          startDir = KFileDialog::getStartUrl(dir, recentDirClass);
        }
        else if ( !specialDir && !defaultDir ) {
          if (!dir.isLocalFile())
              kWarning() << "non-local start dir " << dir;
          startDir = dir;
        }

        QFileDialog::Options opts = (options & KFileDialog::ConfirmOverwrite) ? QFileDialog::Options(0) : QFileDialog::DontConfirmOverwrite;
        const QString result = QFileDialog::getSaveFileName(
            parent,
            caption.isEmpty() ? i18n("Save As") : caption,
            KFileDialogPrivate::Native::staticStartDir( startDir ).toLocalFile(),
            qtFilter(filter),
// TODO use extra args?     QString * selectedFilter = 0, Options opts = 0
            selectedFilter, opts );
        if (!result.isEmpty()) {
            if (!recentDirClass.isEmpty())
                KRecentDirs::add(recentDirClass, KUrl::fromPath(result).url());
            KRecentDocument::add(result);
        }
        return result;
    }

    KFileDialog dlg(dir, filter, parent);

    dlg.setOperationMode( KFileDialog::Saving );
    dlg.setMode( KFile::File | KFile::LocalOnly );
    dlg.setConfirmOverwrite(options & KFileDialog::ConfirmOverwrite);
    dlg.setInlinePreviewShown(options & KFileDialog::ShowInlinePreview);
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
    //TODO KDE5: replace this method by the method below (with default parameter values in declaration)
    // Set no confirm-overwrite mode for backwards compatibility
    return getSaveFileNameWId(dir, filter, parent_id, caption, Options(0));
}

QString KFileDialog::getSaveFileNameWId(const KUrl& dir, const QString& filter,
                                     WId parent_id,
                                     const QString& caption, Options options)
{
    if (KFileDialogPrivate::isNative()) {
        return KFileDialog::getSaveFileName(dir, filter, 0, caption, options); // everything we can do...
    }

    QWidget* parent = QWidget::find( parent_id );
    KFileDialog dlg(dir, filter, parent);
    if( parent == NULL && parent_id != 0 )
        KWindowSystem::setMainWindow( &dlg, parent_id);

    dlg.setOperationMode( KFileDialog::Saving );
    dlg.setMode( KFile::File | KFile::LocalOnly );
    dlg.setConfirmOverwrite(options & ConfirmOverwrite);
    dlg.setInlinePreviewShown(options & ShowInlinePreview);
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
    //TODO KDE5: replace this method by the method below (with default parameter values in declaration)
    // Set no confirm-overwrite mode for backwards compatibility
    return KFileDialogPrivate::getSaveUrl(dir, filter, parent, caption, Options(0), 0);
}

KUrl KFileDialog::getSaveUrl(const KUrl& dir, const QString& filter,
                             QWidget *parent, const QString& caption, Options options)
{
    return KFileDialogPrivate::getSaveUrl(dir, filter, parent, caption, options, 0);
}
KUrl KFileDialogPrivate::getSaveUrl(const KUrl& dir, const QString& filter,
                                    QWidget *parent, const QString& caption, 
                                    KFileDialog::Options options, QString *selectedFilter)
{
    if (KFileDialogPrivate::isNative() && (!dir.isValid() || dir.isLocalFile())) {
        const QString fileName( KFileDialogPrivate::getSaveFileName(
            dir, filter, parent, caption, options, selectedFilter) );
        return fileName.isEmpty() ? KUrl() : KUrl::fromPath(fileName);
    }

    KFileDialogPrivate::Native::s_allowNative = false;

    KFileDialog dlg(dir, filter, parent);

    dlg.setOperationMode( KFileDialog::Saving );
    dlg.setMode( KFile::File );
    dlg.setConfirmOverwrite(options & KFileDialog::ConfirmOverwrite);
    dlg.setInlinePreviewShown(options & KFileDialog::ShowInlinePreview);
    dlg.setCaption(caption.isEmpty() ? i18n("Save As") : caption);

    dlg.exec();
    if(selectedFilter) *selectedFilter = dlg.currentMimeFilter();
    KUrl url = dlg.selectedUrl();
    if (url.isValid())
        KRecentDocument::add( url );

    return url;
}

void KFileDialog::setMode( KFile::Modes m )
{
    if (d->native)
        d->native->mode = m;
    else
        d->w->setMode(m);
}

KFile::Modes KFileDialog::mode() const
{
    if (d->native)
        return d->native->mode;
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
    if (d->native)
        return;
    d->w->setKeepLocation(keep);
}

bool KFileDialog::keepsLocation() const
{
    if (d->native)
        return false;
    return d->w->keepsLocation();
}

void KFileDialog::setOperationMode( OperationMode mode )
{
    if (d->native)
        d->native->operationMode = static_cast<KAbstractFileWidget::OperationMode>(mode);
    else
        d->w->setOperationMode(static_cast<KAbstractFileWidget::OperationMode>(mode));
}

KFileDialog::OperationMode KFileDialog::operationMode() const
{
    if (d->native)
        return static_cast<KFileDialog::OperationMode>(d->native->operationMode);
    return static_cast<KFileDialog::OperationMode>(d->w->operationMode());
}

void KFileDialog::keyPressEvent( QKeyEvent *e )
{
    if (d->native)
        return;

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
    if (d->native)
        return;

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
    if (KFileDialogPrivate::isNative())
        KFileDialogPrivate::Native::s_startDir = directory;
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

#ifdef Q_WS_WIN
int KFileDialog::exec()
{
    if (!d->native || !KFileDialogPrivate::Native::s_allowNative) {
        KFileDialogPrivate::Native::s_allowNative = true;
        return KDialog::exec();
    }

// not clear here to let KFileDialogPrivate::Native::startDir() return a useful value
// d->native->selectedUrls.clear();
    int res = QDialog::Rejected;
    switch (d->native->operationMode) {
    case KAbstractFileWidget::Opening:
    case KAbstractFileWidget::Other:
        if (d->native->mode & KFile::File) {
            KUrl url( KFileDialogPrivate::getOpenUrl(
               d->native->startDir(), d->native->filter, parentWidget(), windowTitle(), &d->native->selectedFilter ) );
            if (url.isEmpty() || !url.isValid()) {
                res = QDialog::Rejected;
                break;
            }
            d->native->selectedUrls.clear();
            d->native->selectedUrls.append(url);
            res = QDialog::Accepted;
            break;
        }
        else if (d->native->mode & KFile::Files) {
            KUrl::List urls( KFileDialogPrivate::getOpenUrls(
                d->native->startDir(), d->native->filter, parentWidget(), windowTitle(), &d->native->selectedFilter ) );
            if (urls.isEmpty()) {
                res = QDialog::Rejected;
                break;
            }
            d->native->selectedUrls = urls;
            res = QDialog::Accepted;
            break;
        }
        else if (d->native->mode & KFile::Directory) {
            KUrl url( KFileDialog::getExistingDirectoryUrl(
                d->native->startDir(), parentWidget(), windowTitle()) );
            if (url.isEmpty() || !url.isValid()) {
                res = QDialog::Rejected;
            }
            d->native->selectedUrls.clear();
            d->native->selectedUrls.append(url);
            res = QDialog::Accepted;
            break;
        }
        break;
    case KAbstractFileWidget::Saving:
        if (d->native->mode & KFile::File) {
            KUrl url( KFileDialogPrivate::getSaveUrl(
                d->native->startDir(), d->native->filter, parentWidget(), windowTitle(), Options(0), &d->native->selectedFilter ) );
            if (url.isEmpty() || !url.isValid())  {
                res = QDialog::Rejected;
                break;
            }
            d->native->selectedUrls.clear();
            d->native->selectedUrls.append(url);
            res = QDialog::Accepted;
            break;
        }
        else if (d->native->mode & KFile::Directory) {
            KUrl url( KFileDialog::getExistingDirectoryUrl(
                d->native->startDir(), parentWidget(), windowTitle()) );
            if (url.isEmpty() || !url.isValid()) {
                res = QDialog::Rejected;
                break;
            }
            d->native->selectedUrls.clear();
            d->native->selectedUrls.append(url);
            res = QDialog::Accepted;
            break;
        }
        break;
    default:;
    }

    setResult(res);
    emit finished();

    if (res == QDialog::Accepted) {
        emit accepted();
    } else {
        emit rejected();
    }

    return res;
}
#endif // Q_WS_WIN

#ifdef Q_WS_WIN
#define KF_EXTERN extern __declspec(dllimport)
#else
#define KF_EXTERN extern
#endif

typedef QString (*_qt_filedialog_existing_directory_hook)(QWidget *parent, const QString &caption,
                                                          const QString &dir,
                                                          QFileDialog::Options options);
KF_EXTERN _qt_filedialog_existing_directory_hook qt_filedialog_existing_directory_hook;

typedef QString (*_qt_filedialog_open_filename_hook)(QWidget * parent, const QString &caption,
                                                     const QString &dir, const QString &filter,
                                                     QString *selectedFilter,
                                                     QFileDialog::Options options);
KF_EXTERN _qt_filedialog_open_filename_hook qt_filedialog_open_filename_hook;

typedef QStringList (*_qt_filedialog_open_filenames_hook)(QWidget * parent, const QString &caption,
                                                          const QString &dir, const QString &filter,
                                                          QString *selectedFilter,
                                                          QFileDialog::Options options);
KF_EXTERN _qt_filedialog_open_filenames_hook qt_filedialog_open_filenames_hook;

typedef QString (*_qt_filedialog_save_filename_hook)(QWidget * parent, const QString &caption,
                                                     const QString &dir, const QString &filter,
                                                     QString *selectedFilter,
                                                     QFileDialog::Options options);
KF_EXTERN _qt_filedialog_save_filename_hook qt_filedialog_save_filename_hook;

/*
 * This class is used to override Qt's QFileDialog calls with KFileDialog ones.
 * This is necessary because QPrintDialog calls QFileDialog::getSaveFileName() for
 * the print to file function.
 */
class KFileDialogQtOverride
{
public:
    KFileDialogQtOverride()
    {
        if(!qt_filedialog_existing_directory_hook)
            qt_filedialog_existing_directory_hook=&getExistingDirectory;
        if(!qt_filedialog_open_filename_hook)
            qt_filedialog_open_filename_hook=&getOpenFileName;
        if(!qt_filedialog_open_filenames_hook)
            qt_filedialog_open_filenames_hook=&getOpenFileNames;
        if(!qt_filedialog_save_filename_hook)
            qt_filedialog_save_filename_hook=&getSaveFileName;
    }

    ~KFileDialogQtOverride() {
        if(qt_filedialog_existing_directory_hook == &getExistingDirectory)
            qt_filedialog_existing_directory_hook = 0;
        if(qt_filedialog_open_filename_hook == &getOpenFileName)
            qt_filedialog_open_filename_hook = 0;
        if(qt_filedialog_open_filenames_hook == &getOpenFileNames)
            qt_filedialog_open_filenames_hook=0;
        if(qt_filedialog_save_filename_hook == &getSaveFileName)
            qt_filedialog_save_filename_hook=0;
    }

    /*
     * Map a Qt filter string into a KDE one.
     */
    static QString qt2KdeFilter(const QString &f)
    {
        QString               filter;
        QTextStream           str(&filter, QIODevice::WriteOnly);
        QStringList           list(f.split(";;").replaceInStrings("/", "\\/"));
        QStringList::const_iterator it(list.begin()),
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
            QStringList::const_iterator it(list.begin()),
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

    static QString getExistingDirectory(QWidget *parent, const QString &caption, const QString &dir,
                                        QFileDialog::Options options)
    {
        if (KFileDialogPrivate::isNative()) {
            if(qt_filedialog_existing_directory_hook)
                qt_filedialog_existing_directory_hook=0; // do not override
            return QFileDialog::getExistingDirectory(parent, caption, dir, options);
        }

        KUrl url(KFileDialog::getExistingDirectory(KUrl(dir), parent, caption));

        if(url.isLocalFile())
            return url.pathOrUrl();
        else
            return QString();
    }

    static QString getOpenFileName(QWidget *parent, const QString &caption, const QString &dir,
                                   const QString &filter, QString *selectedFilter,
                                   QFileDialog::Options options)
    {
        if (KFileDialogPrivate::isNative()) {
            if(qt_filedialog_open_filename_hook)
                qt_filedialog_open_filename_hook=0; // do not override
            return QFileDialog::getOpenFileName(parent, caption, dir, filter, selectedFilter, options);
        }

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
                                        QFileDialog::Options options)
    {
        if (KFileDialogPrivate::isNative()) {
            if(qt_filedialog_open_filenames_hook)
                qt_filedialog_open_filenames_hook=0; // do not override
            return QFileDialog::getOpenFileNames(parent, caption, dir, filter, selectedFilter, options);
        }

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
                                   QFileDialog::Options options)
    {
        if (KFileDialogPrivate::isNative()) {
            if(qt_filedialog_save_filename_hook)
                qt_filedialog_save_filename_hook=0; // do not override
            return QFileDialog::getSaveFileName(parent, caption, dir, filter, selectedFilter, options);
        }

        KFileDialog dlg(KUrl(dir), qt2KdeFilter(filter), parent);

        dlg.setOperationMode(KFileDialog::Saving);
        dlg.setMode(KFile::File|KFile::LocalOnly);
        dlg.setCaption(caption);
        dlg.setConfirmOverwrite(!(options & QFileDialog::DontConfirmOverwrite));
        dlg.exec();

        QString rv(dlg.selectedFile());

        if(!rv.isEmpty())
            kde2QtFilter(filter, dlg.currentFilter(), selectedFilter);

        return rv;
    }

};

static KFileDialogQtOverride qtOverride;

#include "kfiledialog.moc"
