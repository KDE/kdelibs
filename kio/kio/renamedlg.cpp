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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kio/renamedlg.h"
#include "kio/renamedlgplugin.h"
#include <stdio.h>
#include <assert.h>

#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>

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
#include <kwin.h>
#include <kstringhandler.h>
#include <kstdguiitem.h>
#include <kguiitem.h>

using namespace KIO;

class RenameDlg::RenameDlgPrivate
{
 public:
  RenameDlgPrivate(){
    b0 = 0L;
    b1 = b2 = b3 = b4 = b5 = b6 = b7 = b8 = 0L;
    m_pLineEdit=0L;
    m_pLayout=0L;
  }
  KPushButton *b0;
  QPushButton *b1;
  QPushButton *b2;
  QPushButton *b3;
  QPushButton *b4;
  QPushButton *b5;
  QPushButton *b6;
  QPushButton *b7;
  QPushButton *b8; //why isn't it an array
  QLineEdit* m_pLineEdit;
  QVBoxLayout* m_pLayout; // ### doesn't need to be here
  QString src;
  QString dest;
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
  : QDialog ( parent, "KIO::RenameDialog" , _modal )
{
    d = new RenameDlgPrivate( );
    d->modal = _modal;
    // Set "StaysOnTop", because this dialog is typically used in kio_uiserver,
    // i.e. in a separate process.
    // ####### This isn't the case anymore - remove?
#ifndef Q_WS_QWS //FIXME(E): Implement for QT Embedded
    if (d->modal)
        KWin::setState( winId(), NET::StaysOnTop );
#endif

    d->src = _src;
    d->dest = _dest;
    d->plugin = false;


    setCaption( _caption );

    d->b0 = new KPushButton( KStdGuiItem::cancel(), this );
    connect(d->b0, SIGNAL(clicked()), this, SLOT(b0Pressed()));

    if ( ! (_mode & M_NORENAME ) ) {
        d->b1 = new QPushButton( i18n( "&Rename" ), this );
        d->b1->setEnabled(false);
        d->b8 = new QPushButton( i18n( "&Propose" ), this );
        connect(d->b8, SIGNAL(clicked()), this, SLOT(b8Pressed()));
        connect(d->b1, SIGNAL(clicked()), this, SLOT(b1Pressed()));
    }

    if ( ( _mode & M_MULTI ) && ( _mode & M_SKIP ) ) {
        d->b2 = new QPushButton( i18n( "&Skip" ), this );
        connect(d->b2, SIGNAL(clicked()), this, SLOT(b2Pressed()));

        d->b3 = new QPushButton( i18n( "&Auto Skip" ), this );
        connect(d->b3, SIGNAL(clicked()), this, SLOT(b3Pressed()));
    }

    if ( _mode & M_OVERWRITE ) {
        d->b4 = new QPushButton( i18n( "&Overwrite" ), this );
        connect(d->b4, SIGNAL(clicked()), this, SLOT(b4Pressed()));

        if ( _mode & M_MULTI ) {
            d->b5 = new QPushButton( i18n( "O&verwrite All" ), this );
            connect(d->b5, SIGNAL(clicked()), this, SLOT(b5Pressed()));
        }
    }

    if ( _mode & M_RESUME ) {
        d->b6 = new QPushButton( i18n( "&Resume" ), this );
        connect(d->b6, SIGNAL(clicked()), this, SLOT(b6Pressed()));

        if ( _mode & M_MULTI )
        {
            d->b7 = new QPushButton( i18n( "R&esume All" ), this );
            connect(d->b7, SIGNAL(clicked()), this, SLOT(b7Pressed()));
        }
    }

    d->m_pLayout = new QVBoxLayout( this, KDialog::marginHint(),
                                    KDialog::spacingHint() );
    d->m_pLayout->addStrut( 360 );	// makes dlg at least that wide

    // User tries to overwrite a file with itself ?
    if ( _mode & M_OVERWRITE_ITSELF ) {
        QLabel *lb = new QLabel( i18n( "This action would overwrite '%1' with itself.\n"
                                       "Do you want to rename it instead?" ).arg( KStringHandler::csqueeze( d->src,100 ) ), this );
        d->m_pLayout->addWidget( lb );
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
                KLibrary *lib = KLibLoader::self()->library(libName.local8Bit() );
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
                    d->m_pLayout->addWidget(plugin );
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
            d->m_pLayout->addLayout(gridLayout);
            gridLayout->setColStretch(0,0);
            gridLayout->setColStretch(1,10);

            QString sentence1;
            if (mtimeDest < mtimeSrc)
                sentence1 = i18n("An older item named '%1' already exists.");
            else if (mtimeDest == mtimeSrc)
                sentence1 = i18n("A similar file named '%1' already exists.");
            else
                sentence1 = i18n("A newer item named '%1' already exists.");
            QLabel * lb1 = new QLabel( sentence1.arg(KStringHandler::csqueeze(d->dest,100)), this );
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

                QLabel * lb2 = new QLabel( i18n("The source file is '%1'").arg(d->src), this );
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
        // I wonder when this happens. And 'dest' isn't shown at all here...
        // Sounds like the case where we don't want to allow overwriting, the existing
        // file must be preserved. This doesn't happen in KIO though. (David)
        QString sentence1;
        if (mtimeDest < mtimeSrc)
            sentence1 = i18n("An older item than '%1' already exists.\n").arg(d->src);
        else if (mtimeDest == mtimeSrc)
            sentence1 = i18n("A similar file named '%1' already exists.").arg(d->src);
        else
            sentence1 = i18n("A newer item than '%1' already exists.\n").arg(d->src);

        QLabel *lb = new QLabel( sentence1 + i18n("Do you want to use another file name?"), this );
        d->m_pLayout->addWidget(lb);
    }
    d->m_pLineEdit = new QLineEdit( this );
    d->m_pLayout->addWidget( d->m_pLineEdit );
    QString fileName = KURL(d->dest).fileName();
    d->m_pLineEdit->setText( KIO::decodeFileName( fileName ) );
    if (d->b1)
        connect(d->m_pLineEdit, SIGNAL(textChanged(const QString &)),
                SLOT(enableRenameButton(const QString &)));

    d->m_pLayout->addSpacing( 10 );

    QHBoxLayout* layout = new QHBoxLayout();
    d->m_pLayout->addLayout( layout );

    layout->addStretch(1);

    if ( d->b1 )
        layout->addWidget( d->b1 );
    if( d->b8 )
        layout->addWidget( d->b8 );
    if ( d->b2 )
        layout->addWidget( d->b2 );
    if ( d->b3 )
        layout->addWidget( d->b3 );
    if ( d->b4 )
        layout->addWidget( d->b4 );
    if ( d->b5 )
        layout->addWidget( d->b5 );
    if ( d->b6 )
        layout->addWidget( d->b6 );
    if ( d->b7 )
        layout->addWidget( d->b7 );


    d->b0->setDefault( true );
    layout->addWidget( d->b0 );

    resize( sizeHint() );
}

RenameDlg::~RenameDlg()
{
  delete d;
  // no need to delete Pushbuttons,... qt will do this
}

void RenameDlg::enableRenameButton(const QString &newDest)
{
  if (newDest != d->dest)
  {
    d->b1->setEnabled(true);
    d->b1->setDefault(true);
  }
  else
    d->b1->setEnabled(false);
}

KURL RenameDlg::newDestURL()
{
  KURL newDest( d->dest );
  QString fileName = d->m_pLineEdit->text();
  newDest.setFileName( KIO::encodeFileName( fileName ) );
  return newDest;
}

void RenameDlg::b0Pressed()
{
  done( 0 );
}

// Rename
void RenameDlg::b1Pressed()
{
  if ( d->m_pLineEdit->text()  == "" )
    return;

  KURL u = newDestURL();
  if ( u.isMalformed() )
  {
    KMessageBox::error( this, i18n( "Malformed URL\n%1" ).arg( u.prettyURL() ) );
    return;
  }

  done( 1 );
}
// Propose
void RenameDlg::b8Pressed()
{
  int pos;
  if ( d->m_pLineEdit->text().isEmpty() )
    return;
  QString basename, dotSuffix, tmp;
  QFileInfo info ( d->m_pLineEdit->text() );
  basename = info.baseName();
  dotSuffix = info.extension();
  if ( !dotSuffix.isEmpty() )
    dotSuffix = '.' + dotSuffix;
  pos = basename.findRev('_' );
  if(pos != -1 )
  {
    bool ok;
    tmp = basename.right( basename.length() - (pos + 1) );
    int number = tmp.toInt( &ok, 10 );
    if ( !ok ) // ok there is no number
    {
      basename.append("_1" );
      d->m_pLineEdit->setText(basename + dotSuffix );
      b1Pressed(); // prepended now  'click' rename
      return;
    }
    else
    { // yes there's allready a number behind the _ so increment it by one
      QString tmp2 = QString::number ( number + 1 );
      basename.replace( pos+1, tmp.length() ,tmp2);
      d->m_pLineEdit->setText( basename + dotSuffix );
      //b1Pressed();
      return;
    }
  }
  else // no underscore yet
  {
    d->m_pLineEdit->setText( basename + "_1" + dotSuffix );
    //b1Pressed();
    return;

  }
  return; // we should never return from here jic
}

void RenameDlg::b2Pressed()
{
  done( 2 );
}

void RenameDlg::b3Pressed()
{
  done( 3 );
}

void RenameDlg::b4Pressed()
{
  done( 4 );
}

void RenameDlg::b5Pressed()
{
  done( 5 );
}

void RenameDlg::b6Pressed()
{
  done( 6 );
}

void RenameDlg::b7Pressed()
{
  done( 7 );
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
QString RenameDlg::mime( const QString &src )
{
  KMimeType::Ptr type = KMimeType::findByURL(src );
  //if( type->name() == KMimeType::defaultMimeType() ){ // ok no mimetype
    //    QString ty = KIO::NetAccess::mimetype(d->src );
    // return ty;
    return type->name();
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





