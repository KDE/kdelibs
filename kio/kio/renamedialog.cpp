/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
                  2001,2006 Holger Freyther <freyther@kde.org>

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
#include "kio/renamedialogplugin.h"
#include <stdio.h>
#include <assert.h>

#include <QtCore/QDate>
#include <QtCore/QFileInfo>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtCore/QDir>

#include <klineedit.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kio/global.h>
#include <kmimetypetrader.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kurl.h>
#include <kmimetype.h>
#include <kseparator.h>
#include <kstringhandler.h>
#include <kstandardguiitem.h>
#include <kguiitem.h>
#include <ksqueezedtextlabel.h>

using namespace KIO;

/** @internal */
class RenameDialog::RenameDialogPrivate
{
 public:
  RenameDialogPrivate(){
    bCancel = 0;
    bRename = bSkip = bAutoSkip = bOverwrite = bOverwriteAll = 0;
    bResume = bResumeAll = bSuggestNewName = 0;
    m_pLineEdit = 0;
  }
  KPushButton *bCancel;
  QPushButton *bRename;
  QPushButton *bSkip;
  QPushButton *bAutoSkip;
  QPushButton *bOverwrite;
  QPushButton *bOverwriteAll;
  QPushButton *bResume;
  QPushButton *bResumeAll;
  QPushButton *bSuggestNewName;
  KLineEdit* m_pLineEdit;
  KUrl src;
  KUrl dest;
  QString mimeSrc;
  QString mimeDest;
  bool plugin;
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
    d->plugin = false;

    setWindowTitle( _caption );

    d->bCancel = new KPushButton( KStandardGuiItem::cancel(), this );
    connect(d->bCancel, SIGNAL(clicked()), this, SLOT(cancelPressed()));

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

