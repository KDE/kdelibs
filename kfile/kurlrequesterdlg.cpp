/* This file is part of the KDE libraries
    Copyright (C) 2000 Wilco Greven <j.w.greven@student.utwente.nl>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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


KURLRequesterDlg::KURLRequesterDlg( const QString& urlName, QWidget *parent,
        const char *name, bool modal )
    :   KDialogBase( Plain, QString::null, Ok|Cancel|User1, Ok, parent, name,
                modal, true, i18n("Clear") )
{
    QVBoxLayout * topLayout = new QVBoxLayout( plainPage(), 0,
            spacingHint() );

    QLabel * label = new QLabel( i18n( "Location:" ), plainPage() );
    topLayout->addWidget( label );

    urlRequester_ = new KURLRequester( urlName, plainPage(),
            "urlRequester", modal );
    urlRequester_->setMinimumWidth( urlRequester_->sizeHint().width() * 3 );
    topLayout->addWidget( urlRequester_ );
    urlRequester_->setFocus();

    /*
    KFile::Mode mode = static_cast<KFile::Mode>( KFile::File |
            KFile::ExistingOnly );
	urlRequester_->fileDialog()->setMode( mode );
    */
    connect( this, SIGNAL( user1Clicked() ), SLOT( slotClear() ) );
}


KURLRequesterDlg::~KURLRequesterDlg()
{
}

void KURLRequesterDlg::slotClear()
{
    urlRequester_->lineEdit()->clear();
}

KURL KURLRequesterDlg::selectedURL() const
{
    if ( result() == QDialog::Accepted )
        return urlRequester_->url();
    else
        return KURL();
}


KURL KURLRequesterDlg::getURL(const QString& dir, QWidget *parent,
        const QString& caption)
{
    KURLRequesterDlg dlg(dir, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);

    dlg.exec();

    const KURL& url = dlg.selectedURL();
    if (!url.isMalformed())
        KRecentDocument::add(url.url(-1), false);

    return url;
}


#include "kurlrequesterdlg.moc"

// vim:ts=4:sw=4:tw=78
