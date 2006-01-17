#include <kcmdlineargs.h>

#include "krulertest.h"

#include "kruler.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qevent.h>
#include <q3groupbox.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QFrame>
#include <QGridLayout>
#include <QResizeEvent>

/*
void
MyCheckBox::mouseReleaseEvent(QMouseEvent *e )
{
  QButton::mouseReleaseEvent(e);
  if ();
}
*/

MouseWidget::MouseWidget( QWidget *parent )
  : QFrame(parent)
{
}

void
MouseWidget::mousePressEvent( QMouseEvent *e )
{
  mouseButtonDown = true;
  emit newXPos(e->x());
  emit newYPos(e->y());
}

void
MouseWidget::mouseReleaseEvent( QMouseEvent * )
{ mouseButtonDown = false; }

void
MouseWidget::mouseMoveEvent( QMouseEvent *e )
{
  if (mouseButtonDown) {
    emit newXPos(e->x());
    emit newYPos(e->y());
  }
}

void
MouseWidget::resizeEvent( QResizeEvent *r )
{
  emit newWidth(r->size().width());
  emit newHeight(r->size().height());
}


KRulerTest::KRulerTest()
  : KMainWindow(0)
{
  mainframe = new QFrame(this);

  layout = new QGridLayout(mainframe, 2, 2);

  miniwidget = new QFrame(mainframe);
  miniwidget->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  bigwidget = new MouseWidget(mainframe);
  bigwidget->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);

  //  QRect bwrect = bigwidget->frameRect();
  //  qDebug("big rect: top%i left%i bottom%i right%i",
  //	bwrect.top(), bwrect.left(), bwrect.bottom(), bwrect.right());
  hruler = new KRuler(Qt::Horizontal, mainframe);
  //  hruler->setRange( bwrect.left(), bwrect.right() );
  hruler->setRange( 0, 1000 );
  //  hruler->setOffset( bwrect.left() - bigwidget->frameRect().left() );
  hruler->setOffset( 0 );

  vruler = new KRuler(Qt::Vertical, mainframe);
  vruler->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  vruler->setOffset( 0 );
  vruler->setRange( 0, 1000 );

  connect( bigwidget, SIGNAL(newXPos(int)),
	   hruler, SLOT(slotNewValue(int)) );
  connect( bigwidget, SIGNAL(newYPos(int)),
	   vruler, SLOT(slotNewValue(int)) );
  connect( bigwidget, SIGNAL(newWidth(int)),
	   SLOT(slotNewWidth(int)) );
  connect( bigwidget, SIGNAL(newHeight(int)),
	   SLOT(slotNewHeight(int)) );

  layout->addWidget(miniwidget, 0, 0);
  layout->addWidget(hruler, 0, 1);
  layout->addWidget(vruler, 1, 0);
  layout->addWidget(bigwidget, 1, 1);

  mouse_message = new QLabel("Press and hold mouse button\nfor pointer movement", bigwidget);
  mouse_message->adjustSize();
  mouse_message->move(4,4);

  showMarks = new Q3GroupBox("Show which marks ?", bigwidget);
  showMarks->setFixedSize(140, 160);
  showMarks->move(330,4);
  showTM = new QCheckBox("show tiny marks", showMarks);
  showTM->adjustSize();
  showTM->move(5,15);
  showTM->setChecked(true);
  connect(showTM, SIGNAL(toggled(bool)), SLOT(slotSetTinyMarks(bool)) );
  showLM = new QCheckBox("show little marks", showMarks);
  showLM->adjustSize();
  showLM->move(5,35);
  showLM->setChecked(true);
  connect(showLM, SIGNAL(toggled(bool)), SLOT(slotSetLittleMarks(bool)) );
  showMM = new QCheckBox("show medium marks", showMarks);
  showMM->adjustSize();
  showMM->move(5,55);
  showMM->setChecked(true);
  connect(showMM, SIGNAL(toggled(bool)), SLOT(slotSetMediumMarks(bool)) );
  showBM = new QCheckBox("show big marks", showMarks);
  showBM->adjustSize();
  showBM->move(5,75);
  showBM->setChecked(true);
  connect(showBM, SIGNAL(toggled(bool)), SLOT(slotSetBigMarks(bool)) );
  showEM = new QCheckBox("show end marks", showMarks);
  showEM->adjustSize();
  showEM->move(5,95);
  showEM->setChecked(true);
  connect(showEM, SIGNAL(toggled(bool)), SLOT(slotSetEndMarks(bool)) );
  showPT = new QCheckBox("show ruler pointer", showMarks);
  showPT->adjustSize();
  showPT->move(5,115);
  showPT->setChecked(true);
  connect(showPT, SIGNAL(toggled(bool)), SLOT(slotSetRulerPointer(bool)) );
  fixLen = new QCheckBox("fix ruler length", showMarks);
  fixLen->adjustSize();
  fixLen->move(5,135);
  fixLen->setChecked(true);
  connect(fixLen, SIGNAL(toggled(bool)), SLOT(slotFixRulerLength(bool)) );
  connect(fixLen, SIGNAL(toggled(bool)), SLOT(slotCheckLength(bool)) );

  lineEdit = new Q3GroupBox("Value of begin/end", bigwidget);
  lineEdit->setFixedSize(140, 80);
  lineEdit->move(330,4+160);
  beginMark = new KIntNumInput(0, lineEdit);
  beginMark->setRange(-1000, 1000, 1, false);
  beginMark->move(5, 15);
  beginMark->setFixedSize(beginMark->sizeHint());
  connect(beginMark, SIGNAL(valueChanged(int)), 
	  hruler, SLOT(slotNewOffset(int)) );
  connect(beginMark, SIGNAL(valueChanged(int)), 
	  vruler, SLOT(slotNewOffset(int)) );
  endMark = new KIntNumInput(0, lineEdit);
  endMark->setRange(-1000, 1000, 1, false);
  endMark->move(5, 35);
  endMark->setFixedSize(endMark->sizeHint());
  connect(endMark, SIGNAL(valueChanged(int)), 
	  hruler, SLOT(slotEndOffset(int)) );
  connect(endMark, SIGNAL(valueChanged(int)), 
	  vruler, SLOT(slotEndOffset(int)) );
  lengthInput = new KIntNumInput(0, lineEdit);
  lengthInput->setRange(-1000, 1000, 1, false);
  lengthInput->move(5, 55);
  lengthInput->setFixedSize(lengthInput->sizeHint());
  connect(lengthInput, SIGNAL(valueChanged(int)), 
	  hruler, SLOT(slotEndOffset(int)) );
  connect(lengthInput, SIGNAL(valueChanged(int)), 
	  vruler, SLOT(slotEndOffset(int)) );


  vertrot = new Q3GroupBox("Value of rotate translate for Vert.", bigwidget);
  vertrot->setFixedSize(140, 80);
  vertrot->move(330,4+160+80+4);
  transX = new KDoubleNumInput(vertrot);
  transX->setValue(0.0);
  transX->setRange(-1000, 1000, 1, false);
  transX->move(5, 15);
  transX->setFixedSize(transX->sizeHint());
  //transX->setLabel("transx", AlignLeft);
  connect(transX, SIGNAL(valueChanged(double)), 
	  SLOT(slotSetXTrans(double)) );
  transY = new KDoubleNumInput(vertrot);
  transY->setValue(-12.0);
  transY->setRange(-1000, 1000, 1, false);
  transY->move(5, 35);
  transY->setFixedSize(transY->sizeHint());
  //transY->setLabel("transy", AlignLeft);
  connect(transY, SIGNAL(valueChanged(double)), 
	  SLOT(slotSetYTrans(double)) );
  rotV = new KDoubleNumInput(vertrot);
  rotV->setValue(90.0);
  rotV->setRange(-1000, 1000, 1, false);
  rotV->move(5, 55);
  rotV->setFixedSize(rotV->sizeHint());
  //rotV->setLabel("rot", AlignLeft);
  connect(rotV, SIGNAL(valueChanged(double)), 
	  SLOT(slotSetRotate(double)) );
  

  metricstyle = new Q3ButtonGroup("metric styles", bigwidget);
  metricstyle->setFixedSize(100, 120);
  metricstyle->move(330-110,4);
  pixelmetric = new QRadioButton("pixel", metricstyle);
  pixelmetric->adjustSize();
  pixelmetric->move(5,15);
  metricstyle->insert(pixelmetric, (int)KRuler::Pixel);
  inchmetric = new QRadioButton("inch", metricstyle);
  inchmetric->adjustSize();
  inchmetric->move(5,35);
  metricstyle->insert(inchmetric, (int)KRuler::Inch);
  mmmetric = new QRadioButton("millimeter", metricstyle);
  mmmetric->adjustSize();
  mmmetric->move(5,55);
  metricstyle->insert(mmmetric, (int)KRuler::Millimetres);
  cmmetric = new QRadioButton("centimeter", metricstyle);
  cmmetric->adjustSize();
  cmmetric->move(5,75);
  metricstyle->insert(cmmetric, (int)KRuler::Centimetres);
  mmetric = new QRadioButton("meter", metricstyle);
  mmetric->adjustSize();
  mmetric->move(5,95);
  metricstyle->insert(mmetric, (int)KRuler::Metres);
  connect ( metricstyle, SIGNAL(clicked(int)), SLOT(slotSetMStyle(int)) );

  setCentralWidget(mainframe);

  slotUpdateShowMarks();
}

