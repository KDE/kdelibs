#include <stdio.h>
#include <qstrlist.h>

#include <kapp.h>
#include <klocale.h>
#include "kspelldlg.moc"
#include "version.h"

KSpellDlg::KSpellDlg (QWidget *_parent, const char *name,
		      bool _progressbar, bool _modal ) :
  QWidget (_parent, name, WStyle_DialogBorder |
	   WType_TopLevel | ( _modal ? WType_Modal : 0 ) )
{

  progressbar = _progressbar;

  children=new QList<QWidget>;
  if (progressbar)
    layout = new QGridLayout (this, 8, 4, 10, 1);
  else
    layout = new QGridLayout (this, 7, 4, 10, 1);

  //The dialog box layout (not all here yet)
  const unsigned int
    rprogbar = 9, cprogbara = 0, cprogbarb = 3;


  //Dlgedit
  
  KLineEdit* tmpKLineEdit;
  tmpKLineEdit = new KLineEdit( this, "LineEdit_1" );
  tmpKLineEdit->setGeometry( 110, 50, 190, 30 );
  tmpKLineEdit->setText( "" );
  tmpKLineEdit->setMaxLength( 32767 );
  tmpKLineEdit->setEchoMode( KLineEdit::Normal );
  tmpKLineEdit->setFrame( TRUE );

  children->append (tmpKLineEdit);
  tmpKLineEdit->setMinimumSize (tmpKLineEdit->sizeHint());
  layout->addWidget (tmpKLineEdit,1,1);
	
	
  QPushButton *tmpQPushButtonRA = 
    new QPushButton( this, "PushButton_7" );
  tmpQPushButtonRA->setGeometry( 430, 80, 100, 30 );
  connect( tmpQPushButtonRA, SIGNAL(clicked()), SLOT(replaceAll()) );
  tmpQPushButtonRA->setText( i18n("R&eplace All") );
  tmpQPushButtonRA->setAutoRepeat( FALSE );
  tmpQPushButtonRA->setAutoResize( FALSE );
  tmpQPushButtonRA->setMinimumSize (tmpQPushButtonRA->sizeHint());
  layout->addWidget (tmpQPushButtonRA, 1, 3);
  

  QListBox* tmpQListBox;
  tmpQListBox = new QListBox( this, "ListBox_1" );
  tmpQListBox->setFrameStyle( 51 );
  tmpQListBox->setLineWidth( 2 );
  tmpQListBox->setMinimumSize (tmpQListBox->sizeHint());
  children->append (tmpQListBox);
  layout->addMultiCellWidget (tmpQListBox, 2, 5, 1, 1);

  
  QLabel* tmpQLabel;
  tmpQLabel = new QLabel( this, "Label_1" );
  tmpQLabel->setGeometry( 10, 10, 100, 30 );
  tmpQLabel->setText( i18n("Misspelled Word:") );
  tmpQLabel->setAlignment( 290 );
  layout->addWidget (tmpQLabel,0,0);
  tmpQLabel->setMinimumSize (tmpQLabel->sizeHint());
  
  
  tmpQLabel = new QLabel( this, "Label_2" );
  tmpQLabel->setGeometry( 10, 50, 100, 30 );
  tmpQLabel->setText( i18n("Replacement:") );
  tmpQLabel->setAlignment( 290 );
  layout->addWidget (tmpQLabel, 1, 0);
  tmpQLabel->setMinimumSize (tmpQLabel->sizeHint());
  
  tmpQLabel = new QLabel( this, "Label_3" );
  tmpQLabel->setGeometry( 10, 80, 100, 30 );
  tmpQLabel->setText( i18n("Suggestions:") );
  tmpQLabel->setAlignment( 290 );
  layout->addWidget (tmpQLabel, 2, 0);
  tmpQLabel->setMinimumSize (tmpQLabel->sizeHint());
  
  QPushButton* tmpQPushButton;
  tmpQPushButton = new QPushButton( this, "PushButton_1" );
  tmpQPushButton->setGeometry( 320, 120, 100, 30 );
  connect( tmpQPushButton, SIGNAL(clicked()), SLOT(ignore()) );
  tmpQPushButton->setText( i18n("&Ignore") );
  tmpQPushButton->setAutoRepeat( FALSE );
  tmpQPushButton->setAutoResize( FALSE );
  tmpQPushButton->setMinimumSize (tmpQPushButton->sizeHint());
  children->append (tmpQPushButton);
  layout->addWidget (tmpQPushButton, 2, 2);
  
  tmpQPushButton = new QPushButton( this, "PushButton_2" );
  tmpQPushButton->setGeometry( 430, 120, 100, 30 );
  connect( tmpQPushButton, SIGNAL(clicked()), SLOT(ignoreAll()) );
  tmpQPushButton->setText( i18n("I&gnore All") );
  tmpQPushButton->setAutoRepeat( FALSE );
  tmpQPushButton->setAutoResize( FALSE );
  tmpQPushButton->setMinimumSize (tmpQPushButton->sizeHint());
  children->append (tmpQPushButton);
  layout->addWidget (tmpQPushButton, 2, 3);
  
  QPushButton *tmpQPushButtonR = new QPushButton( this, "PushButton_3" );
  tmpQPushButtonR->setGeometry( 320, 80, 100, 30 );
  connect( tmpQPushButtonR, SIGNAL(clicked()), SLOT(replace()) );
  tmpQPushButtonR->setText( i18n("&Replace") );
  tmpQPushButtonR->setAutoRepeat( FALSE );
  tmpQPushButtonR->setAutoResize( FALSE );
  tmpQPushButtonR->setDefault ( TRUE );
  layout->addWidget (tmpQPushButtonR, 1, 2);
  tmpQPushButtonR->setMinimumSize (tmpQPushButtonR->sizeHint());
  
  tmpQPushButton = new QPushButton( this, "PushButton_4" );
  tmpQPushButton->setGeometry( 320, 160, 100, 30 );
  connect( tmpQPushButton, SIGNAL(clicked()), SLOT(add()) );
  tmpQPushButton->setText( i18n("A&dd") );
  tmpQPushButton->setAutoRepeat( FALSE );
  tmpQPushButton->setAutoResize( FALSE );
  tmpQPushButton->setMinimumSize (tmpQPushButton->sizeHint());
  children->append (tmpQPushButton);
  layout->addWidget (tmpQPushButton, 3, 2);
  
  tmpQPushButton = new QPushButton( this, "PushButton_5" );
  tmpQPushButton->setGeometry( 430, 160, 100, 30 );
  connect( tmpQPushButton, SIGNAL(clicked()), SLOT(help()) );
  tmpQPushButton->setText( i18n("&Help") );
  tmpQPushButton->setAutoRepeat( FALSE );
  tmpQPushButton->setAutoResize( FALSE );
  tmpQPushButton->setMinimumSize (tmpQPushButton->sizeHint());
  children->append (tmpQPushButton);
  layout->addWidget (tmpQPushButton, 3, 3);
  
  tmpQPushButton = new QPushButton( this, "PushButton_6" );
  tmpQPushButton->setGeometry( 430, 200, 100, 30 );
  connect( tmpQPushButton, SIGNAL(clicked()), SLOT(cancel()) );
  tmpQPushButton->setText( i18n("&Cancel") );
  tmpQPushButton->setAutoRepeat( FALSE );
  tmpQPushButton->setAutoResize( FALSE );
  tmpQPushButton->setMinimumSize (tmpQPushButton->sizeHint());
  layout->addWidget (tmpQPushButton, 4, 3);
  
  
  // Label for the misspelled word
  wordlabel = new QLabel( this, "Label_4" );
  wordlabel->setGeometry( 110, 10, 190, 30 );
  wordlabel->setFrameStyle( 17 );
  wordlabel->setText( "MMPQRS" );
  wordlabel->setAutoResize ( FALSE );
  wordlabel->setAlignment( 289 );
  children->append (wordlabel);
  layout->addWidget (wordlabel, 0, 1);
  wordlabel->setMinimumSize (wordlabel->sizeHint());
  

  tmpQPushButton = new QPushButton( this, "PushButton_8" );
  tmpQPushButton->setGeometry( 320, 200, 100, 30 );
  connect( tmpQPushButton, SIGNAL(clicked()), SLOT(stop()) );
  tmpQPushButton->setText( i18n("&Stop") );
  tmpQPushButton->setAutoRepeat( FALSE );
  tmpQPushButton->setAutoResize( FALSE );
  tmpQPushButton->setMinimumSize (tmpQPushButton->sizeHint());
  layout->addWidget (tmpQPushButton, 4, 2);

  if (progressbar)
    {
      progbar = new KProgress (0, 100, 0, KProgress::Horizontal, this);
      progbar->setBarStyle (KProgress::Blocked);
      progbar->setMinimumHeight (progbar->sizeHint().height());
      //      progbar->setMaximumHeight (progbar->sizeHint().height());
      layout->addMultiCellWidget (progbar, rprogbar, rprogbar,
				  cprogbara, cprogbarb);
    }

  layout->freeze();
  //  resize(478, 222);

  
   //end Dlgedit

  //done once per session
  wordlabel->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
  
  qpbrep=tmpQPushButtonR;
  qpbrepa=tmpQPushButtonRA;
  editbox=tmpKLineEdit;
  listbox=tmpQListBox;
  
  
  
  connect (editbox,SIGNAL (textChanged (const QString &)),
	   this, SLOT (textChanged (const QString &)));
  connect (editbox,SIGNAL (returnPressed ()),
	   this, SLOT (replace ()));
  
  listbox->setAutoScrollBar (TRUE);
  connect (listbox,SIGNAL (selected(int)),this, SLOT (selected (int)));
  connect (listbox,SIGNAL (highlighted(int)),
	   this, SLOT (highlighted (int)));
  
  
  
  layout->activate();
  standby();
}