        d->bAutoSkip = new QPushButton( i18n( "&Auto Skip" ), this );
        d->bAutoSkip->setToolTip((_mode & M_ISDIR) ? i18n("Do not copy or move any folder that already exists in the destination folder.\nYou will be prompted again in case of a conflict with an existing file though.")
                                 : i18n("Do not copy or move any file that already exists in the destination folder.\nYou will be prompted again in case of a conflict with an existing directory though."));
        connect(d->bAutoSkip, SIGNAL(clicked()), this, SLOT(autoSkipPressed()));
    }

    if ( _mode & M_OVERWRITE ) {
        const QString text = (_mode & M_ISDIR) ? i18nc("Write files into an existing folder", "&Write Into") : i18n("&Overwrite");
        d->bOverwrite = new QPushButton(text, this);
        d->bOverwrite->setToolTip(i18n("Files and folders will be copied into the existing directory, alongside its existing contents.\nYou will be prompted again in case of a conflict with an existing file in the directory."));
        connect(d->bOverwrite, SIGNAL(clicked()), this, SLOT(overwritePressed()));

        if ( _mode & M_MULTI ) {
            const QString textAll = (_mode & M_ISDIR) ? i18nc("Write files into any existing directory", "&Write Into All") : i18n("&Overwrite All");
            d->bOverwriteAll = new QPushButton( textAll, this );
            d->bOverwriteAll->setToolTip(i18n("Files and folders will be copied into any existing directory, alongside its existing contents.\nYou will be prompted again in case of a conflict with an existing file in a directory, but not in case of another existing directory."));
            connect(d->bOverwriteAll, SIGNAL(clicked()), this, SLOT(overwriteAllPressed()));
        }
    }

    if ( _mode & M_RESUME ) {
        d->bResume = new QPushButton( i18n( "&Resume" ), this );
        connect(d->bResume, SIGNAL(clicked()), this, SLOT(resumePressed()));

        if ( _mode & M_MULTI )
        {
            d->bResumeAll = new QPushButton( i18n( "R&esume All" ), this );
            connect(d->bResumeAll, SIGNAL(clicked()), this, SLOT(resumeAllPressed()));
        }
    }

    QVBoxLayout* pLayout = new QVBoxLayout( this );
    pLayout->setMargin( KDialog::marginHint() );
    pLayout->setSpacing( KDialog::spacingHint() );
    pLayout->addStrut( 360 );	// makes dlg at least that wide

    // User tries to overwrite a file with itself ?
    if ( _mode & M_OVERWRITE_ITSELF ) {
        QLabel *lb = new QLabel( i18n( "This action would overwrite '%1' with itself.\n"
                                       "Please enter a new file name:" ,  KStringHandler::csqueeze( d->src.pathOrUrl(),100 ) ), this );
        d->bRename->setText(i18n("C&ontinue"));
        pLayout->addWidget( lb );
    }
    else if ( _mode & M_OVERWRITE ) {

        // Figure out the mimetype and load one plugin
        // (This is the only mode that is handled by plugins)
        pluginHandling();
        KService::List plugin_offers;
        if( d->mimeSrc != KMimeType::defaultMimeType()   ){
            plugin_offers = KMimeTypeTrader::self()->query(d->mimeSrc, "RenameDialog/Plugin");

        }else if(d->mimeDest != KMimeType::defaultMimeType() ) {
            plugin_offers = KMimeTypeTrader::self()->query(d->mimeDest, "RenameDialog/Plugin");
        }
        if(!plugin_offers.isEmpty() ){
            RenameDialogPlugin::FileItem src( _src, d->mimeSrc, sizeSrc, ctimeSrc, mtimeSrc );
            RenameDialogPlugin::FileItem dst( _dest,d->mimeDest, sizeDest, ctimeDest, mtimeDest );
            foreach (const KService::Ptr &ptr, plugin_offers) {
                RenameDialogPlugin *plugin = ptr->createInstance<RenameDialogPlugin>(this);
                if( !plugin )
                    continue;

                plugin->setObjectName( ptr->name() );
                if( plugin->wantToHandle( _mode, src, dst ) ) {
                    d->plugin = true;
                    plugin->handle( _mode, src, dst );
                    pLayout->addWidget(plugin );
                    break;
                } else {
                    delete plugin;
                }
            }

        }

        if( !d->plugin ){
            // No plugin found, build default dialog
            QGridLayout * gridLayout = new QGridLayout();
            gridLayout->setMargin( KDialog::marginHint() );
            gridLayout->setSpacing( KDialog::spacingHint() );
            pLayout->addLayout(gridLayout);
            gridLayout->setColumnStretch(0,0);
            gridLayout->setColumnStretch(1,10);

            QString sentence1;
            if (mtimeDest < mtimeSrc)
                sentence1 = i18n("An older item named '%1' already exists.", d->dest.pathOrUrl());
            else if (mtimeDest == mtimeSrc)
                sentence1 = i18n("A similar file named '%1' already exists.", d->dest.pathOrUrl());
            else
                sentence1 = i18n("A newer item named '%1' already exists.", d->dest.pathOrUrl());

            QLabel * lb1 = new KSqueezedTextLabel( sentence1, this );
            gridLayout->addWidget( lb1, 0, 0, 1, 2 ); // takes the complete first line

            lb1 = new QLabel( this );
            lb1->setPixmap( KIO::pixmapForUrl( d->dest ) );
            gridLayout->addWidget( lb1, 1, 0, 3, 1 ); // takes the first column on rows 1-3

            int row = 1;
            if ( sizeDest != (KIO::filesize_t)-1 )
            {
                QLabel * lb = new QLabel( i18n("size %1",  KIO::convertSize(sizeDest) ), this );
                gridLayout->addWidget( lb, row, 1 );
                row++;

            }
            if ( ctimeDest != (time_t)-1 )
            {
                QDateTime dctime; dctime.setTime_t( ctimeDest );
                QLabel * lb = new QLabel( i18n("created on %1",  KGlobal::locale()->formatDateTime(dctime) ), this );
                gridLayout->addWidget( lb, row, 1 );
                row++;
            }
            if ( mtimeDest != (time_t)-1 )
            {
                QDateTime dmtime; dmtime.setTime_t( mtimeDest );
                QLabel * lb = new QLabel( i18n("modified on %1",  KGlobal::locale()->formatDateTime(dmtime) ), this );
                gridLayout->addWidget( lb, row, 1 );
                row++;
            }

            if ( !d->src.isEmpty() )
            {
                // rows 1 to 3 are the details (size/ctime/mtime), row 4 is empty

                QLabel * lb2 = new KSqueezedTextLabel( i18n("The source file is '%1'", d->src.pathOrUrl()), this );
                gridLayout->addWidget( lb2, 5, 0, 1, 2 ); // takes the complete first line

                lb2 = new QLabel( this );
                lb2->setPixmap( KIO::pixmapForUrl( d->src ) );
                gridLayout->addWidget( lb2, 6, 0, 3, 1 ); // takes the first column on rows 6-8

                row = 6;

                if ( sizeSrc != (KIO::filesize_t)-1 )
                {
                    QLabel * lb = new QLabel( i18n("size %1",  KIO::convertSize(sizeSrc) ), this );
                    gridLayout->addWidget( lb, row, 1 );
                    row++;
                }
                if ( ctimeSrc != (time_t)-1 )
                {
                    QDateTime dctime; dctime.setTime_t( ctimeSrc );
                    QLabel * lb = new QLabel( i18n("created on %1",  KGlobal::locale()->formatDateTime(dctime) ), this );
                    gridLayout->addWidget( lb, row, 1 );
                    row++;
                }
                if ( mtimeSrc != (time_t)-1 )
                {
                    QDateTime dmtime; dmtime.setTime_t( mtimeSrc );
                    QLabel * lb = new QLabel( i18n("modified on %1",  KGlobal::locale()->formatDateTime(dmtime) ), this );
                    gridLayout->addWidget( lb, row, 1 );
                    row++;
                }
            }
        }
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
    QString fileName = d->dest.fileName();
    d->m_pLineEdit->setText( KIO::decodeFileName( fileName ) );
    if ( d->bRename || d->bOverwrite )
        connect(d->m_pLineEdit, SIGNAL(textChanged(const QString &)),
                SLOT(enableRenameButton(const QString &)));
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
    if ( d->bAutoSkip )
    {
        layout->addWidget( d->bAutoSkip );
        setTabOrder( d->bAutoSkip, d->bCancel );
    }
    if ( d->bOverwrite )
    {
        layout->addWidget( d->bOverwrite );
        setTabOrder( d->bOverwrite, d->bCancel );
    }
    if ( d->bOverwriteAll )
    {
        layout->addWidget( d->bOverwriteAll );
        setTabOrder( d->bOverwriteAll, d->bCancel );
    }
    if ( d->bResume )
    {
        layout->addWidget( d->bResume );
        setTabOrder( d->bResume, d->bCancel );
    }
    if ( d->bResumeAll )
    {
        layout->addWidget( d->bResumeAll );
        setTabOrder( d->bResumeAll, d->bCancel );
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

void RenameDialog::cancelPressed()
{
    done( R_CANCEL );
}

// Rename
void RenameDialog::renamePressed()
{
  if ( d->m_pLineEdit->text().isEmpty() )
    return;

  KUrl u = newDestUrl();
  if ( !u.isValid() )
  {
    KMessageBox::error( this, i18n( "Malformed URL\n%1" ,  u.url() ) );
    return;
  }

  done( R_RENAME );
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
    exists = QFileInfo( baseURL.path(KUrl::AddTrailingSlash) + suggestedName ).exists();

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
  d->m_pLineEdit->setText( suggestName( destDirectory, d->m_pLineEdit->text() ) );
  return;
}

void RenameDialog::skipPressed()
{
  done( R_SKIP );
}

void RenameDialog::autoSkipPressed()
{
  done( R_AUTO_SKIP );
}

void RenameDialog::overwritePressed()
{
  done( R_OVERWRITE );
}

void RenameDialog::overwriteAllPressed()
{
  done( R_OVERWRITE_ALL );
}

void RenameDialog::resumePressed()
{
  done( R_RESUME );
}

void RenameDialog::resumeAllPressed()
{
  done( R_RESUME_ALL );
}

static QString mime( const KUrl& src )
{
  KMimeType::Ptr type = KMimeType::findByUrl( src );
  //if( type->name() == KMimeType::defaultMimeType() ){ // ok no mimetype
    //    QString ty = KIO::NetAccess::mimetype(d->src );
    // return ty;
    return type->name();
}

/** This will figure out the mimetypes and query for a plugin
 *  Loads it then and asks the plugin if it wants to do the job
 *  We'll take the first available mimetype
 *  The scanning for a mimetype will be done in 2 ways
 *
 */
void RenameDialog::pluginHandling()
{
  d->mimeSrc = mime( d->src );
  d->mimeDest = mime(d->dest );

  kDebug(7024) << "Source Mimetype: "<< d->mimeSrc;
  kDebug(7024) << "Dest Mimetype: "<< d->mimeDest;
}

#include "renamedialog.moc"
