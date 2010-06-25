/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                  1999 - 2008 David Faure <faure@kde.org>
                  2001, 2006 Holger Freyther <freyther@kde.org>

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

#include "kio/renamedialog.h"
#include <stdio.h>
#include <assert.h>

#include <QtCore/QDate>
#include <QtCore/QFileInfo>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPixmap>
#include <QtGui/QScrollArea>
#include <QtCore/QDir>

#include <klineedit.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kio/global.h>
#include <kio/udsentry.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kurl.h>
#include <kfileitem.h>
#include <kmimetype.h>
#include <kseparator.h>
#include <kstringhandler.h>
#include <kstandardguiitem.h>
#include <kguiitem.h>
#include <ksqueezedtextlabel.h>
#include <kfilemetadatawidget.h>
#include <previewjob.h>

using namespace KIO;

/** @internal */
class RenameDialog::RenameDialogPrivate
{
 public:
  RenameDialogPrivate(){
    bCancel = 0;
    bRename = bSkip = bOverwrite = 0;
    bResume = bSuggestNewName = 0;
    bApplyAll = 0;
    m_pLineEdit = 0;
    m_srcPendingPreview = false;
    m_destPendingPreview = false;
    m_srcPreview = 0;
    m_destPreview = 0;
  }
  void setRenameBoxText(const QString& fileName) {
    // sets the text in file name line edit box, selecting the filename (but not the extension if there is one).
    const QString extension = KMimeType::extractKnownExtension(fileName);
    m_pLineEdit->setText(fileName);
    if (!extension.isEmpty()) {
        const int selectionLength = fileName.length() - extension.length() - 1;
        m_pLineEdit->setSelection(0, selectionLength);
    } else {
        m_pLineEdit->selectAll();
    }
  }
  KPushButton *bCancel;
  QPushButton *bRename;
  QPushButton *bSkip;
  QPushButton *bOverwrite;
  QPushButton *bResume;
  QPushButton *bSuggestNewName;
  QCheckBox *bApplyAll;
  KLineEdit* m_pLineEdit;
  KUrl src;
  KUrl dest;
  bool m_srcPendingPreview;
  bool m_destPendingPreview;
  QLabel* m_srcPreview;
  QLabel* m_destPreview;
};