void
KSpellDlg::init (const QString& _word, QStrList *_sugg)
{
  //  printf ("init %s\n", (const char *)_word);
  sugg=_sugg;
  word=_word;
  listbox->clear();
  listbox->insertStrList (sugg);

  QWidget *widget;

  widget=children->first();
  do {
    widget->setEnabled (TRUE);
  } while ((widget=children->next())!=0);


  wordlabel->setText (_word);

  if (sugg->count()!=0)
    {
      editbox->setText (sugg->at(0));
      //      printf ("ENABLE\n");
      qpbrep->setEnabled (TRUE);
      qpbrepa->setEnabled (TRUE);
    }
  else
    {
      //      printf ("DISABLE\n");
      qpbrep->setEnabled (FALSE);
      qpbrepa->setEnabled (FALSE);
      editbox->setText (_word);
    }
  


}

void
KSpellDlg::standby ()
{
  QWidget *widget;
  //  printf ("STANDBY\n");

  widget=children->first();
  do {
    widget->setEnabled (FALSE);
  } while ((widget=children->next())!=0);

  qpbrep->setEnabled (FALSE);
  qpbrepa->setEnabled (FALSE);
}

void
KSpellDlg::slotProgress (unsigned int p)
{
  if (!progressbar)
    return;


  progbar->setValue ((int) p);
}

void
KSpellDlg::textChanged (const QString &)
{
  qpbrep->setEnabled (TRUE);
  qpbrepa->setEnabled (TRUE);
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

void
KSpellDlg::help()
{
  //give some help
  QString file ("kspell/ksdialog.html"), label ("");
  kapp->invokeHTMLHelp (file, label);
}
