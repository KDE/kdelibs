/* This file is part of the KDE libraries
   Copyright (C) 1999 Preston Brown <pbrown@kde.org>

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
#include <config.h>

#include <qvalidator.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#undef Unsorted // Required for --enable-final (qdir.h)
#include <qfiledialog.h>

#include <kbuttonbox.h>
#include <klocale.h>
#include <kapplication.h>
#include <klineedit.h>
#include <kstdguiitem.h>

#include "klineeditdlg.h"

KLineEditDlg::KLineEditDlg( const QString&_text, const QString& _value,
			    QWidget *parent )
  : KDialogBase( Plain, QString::null, Ok|Cancel|User1, Ok, parent, 0L, true,
		 true, KStdGuiItem::clear() )
{
  QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint() );
  QLabel *label = new QLabel(_text, plainPage() );
  topLayout->addWidget( label, 1 );

  edit = new KLineEdit( plainPage(), 0L );
  edit->setMinimumWidth(edit->sizeHint().width() * 3);
  label->setBuddy(edit);  // please "scheck" style
  //  connect( edit, SIGNAL(returnPressed()), SLOT(accept()) );
  connect( edit, SIGNAL(textChanged(const QString&)),
	   SLOT(slotTextChanged(const QString&)) );
  topLayout->addWidget( edit, 1 );

  connect( this, SIGNAL(user1Clicked()), this, SLOT(slotClear()) );
  edit->setText( _value );
  if ( _value.isEmpty() )
      enableButtonOK( false );
  edit->setSelection(0, edit->text().length());
  edit->setFocus();
}



#if 0
KLineEditDlg::KLineEditDlg( const QString&_text, const QString& _value,
			    QWidget *parent, bool _file_mode )
    : QDialog( parent, 0L, true )
{
  QGridLayout *layout = new QGridLayout(this, 4, 3, 10);

  QLabel *label = new QLabel(_text, this);
  layout->addWidget(label, 0, 0, AlignLeft);

  edit = new KLineEdit( this, 0L );
  edit->setMinimumWidth(edit->sizeHint().width() * 3);
  connect( edit, SIGNAL(returnPressed()), SLOT(accept()) );

  if ( _file_mode ) {
    completion = new KURLCompletion();
  	edit->setCompletionObject( completion );
	edit->setAutoDeleteCompletionObject( true );
  } else
    completion = 0L;

  layout->addMultiCellWidget(edit, 1, 1, 0, _file_mode ? 1 : 2);
  layout->setColStretch(1, 1);

  if (_file_mode) {
    QPushButton *browse = new QPushButton(i18n("&Browse..."), this);
    layout->addWidget(browse, 1, 2, AlignCenter);
    connect(browse, SIGNAL(clicked()),
	    SLOT(slotBrowse()));
  }

  QFrame *hLine = new QFrame(this);
  hLine->setFrameStyle(QFrame::Sunken|QFrame::HLine);
  layout->addMultiCellWidget(hLine, 2, 2, 0, 2);

  KButtonBox *bBox = new KButtonBox(this);
  layout->addMultiCellWidget(bBox, 3, 3, 0, 2);

  QPushButton *ok = bBox->addButton(i18n("&OK"));
  ok->setDefault(true);
  connect( ok, SIGNAL(clicked()), SLOT(accept()));

  bBox->addStretch(1);

  QPushButton *clear = bBox->addButton(i18n("C&lear"));
  connect( clear, SIGNAL(clicked()), SLOT(slotClear()));

  bBox->addStretch(1);

  QPushButton *cancel = bBox->addButton(i18n("&Cancel"));
  connect( cancel, SIGNAL(clicked()), SLOT(reject()));

  bBox->layout();

  layout->activate();

  edit->setText( _value );
  edit->setSelection(0, edit->text().length());
  edit->setFocus();
}
#endif


KLineEditDlg::~KLineEditDlg()
{
}

void KLineEditDlg::slotClear()
{
    edit->setText(QString::null);
}

void KLineEditDlg::slotTextChanged(const QString &text)
{
  bool on;
  if ( edit->validator() ) {
    QString str = edit->text();
    int index = edit->cursorPosition();
    on = ( edit->validator()->validate( str, index )
	   == QValidator::Acceptable );
  } else {
    on = !text.isEmpty();
  }
  enableButtonOK( on );
}

QString KLineEditDlg::text() const
{
    return edit->text();
}

QString KLineEditDlg::getText(const QString &_text, const QString& _value,
                              bool *ok, QWidget *parent, QValidator *_validator )
{
    KLineEditDlg dlg(_text, _value, parent );
    dlg.lineEdit()->setValidator( _validator );
    dlg.slotTextChanged( _value ); // trigger validation

    bool ok_ = dlg.exec() == QDialog::Accepted;
    if ( ok )
        *ok = ok_;
    if ( ok_ )
        return dlg.text();
    return QString::null;
}

QString KLineEditDlg::getText(const QString &_caption, const QString &_text,
                              const QString& _value,
                              bool *ok, QWidget *parent, QValidator *_validator )
{
    KLineEditDlg dlg( _text, _value, parent );
    dlg.setCaption( _caption );
    dlg.lineEdit()->setValidator( _validator );
    dlg.slotTextChanged( _value ); // trigger validation

    bool ok_ = dlg.exec() == QDialog::Accepted;
    if ( ok )
        *ok = ok_;
    if ( ok_ )
        return dlg.text();
    return QString::null;
}

void KLineEditDlg::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "klineeditdlg.moc"