KRulerTest::~KRulerTest()
{
  delete layout;
  delete hruler;
  delete vruler;
  delete miniwidget;
  delete bigwidget;
  delete mainframe;
}

void
KRulerTest::slotNewWidth(int width)
{
  hruler->setMaxValue(width);
}

void
KRulerTest::slotNewHeight(int height)
{
  vruler->setMaxValue(height);
}

void
KRulerTest::slotSetTinyMarks(bool set)
{
  hruler->setShowTinyMarks(set);
  vruler->setShowTinyMarks(set);
}

void
KRulerTest::slotSetLittleMarks(bool set)
{
  hruler->setShowLittleMarks(set);
  vruler->setShowLittleMarks(set);
}

void
KRulerTest::slotSetMediumMarks(bool set)
{
  hruler->setShowMediumMarks(set);
  vruler->setShowMediumMarks(set);
}

void
KRulerTest::slotSetBigMarks(bool set)
{
  hruler->setShowBigMarks(set);
  vruler->setShowBigMarks(set);
}

void
KRulerTest::slotSetEndMarks(bool set)
{
  hruler->setShowEndMarks(set);
  vruler->setShowEndMarks(set);
}

void 
KRulerTest::slotSetRulerPointer(bool set)
{
  hruler->setShowPointer(set);
  vruler->setShowPointer(set);
}

