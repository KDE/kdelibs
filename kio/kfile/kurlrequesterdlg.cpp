/* This file is part of the KDE libraries
    Copyright (C) 2000 Wilco Greven <greven@kde.org>

    library is free software; you can redistribute it and/or
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


#include <sys/stat.h>
#include <unistd.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qstring.h>
#include <qtoolbutton.h>

#include <kaccel.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <krecentdocument.h>
#include <kurl.h>
#include <kurlrequester.h>

#include "kurlrequesterdlg.h"


KUrlRequesterDlg::KUrlRequesterDlg( const QString& urlName, QWidget *parent,
        const char *name, bool modal )
    :   KDialogBase( Plain, QString(), Ok|Cancel|User1, Ok, parent, name,
                modal, true, KStdGuiItem::clear() )
{
  initDialog(i18n( "Location:" ), urlName);
}

KUrlRequesterDlg::KUrlRequesterDlg( const QString& urlName, const QString& _text, QWidget *parent, const char *name, bool modal )
    :   KDialogBase( Plain, QString(), Ok|Cancel|User1, Ok, parent, name,
                modal, true, KStdGuiItem::clear() )
{
  initDialog(_text, urlName);
}

KUrlRequesterDlg::~KUrlRequesterDlg()
{
}

void KUrlRequesterDlg::initDialog(const QString &text,const QString &urlName)
{
   QVBoxLayout * topLayout = new QVBoxLayout( plainPage(), 0,
            spacingHint() );

    QLabel * label = new QLabel( text , plainPage() );
    topLayout->addWidget( label );

    urlRequester_ = new KUrlRequester( urlName, plainPage());
    urlRequester_->setMinimumWidth( urlRequester_->sizeHint().width() * 3 );
    topLayout->addWidget( urlRequester_ );
    urlRequester_->setFocus();
    connect( urlRequester_->lineEdit(), SIGNAL(textChanged(const QString&)),
             SLOT(slotTextChanged(const QString&)) );
    bool state = !urlName.isEmpty();
    enableButtonOK( state );
    enableButton( KDialogBase::User1, state );
    /*
    KFile::Mode mode = static_cast<KFile::Mode>( KFile::File |
            KFile::ExistingOnly );
	urlRequester_->setMode( mode );
    */
    connect( this, SIGNAL( user1Clicked() ), SLOT( slotClear() ) );
}

void KUrlRequesterDlg::slotTextChanged(const QString & text)
{
    bool state = !text.trimmed().isEmpty();
    enableButtonOK( state );
    enableButton( KDialogBase::User1, state );
}

void KUrlRequesterDlg::slotClear()
{
    urlRequester_->clear();
}

KUrl KUrlRequesterDlg::selectedURL() const
{
    if ( result() == QDialog::Accepted )
        return KUrl::fromPathOrURL( urlRequester_->url() );
    else
        return KUrl();
}


KUrl KUrlRequesterDlg::getURL(const QString& dir, QWidget *parent,
        const QString& caption)
{
    KUrlRequesterDlg dlg(dir, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);

    dlg.exec();

    const KUrl& url = dlg.selectedURL();
    if (url.isValid())
        KRecentDocument::add(url);

    return url;
}

KFileDialog * KUrlRequesterDlg::fileDialog()
{
    return urlRequester_->fileDialog();
}

KUrlRequester * KUrlRequesterDlg::urlRequester()
{
    return urlRequester_;
}

#include "kurlrequesterdlg.moc"

// vim:ts=4:sw=4:tw=78
