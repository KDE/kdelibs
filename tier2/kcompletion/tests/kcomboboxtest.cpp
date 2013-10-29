#include "kcomboboxtest.h"

#include <assert.h>

#include <kconfiggroup.h>
#include <kconfig.h>

#include <QApplication>
#include <QPushButton>
#include <QLayout>
#include <QPixmap>
#include <QLabel>
#include <QtCore/QTimer>
#include <QBoxLayout>
#include <QDebug>

#include <kcombobox.h>
#include <khistorycombobox.h>

KComboBoxTest::KComboBoxTest(QWidget* widget)
              :QWidget(widget)
{
  QVBoxLayout *vbox = new QVBoxLayout (this);

  // Qt combobox
  QHBoxLayout* hbox = new QHBoxLayout(this);
  QLabel* lbl = new QLabel("&QCombobox:", this);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);
  hbox->addWidget(lbl);

  m_qc = new QComboBox(this);
  m_qc->setObjectName( QLatin1String( "QtReadOnlyCombo" ) );
  lbl->setBuddy (m_qc);
  connectComboSignals(m_qc);
  hbox->addWidget(m_qc);

  vbox->addLayout (hbox);

  // Read-only combobox
  hbox = new QHBoxLayout(this);
  lbl = new QLabel("&Read-Only Combo:", this);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);
  hbox->addWidget(lbl);

  m_ro = new KComboBox(this);
  m_ro->setObjectName( "ReadOnlyCombo" );
  lbl->setBuddy (m_ro);
  m_ro->setCompletionMode( KCompletion::CompletionAuto );
  connectComboSignals(m_ro);
  hbox->addWidget(m_ro);

  vbox->addLayout(hbox);

  // Read-write combobox
  hbox = new QHBoxLayout(this);
  lbl = new QLabel("&Editable Combo:", this);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);
  hbox->addWidget(lbl);

  m_rw = new KComboBox( true, this );
  m_rw->setObjectName( "ReadWriteCombo" );
  lbl->setBuddy (m_rw);
  m_rw->setDuplicatesEnabled( true );
  m_rw->setInsertPolicy( QComboBox::NoInsert );
  m_rw->setTrapReturnKey( true );
  connectComboSignals(m_rw);
  hbox->addWidget(m_rw);
  vbox->addLayout(hbox);

  // History combobox...
  hbox = new QHBoxLayout(this);
  lbl = new QLabel("&History Combo:", this);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);
  hbox->addWidget(lbl);

  m_hc = new KHistoryComboBox( this );
  m_hc->setObjectName( "HistoryCombo" );
  lbl->setBuddy (m_hc);
  m_hc->setDuplicatesEnabled( true );
  m_hc->setInsertPolicy( QComboBox::NoInsert );
  connectComboSignals(m_hc);
  m_hc->setTrapReturnKey(true);
  hbox->addWidget(m_hc);
  vbox->addLayout(hbox);

  // Read-write combobox that is a replica of code in konqueror...
  hbox = new QHBoxLayout(this);
  lbl = new QLabel( "&Konq's Combo:", this);
  lbl->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Preferred);
  hbox->addWidget(lbl);

  m_konqc = new KComboBox( true, this );
  m_konqc->setObjectName( "KonqyCombo" );
  lbl->setBuddy (m_konqc);
  m_konqc->setMaxCount( 10 );
  connectComboSignals(m_konqc);
  hbox->addWidget(m_konqc);
  vbox->addLayout(hbox);

  // Create an exit button
  hbox = new QHBoxLayout(this);
  m_btnExit = new QPushButton( "E&xit", this );
  QObject::connect( m_btnExit, SIGNAL(clicked()), SLOT(quitApp()) );
  hbox->addWidget(m_btnExit);

  // Create a disable button...
  m_btnEnable = new QPushButton( "Disa&ble", this );
  QObject::connect (m_btnEnable, SIGNAL(clicked()), SLOT(slotDisable()));
  hbox->addWidget(m_btnEnable);

  vbox->addLayout(hbox);

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

  // Setup history combo
  m_hc->addItems( list );
  m_hc->completionObject()->setItems( list + QStringList() << "One" << "Two" << "Three" );

  // Setup konq's combobox
  KConfig historyConfig( "konq_history", KConfig::SimpleConfig );
  KConfigGroup cg(&historyConfig, "Location Bar" );
  KCompletion * s_pCompletion = new KCompletion;
  s_pCompletion->setOrder( KCompletion::Weighted );
  s_pCompletion->setItems( cg.readEntry( "ComboContents", QStringList() ) );
  s_pCompletion->setCompletionMode( KCompletion::CompletionPopup );
  m_konqc->setCompletionObject( s_pCompletion );

  QPixmap pix(16, 16);
  pix.fill(Qt::blue);
  m_konqc->addItem( pix, "http://www.kde.org" );
  m_konqc->setCurrentIndex( m_konqc->count()-1 );

  m_timer = new QTimer (this);
  connect (m_timer, SIGNAL (timeout()), SLOT (slotTimeout()));
}

KComboBoxTest::~KComboBoxTest()
{
    delete m_timer;
    m_timer = 0;
}

void KComboBoxTest::connectComboSignals(QComboBox* combo)
{
    QObject::connect(combo, SIGNAL(activated(int)), SLOT(slotActivated(int)));
    QObject::connect(combo, SIGNAL(activated(QString)), SLOT(slotActivated(QString)));
    QObject::connect(combo, SIGNAL(currentIndexChanged(int)), SLOT(slotCurrentIndexChanged(int)));
    QObject::connect(combo, SIGNAL(currentIndexChanged(QString)), SLOT(slotCurrentIndexChanged(QString)));
    QObject::connect(combo, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
    QObject::connect(combo, SIGNAL(returnPressed(QString)), SLOT(slotReturnPressed(QString)));
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

void KComboBoxTest::slotCurrentIndexChanged(int index)
{
  qDebug() << qPrintable(sender()->objectName()) << ", index:" << index;
}

void KComboBoxTest::slotCurrentIndexChanged(const QString& item)
{
  qDebug() << qPrintable(sender()->objectName()) << ", item:" << item;
}

void KComboBoxTest::slotActivated( int index )
{
  qDebug() << "Activated Combo:" << qPrintable(sender()->objectName()) << ", index:" << index;
}

void KComboBoxTest::slotActivated (const QString& item)
{
  qDebug() << "Activated Combo:" << qPrintable(sender()->objectName()) << ", item:" << item;
}

void KComboBoxTest::slotReturnPressed ()
{
  qDebug() << "Return Pressed:" << qPrintable(sender()->objectName());
}

void KComboBoxTest::slotReturnPressed(const QString& item)
{
  qDebug() << "Return Pressed:" << qPrintable(sender()->objectName()) << ", value =" << item;
}

void KComboBoxTest::quitApp()
{
  qApp->closeAllWindows();
}

int main ( int argc, char **argv)
{
  QApplication::setApplicationName("kcomboboxtest");

  QApplication a(argc, argv);

  KComboBoxTest* t= new KComboBoxTest;
  t->show ();
  return a.exec();
}

