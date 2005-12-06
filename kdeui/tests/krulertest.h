/* -*- c++ -*- */

#ifndef krulertest_h
#define krulertest_h

#include <kapplication.h>
#include <kmainwindow.h>
#include <qwidget.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
#include <knuminput.h>

class KRuler;
class QWidget;
class QFrame;
class QGridLayout;
class QCheckBox;
class Q3GroupBox;


class MouseWidget : public QFrame
{
Q_OBJECT
public:
MouseWidget( QWidget *parent=0, const char *name=0, Qt::WFlags f=0 );

signals:
  void newXPos(int);
  void newYPos(int);
  void newWidth(int);
  void newHeight(int);

protected:
  virtual void mousePressEvent   ( QMouseEvent * );
  virtual void mouseReleaseEvent ( QMouseEvent * );
  virtual void mouseMoveEvent    ( QMouseEvent * );
  virtual void resizeEvent       ( QResizeEvent * );
private:
  bool mouseButtonDown;

};


class KRulerTest : public KMainWindow
{
Q_OBJECT
public:
KRulerTest( const char *name = 0L );
~KRulerTest();

private slots:
  void slotNewWidth(int);
  void slotNewHeight(int);

  void slotSetTinyMarks(bool);
  void slotSetLittleMarks(bool);
  void slotSetMediumMarks(bool);
  void slotSetBigMarks(bool);
  void slotSetEndMarks(bool);
  void slotSetRulerPointer(bool);

  void slotSetRulerLength(int);
  void slotFixRulerLength(bool);
  void slotSetMStyle(int);
  void slotUpdateShowMarks();
  void slotCheckLength(bool);

  void slotSetRotate(double);
  void slotSetXTrans(double);
  void slotSetYTrans(double);

private:

  KRuler *hruler, *vruler;
  QGridLayout *layout;
  QFrame *miniwidget, *bigwidget, *mainframe;

  QLabel *mouse_message;
  Q3GroupBox *showMarks, *lineEdit, *vertrot;
  QCheckBox *showTM, *showLM, *showMM, *showBM, *showEM, *showPT, *fixLen;
  KIntNumInput *beginMark, *endMark, *lengthInput;
  KDoubleNumInput *transX, *transY, *rotV;
  Q3ButtonGroup *metricstyle;
  QRadioButton *pixelmetric, *inchmetric, *mmmetric, *cmmetric, *mmetric;

};
#endif

