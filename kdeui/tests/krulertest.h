/* -*- c++ -*- */

#ifndef krulertest_h
#define krulertest_h

#include <QtGui/QCheckBox>
#include <QtGui/QRadioButton>
#include <QtGui/QFrame>

#include <kapplication.h>
#include <kmainwindow.h>
#include <knuminput.h>

class KRuler;
class QWidget;
class QGridLayout;
class QCheckBox;
class QGroupBox;
class QLabel;

class MouseWidget : public QFrame
{
Q_OBJECT
public:
MouseWidget( QWidget *parent=0 );

Q_SIGNALS:
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
KRulerTest();
~KRulerTest();

private Q_SLOTS:
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
  QGroupBox *showMarks, *lineEdit, *vertrot;
  QCheckBox *showTM, *showLM, *showMM, *showBM, *showEM, *showPT, *fixLen;
  KIntNumInput *beginMark, *endMark, *lengthInput;
  KDoubleNumInput *transX, *transY, *rotV;
  QGroupBox *metricstyle;
  QRadioButton *pixelmetric, *inchmetric, *mmmetric, *cmmetric, *mmetric;

};
#endif

