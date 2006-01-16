/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
                  2001 Holger Freyther <freyther@kde.org>

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

#include "kio/renamedlg.h"
#include "kio/renamedlgplugin.h"
#include <stdio.h>
#include <assert.h>

#include <qdatetime.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qdir.h>

#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kapplication.h>
#include <kio/global.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kurl.h>
#include <kmimetype.h>
#include <kseparator.h>
#include <kstringhandler.h>
#include <kstdguiitem.h>
#include <kguiitem.h>
#include <ksqueezedtextlabel.h>

#ifdef Q_WS_X11
#include <kwin.h>
#endif

using namespace KIO;

class RenameDlg::RenameDlgPrivate
{
 public:
  RenameDlgPrivate(){
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
  QLineEdit* m_pLineEdit;
  KURL src;
  KURL dest;
  QString mimeSrc;
  QString mimeDest;
  bool modal;
  bool plugin;
};

RenameDlg::RenameDlg(QWidget *parent, const QString & _caption,
                     const QString &_src, const QString &_dest,
                     RenameDlg_Mode _mode,
                     KIO::filesize_t sizeSrc,
                     KIO::filesize_t sizeDest,
                     time_t ctimeSrc,
                     time_t ctimeDest,
                     time_t mtimeSrc,
                     time_t mtimeDest,
                     bool _modal)
  : QDialog ( parent, "KIO::RenameDialog" , _modal ),d(new RenameDlgPrivate( ))
{
    d->modal = _modal;
#if 0
    // Set "StaysOnTop", because this dialog is typically used in kio_uiserver,
    // i.e. in a separate process.
    // ####### This isn't the case anymore - remove?
#if !defined(Q_WS_QWS) && !defined(Q_WS_WIN) //FIXME(E): Implement for QT Embedded & win32
    if (d->modal)
        KWin::setState( winId(), NET::StaysOnTop );
#endif
#endif

    d->src = _src;
    d->dest = _dest;
    d->plugin = false;


    setCaption( _caption );

    d->bCancel = new KPushButton( KStdGuiItem::cancel(), this );
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
        connect(d->bSkip, SIGNAL(clicked()), this, SLOT(skipPressed()));

        d->bAutoSkip = new QPushButton( i18n( "&Auto Skip" ), this );
        connect(d->bAutoSkip, SIGNAL(clicked()), this, SLOT(autoSkipPressed()));
    }

