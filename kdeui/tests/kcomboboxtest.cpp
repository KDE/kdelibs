#define protected public // for delegate()
#include <kcombobox.h>
#undef protected

#include "kcomboboxtest.h"

#include <assert.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <ksimpleconfig.h>

#include <qpushbutton.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <q3hbox.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QVBoxLayout>


KComboBoxTest::KComboBoxTest(QWidget* widget, const char* name )
              :QWidget(widget, name)
{
  QVBoxLayout *vbox = new QVBoxLayout (this, KDialog::marginHint(), KDialog::spacingHint());

  // Test for KCombo's KLineEdit destruction
  KComboBox *testCombo = new KComboBox( true, this ); // rw, with KLineEdit
  testCombo->setEditable( false ); // destroys our KLineEdit
  assert( testCombo->delegate() == 0L );
  delete testCombo; // not needed anymore

  // Qt combobox
  Q3HBox* hbox = new Q3HBox(this);
  hbox->setSpacing (KDialog::spacingHint());
  QLabel* lbl = new QLabel("&QCombobox:", hbox);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);

  m_qc = new QComboBox(hbox, "QtReadOnlyCombo" );
  lbl->setBuddy (m_qc);
  QObject::connect (m_qc, SIGNAL(activated(int)), SLOT(slotActivated(int)));
  QObject::connect (m_qc, SIGNAL(activated(const QString&)), SLOT (slotActivated(const QString&)));
  vbox->addWidget (hbox);

  // Read-only combobox
  hbox = new Q3HBox(this);
  hbox->setSpacing (KDialog::spacingHint());
  lbl = new QLabel("&Read-Only Combo:", hbox);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);

  m_ro = new KComboBox(hbox );
  m_ro->setObjectName( "ReadOnlyCombo" );
  lbl->setBuddy (m_ro);
  m_ro->setCompletionMode( KGlobalSettings::CompletionAuto );
  QObject::connect (m_ro, SIGNAL(activated(int)), SLOT(slotActivated(int)));
  QObject::connect (m_ro, SIGNAL(activated(const QString&)), SLOT (slotActivated(const QString&)));
  vbox->addWidget (hbox);

  // Read-write combobox
  hbox = new Q3HBox(this);
  hbox->setSpacing (KDialog::spacingHint());
  lbl = new QLabel("&Editable Combo:", hbox);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);

  m_rw = new KComboBox( true, hbox );
  m_rw->setObjectName( "ReadWriteCombo" );
  lbl->setBuddy (m_rw);
  m_rw->setDuplicatesEnabled( true );
  m_rw->setInsertPolicy( QComboBox::NoInsert );
  m_rw->setTrapReturnKey( true );
  QObject::connect (m_rw, SIGNAL(activated(int)), SLOT(slotActivated(int)));
  QObject::connect (m_rw, SIGNAL(activated(const QString&)), SLOT(slotActivated(const QString&)));
  QObject::connect (m_rw, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
  QObject::connect (m_rw, SIGNAL(returnPressed(const QString&)), SLOT(slotReturnPressed(const QString&)));
  vbox->addWidget (hbox);

  // History combobox...
  hbox = new Q3HBox(this);
  hbox->setSpacing (KDialog::spacingHint());
  lbl = new QLabel("&History Combo:", hbox);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);

  m_hc = new KHistoryCombo( true, hbox );
  m_hc->setObjectName( "HistoryCombo" );
  lbl->setBuddy (m_hc);
  m_hc->setDuplicatesEnabled( true );
  m_hc->setInsertPolicy( QComboBox::NoInsert );
  QObject::connect (m_hc, SIGNAL(activated(int)), SLOT(slotActivated(int)));
  QObject::connect (m_hc, SIGNAL(activated(const QString&)), SLOT(slotActivated(const QString&)));
  QObject::connect (m_hc, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
  vbox->addWidget (hbox);
  m_hc->setTrapReturnKey(true);

  // Read-write combobox that is a replica of code in konqueror...
  hbox = new Q3HBox(this);
  hbox->setSpacing (KDialog::spacingHint());
  lbl = new QLabel( "&Konq's Combo:", hbox);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);

  m_konqc = new KComboBox( true, hbox );
  m_konqc->setObjectName( "KonqyCombo" );
  lbl->setBuddy (m_konqc);
  m_konqc->setMaxCount( 10 );
  QObject::connect (m_konqc, SIGNAL(activated(int)), SLOT(slotActivated(int)));
  QObject::connect (m_konqc, SIGNAL(activated(const QString&)), SLOT (slotActivated(const QString&)));
  QObject::connect (m_konqc, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
  vbox->addWidget (hbox);

  // Create an exit button
  hbox = new Q3HBox (this);
  m_btnExit = new QPushButton( "E&xit", hbox );
  QObject::connect( m_btnExit, SIGNAL(clicked()), SLOT(quitApp()) );

  // Create a disable button...
  m_btnEnable = new QPushButton( "Disa&ble", hbox );
  QObject::connect (m_btnEnable, SIGNAL(clicked()), SLOT(slotDisable()));

  vbox->addWidget (hbox);

  // Popuplate the select-only list box
  QStringList list;
  list << "Stone" << "Tree" << "Peables" << "Ocean" << "Sand" << "Chips"
       << "Computer" << "Mankind";
  list.sort();

  // Setup the qcombobox
  m_qc->insertStringList (list);

  // Setup read-only combo
  m_ro->insertStringList( list );
  m_ro->completionObject()->setItems( list );

  // Setup read-write combo
  m_rw->insertStringList( list );
  m_rw->completionObject()->setItems( list );

  // Setup read-write combo
  m_hc->insertStringList( list );
  m_hc->completionObject()->setItems( list );

  // Setup konq's combobox
  KSimpleConfig historyConfig( "konq_history" );
  historyConfig.setGroup( "Location Bar" );
  KCompletion * s_pCompletion = new KCompletion;
  s_pCompletion->setOrder( KCompletion::Weighted );
  s_pCompletion->setItems( historyConfig.readListEntry( "ComboContents" ) );
  s_pCompletion->setCompletionMode( KGlobalSettings::completionMode() );
  m_konqc->setCompletionObject( s_pCompletion );

  QPixmap pix = SmallIcon("www");
  m_konqc->insertItem( pix, "http://www.kde.org" );
  m_konqc->setCurrentItem( m_konqc->count()-1 );

  m_timer = new QTimer (this);
  connect (m_timer, SIGNAL (timeout()), SLOT (slotTimeout()));
}