void 
KRulerTest::slotSetRulerLength(int len)
{
  hruler->setLength(len);
  vruler->setLength(len);
}

void 
KRulerTest::slotFixRulerLength(bool fix)
{
  hruler->setLengthFixed(fix);
  vruler->setLengthFixed(fix);
}

void
KRulerTest::slotSetMStyle(int style)
{
  hruler->setRulerMetricStyle((KRuler::MetricStyle)style);
  vruler->setRulerMetricStyle((KRuler::MetricStyle)style);
  slotUpdateShowMarks();
}


void
KRulerTest::slotUpdateShowMarks()
{
  showTM->setChecked(hruler->showTinyMarks());
  showLM->setChecked(hruler->showLittleMarks());
  showMM->setChecked(hruler->showMediumMarks());
  showBM->setChecked(hruler->showBigMarks());
  showEM->setChecked(hruler->showEndMarks());
}

void 
KRulerTest::slotCheckLength(bool fixlen)
{
  beginMark->setValue(hruler->offset());
  endMark->setValue(hruler->endOffset());
  lengthInput->setValue(hruler->length());
}

void 
KRulerTest::slotSetRotate(double d)
{
#ifdef KRULER_ROTATE_TEST
  vruler->rotate = d;
  vruler->update();
  //debug("rotate %.1f", d);
#endif
}

void 
KRulerTest::slotSetXTrans(double d)
{
#ifdef KRULER_ROTATE_TEST
  vruler->xtrans = d;
  vruler->update();
  //debug("trans x %.1f", d);
#endif
}

void 
KRulerTest::slotSetYTrans(double d)
{
#ifdef KRULER_ROTATE_TEST
  vruler->ytrans = d;
  vruler->update();
  //debug("trans y %.1f", d);
#endif
}


/* --- MAIN -----------------------*/
int main(int argc, char **argv)
{
  KCmdLineArgs::init( argc, argv, "test", "Test" ,"test app" ,"1.0" );
  KApplication *testapp;
  KRulerTest   *window;

  testapp = new KApplication;
  testapp->setFont(QFont("Helvetica",12),true);

  window = new KRulerTest();
  testapp->setMainWidget(window);
  window->setCaption("KRulerTest");
  window->show();
  return testapp->exec();
}

#include "krulertest.moc"

