/******************************************************************/
/* KCharSelect - (c) by Reginald Stadlbauer 1999                  */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/******************************************************************/

#ifndef kcharselect_h
#define kcharselect_h

#include <qtableview.h>
#include <qvbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qfont.h>
#include <qstring.h>
#include <qsize.h>
#include <qpoint.h>
#include <qstrlist.h>

class QMouseEvent;
class QSpinBox;

/******************************************************************/
/* Class: KCharSelectTable                                        */
/******************************************************************/

class KCharSelectTable : public QTableView
{
  Q_OBJECT

public:
  KCharSelectTable(QWidget *parent,const char *name,const QString &_font,const QChar &_chr,int _tableNum);

  virtual QSize sizeHint();

  virtual void setFont(const QString &_font);
  virtual void setChar(const QChar &_chr);
  virtual void setTableNum(int _tableNum);
  
  virtual QChar chr()
  { return vChr; }
  
protected:
  virtual void paintCell(class QPainter *p,int row,int col);

  virtual void mousePressEvent(QMouseEvent *e)
  {  mouseMoveEvent(e); }
  virtual void mouseReleaseEvent(QMouseEvent *e)
  { mouseMoveEvent(e); emit activated(chr()); emit activated(); }
  virtual void mouseMoveEvent(QMouseEvent *e);

  QString vFont;
  QChar vChr;
  int vTableNum;
  QPoint vPos;  
  
signals:
  void highlighted(const QChar &c);
  void highlighted();
  void activated(const QChar &c);
  void activated();
  
};

/******************************************************************/
/* Class: KCharSelect                                             */
/******************************************************************/

class KCharSelect : public QVBox
{
  Q_OBJECT
  
public:
  KCharSelect(QWidget *parent,const char *name,const QString &_font = QString::null,const QChar &_chr = ' ',int _tableNum = 0);
  
  virtual QSize sizeHint();

  virtual void setFont(const QString &_font);
  virtual void setChar(const QChar &_chr);
  virtual void setTableNum(int _tableNum);
  
  virtual QChar chr()
  { return charTable->chr(); }
  virtual QString font()
  { return fontCombo->currentText(); }
  virtual int tableNum()
  { return tableSpinBox->value(); }

  virtual void enableFontCombo(bool e)
  { fontCombo->setEnabled(e); }
  virtual void enableTableSpinBox(bool e)
  { tableSpinBox->setEnabled(e); }
  
  virtual bool isFontComboEnabled()
  { return fontCombo->isEnabled(); }
  virtual bool isTableSpinBoxEnabled()
  { return tableSpinBox->isEnabled(); }
  
protected:
  virtual void fillFontCombo();
  
  QComboBox *fontCombo;
  QSpinBox *tableSpinBox;
  KCharSelectTable *charTable;
  QStrList fontList;

protected slots:
  void fontChanged(const QString &_font);
  void tableChanged(int _value);
  void charHighlighted(const QChar &c)
  { emit highlighted(c); }
  void charHighlighted()
  { emit highlighted(); }
  void charActivated(const QChar &c)
  { emit activated(c); }
  void charActivated()
  { emit activated(); }
  
signals:
  void highlighted(const QChar &c);
  void highlighted();
  void activated(const QChar &c);
  void activated();
  
};
  
#endif