KComboBoxTest::~KComboBoxTest()
{
  if (m_timer)
  {
    delete m_timer;
    m_timer = 0;
  }
}

void KComboBoxTest::slotDisable ()
{
  if (m_timer->isActive())
    return;

  m_btnEnable->setEnabled (!m_btnEnable->isEnabled());

  m_timer->start (5000, true);
}

void KComboBoxTest::slotTimeout ()
{
  bool enabled = m_ro->isEnabled();

  if (enabled)
    m_btnEnable->setText ("Ena&ble");
  else
    m_btnEnable->setText ("Disa&ble");

  m_qc->setEnabled (!enabled);
  m_ro->setEnabled (!enabled);
  m_rw->setEnabled (!enabled);
  m_hc->setEnabled (!enabled);
  m_konqc->setEnabled (!enabled);

  m_btnEnable->setEnabled (!m_btnEnable->isEnabled());
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

void KComboBoxTest::slotReturnPressed(const QString& item)
{
  kdDebug() << "Return Pressed, value = " << item << endl;
}

void KComboBoxTest::quitApp()
{
  kapp->closeAllWindows();
}

int main ( int argc, char **argv)
{
  KAboutData about("kcomboboxtest", "kcomboboxtest", "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication a;

  KComboBoxTest* t= new KComboBoxTest;
  a.setMainWidget (t);
  t->show ();
  return a.exec();
}

#include "kcomboboxtest.moc"
