#include "kcomboboxtest.h"

#include <qpushbutton.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qhbox.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <ksimpleconfig.h>

KComboBoxTest::KComboBoxTest(QWidget* widget, const char* name )
              :QWidget(widget, name)
{
  QVBoxLayout *vbox = new QVBoxLayout (this, KDialog::marginHint(), KDialog::spacingHint());
  
  // Read-only combobox
  QHBox* hbox = new QHBox(this);
  hbox->setSpacing (KDialog::spacingHint());
  QLabel* lbl = new QLabel("&Read-Only Combo:", hbox);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);
  
  KComboBox *ro = new KComboBox(hbox, "ReadOnlyCombo" );
  lbl->setBuddy (ro);  
  ro->setCompletionMode( KGlobalSettings::CompletionAuto );
  QObject::connect (ro, SIGNAL(activated(int)), SLOT(slotActivated(int)));
  QObject::connect (ro, SIGNAL(activated(const QString&)), SLOT (slotActivated(const QString&)));
  vbox->addWidget (hbox);
    
  // Read-write combobox
  hbox = new QHBox(this);
  hbox->setSpacing (KDialog::spacingHint());
  lbl = new QLabel("&Editable Combo:", hbox);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);  
  
  KComboBox *rw = new KComboBox( true, hbox, "ReadWriteCombo" );
  lbl->setBuddy (rw);
  rw->setDuplicatesEnabled( true );  
  rw->setInsertionPolicy( QComboBox::NoInsertion );
  QObject::connect (rw, SIGNAL(activated(int)), SLOT(slotActivated(int)));
  QObject::connect (rw, SIGNAL(activated(const QString&)), SLOT(slotActivated(const QString&)));
  QObject::connect (rw, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
  vbox->addWidget (hbox);
  
  // History combobox...
  hbox = new QHBox(this);
  hbox->setSpacing (KDialog::spacingHint());
  lbl = new QLabel("&History Combo:", hbox);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);  
  
  KComboBox *hc = new KHistoryCombo( true, hbox, "ReadWriteCombo" );
  lbl->setBuddy (rw);
  rw->setDuplicatesEnabled( true );  
  rw->setInsertionPolicy( QComboBox::NoInsertion );
  QObject::connect (hc, SIGNAL(activated(int)), SLOT(slotActivated(int)));
  QObject::connect (hc, SIGNAL(activated(const QString&)), SLOT(slotActivated(const QString&)));
  QObject::connect (hc, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
  vbox->addWidget (hbox);  
    
  // Read-write combobox that is a replica of code in konqueror...
  hbox = new QHBox(this);
  hbox->setSpacing (KDialog::spacingHint());  
  lbl = new QLabel( "&Konq's Combo:", hbox);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);  
  
  KComboBox *konqc = new KComboBox( true, hbox, "KonqyCombo" );
  lbl->setBuddy (konqc);
  konqc->setMaxCount( 10 );
  QObject::connect (konqc, SIGNAL(activated(int)), SLOT(slotActivated(int)));
  QObject::connect (konqc, SIGNAL(activated(const QString&)), SLOT (slotActivated(const QString&)));
  QObject::connect (konqc, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
  vbox->addWidget (hbox);
  
  // Create an exit button
  QPushButton * push = new QPushButton( "E&xit", this );
  QObject::connect( push, SIGNAL(clicked()), SLOT(quitApp()) );
  vbox->addWidget (push);
  
  // Popuplate the select-only list box
  QStringList list;
  list << "Stone" << "Tree" << "Peables" << "Ocean" << "Sand" << "Chips" 
       << "Computer" << "Mankind";
  list.sort();
  
  // Setup read-only combo
  ro->insertStringList( list );
  ro->completionObject()->setItems( list );
  
  // Setup read-write combo
  rw->insertStringList( list );
  rw->completionObject()->setItems( list );
  
  // Setup read-write combo
  hc->insertStringList( list );
  hc->completionObject()->setItems( list );
  
  // Setup konq's combobox
  KSimpleConfig historyConfig( "konq_history" );
  historyConfig.setGroup( "Location Bar" );
  KCompletion * s_pCompletion = new KCompletion;
  s_pCompletion->setOrder( KCompletion::Weighted );
  s_pCompletion->setItems( historyConfig.readListEntry( "ComboContents" ) );
  s_pCompletion->setCompletionMode( KGlobalSettings::completionMode() );
  konqc->setCompletionObject( s_pCompletion );
  
  QPixmap pix = SmallIcon("www");
  konqc->insertItem( pix, "http://www.kde.org" );
  konqc->setCurrentItem( konqc->count()-1 );
}

KComboBoxTest::~KComboBoxTest()
{
}

void KComboBoxTest::slotActivated( int index ) 
{
  kdDebug() << "Activated Combo: " << sender()->name() << ", index:" << index << endl;
}

void KComboBoxTest::slotActivated (const QString& item)
{
  kdDebug() << "Activated Combo: " << sender()->name() << ", item: " << item << endl;
}

void KComboBoxTest::slotReturnPressed ()
{
  kdDebug() << "Return Pressed: " << sender()->name() << endl;
}

void KComboBoxTest::quitApp()
{
  kapp->closeAllWindows();
}

int main ( int argc, char **argv)
{
  KApplication a(argc, argv, "kcomboboxtest");
  KComboBoxTest* t= new KComboBoxTest;    
  a.setMainWidget (t);
  t->show ();
  return a.exec();
}

#include "kcomboboxtest.moc"