    if ( _mode & M_OVERWRITE ) {
        d->bOverwrite = new QPushButton( i18n( "&Overwrite" ), this );
        connect(d->bOverwrite, SIGNAL(clicked()), this, SLOT(overwritePressed()));

        if ( _mode & M_MULTI ) {
            d->bOverwriteAll = new QPushButton( i18n( "O&verwrite All" ), this );
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

    QVBoxLayout* pLayout = new QVBoxLayout( this, KDialog::marginHint(),
                                    KDialog::spacingHint() );
    pLayout->addStrut( 360 );	// makes dlg at least that wide

    // User tries to overwrite a file with itself ?
    if ( _mode & M_OVERWRITE_ITSELF ) {
        QLabel *lb = new QLabel( i18n( "This action would overwrite '%1' with itself.\n"
                                       "Please enter a new file name:" ).arg( KStringHandler::csqueeze( d->src.pathOrURL(),100 ) ), this );
        d->bRename->setText(i18n("C&ontinue"));
        pLayout->addWidget( lb );
    }
    else if ( _mode & M_OVERWRITE ) {

        // Figure out the mimetype and load one plugin
        // (This is the only mode that is handled by plugins)
        pluginHandling();
        KTrader::OfferList plugin_offers;
        if( d->mimeSrc != KMimeType::defaultMimeType()   ){
            plugin_offers = KTrader::self()->query(d->mimeSrc, "'RenameDlg/Plugin' in ServiceTypes");

        }else if(d->mimeDest != KMimeType::defaultMimeType() ) {
            plugin_offers = KTrader::self()->query(d->mimeDest, "'RenameDlg/Plugin' in ServiceTypes");
        }
        if(!plugin_offers.isEmpty() ){
            kdDebug(7024) << "Offers" << endl;
            KTrader::OfferList::ConstIterator it = plugin_offers.begin();
            KTrader::OfferList::ConstIterator end = plugin_offers.end();
            for( ; it != end; ++it ){
                QString libName = (*it)->library();
                if( libName.isEmpty() ){
                    kdDebug(7024) << "lib is empty" << endl;
                    continue;
                }
                KLibrary *lib = KLibLoader::self()->library(libName.toLocal8Bit() );
                if(!lib) {
                    continue;
                }
                KLibFactory *factory = lib->factory();
                if(!factory){
                    lib->unload();
                    continue;
                }
                QObject *obj = factory->create( this, (*it)->name().latin1() );
                if(!obj) {
                    lib->unload();
                    continue;
                }
                RenameDlgPlugin *plugin = static_cast<RenameDlgPlugin *>(obj);
                if(!plugin ){
                    delete obj;
                    continue;
                }
                if( plugin->initialize( _mode, _src, _dest, d->mimeSrc,
                                        d->mimeDest, sizeSrc, sizeDest,
                                        ctimeSrc, ctimeDest,
                                        mtimeSrc, mtimeDest ) ) {
                    d->plugin = true;
                    pLayout->addWidget(plugin );
                    kdDebug(7024) << "RenameDlgPlugin" << endl;
                    break;
                } else {
                    delete obj;
                }
            }

        }

        if( !d->plugin ){
            // No plugin found, build default dialog
            QGridLayout * gridLayout = new QGridLayout( 0L, 9, 2, KDialog::marginHint(),
                                                        KDialog::spacingHint() );
            pLayout->addLayout(gridLayout);
            gridLayout->setColStretch(0,0);
            gridLayout->setColStretch(1,10);

            QString sentence1;
            if (mtimeDest < mtimeSrc)
                sentence1 = i18n("An older item named '%1' already exists.");
            else if (mtimeDest == mtimeSrc)
                sentence1 = i18n("A similar file named '%1' already exists.");
            else
                sentence1 = i18n("A newer item named '%1' already exists.");

            QLabel * lb1 = new KSqueezedTextLabel( sentence1.arg(d->dest.pathOrURL() ), this );
            gridLayout->addMultiCellWidget( lb1, 0, 0, 0, 1 ); // takes the complete first line

            lb1 = new QLabel( this );
            lb1->setPixmap( KMimeType::pixmapForURL( d->dest ) );
            gridLayout->addMultiCellWidget( lb1, 1, 3, 0, 0 ); // takes the first column on rows 1-3

            int row = 1;
            if ( sizeDest != (KIO::filesize_t)-1 )
            {
                QLabel * lb = new QLabel( i18n("size %1").arg( KIO::convertSize(sizeDest) ), this );
                gridLayout->addWidget( lb, row, 1 );
                row++;

            }
            if ( ctimeDest != (time_t)-1 )
            {
                QDateTime dctime; dctime.setTime_t( ctimeDest );
                QLabel * lb = new QLabel( i18n("created on %1").arg( KGlobal::locale()->formatDateTime(dctime) ), this );
                gridLayout->addWidget( lb, row, 1 );
                row++;
            }
            if ( mtimeDest != (time_t)-1 )
            {
                QDateTime dmtime; dmtime.setTime_t( mtimeDest );
                QLabel * lb = new QLabel( i18n("modified on %1").arg( KGlobal::locale()->formatDateTime(dmtime) ), this );
                gridLayout->addWidget( lb, row, 1 );
                row++;
            }

            if ( !d->src.isEmpty() )
            {
                // rows 1 to 3 are the details (size/ctime/mtime), row 4 is empty
                gridLayout->addRowSpacing( 4, 20 );

                QLabel * lb2 = new KSqueezedTextLabel( i18n("The source file is '%1'").arg(d->src.pathOrURL()), this );
                gridLayout->addMultiCellWidget( lb2, 5, 5, 0, 1 ); // takes the complete first line

                lb2 = new QLabel( this );
                lb2->setPixmap( KMimeType::pixmapForURL( d->src ) );
                gridLayout->addMultiCellWidget( lb2, 6, 8, 0, 0 ); // takes the first column on rows 6-8

                row = 6;

                if ( sizeSrc != (KIO::filesize_t)-1 )
                {
                    QLabel * lb = new QLabel( i18n("size %1").arg( KIO::convertSize(sizeSrc) ), this );
                    gridLayout->addWidget( lb, row, 1 );
                    row++;
                }
                if ( ctimeSrc != (time_t)-1 )
                {
                    QDateTime dctime; dctime.setTime_t( ctimeSrc );
                    QLabel * lb = new QLabel( i18n("created on %1").arg( KGlobal::locale()->formatDateTime(dctime) ), this );
                    gridLayout->addWidget( lb, row, 1 );
                    row++;
                }
                if ( mtimeSrc != (time_t)-1 )
                {
                    QDateTime dmtime; dmtime.setTime_t( mtimeSrc );
                    QLabel * lb = new QLabel( i18n("modified on %1").arg( KGlobal::locale()->formatDateTime(dmtime) ), this );
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
            sentence1 = i18n("An older item named '%1' already exists.");
        else if (mtimeDest == mtimeSrc)
            sentence1 = i18n("A similar file named '%1' already exists.");
        else
            sentence1 = i18n("A newer item named '%1' already exists.");

        QLabel *lb = new KSqueezedTextLabel( sentence1.arg(d->dest.pathOrURL()), this );
        pLayout->addWidget(lb);
    }
    QHBoxLayout* layout2 = new QHBoxLayout();
    pLayout->addLayout( layout2 );

    d->m_pLineEdit = new QLineEdit( this );
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

RenameDlg::~RenameDlg()
{
  delete d;
  // no need to delete Pushbuttons,... qt will do this
}

void RenameDlg::enableRenameButton(const QString &newDest)
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

KURL RenameDlg::newDestURL()
{
  KURL newDest( d->dest );
  QString fileName = d->m_pLineEdit->text();
  newDest.setFileName( KIO::encodeFileName( fileName ) );
  return newDest;
}

void RenameDlg::cancelPressed()
{
  done( 0 );
}

// Rename
void RenameDlg::renamePressed()
{
  if ( d->m_pLineEdit->text().isEmpty() )
    return;

  KURL u = newDestURL();
  if ( !u.isValid() )
  {
    KMessageBox::error( this, i18n( "Malformed URL\n%1" ).arg( u.url() ) );
    return;
  }

  done( 1 );
}

QString RenameDlg::suggestName(const KURL& baseURL, const QString& oldName)
{
  QString dotSuffix, suggestedName;
  QString basename = oldName;

  int index = basename.find( '.' );
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
      suggestedName = basename + "1" + dotSuffix;
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
     exists = QFileInfo( baseURL.path(+1) + suggestedName ).exists();

  if ( !exists )
    return suggestedName;
  else // already exists -> recurse
    return suggestName( baseURL, suggestedName );
}

// Propose button clicked
void RenameDlg::suggestNewNamePressed()
{
  /* no name to play with */
  if ( d->m_pLineEdit->text().isEmpty() )
    return;

  KURL destDirectory( d->dest );
  destDirectory.setPath( destDirectory.directory() );
  d->m_pLineEdit->setText( suggestName( destDirectory, d->m_pLineEdit->text() ) );
  return;
}

void RenameDlg::skipPressed()
{
  done( 2 );
}

void RenameDlg::autoSkipPressed()
{
  done( 3 );
}

void RenameDlg::overwritePressed()
{
  done( 4 );
}

void RenameDlg::overwriteAllPressed()
{
  done( 5 );
}

void RenameDlg::resumePressed()
{
  done( 6 );
}

void RenameDlg::resumeAllPressed()
{
  done( 7 );
}

static QString mime( const KURL& src )
{
  KMimeType::Ptr type = KMimeType::findByURL( src );
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
void RenameDlg::pluginHandling()
{
  d->mimeSrc = mime( d->src );
  d->mimeDest = mime(d->dest );

  kdDebug(7024) << "Source Mimetype: "<< d->mimeSrc << endl;
  kdDebug(7024) << "Dest Mimetype: "<< d->mimeDest << endl;
}


RenameDlg_Result KIO::open_RenameDlg( const QString & _caption,
                                      const QString & _src, const QString & _dest,
                                      RenameDlg_Mode _mode,
                                      QString& _new,
                                      KIO::filesize_t sizeSrc,
                                      KIO::filesize_t sizeDest,
                                      time_t ctimeSrc,
                                      time_t ctimeDest,
                                      time_t mtimeSrc,
                                      time_t mtimeDest)
{
  Q_ASSERT(kapp);

  RenameDlg dlg( 0L, _caption, _src, _dest, _mode,
                 sizeSrc, sizeDest, ctimeSrc, ctimeDest, mtimeSrc, mtimeDest,
                 true /*modal*/ );
  int i = dlg.exec();
  _new = dlg.newDestURL().path();

  return (RenameDlg_Result)i;
}

#include "renamedlg.moc"





