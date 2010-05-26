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

#include "kurlrequesterdialog.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>

#include <kfiledialog.h>
#include <kguiitem.h>
#include <klineedit.h>
#include <klocale.h>
#include <krecentdocument.h>
#include <kurl.h>
#include <kurlrequester.h>

class KUrlRequesterDialogPrivate
{
public:
    KUrlRequesterDialogPrivate(KUrlRequesterDialog *qq)
        : q(qq)
    {
    }

    KUrlRequesterDialog *q;

    void initDialog(const QString &text, const QString &url);

    // slots
    void _k_slotClear();
    void _k_slotTextChanged(const QString &);

    KUrlRequester *urlRequester;
};


KUrlRequesterDialog::KUrlRequesterDialog( const QString& urlName, QWidget *parent)
    : KDialog(parent), d(new KUrlRequesterDialogPrivate(this))
{
  setButtons( Ok | Cancel | User1 );
  setButtonGuiItem( User1, KStandardGuiItem::clear() );

    d->initDialog(i18n("Location:"), urlName);
}

KUrlRequesterDialog::KUrlRequesterDialog( const QString& urlName, const QString& _text, QWidget *parent)
    : KDialog(parent), d(new KUrlRequesterDialogPrivate(this))
{
  setButtons( Ok | Cancel | User1 );
  setButtonGuiItem( User1, KStandardGuiItem::clear() );

    d->initDialog(_text, urlName);
}

KUrlRequesterDialog::~KUrlRequesterDialog()
{
    delete d;
}

void KUrlRequesterDialogPrivate::initDialog(const QString &text,const QString &urlName)
{
    q->setDefaultButton(KDialog::Ok);
    QWidget *plainPage = q->mainWidget();
   QVBoxLayout * topLayout = new QVBoxLayout( plainPage );
   topLayout->setMargin( 0 );

    QLabel * label = new QLabel( text , plainPage );
    topLayout->addWidget( label );

    urlRequester = new KUrlRequester(urlName, plainPage);
    urlRequester->setMinimumWidth(urlRequester->sizeHint().width() * 3);
    topLayout->addWidget(urlRequester);
    urlRequester->setFocus();
    QObject::connect(urlRequester->lineEdit(), SIGNAL(textChanged(QString)),
                     q, SLOT(_k_slotTextChanged(QString)));
    bool state = !urlName.isEmpty();
    q->enableButtonOk(state);
    q->enableButton(KDialog::User1, state);
    /*
    KFile::Mode mode = static_cast<KFile::Mode>( KFile::File |
            KFile::ExistingOnly );
	urlRequester_->setMode( mode );
    */
    QObject::connect(q, SIGNAL(user1Clicked()), q, SLOT(_k_slotClear()));
}

void KUrlRequesterDialogPrivate::_k_slotTextChanged(const QString & text)
{
    bool state = !text.trimmed().isEmpty();
    q->enableButtonOk(state);
    q->enableButton(KDialog::User1, state);
}

void KUrlRequesterDialogPrivate::_k_slotClear()
{
    urlRequester->clear();
}

KUrl KUrlRequesterDialog::selectedUrl() const
{
    if ( result() == QDialog::Accepted )
        return d->urlRequester->url();
    else
        return KUrl();
}


KUrl KUrlRequesterDialog::getUrl(const QString& dir, QWidget *parent,
                              const QString& caption)
{
    KUrlRequesterDialog dlg(dir, parent);

    dlg.setCaption(caption.isEmpty() ? i18n("Open") : caption);

    dlg.exec();

    const KUrl& url = dlg.selectedUrl();
    if (url.isValid())
        KRecentDocument::add(url);

    return url;
}

KFileDialog * KUrlRequesterDialog::fileDialog()
{
    return d->urlRequester->fileDialog();
}

KUrlRequester * KUrlRequesterDialog::urlRequester()
{
    return d->urlRequester;
}

#include "kurlrequesterdialog.moc"

// vim:ts=4:sw=4:tw=78
