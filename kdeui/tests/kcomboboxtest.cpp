#include "kcomboboxtest.h"

#include <assert.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>

#include <QtGui/QPushButton>
#include <QtGui/QLayout>
#include <QtGui/QPixmap>
#include <QtGui/QLabel>
#include <khbox.h>
#include <QtCore/QTimer>
#include <QtGui/QBoxLayout>

#include <kcombobox.h>
#include <khistorycombobox.h>

KComboBoxTest::KComboBoxTest(QWidget* widget)
              :QWidget(widget)
{
  QVBoxLayout *vbox = new QVBoxLayout (this);

  // Qt combobox
  KHBox* hbox = new KHBox(this);
  hbox->setSpacing (-1);
  QLabel* lbl = new QLabel("&QCombobox:", hbox);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);

  m_qc = new QComboBox(hbox);
  m_qc->setObjectName( QLatin1String( "QtReadOnlyCombo" ) );
  lbl->setBuddy (m_qc);
  QObject::connect (m_qc, SIGNAL(activated(int)), SLOT(slotActivated(int)));
  QObject::connect (m_qc, SIGNAL(activated(const QString&)), SLOT (slotActivated(const QString&)));
  vbox->addWidget (hbox);

  // Read-only combobox
  hbox = new KHBox(this);
  hbox->setSpacing (-1);
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
  hbox = new KHBox(this);
  hbox->setSpacing (-1);
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
  hbox = new KHBox(this);
  hbox->setSpacing (-1);
  lbl = new QLabel("&History Combo:", hbox);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);

  m_hc = new KHistoryComboBox( hbox );
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
  hbox = new KHBox(this);
  hbox->setSpacing (-1);
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
  hbox = new KHBox (this);
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
  m_qc->addItems(list );

  // Setup read-only combo
  m_ro->addItems( list );
  m_ro->completionObject()->setItems( list );

  // Setup read-write combo
  m_rw->addItems( list );
  m_rw->completionObject()->setItems( list );

  // Setup read-write combo
  m_hc->addItems( list );
  m_hc->completionObject()->setItems( list );

  // Setup konq's combobox
  KConfig historyConfig( "konq_history", KConfig::SimpleConfig );
  KConfigGroup cg(&historyConfig, "Location Bar" );
  KCompletion * s_pCompletion = new KCompletion;
  s_pCompletion->setOrder( KCompletion::Weighted );
  s_pCompletion->setItems( cg.readEntry( "ComboContents", QStringList() ) );
  s_pCompletion->setCompletionMode( KGlobalSettings::completionMode() );
  m_konqc->setCompletionObject( s_pCompletion );

  QPixmap pix = SmallIcon("www");
  m_konqc->addItem( pix, "http://www.kde.org" );
  m_konqc->setCurrentIndex( m_konqc->count()-1 );

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

  m_timer->setSingleShot(true);
  m_timer->start (5000);
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
  kDebug() << "Activated Combo: " << qPrintable(sender()->objectName()) << ", index:" << index;
}

void KComboBoxTest::slotActivated (const QString& item)
{
  kDebug() << "Activated Combo: " << qPrintable(sender()->objectName()) << ", item: " << item;
}

void KComboBoxTest::slotReturnPressed ()
{
  kDebug() << "Return Pressed: " << qPrintable(sender()->objectName());
}

void KComboBoxTest::slotReturnPressed(const QString& item)
{
  kDebug() << "Return Pressed, value = " << item;
}

void KComboBoxTest::quitApp()
{
  kapp->closeAllWindows();
}

int main ( int argc, char **argv)
{
  KAboutData about("kcomboboxtest", 0, ki18n("kcomboboxtest"), "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication a;

  KComboBoxTest* t= new KComboBoxTest;
  t->show ();
  return a.exec();
}

#include "kcomboboxtest.moc"