RenameDialog::RenameDialog(QWidget *parent, const QString & _caption,
                     const KUrl &_src, const KUrl &_dest,
                     RenameDialog_Mode _mode,
                     KIO::filesize_t sizeSrc,
                     KIO::filesize_t sizeDest,
                     time_t ctimeSrc,
                     time_t ctimeDest,
                     time_t mtimeSrc,
                     time_t mtimeDest)
  : QDialog ( parent ), d(new RenameDialogPrivate)
{
    setObjectName( "KIO::RenameDialog" );

    d->src = _src;
    d->dest = _dest;

    setWindowTitle( _caption );

    d->bCancel = new KPushButton( KStandardGuiItem::cancel(), this );
    connect(d->bCancel, SIGNAL(clicked()), this, SLOT(cancelPressed()));


    if (_mode & M_MULTI) {
        d->bApplyAll = new QCheckBox(i18n("Appl&y to All"), this);
        d->bApplyAll->setToolTip((_mode & M_ISDIR) ? i18n("When this is checked the button pressed will be applied to all subsequent folder conflicts for the remainder of the current job.\nUnless you press Skip you will still be prompted in case of a conflict with an existing file in the directory.")
                                                      : i18n("When this is checked the button pressed will be applied to all subsequent conflicts for the remainder of the current job."));
        connect(d->bApplyAll, SIGNAL(clicked()), this, SLOT(applyAllPressed()));
    }

    if ( ! (_mode & M_NORENAME ) ) {
        d->bRename = new QPushButton( i18n( "&Rename" ), this );
        d->bRename->setEnabled(false);
        d->bSuggestNewName = new QPushButton( i18n( "Suggest New &Name" ), this );
        connect(d->bSuggestNewName, SIGNAL(clicked()), this, SLOT(suggestNewNamePressed()));
        connect(d->bRename, SIGNAL(clicked()), this, SLOT(renamePressed()));
    }

    if ( ( _mode & M_MULTI ) && ( _mode & M_SKIP ) ) {
        d->bSkip = new QPushButton( i18n( "&Skip" ), this );
        d->bSkip->setToolTip((_mode & M_ISDIR) ? i18n("Do not copy or move this folder, skip to the next item instead")
                             : i18n("Do not copy or move this file, skip to the next item instead"));
        connect(d->bSkip, SIGNAL(clicked()), this, SLOT(skipPressed()));
    }

    if ( _mode & M_OVERWRITE ) {
        const QString text = (_mode & M_ISDIR) ? i18nc("Write files into an existing folder", "&Write Into") : i18n("&Overwrite");
        d->bOverwrite = new QPushButton(text, this);
        d->bOverwrite->setToolTip(i18n("Files and folders will be copied into the existing directory, alongside its existing contents.\nYou will be prompted again in case of a conflict with an existing file in the directory."));
        connect(d->bOverwrite, SIGNAL(clicked()), this, SLOT(overwritePressed()));
    }

    if ( _mode & M_RESUME ) {
        d->bResume = new QPushButton( i18n( "&Resume" ), this );
        connect(d->bResume, SIGNAL(clicked()), this, SLOT(resumePressed()));
    }

    QVBoxLayout* pLayout = new QVBoxLayout( this );
    pLayout->addStrut( 360 );	// makes dlg at least that wide

    // User tries to overwrite a file with itself ?
    if ( _mode & M_OVERWRITE_ITSELF ) {
        QLabel *lb = new QLabel( i18n( "This action would overwrite '%1' with itself.\n"
                                       "Please enter a new file name:" ,  KStringHandler::csqueeze( d->src.pathOrUrl(),100 ) ), this );
        d->bRename->setText(i18n("C&ontinue"));
        pLayout->addWidget( lb );
    }
    else if ( _mode & M_OVERWRITE ) {
        KFileItem srcItem;
        KFileItem destItem;

        if (d->src.isLocalFile()) {
            srcItem = KFileItem(KFileItem::Unknown, KFileItem::Unknown, d->src);
        } else {
            UDSEntry srcUds;
            srcUds.insert(UDSEntry::UDS_NAME, d->src.fileName());
            srcUds.insert(UDSEntry::UDS_MODIFICATION_TIME, mtimeSrc);
            srcUds.insert(UDSEntry::UDS_CREATION_TIME, ctimeSrc);
            srcUds.insert(UDSEntry::UDS_SIZE, sizeSrc);

            srcItem = KFileItem(srcUds, d->src);
        }

        if (d->dest.isLocalFile()) {
            destItem = KFileItem(KFileItem::Unknown, KFileItem::Unknown, d->dest);
        } else {
            UDSEntry destUds;
            destUds.insert(UDSEntry::UDS_NAME, d->dest.fileName());
            destUds.insert(UDSEntry::UDS_MODIFICATION_TIME, mtimeDest);
            destUds.insert(UDSEntry::UDS_CREATION_TIME, ctimeDest);
            destUds.insert(UDSEntry::UDS_SIZE, sizeDest);

            destItem = KFileItem(destUds, d->dest);
        }

        d->m_srcPreview = createLabel(parent, QString());
        d->m_destPreview = createLabel(parent, QString());

        d->m_srcPreview->setMinimumHeight(KIconLoader::SizeEnormous);
        d->m_destPreview->setMinimumHeight(KIconLoader::SizeEnormous);

        d->m_srcPendingPreview = true;
        d->m_destPendingPreview = true;
        KIO::PreviewJob* srcJob = KIO::filePreview(KFileItemList() << srcItem,
                                                   d->m_srcPreview->width(),
                                                   d->m_srcPreview->height(),
                                                   0,
                                                   0,
                                                   false /*don't scale*/);
        KIO::PreviewJob* destJob = KIO::filePreview(KFileItemList() << destItem,
                                                    d->m_destPreview->width(),
                                                    d->m_destPreview->height(),
                                                    0,
                                                    0,
                                                    false /*don't scale*/);

        connect(srcJob, SIGNAL(gotPreview(const KFileItem&, const QPixmap&)),
                this, SLOT(showSrcPreview(const KFileItem&, const QPixmap&)));
        connect(destJob, SIGNAL(gotPreview(const KFileItem&, const QPixmap&)),
                this, SLOT(showDestPreview(const KFileItem&, const QPixmap&)));
        connect(srcJob, SIGNAL(failed(const KFileItem&)),
                this, SLOT(showSrcIcon(const KFileItem&)));
        connect(destJob, SIGNAL(failed(const KFileItem&)),
                this, SLOT(showDestIcon(const KFileItem&)));


        // widget
        QScrollArea* srcWidget = createContainerLayout(parent, srcItem, d->m_srcPreview);
        QScrollArea* destWidget = createContainerLayout(parent, destItem, d->m_destPreview);

        // create layout
        QGridLayout* gridLayout = new QGridLayout();
        pLayout->addLayout(gridLayout);

        QString sentence1;
        if (mtimeDest < mtimeSrc)
            sentence1 = i18n("This action will overwrite '%1' with a newer file '%2'.", d->dest.pathOrUrl(), d->src.pathOrUrl());
        else if (mtimeDest == mtimeSrc)
            sentence1 = i18n("This action will overwrite '%1' with a file of the same age '%2'.", d->dest.pathOrUrl(), d->src.pathOrUrl());
        else
            sentence1 = i18n("This action will overwrite '%1' with an older file '%2'.", d->dest.pathOrUrl(), d->src.pathOrUrl());

        QLabel* titleLabel = new KSqueezedTextLabel( sentence1, this );

        QLabel* srcTitle = createLabel(parent, i18n("Source"));
        QLabel* destTitle = createLabel(parent, i18n("Destination"));

        gridLayout->addWidget( titleLabel, 0, 0, 1, 2 ); // takes the complete first line

        gridLayout->addWidget( srcTitle, 1, 0 );
        gridLayout->addWidget( srcWidget, 2, 0 );

        gridLayout->addWidget( destTitle, 1, 1 );
        gridLayout->addWidget( destWidget, 2, 1 );

        gridLayout->setRowStretch(1, 2);
    }
    else
    {
        // This is the case where we don't want to allow overwriting, the existing
        // file must be preserved (e.g. when renaming).
        QString sentence1;
        if (mtimeDest < mtimeSrc)
            sentence1 = i18n("An older item named '%1' already exists.", d->dest.pathOrUrl());
        else if (mtimeDest == mtimeSrc)
            sentence1 = i18n("A similar file named '%1' already exists.", d->dest.pathOrUrl());
        else
            sentence1 = i18n("A newer item named '%1' already exists.", d->dest.pathOrUrl());

        QLabel *lb = new KSqueezedTextLabel( sentence1, this );
        pLayout->addWidget(lb);
    }
    QHBoxLayout* layout2 = new QHBoxLayout();
    pLayout->addLayout( layout2 );

    d->m_pLineEdit = new KLineEdit( this );
    layout2->addWidget( d->m_pLineEdit );
    if ( d->bRename ) {
        const QString fileName = d->dest.fileName();
        d->setRenameBoxText( KIO::decodeFileName( fileName ) );
        connect(d->m_pLineEdit, SIGNAL(textChanged(const QString &)),
                SLOT(enableRenameButton(const QString &)));
        d->m_pLineEdit->setFocus();
    } else {
        d->m_pLineEdit->hide();
    }
    if ( d->bSuggestNewName )
    {
        layout2->addWidget( d->bSuggestNewName );
        setTabOrder( d->m_pLineEdit, d->bSuggestNewName );
    }

    KSeparator* separator = new KSeparator( this );
    pLayout->addWidget( separator );

    QHBoxLayout* layout = new QHBoxLayout();
    pLayout->addLayout( layout );

    layout->addStretch(1);

    if (d->bApplyAll) {
        layout->addWidget(d->bApplyAll);
        setTabOrder(d->bApplyAll, d->bCancel);
    }
    if ( d->bRename )
    {
        layout->addWidget( d->bRename );
        setTabOrder( d->bRename, d->bCancel );
    }
    if ( d->bSkip )
    {
        layout->addWidget( d->bSkip );
        setTabOrder( d->bSkip, d->bCancel );
    }
    if ( d->bOverwrite )
    {
        layout->addWidget( d->bOverwrite );
        setTabOrder( d->bOverwrite, d->bCancel );
    }
    if ( d->bResume )
    {
        layout->addWidget( d->bResume );
        setTabOrder( d->bResume, d->bCancel );
    }

    d->bCancel->setDefault( true );
    layout->addWidget( d->bCancel );

    resize( sizeHint() );
}

