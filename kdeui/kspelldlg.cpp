/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>
   Copyright (C) 2000 Rik Hemsley <rik@kde.org>
   Copyright (C) 2000-2001 Wolfram Diestel <wolfram@steloj.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qstringlist.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kapplication.h>
#include <klocale.h>
#include <klistbox.h>
#include <klineedit.h>
#include <kprogress.h>
#include <kbuttonbox.h>
#include <kdebug.h>

#include "kspelldlg.h"

KSpellDlg::KSpellDlg(
  QWidget * parent,
  const char * name,
  bool _progressbar,
  bool _modal
)
  : KDialogBase(
      parent, name, _modal, i18n("Check spelling"), Help|Cancel|User1, 
      Cancel, true, i18n("&Stop")
    ),
    progressbar(_progressbar)
{
  QWidget * w = new QWidget(this);
  setMainWidget(w);

  wordlabel = new QLabel(w, "wordlabel");
  wordlabel->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);

  editbox = new KLineEdit(w, "editbox");
 
  listbox = new KListBox(w, "listbox");

  QLabel * l_misspelled =
    new QLabel(i18n("Misspelled word:"), w, "l_misspelled");

  QLabel * l_replacement =
    new QLabel(i18n("Replacement:"), w, "l_replacement");
  
  QLabel * l_suggestions =
    new QLabel(i18n("Suggestions:"), w, "l_suggestions");
  l_suggestions->setAlignment(Qt::AlignLeft | Qt::AlignTop );
 
  KButtonBox * buttonBox = new KButtonBox(w, Vertical);

  QPushButton * b = 0L;

  b = buttonBox->addButton(i18n("&Replace"), this, SLOT(replace()));
  connect(this, SIGNAL(ready(bool)), b, SLOT(setEnabled(bool)));
  qpbrep = b;

  b = buttonBox->addButton(i18n("Replace &All"), this, SLOT(replaceAll()));
  connect(this, SIGNAL(ready(bool)), b, SLOT(setEnabled(bool)));
  qpbrepa = b;

  b = buttonBox->addButton(i18n("&Ignore"), this, SLOT(ignore()));
  connect(this, SIGNAL(ready(bool)), b, SLOT(setEnabled(bool)));

  b = buttonBox->addButton(i18n("I&gnore All"), this, SLOT(ignoreAll()));
  connect(this, SIGNAL(ready(bool)), this, SLOT(setEnabled(bool)));

  b = buttonBox->addButton(i18n("A&dd"), this, SLOT(add()));
  connect(this, SIGNAL(ready(bool)), b, SLOT(setEnabled(bool)));

  connect(this, SIGNAL(user1Clicked()), this, SLOT(stop()));

  buttonBox->layout();

  QHBoxLayout * layout = 0L;

  if (progressbar) {

    QVBoxLayout * topLayout =
      new QVBoxLayout(w, KDialog::marginHint(), KDialog::spacingHint());

    layout = new QHBoxLayout(topLayout);
    progbar = new KProgress (w);
    topLayout->addWidget(progbar);

  } else {

    layout =
      new QHBoxLayout(w, KDialog::marginHint(), KDialog::spacingHint());
  }
  
  QGridLayout * leftGrid = new QGridLayout(layout);

  leftGrid->addWidget(l_misspelled,   0, 0);
  leftGrid->addWidget(l_replacement,  1, 0);
  leftGrid->addWidget(l_suggestions,  2, 0);
  leftGrid->addWidget(wordlabel,      0, 1);
  leftGrid->addWidget(editbox,        1, 1);
  leftGrid->addWidget(listbox,        2, 1);

  layout->addWidget(buttonBox);

  connect(
    editbox,
    SIGNAL(textChanged(const QString &)),
    SLOT(textChanged(const QString &))
  );

  connect(editbox, SIGNAL(returnPressed()),   SLOT(replace()));
  connect(listbox, SIGNAL(selected(int)),     SLOT(selected(int)));
  connect(listbox, SIGNAL(highlighted(int)),  SLOT(highlighted (int)));

  QSize bs = sizeHint();
  if (bs.width() < bs.height()) {
    resize(9 * bs.height() / 6, bs.height());
  }

  setHelp("spelldlg", "kspell");
  
  emit(ready(false));
}

void
KSpellDlg::init(const QString & _word, QStringList * _sugg)
{
  sugg = _sugg;
  word = _word;

  listbox->clear();
  listbox->insertStringList(*sugg);

  kdDebug(750) << "KSpellDlg::init [" << word << "]" << endl; 

  emit(ready(true));

  wordlabel->setText(_word);

  if (sugg->count() == 0) {

    editbox->setText(_word);
    qpbrep->setEnabled(false);
    qpbrepa->setEnabled(false);

  } else {

    editbox->setText((*sugg)[0]);
    qpbrep->setEnabled(true);
    qpbrepa->setEnabled(true);
    listbox->setCurrentItem (0);
  }
}

void
KSpellDlg::slotProgress (unsigned int p)
{
  if (!progressbar)
    return;

  progbar->setValue((int) p);
}

void
KSpellDlg::textChanged (const QString &)
{
  qpbrep->setEnabled(true);
  qpbrepa->setEnabled(true);
}

void
KSpellDlg::selected (int i)
{
  highlighted (i);
  replace();
}

void
KSpellDlg::highlighted (int i)
{
  if (listbox->text (i)!=0)
    editbox->setText (listbox->text (i));
}

/*
  exit functions
  */

void
KSpellDlg::closeEvent( QCloseEvent * )
{
	cancel();
}

void
KSpellDlg::done (int result)
{
  emit command (result);
}
void
KSpellDlg::ignore()
{
  newword = word;
  done (KS_IGNORE);
}

void
KSpellDlg::ignoreAll()
{
  newword = word;
  done (KS_IGNOREALL);
}

void
KSpellDlg::add()
{
  newword = word;
  done (KS_ADD);
}


void
KSpellDlg::cancel()
{
  newword=word;
  done (KS_CANCEL);
}

void
KSpellDlg::replace()
{
  newword = editbox->text();
  done (KS_REPLACE);
}

void
KSpellDlg::stop()
{
  newword = word;
  done (KS_STOP);
}

void
KSpellDlg::replaceAll()
{
  newword = editbox->text();
  done (KS_REPLACEALL);
}

#include "kspelldlg.moc"
