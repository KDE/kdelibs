

#include "krulertest.moc"

#include "kruler.h"
#include <qlayout.h>
#include <qgrpbox.h>

/*
void
MyCheckBox::mouseReleaseEvent(QMouseEvent *e )
{
  QButton::mouseReleaseEvent(e);
  if ();
}
*/

MouseWidget::MouseWidget( QWidget *parent=0, const char *name=0, WFlags f=0 )
  : QFrame(parent, name, f)
{
}

void 
MouseWidget::mousePressEvent( QMouseEvent *e )
{
  mouseButtonDown = TRUE;
  emit newXPos(e->x());
  emit newYPos(e->y());
}

void 
MouseWidget::mouseReleaseEvent( QMouseEvent * )
{ mouseButtonDown = FALSE; }

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


KRulerTest::KRulerTest( const char *name = 0L )
  : KTopLevelWidget(name)
{
  mainframe = new QFrame(this);

  layout = new QGridLayout(mainframe, 2, 2);

  miniwidget = new QFrame(mainframe);
  miniwidget->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  bigwidget = new MouseWidget(mainframe);
  bigwidget->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);

  //  QRect bwrect = bigwidget->frameRect();
  //  debug("big rect: top%i left%i bottom%i right%i",
  //	bwrect.top(), bwrect.left(), bwrect.bottom(), bwrect.right());
  hruler = new KRuler(KRuler::horizontal, mainframe);
  //  hruler->setRange( bwrect.left(), bwrect.right() );
  hruler->setRange( 0, 1000 );
  //  hruler->setOffset( bwrect.left() - bigwidget->frameRect().left() );
  hruler->setOffset( 0 );

  vruler = new KRuler(KRuler::vertical, mainframe);
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

  showMarks = new QGroupBox("Show which marks ?", bigwidget);
  showMarks->setFixedSize(140, 120);
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

  lineEdit = new QGroupBox("Value of begin/end", bigwidget);
  lineEdit->setFixedSize(140, 60);
  lineEdit->move(330,4+120);
  beginMark = new KIntegerLine(lineEdit);
  beginMark->move(5, 15);
  beginMark->setValue(0);
  beginMark->setFixedSize(60, 20);
  endMark = new KIntegerLine(lineEdit);
  endMark->move(5, 35);
  endMark->setFixedSize(60, 20);

  metricstyle = new QButtonGroup("metric styles", bigwidget);
  metricstyle->setFixedSize(100, 120);
  metricstyle->move(330-110,4);
  pixelmetric = new QRadioButton("pixel", metricstyle);
  pixelmetric->adjustSize();
  pixelmetric->move(5,15);
  metricstyle->insert(pixelmetric, (int)KRuler::pixel);
  inchmetric = new QRadioButton("inch", metricstyle);
  inchmetric->adjustSize();
  inchmetric->move(5,35);
  metricstyle->insert(inchmetric, (int)KRuler::inch);
  mmmetric = new QRadioButton("millimeter", metricstyle);
  mmmetric->adjustSize();
  mmmetric->move(5,55);
  metricstyle->insert(mmmetric, (int)KRuler::millimetres);
  cmmetric = new QRadioButton("centimeter", metricstyle);
  cmmetric->adjustSize();
  cmmetric->move(5,75);
  metricstyle->insert(cmmetric, (int)KRuler::centimetres);
  mmetric = new QRadioButton("meter", metricstyle);
  mmetric->adjustSize();
  mmetric->move(5,95);
  metricstyle->insert(mmetric, (int)KRuler::metres);
  connect ( metricstyle, SIGNAL(clicked(int)), SLOT(slotSetMStyle(int)) );

  setView (mainframe);
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
  hruler->showTinyMarks(set);
  vruler->showTinyMarks(set);
}

void 
KRulerTest::slotSetLittleMarks(bool set)
{
  hruler->showLittleMarks(set);
  vruler->showLittleMarks(set);
}

void 
KRulerTest::slotSetMediumMarks(bool set)
{
  hruler->showMediumMarks(set);
  vruler->showMediumMarks(set);
}

void 
KRulerTest::slotSetBigMarks(bool set)
{
  hruler->showBigMarks(set);
  vruler->showBigMarks(set);
}

void
KRulerTest::slotSetMStyle(int style)
{
  hruler->setRulerStyle((KRuler::metric_style)style);
  vruler->setRulerStyle((KRuler::metric_style)style);
}


/* --- MAIN -----------------------*/
int main(int argc, char **argv) 
{
  KApplication *testapp;
  KRulerTest   *window;

  testapp = new KApplication(argc, argv,"krulertest");
  testapp->setFont(QFont("Helvetica",12),true);

  window = new KRulerTest("main");
  testapp->setMainWidget(window);
  window->setCaption("KRulerTest");
  window->show();
  return testapp->exec();
}