RenameDialog::~RenameDialog()
{
  delete d;
  // no need to delete Pushbuttons,... qt will do this
}

void RenameDialog::enableRenameButton(const QString &newDest)
{
  if ( newDest != KIO::decodeFileName( d->dest.fileName() ) && !newDest.isEmpty() )
  {
    d->bRename->setEnabled( true );
    d->bRename->setDefault( true );
    if ( d->bOverwrite )
        d->bOverwrite->setEnabled( false ); // prevent confusion (#83114)
  }
  else
  {
    d->bRename->setEnabled( false );
    if ( d->bOverwrite )
        d->bOverwrite->setEnabled( true );
  }
}

KUrl RenameDialog::newDestUrl()
{
  KUrl newDest( d->dest );
  QString fileName = d->m_pLineEdit->text();
  newDest.setFileName( KIO::encodeFileName( fileName ) );
  return newDest;
}

KUrl RenameDialog::autoDestUrl() const
{
    KUrl newDest(d->dest);
    KUrl destDirectory(d->dest);
    destDirectory.setPath(destDirectory.directory());
    newDest.setFileName(suggestName(destDirectory, d->dest.fileName()));
    return newDest;
}

void RenameDialog::cancelPressed()
{
    done( R_CANCEL );
}

// Rename
void RenameDialog::renamePressed()
{
    if (d->m_pLineEdit->text().isEmpty()) {
        return;
    }

    if (d->bApplyAll  && d->bApplyAll->isChecked()) {
        done(R_AUTO_RENAME);
    }
    else {
        KUrl u = newDestUrl();
        if (!u.isValid()) {
            KMessageBox::error(this, i18n("Malformed URL\n%1" ,  u.url()));
            return;
        }
        done(R_RENAME);
    }
}

