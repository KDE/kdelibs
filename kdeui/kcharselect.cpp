/******************************************************************/
/* KCharSelect - (c) by Reginald Stadlbauer 1999                  */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/******************************************************************/

#include "kcharselect.h"
#include "kcharselect.moc"

#include <qevent.h>
#include <qfont.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qhbox.h>

#include <klocale.h>
#include <kapp.h>

#include <X11/Xlib.h>

/******************************************************************/
/* Class: KCharSelectTable                                        */
/******************************************************************/

//==================================================================
KCharSelectTable::KCharSelectTable(QWidget *parent,const char *name,const QString &_font,const QChar &_chr,int _tableNum)
  : QTableView(parent,name), vFont(_font), vChr(_chr), vTableNum(_tableNum), vPos(0,0)
{
  setBackgroundColor(colorGroup().base());

  setCellWidth(20);
  setCellHeight(25);

  setNumCols(32);
  setNumRows(8);
  
  repaint(true);
}

//==================================================================
void KCharSelectTable::setFont(const QString &_font)
{
  vFont = _font;
  repaint(true);
}

//==================================================================
void KCharSelectTable::setChar(const QChar &_chr)
{
  vChr = _chr;
  repaint(true);
}

//==================================================================
void KCharSelectTable::setTableNum(int _tableNum)
{
  vTableNum = _tableNum;
  repaint(true);
}

//==================================================================
QSize KCharSelectTable::sizeHint()
{
  int w = cellWidth();
  int h = cellHeight();

  w *= numCols();
  h *= numRows();

  return QSize(w,h);
}

//==================================================================
void KCharSelectTable::paintCell(class QPainter* p,int row,int col)
{
  int w = cellWidth(col);
  int h = cellHeight(row);
  int x2 = w - 1;
  int y2 = h - 1;

  unsigned short c = vTableNum * 255;
  c += row * numCols();
  c += col;

  if (c == vChr.unicode())
    {
      p->setBrush(QBrush(colorGroup().highlight()));
      p->setPen(NoPen);
      p->drawRect(0,0,w,h);
      p->setPen(colorGroup().highlightedText());
      vPos = QPoint(col,row);
    }
  else
    p->setPen(colorGroup().text());

  p->setFont(QFont(vFont));
  
  p->drawText(0,0,x2,y2,AlignHCenter | AlignVCenter,QString(QChar(c)));

  p->setPen(colorGroup().text());

  p->drawLine(x2,0,x2,y2);
  p->drawLine(0,y2,x2,y2);

  if (row == 0)
    p->drawLine(0,0,x2,0);
  if (col == 0)
    p->drawLine(0,0,0,y2);
}

//==================================================================
void KCharSelectTable::mouseMoveEvent(QMouseEvent *e)
{
  if (findRow(e->y()) != -1 && findCol(e->x()) != -1)
    {
      QPoint oldPos = vPos;

      vPos.setX(findCol(e->x()));
      vPos.setY(findRow(e->y()));

      vChr = QChar(vTableNum * 255 + 32 * vPos.y() + vPos.x());
      
      updateCell(oldPos.y(),oldPos.x(),true);
      updateCell(vPos.y(),vPos.x(),true);
      
      emit highlighted(vChr);
      emit highlighted();
    }
}

/******************************************************************/
/* Class: KCharSelect                                             */
/******************************************************************/

//==================================================================
KCharSelect::KCharSelect(QWidget *parent,const char *name,const QString &_font,const QChar &_chr,int _tableNum)
  : QVBox(parent,name)
{
  QHBox *bar = new QHBox(this);
  
  QLabel *lFont = new QLabel(i18n("  Font:  "),bar);
  lFont->resize(lFont->sizeHint());
  lFont->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  lFont->setMaximumWidth(lFont->sizeHint().width());
  
  fontCombo = new QComboBox(true,bar);
  fillFontCombo();
  fontCombo->resize(fontCombo->sizeHint());
  
  connect(fontCombo,SIGNAL(activated(const QString &)),this,SLOT(fontSelected(const QString &)));
  
  QLabel *lTable = new QLabel(i18n("  Table:  "),bar);
  lTable->resize(lTable->sizeHint());
  lTable->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  lTable->setMaximumWidth(lTable->sizeHint().width());

  tableSpinBox = new QSpinBox(0,255,1,bar);
  tableSpinBox->resize(tableSpinBox->sizeHint());
  
  connect(tableSpinBox,SIGNAL(valueChanged(int)),this,SLOT(tableChanged(int)));
  
  charTable = new KCharSelectTable(this,name,_font.isEmpty() ? QVBox::font().family() : _font,_chr,_tableNum);
  charTable->resize(charTable->sizeHint());
  charTable->setMaximumSize(charTable->size());
  charTable->setMinimumSize(charTable->size());

  setFont(_font.isEmpty() ? QVBox::font().family() : _font);
  setTableNum(_tableNum);

  connect(charTable,SIGNAL(highlighted(const QChar &)),this,SLOT(charHighlighted(const QChar &)));
  connect(charTable,SIGNAL(highlighted()),this,SLOT(charHighlighted()));
  connect(charTable,SIGNAL(activated(const QChar &)),this,SLOT(charActivated(const QChar &)));
  connect(charTable,SIGNAL(activated()),this,SLOT(charActivated()));
}

//==================================================================
QSize KCharSelect::sizeHint()
{
  return QVBox::sizeHint();
}

//==================================================================
void KCharSelect::setFont(const QString &_font)
{
  int i = fontList.find(_font);
  if (i != -1)
    {
      fontCombo->setCurrentItem(i);
      charTable->setFont(_font);
    }
  else
    warning(i18n("Can't find Font: %s"),_font.ascii());
}

//==================================================================
void KCharSelect::setChar(const QChar &_chr)
{
  charTable->setChar(_chr);
}

//==================================================================
void KCharSelect::setTableNum(int _tableNum)
{
  tableSpinBox->setValue(_tableNum);
  charTable->setTableNum(_tableNum);
}

//==================================================================
void KCharSelect::fillFontCombo()
{
  int numFonts;
  Display *kde_display;
  char** fontNames;
  char** fontNames_copy;
  QString qfontname;
  
  bool have_installed = kapp->getKDEFonts(&fontList);

  if (!have_installed)
    {
      kde_display = kapp->getDisplay();

      fontNames = XListFonts(kde_display,"*",32767,&numFonts);
      fontNames_copy = fontNames;

      for(int i = 0; i < numFonts; i++)
	{
	  if (**fontNames != '-')
	    {
	      fontNames++;
	      continue;
	    };

	  qfontname = "";
	  qfontname = *fontNames;
	  int dash = qfontname.find ('-',1,true);

	  if (dash == -1)
	    {
	      fontNames++;
	      continue;
	    }

	  int dash_two = qfontname.find ('-',dash + 1 ,true);

	  if (dash == -1)
	    {
	      fontNames++;
	      continue;
	    }

	  qfontname = qfontname.mid(dash +1,dash_two - dash -1);

	  if (!qfontname.contains("open look",true))
	    {
	      if (qfontname != "nil")
		{
		  if (fontList.find(qfontname) == -1)
		    fontList.inSort(qfontname);
		}
	    }

	  fontNames++;
	}
    
      XFreeFontNames(fontNames_copy);
    }
  
  fontCombo->insertStrList(fontList);
}

//==================================================================
void KCharSelect::fontSelected(const QString &_font)
{
  charTable->setFont(_font);
  emit fontChanged(_font);
}

//==================================================================
void KCharSelect::tableChanged(int _value)
{
  charTable->setTableNum(_value);
}