QString RenameDialog::suggestName(const KUrl& baseURL, const QString& oldName)
{
  QString dotSuffix, suggestedName;
  QString basename = oldName;

  int index = basename.indexOf( '.' );
  if ( index != -1 ) {
    dotSuffix = basename.mid( index );
    basename.truncate( index );
  }

  int pos = basename.lastIndexOf( '_' );
  if(pos != -1 ){
    QString tmp = basename.mid( pos+1 );
    bool ok;
    int number = tmp.toInt( &ok );
    if ( !ok ) {// ok there is no number
      suggestedName = basename + '1' + dotSuffix;
    }
    else {
     // yes there's already a number behind the _ so increment it by one
      basename.replace( pos+1, tmp.length(), QString::number(number+1) );
      suggestedName = basename + dotSuffix;
    }
  }
  else // no underscore yet
    suggestedName = basename + "_1" + dotSuffix ;

  // Check if suggested name already exists
  bool exists = false;
  // TODO: network transparency. However, using NetAccess from a modal dialog
  // could be a problem, no? (given that it uses a modal widget itself....)
  if ( baseURL.isLocalFile() )
    exists = QFileInfo( baseURL.toLocalFile(KUrl::AddTrailingSlash) + suggestedName ).exists();

  if ( !exists )
    return suggestedName;
  else // already exists -> recurse
    return suggestName( baseURL, suggestedName );
}

// Propose button clicked
void RenameDialog::suggestNewNamePressed()
{
  /* no name to play with */
  if ( d->m_pLineEdit->text().isEmpty() )
    return;

  KUrl destDirectory( d->dest );
  destDirectory.setPath( destDirectory.directory() );
  d->setRenameBoxText( suggestName( destDirectory, d->m_pLineEdit->text() ) );
  return;
}

void RenameDialog::skipPressed()
{
    if (d->bApplyAll  && d->bApplyAll->isChecked()) {
        done(R_AUTO_SKIP);
    }
    else {
        done(R_SKIP);
    }
}

void RenameDialog::autoSkipPressed()
{
  done( R_AUTO_SKIP );
}

void RenameDialog::overwritePressed()
{
    if (d->bApplyAll  && d->bApplyAll->isChecked()) {
        done(R_OVERWRITE_ALL);
    }
    else {
        done(R_OVERWRITE);
    }
}

void RenameDialog::overwriteAllPressed()
{
  done( R_OVERWRITE_ALL );
}

void RenameDialog::resumePressed()
{
    if (d->bApplyAll  && d->bApplyAll->isChecked()) {
        done(R_RESUME_ALL);
    }
    else {
        done(R_RESUME);
    }
}

void RenameDialog::resumeAllPressed()
{
  done( R_RESUME_ALL );
}

void RenameDialog::applyAllPressed()
{
    if (d->bApplyAll  && d->bApplyAll->isChecked()) {
        d->m_pLineEdit->setText(KIO::decodeFileName(d->dest.fileName()));
        d->m_pLineEdit->setEnabled(false);

        if (d->bRename) {
            d->bRename->setEnabled(true);
        }
        if (d->bSuggestNewName) {
            d->bSuggestNewName->setEnabled(false);
        }
    }
    else {
        d->m_pLineEdit->setEnabled(true);

        if (d->bRename) {
            d->bRename->setEnabled(false);
        }
        if (d->bSuggestNewName) {
            d->bSuggestNewName->setEnabled(true);
        }
    }
}

void RenameDialog::showSrcIcon(const KFileItem& fileitem)
{
    // The preview job failed, show a standard file icon.
    d->m_destPendingPreview = false;
    d->m_destPreview->setPixmap(fileitem.pixmap(d->m_srcPreview->height()));
}

void RenameDialog::showDestIcon(const KFileItem& fileitem)
{
    // The preview job failed, show a standard file icon.
    d->m_srcPendingPreview = false;
    d->m_srcPreview->setPixmap(fileitem.pixmap(d->m_srcPreview->height()));
}

void RenameDialog::showSrcPreview(const KFileItem& fileitem, const QPixmap& pixmap)
{
    Q_UNUSED(fileitem);
    if (d->m_srcPendingPreview) {
        d->m_srcPreview->setPixmap(pixmap);
        d->m_srcPendingPreview = false;
    }
}

void RenameDialog::showDestPreview(const KFileItem& fileitem, const QPixmap& pixmap)
{
    Q_UNUSED(fileitem);
    if (d->m_destPendingPreview) {
        d->m_destPreview->setPixmap(pixmap);
        d->m_destPendingPreview = false;
    }
}

QScrollArea* RenameDialog::createContainerLayout(QWidget* parent, const KFileItem& item, QLabel* preview)
{
    KFileItemList itemList;
    itemList << item;

    // widget
    KFileMetaDataWidget* metaWidget =  new KFileMetaDataWidget(this);

    metaWidget->setReadOnly(true);

    metaWidget->setItems(itemList);

    // Encapsulate the MetaDataWidgets inside a container with stretch at the bottom.
    // This prevents that the meta data widgets get vertically stretched
    // in the case where the height of m_metaDataArea > m_metaDataWidget.

    QWidget* widgetContainer = new QWidget(parent);
    QVBoxLayout* containerLayout = new QVBoxLayout(widgetContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);
    containerLayout->addWidget(preview);
    containerLayout->addWidget(metaWidget);
    containerLayout->addStretch(1);

    QScrollArea* metaDataArea = new QScrollArea(parent);
    metaDataArea->setWidget(widgetContainer);
    metaDataArea->setWidgetResizable(true);
    metaDataArea->setFrameShape(QFrame::NoFrame);

    return metaDataArea;
}

QLabel* RenameDialog::createLabel(QWidget* parent, const QString& text)
{
    QLabel* label = new QLabel(parent);

    QFont font = label->font();
    font.setBold(true);
    label->setFont(font);

    label->setAlignment(Qt::AlignHCenter);

    label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    label->setText(text);

    return label;
}

#include "renamedialog.moc"
