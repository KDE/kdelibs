/* This file is part of the KDE libraries
    Copyright (C) 1997 The KDE Team

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//  Written by Stefan Taferner <taferner@kde.org>
//  Modified by Alessandro Russo <axela@bigfoot.org>
//
// BugFix:
//  -The resize cursor appared in the wrong position when the tab is
//   scrolled to the right.
//  -Sometimes some columns became suddenly pressed.
//  -Corrected resizing of the last column for fill the real size of the table.
//
// New Features:
//  -Drag&Slide of the columns bar like Window95.
//  -Complete key Bindings (Must be enable by enableKey(). )
//  -Three Mode to Order the content of table
//   (this mode are column specific, but you should'nt mix SimpleOrder with
//    ComplexOrder for obvious reason):
//      NoOrder = Old mode, do not order.
//      SimpleOrder = An arrow appear near the title of column, are all
//                    disabled except the current selected column.
//      ComplexOrder= Use a new widget KNumCheckButton, the behaviour is very
//                    complex:
//               Single Left Click : set to the next free number if the
//                                   button is blank,or else do nothing.
//               Single Right Click: clear the button and adjust the others.
//               Double Left Click : clear all buttons and set the current
//                                   to '1'.
//               Double Right Click: clear all buttons.
//
//
//   Todo:
//    - Save the current status of checkbutton/arrows to the config file.

#include "ktablistbox.h"

#include <qfontmetrics.h>
#include <qpainter.h>
#include <qkeycode.h>
#include <qbitmap.h>
#include <qdrawutil.h>
#include <qscrollbar.h>
#include <kapp.h>

// This is only for flushKeys().
#include <X11/Xlib.h>


#include <stdarg.h>

#define INIT_MAX_ITEMS 16
#define ARROW_SPACE 15
#define BUTTON_SPACE 4
#define MINIMUM_SPACE 9

#include "ktablistbox.h"


//=============================================================================
//
//  C L A S S   KTabListBoxItem
//
//=============================================================================
KTabListBoxItem::KTabListBoxItem(int aColumns)
{
  columns = aColumns;
  txt = new QString[columns];
  mark = -2;
}


//-----------------------------------------------------------------------------
KTabListBoxItem::~KTabListBoxItem()
{
  if (txt) delete[] txt;
  txt = 0L;
}


//-----------------------------------------------------------------------------
KTabListBoxItem& KTabListBoxItem::operator=(const KTabListBoxItem& from)
{
  int i;

  for (i=0; i<columns; i++)
    txt[i] = from.txt[i];

  fgColor = from.fgColor;

  return *this;
}


//=============================================================================
//
//  C L A S S   KTabListBoxColumn
//
//=============================================================================
//-----------------------------------------------------------------------------
KTabListBoxColumn::KTabListBoxColumn(KTabListBox* pa, int w): QObject()
{
  initMetaObject();
  iwidth = w;
  idefwidth = w;
  colType = KTabListBox::TextColumn;
  ordmode = KTabListBox::Descending;
  ordtype = KTabListBox::NoOrder;
  parent = pa;
  inumber=0;
  vline=false;
  mbut=0L;
}


//-----------------------------------------------------------------------------
KTabListBoxColumn::~KTabListBoxColumn()
{
  if(mbut) delete mbut;
}

//-----------------------------------------------------------------------------
bool KTabListBoxColumn::changeMode()
{
  if(inumber>0)
  {
    if(ordmode==KTabListBox::Ascending)
      ordmode=KTabListBox::Descending;
    else
      ordmode=KTabListBox::Ascending;
    return true;
  } else if(ordtype==KTabListBox::SimpleOrder)
  {
    parent->clearAllNum();
    inumber=1;
    return true;
  }
  return false;
}


//-----------------------------------------------------------------------------
void KTabListBoxColumn::setNumber(int num)
{
   bool changed=false;
   if((inumber==0 && num!=0) || (inumber!=0 && num==0) ) changed=true;
   if(num>9) num=0;
   inumber=num;
   char tmp[2];
   tmp[0]=48+inumber; tmp[1]=0;
   if(!inumber) tmp[0]=' ';
   if(mbut) mbut->setText(tmp);
   if(changed) parent->reorderRows();
}


//-----------------------------------------------------------------------------
void KTabListBoxColumn::setOrder(KTabListBox::OrderType ort,
				     KTabListBox::OrderMode mode)
{
  ordtype = ort;
  ordmode = mode;
  if(ordtype==KTabListBox::ComplexOrder && !mbut)
  {
    mbut = new KNumCheckButton(parent);
    connect(mbut,SIGNAL(selected() ),this,SLOT(setButton() ) );
    connect(mbut,SIGNAL(deselected() ),this,SLOT(resetButton() ) );
    connect(mbut,SIGNAL(doubleclick(bool)),this,SLOT(clearAll(bool) ) );
  }
}


//-----------------------------------------------------------------------------
void KTabListBoxColumn::paintCell(QPainter* paint, int row,
				  const QString& string, bool marked)
{
  QFontMetrics fm = paint->fontMetrics();
  QPixmap* pix = 0L;
  int beg, end, x;
  QPen pen, oldPen;

  if (marked)
  {
    paint->fillRect(0, 0, iwidth, parent->cellHeight(row),
		     parent->highlightColor);
    pen.setColor(kapp->selectTextColor);
    oldPen = paint->pen();
    paint->setPen(pen);
  }

  if (!string.isEmpty())
    switch(colType)
  {
  case KTabListBox::PixmapColumn:
    if (string) pix = parent->dict().find(string);
    if (pix && !pix->isNull())
    {
      paint->drawPixmap(0, 0, *pix);
      break;
    }
    /*else output as string*/

  case KTabListBox::TextColumn:
    paint->drawText(1, fm.ascent() +(fm.leading()),
		    (const char*)string);
    break;

  case KTabListBox::MixedColumn:
    QString pixName;
    for (x=0, beg=0; string[beg] == '\t'; x+=parent->tabPixels, beg++)
      ;
    end = beg-1;

    while(string[beg] == '{')
    {
      end = string.find('}', beg+1);
      if (end >= 0)
      {
	pixName = string.mid(beg+1, end-beg-1);
	pix = parent->dict().find(pixName);
	if (!pix)
	{
	  debug("KTabListBox "+QString(name())+
		":\nno pixmap for\n`"+pixName+"' registered.");
	}
	if (!pix->isNull()) paint->drawPixmap(x, 0, *pix);
	x += pix->width()+1;
	beg = end+1;
      }
      else
          break;
    }

    paint->drawText(x+1, fm.ascent() +(fm.leading()),
		    (const char*)string.mid(beg, string.length()-beg));
    break;
  }

  if (marked)
  {
    paint->fillRect(iwidth-6, 0, iwidth, 128, parent->highlightColor);
    paint->setPen(oldPen);
  }
  else
    paint->eraseRect(iwidth-6, 0, iwidth, 128);
}


//-----------------------------------------------------------------------------
void KTabListBoxColumn::paint(QPainter* paint)
{
  int t=3;
  QPoint a,b(0,0);
  QFontMetrics fm = paint->fontMetrics();
  if(ordtype!=KTabListBox::NoOrder)
  {
    t+=15;
    if(inumber>0)
      if(ordmode==KTabListBox::Ascending)
         paint->drawPixmap(-3, -2, parent->upPix);
      else
         paint->drawPixmap(-3, -1, parent->downPix);
    else
      if(ordmode==KTabListBox::Ascending)
         paint->drawPixmap(-3, -2, parent->disabledUpPix);
      else
         paint->drawPixmap(-3, -1, parent->disabledDownPix);

    if(ordtype==KTabListBox::ComplexOrder)
    {
      a=paint->xForm(b);
      mbut->move(a.x()+15,a.y()+2);
      if(!mbut->isVisible()) mbut->show();
      t+=parent->labelHeight-4;
    }
  }
  paint->drawText(t,(parent->labelHeight-4+fm.ascent())/2,(const char*)name());
}


//-----------------------------------------------------------------------------
void KTabListBoxColumn::setButton(void)
{
  if(inumber<=0)
  {
    parent->lastSelectedColumn++;
    inumber=parent->lastSelectedColumn;
    char tmp[2];
    tmp[0]=48+inumber; tmp[1]=0;
    if(mbut) mbut->setText(tmp);
    parent->reorderRows();
  }
}


//-----------------------------------------------------------------------------
void KTabListBoxColumn::resetButton(void)
{
  if(inumber>0)
  {
    if(parent->lastSelectedColumn==inumber)
    {
      parent->lastSelectedColumn--;
      inumber=0;
      if(mbut) mbut->setText(" ");
      if(parent->lastSelectedColumn)
        parent->repaint();
      else
        // This is for simulating a NoSort (useful for special cases like
        // the trash folder of kmail?)
        parent->reorderRows();
    }
    else
    {
      parent->adjustNumber(inumber);
      inumber=0;
      if(mbut) mbut->setText(" ");
      parent->reorderRows();
    }
  }
}


//-----------------------------------------------------------------------------
void KTabListBoxColumn::clearAll(bool leftbutton)
{
  parent->clearAllNum();
  if(leftbutton) setButton();
}


//=============================================================================
//
//   C L A S S   KTabListBox
//
//=============================================================================
KTabListBox::KTabListBox(QWidget *parent, const char *name, int columns,
			 WFlags _f):
    KTabListBoxInherited(parent, name, _f), lbox(this)
{
  QFontMetrics fm = fontMetrics();
  QString f;
  QColorGroup g = colorGroup();

  initMetaObject();

  f = kapp->kde_datadir().copy();
  f += "/khtmlw/pics/khtmlw_dnd.xpm";
  dndDefaultPixmap.load(f.data());

  f = kapp->kde_toolbardir().copy();
  f += "/up.xpm";
  upPix.load(f.data());
  f = kapp->kde_toolbardir().copy();
  f += "/down.xpm";
  downPix.load(f.data());

  QPalette pal = palette();
  QColorGroup g1 = pal.disabled();
  // Prepare the disabledPixmap for drawing
  // Maybe we can semplify it a bit, some suggestions??

  disabledUpPix.resize(upPix.width(), upPix.height());
  disabledDownPix.resize(downPix.width(), downPix.height());

  disabledUpPix.fill( g1.background() );
  disabledDownPix.fill( g1.background() );

  const QBitmap *mask = upPix.mask();
  const QBitmap *mask1= downPix.mask();

  bool allocated = false;
  if (!mask) {// This shouldn't occur anymore!
    mask = new QBitmap(upPix.createHeuristicMask());
    allocated = true;
  }
  bool allocated1 = false;
  if (!mask1) {// This shouldn't occur anymore!
    mask1 = new QBitmap(downPix.createHeuristicMask());
    allocated1 = true;
  }

  QBitmap bitmap = *mask; // YES! make a DEEP copy before setting the mask!
  QBitmap bitmap1 = *mask1;
  bitmap.setMask(*mask);
  bitmap1.setMask(*mask1);

  QPainter p;
  p.begin( &disabledUpPix );
  p.setPen( g1.light() );
  p.drawPixmap(1, 1, bitmap);
  p.setPen( g1.mid() );
  p.drawPixmap(0, 0, bitmap);
  p.end();
  QPainter p1;
  p1.begin( &disabledDownPix );
  p1.setPen( g1.light() );
  p1.drawPixmap(1, 1, bitmap1);
  p1.setPen( g1.mid() );
  p1.drawPixmap(0, 0, bitmap1);
  p1.end();


  if (allocated) // This shouldn't occur anymore!
    delete mask;
  if(allocated1)
    delete mask1;

  lastSelectedColumn=0;
  tabPixels = 10;
  maxItems  = 0;
  nMarked=0;
  current   = -1;
  colList   = 0L;
  colShowList =0L;
  itemList  = 0L;
  itemShowList=0L;
  sepChar   = '\n';
  labelHeight = fm.height() + 4;
  columnPadding = fm.height() / 2;
  highlightColor = kapp->selectColor;
  mResizeCol = false;
  stopOrdering=false;
  needsSort=false;
  mSortCol   = -1;
  numColumns = columns;

  mMouseCol=-1;
  mMouseColLeft=0;
  mMouseColWidth=0;
  mMouseAction=false;
  mMouseDragColumn=false;
  setMouseTracking(TRUE);

  lbox.setGeometry(0, labelHeight, width(), height()-labelHeight);

  if (columns > 0) setNumCols(columns);
}


//-----------------------------------------------------------------------------
KTabListBox::~KTabListBox()
{
  int i;
  if (colList)
  {
    for (i=0; i<numColumns; i++)
    {
      delete colList[i];
      colList[i] = NULL;
    }
    delete[] colList;
    delete[] colShowList;
  }
  if (itemList)
  {
    for (i=0; i<maxItems; i++)
    {
      delete itemList[i];
      itemList[i] = NULL;
    }
    delete[] itemList;
    delete[] itemShowList;
  }
}


//-----------------------------------------------------------------------------
void KTabListBox::setNumRows(int aRows)
{
  lbox.setNumRows(aRows);
}


//-----------------------------------------------------------------------------
void KTabListBox::setTabWidth(int aTabWidth)
{
  tabPixels = aTabWidth;
}


//-----------------------------------------------------------------------------
void KTabListBox::setNumCols(int aCols)
{
  int i;
  if (colList)
  {
    for (i=0; i<numColumns; i++)
      delete colList[i];
    delete [] colList;
    delete [] colShowList;
  }
  if (itemList)
  {
    for (i=0; i<maxItems; i++)
      delete itemList[i];

    delete [] itemList;
  }
  if (itemShowList)
    delete [] itemShowList;

  colList  = 0L;
  colShowList=0L;
  itemList = 0L;
  itemShowList=0L;
  maxItems = 0;

  if (aCols < 0) aCols = 0;
  lbox.setNumCols(aCols);
  numColumns = aCols;
  if (aCols <= 0) return;

  colList  = new KTabListBoxColumnPtr[aCols];
  colShowList = new int[aCols];
  for (i=0; i<aCols; i++)
  {
    colList[i] = newKTabListBoxColumn();
    colShowList[i] = i;
  }
  itemList = new KTabListBoxItemPtr[INIT_MAX_ITEMS];
  itemShowList = new int[INIT_MAX_ITEMS];
  for (i=0; i<INIT_MAX_ITEMS; i++)
  {
    itemList[i] = new KTabListBoxItem(aCols);
    itemShowList[i]=i;
  }
  maxItems = INIT_MAX_ITEMS;
}


//-----------------------------------------------------------------------------
KTabListBoxColumn* KTabListBox::newKTabListBoxColumn(void)
{
  return (new KTabListBoxColumn(this));
}


//-----------------------------------------------------------------------------
void KTabListBox::readConfig(void)
{
  KConfig* conf = KApplication::getKApplication()->getConfig();
  int beg, end, i, w;
  int cols = numCols(),fixedmin=MINIMUM_SPACE;
  QString str, substr;

  conf->setGroup(name());
  str = conf->readEntry("colwidth");

  if (!str.isEmpty())
    for (i=0, beg=0, end=0; i<cols;)
  {
    end = str.find(',', beg);
    if (end < 0) break;
    w = str.mid(beg,end-beg).toInt();
    if (colList[i]->orderType()==ComplexOrder)
       fixedmin+=ARROW_SPACE+labelHeight-BUTTON_SPACE;
    else
       if (colList[i]->orderType()==SimpleOrder)
          fixedmin+=ARROW_SPACE;
    if (w < fixedmin) w = fixedmin;
    colList[i]->setWidth(w);
    colList[i]->setDefaultWidth(w);
    i++;
    beg = end+1;
  }
  else
   for(i=0;i<cols;i++)
   {
     colList[i]->setWidth(60);
     colList[i]->setDefaultWidth(60);
   }
}


//-----------------------------------------------------------------------------
void KTabListBox::writeConfig(void)
{
  KConfig* conf = KApplication::getKApplication()->getConfig();
  int t;
  char str[200]="";

  conf->setGroup(name());
  for(t=0;t<numCols();t++) sprintf(str,"%s%d,",str,colList[t]->defaultWidth());
  conf->writeEntry("colwidth",str);
  conf->sync();
}


//-----------------------------------------------------------------------------
void KTabListBox::setDefaultColumnWidth(int aWidth, ...)
{
  va_list ap;
  int i, cols;

  cols = numColumns;
  va_start(ap, aWidth);
  for (i=0; aWidth && i<cols; i++)
  {
    colList[i]->setDefaultWidth(aWidth);
    aWidth = va_arg(ap, int);
  }
  va_end(ap);
}


//-----------------------------------------------------------------------------
void KTabListBox::setColumnWidth(int col, int aWidth)
{
  if (col<0 || col>=numCols()) return;
  colList[col]->setWidth(aWidth);
  colList[col]->setDefaultWidth(aWidth);
  lbox.updateTableSize();
}


//-----------------------------------------------------------------------------
void KTabListBox::reorderRows()
{
  int t=1,t1=0,n,maxVal,iColNum[10],c1;
  int result,itmp,totRows=numRows();
  bool change;
  // Who need more than 9 columns of priority for sorting??
  KTabListBoxColumn *pc[10];
  int (*columnSort)(const char *, const char *);
  OrderMode fmode;

  needsSort=true;
  if(!lbox.autoUpdate()) return;
  needsSort=false;

  for(n=0;n<10;n++) { pc[n]=0L; iColNum[n]=0;}
  while(t<10)
  {
    if(colList[t1]->number()==t)
    {
      iColNum[t-1]=t1;
      pc[t]=colList[t1];
      t1=0;t++;
    }
    else t1++;
    if(t1==numColumns) break;
  }
  maxVal=t-1;
  // if none column is selected no sort.
  if(!maxVal)
  {
    for(t=0;t<totRows;t++) itemShowList[t]=t;
    repaint();
    return;
  }

  columnSort=pc[1]->columnSort; c1=iColNum[0];
  fmode=pc[1]->orderMode();

  change=false;
  for(n=0;n<totRows-1;n++)
  {
      result=columnSort(
                     itemList[itemShowList[n]]->text(c1).data(),
                     itemList[itemShowList[n+1]]->text(c1).data() );
      if(result==0)
      {
        if(maxVal>=2 && recursiveSort(2,n,pc,iColNum))
        {
              itmp=itemShowList[n];
              itemShowList[n]=itemShowList[n+1];
              itemShowList[n+1]=itmp;
              change=true;
              if(n>0) n-=2;
        }
      }
      else if((result<0 && fmode==Ascending) ||
                (result>0 && fmode==Descending) )
      {
          itmp=itemShowList[n];
          itemShowList[n]=itemShowList[n+1];
          itemShowList[n+1]=itmp;
          change=true;
          if(n>0) n-=2;
      }
  }
  //We need a repaint also if change==false!!! We must redraw the arrows!
  if(change) repaint();
  else QWidget::repaint();
}


//-----------------------------------------------------------------------------
bool KTabListBox::recursiveSort(int level,int n,
                                KTabListBoxColumn **pc,int *iCol)
{
  int result;

  result=pc[level]->columnSort(
           itemList[itemShowList[n]]->text(iCol[level-1]).data(),
           itemList[itemShowList[n+1]]->text(iCol[level-1]).data() );
  if(result==0)
    if(pc[level+1] && recursiveSort(level+1,n,pc,iCol))
       return true;
    else return false;
  else
    if(  (result<0 && pc[level]->orderMode()==Ascending) ||
         (result>0 && pc[level]->orderMode()==Descending) )
      return true;
    else
      return false;
}


//-----------------------------------------------------------------------------
void KTabListBox::setColumn(int col, const char* aName, int aWidth,
			  ColumnType aType, OrderType aOType,
			  OrderMode aOMode,bool verticalLine,
			  int (*compar)(const char *, const char *))
{
  if (col<0 || col>=numCols()) return;

  setColumnWidth(col,aWidth);
  colList[col]->setName(aName);
  colList[col]->setType(aType);
  colList[col]->setOrder(aOType,aOMode);
  colList[col]->vline=verticalLine;
  if(compar)
    colList[col]->columnSort=compar;
  else
    colList[col]->columnSort=strcmp;
  update();
}


//-----------------------------------------------------------------------------
void KTabListBox::setCurrentItem(int idx, int colId)
{
  int i;

  if (idx>=numRows()) return;

  unmarkAll();

  
  if ( idx <= topItem() && idx < lbox.lastRowVisible() )
      lbox.setTopCell( itemPosList(idx));
  else if ( idx >= lbox.lastRowVisible() )
      {
	  int i = itemPosList(idx)+1;
	  int y = 0;
	  while (i) y += cellHeight(--i);
	  y -= lbox.viewHeight();
	  lbox.setYOffset( y );
      }
  
  if (idx != current)
  {
    i = current;
    current = idx;

    updateItem(i,FALSE);
  }

  if (current >= 0)
  {
    markItem(idx);
    emit highlighted(current, colId);
  }
}


//-----------------------------------------------------------------------------
// This is an internal method...
void KTabListBox::setCItem(int idx)
{
  int i;

  if (idx >= numRows() || idx < 0) return;

  if (idx != current)
  {
    unmarkItem(current);
    i = current;
    current = idx;
    updateItem(i,FALSE);
  }
  unmarkAll();
  markItem(idx);
}


//-----------------------------------------------------------------------------
bool KTabListBox::isMarked(int idx) const
{
  return (itemList[idx]->marked() >= -1);
}


//-----------------------------------------------------------------------------
void KTabListBox::markItem(int idx, int colId)
{
  if (itemList[idx]->marked()==colId) return;
  itemList[idx]->setMarked(colId);
  nMarked++;
  updateItem(idx,FALSE);
}


//-----------------------------------------------------------------------------
void KTabListBox::unmarkItem(int idx)
{
  int mark;

  mark = itemList[idx]->marked();
  itemList[idx]->setMarked(-2);
  if (mark>=-1)
  {
    nMarked--;
    updateItem(idx);
  }
}


//-----------------------------------------------------------------------------
void KTabListBox::unmarkAll(void)
{
  int i;

  if(!nMarked) return;
  for (i=numRows()-1; i>=0; i--)
    unmarkItem(i);
  nMarked=0;
}


//-----------------------------------------------------------------------------
const QString& KTabListBox::text(int row, int col) const
{
  const KTabListBoxItem* item = getItem(row);
  static QString str;
  int i, cols;

  if (!item)
  {
    str = 0L;
    return str;
  }
  if (col >= 0) return item->text(col);

  cols = item->columns - 1;
  for (str="",i=0; i<=cols; i++)
  {
    str += item->text(i);
    if (i<cols) str += sepChar;
  }

  return str;
}


//-----------------------------------------------------------------------------
void KTabListBox::insertItem(const char* aStr, int row)
{
  KTabListBoxItemPtr it;
  int i, newSize;

  if (row < 0) row = numRows();
  if (numRows() >= maxItems)
  {
    newSize = (maxItems << 1);
    if (newSize <= row) newSize = row+2;
    resizeList(newSize);
  }

  it = itemList[numRows()];

  if(row!=numRows())
  {
    for (i=numRows()-1; i>=row; i--)
      itemList[i+1] = itemList[i];
    for(i=0;i<numRows();i++) if(itemShowList[i]>=row) itemShowList[i]++;
  }
  itemList[row] = it;
  itemShowList[row]=row;

  if (itemPosList(current) >itemPosList(row))
    current=itemShowList[itemPosList(current+1)];

  setNumRows(numRows()+1);
  changeItem(aStr, row);

  reorderRows();
  if (needsUpdate(row))
      lbox.repaint();
}

//-----------------------------------------------------------------------------
void KTabListBox::appendStrList( QStrList const *strLst )
{
  bool update;
  uint i;

  if (!strLst) return;
  QStrListIterator it(*strLst);

  update = autoUpdate();
  setAutoUpdate(FALSE);
  for (i=0; i<strLst->count(); i++)
  {
    insertItem(it.current());
    ++it;
  }
  setAutoUpdate(update);
  lbox.repaint();
}

//-----------------------------------------------------------------------------
void KTabListBox::changeItem(const char* aStr, int row)
{
  QString str;
  char  sepStr[2];
  char* pos;
  int   i;
  KTabListBoxItem* item;

  if (row < 0 || row >= numRows()) return;

  str = aStr;
  str.detach();

  sepStr[0] = sepChar;
  sepStr[1] = '\0';

  item = itemList[row];
  pos = strtok(str.data(), sepStr);
  for (i=0; pos && *pos && i<numCols(); i++)
  {
    item->setText(i, pos);
    pos = strtok(0L, sepStr);
  }
  item->setForeground(black);

  if (needsUpdate(row)) lbox.repaint();
}


//-----------------------------------------------------------------------------
void KTabListBox::changeItemPart(const char* aStr, int row, int col)
{
  if (row < 0 || row >= numRows()) return;
  if (col < 0 || col >= numCols()) return;

  itemList[row]->setText(col, aStr);
  if (needsUpdate(row)) lbox.repaint();
}


//-----------------------------------------------------------------------------
void KTabListBox::changeItemColor(const QColor& newColor, int row)
{
  if (row >= numRows()) return;
  if (row < 0) row = numRows()-1;

  itemList[row]->setForeground(newColor);
  if (needsUpdate(row)) lbox.repaint();
}


//-----------------------------------------------------------------------------
void KTabListBox::removeItem(int row)
{
  KTabListBoxItemPtr it;
  int i, nr;
  bool upd;

  if (row < 0 || row >= numRows()) return;
  upd = needsUpdate(row);
  if (itemPosList(current) >itemPosList(row))
    current=itemShowList[itemPosList(current-1)];

  nr = numRows()-1;
  it = itemList[row];
  for (i=row; i<nr; i++)
    itemList[i] = itemList[i+1];
  for(i=itemPosList(row); i<nr;i++)
    itemShowList[i]=itemShowList[i+1];
  for(i=0;i<nr;i++) if(itemShowList[i]>row) itemShowList[i]--;
  itemList[nr] = it;

  setNumRows(nr);
  if (nr==0) current = -1;
  if (upd) lbox.repaint();
}


//-----------------------------------------------------------------------------
void KTabListBox::updateItem(int row, bool erase)
{
  int i;

  for (i=numCols()-1; i>=0; i--)
    lbox.updateCell(itemPosList(row), i, erase);
}


//-----------------------------------------------------------------------------
bool KTabListBox::needsUpdate(int id)
{
return (lbox.autoUpdate() && itemVisible(itemPosList(id)));
}


//-----------------------------------------------------------------------------
void KTabListBox::changeMode(int col)
{
  // Add optimization HERE!!!!
  if(col >= lbox.numCols() || col < 0)
    return;
  if(colList[col]->changeMode())
    reorderRows();
}


//-----------------------------------------------------------------------------
void KTabListBox::adjustNumber(int num)
{
  int t;
  for(t=0;t<numColumns;t++)
    if(colList[t]->number()>num)
      colList[t]->setNumber(colList[t]->number()-1);
  lastSelectedColumn--;
}


//-----------------------------------------------------------------------------
int KTabListBox::colPosList(int cl)
{
  int t;
  if (cl<0) return cl;
  for(t=0;t<numColumns;t++) if(cl==colShowList[t]) break;
  return t;
}


//-----------------------------------------------------------------------------
int KTabListBox::itemPosList(int it)
{
  int t;
  for(t=0;t<maxItems;t++) if(it==itemShowList[t]) break;
  return t;
}



//-----------------------------------------------------------------------------
void KTabListBox::clearAllNum()
{
  int t;
  for(t=0;t<numColumns;t++)
    colList[t]->setNumber(0);
  lastSelectedColumn=0;
}


//-----------------------------------------------------------------------------
void KTabListBox::clear(void)
{
  int i;

  for (i=maxItems-1; i>=0; i--)
  {
    if (itemList[i])
      itemList[i]->setMarked(-2);
  }

  setNumRows(0);
  lbox.setTopLeftCell(0,0);
  current = -1;
}


//-----------------------------------------------------------------------------
void KTabListBox::repaint()
{
  if(needsSort) reorderRows();
  QWidget::repaint();
  lbox.repaint();
}


//-----------------------------------------------------------------------------
void KTabListBox::resizeList(int newNumItems)
{
  KTabListBoxItemPtr* newItemList;
  int *newItemShowList;
  int i, ih, nc, oldNum;

  if (newNumItems < 0) newNumItems = (maxItems << 1);
  if (newNumItems < INIT_MAX_ITEMS) newNumItems = INIT_MAX_ITEMS;

  nc = numCols();
  oldNum = numRows();
  newItemList = new KTabListBoxItemPtr[newNumItems];
  newItemShowList = new int[newNumItems];
  ih = newNumItems < oldNum ? newNumItems : oldNum;
  for (i = ih-1; i>=0; i--)
  {
    newItemList[i] = itemList[i];
    newItemShowList[i] = itemShowList[i];
  }
  if (newNumItems > oldNum)
  {
    for (i = oldNum; i < newNumItems; i++) {
      newItemList[i] = new KTabListBoxItem(nc);
      newItemShowList[i] = 0;
    }
  }
  else
  {
    for (i = newNumItems; i < oldNum; i++)
      delete itemList[i];
  }

  if (itemList) delete [] itemList;
  if (itemShowList) delete [] itemShowList;
  itemList = newItemList;
  itemShowList = newItemShowList;
  maxItems = newNumItems;

  setNumRows(ih);
}


//-----------------------------------------------------------------------------
// This should really go into kdecore. Currently it's used by khtml and this.
// It prevents the tablist to continue to scroll after the user has released
// the key.
void KTabListBox::flushKeys()
{
    XEvent ev_return;
    Display *dpy = qt_xdisplay();
    while ( XCheckTypedEvent( dpy, KeyPress, &ev_return ) );
}


//-----------------------------------------------------------------------------
void KTabListBox::keyPressEvent( QKeyEvent *e )
{

    QScrollBar *sY=(QScrollBar*)lbox.verticalScrollBar();
    QScrollBar *sX=(QScrollBar*)lbox.horizontalScrollBar();
    bool thereIsX=sX->isVisible();
    //int pageJump = lbox.lastRowVisible()-lbox.topCell();
    int t;

    switch ( e->key() ) {
    case Key_Left:
        if(thereIsX) sX->setValue( sX->value() - sX->lineStep() );
        flushKeys();
        break;
    case Key_Right:
        if(thereIsX) sX->setValue( sX->value() + sX->lineStep() );
        flushKeys();
        break;
    case Key_Up:
        if(current==topCell())
          sY->setValue( sY->value() - sY->lineStep() );
        setCItem(itemShowList[itemPosList(current)-1]);
        flushKeys();
        break;
    case Key_Down:
        if(current==lastRowVisible())
          sY->setValue( sY->value() + sY->lineStep() );
        if((t=itemPosList(current)+1)<numRows())
          setCItem(itemShowList[t]);
        flushKeys();
        break;
    case Key_PageUp:
        sY->setValue( sY->value() - sY->pageStep() );
        t=lbox.lastRowVisible()-1;
        if(lbox.topCell()==0) t=0;
        /*if((t=itemPosList(current)-pageJump)>0)
          setCItem(itemShowList[t]);
        else
          setCItem(itemShowList[0]);*/
        setCItem(itemShowList[t]);
        flushKeys();
        break;
    case Key_PageDown:
        sY->setValue( sY->value() + sY->pageStep() );
        t=lbox.topCell()+1;
        if(lbox.lastRowVisible()==numRows()-1) t=numRows()-1;
        /*if((t=itemPosList(current)+pageJump+1)<numRows())
          setCItem(itemShowList[t]);
        else
          setCItem(itemShowList[numRows()-1]);*/
        setCItem(itemShowList[t]);
        flushKeys();
        break;
    case Key_Home:
        sY->setValue( sY->minValue() );
        setCItem(itemShowList[0]);
        flushKeys();
        break;
    case Key_End:
        sY->setValue( sY->maxValue() );
        setCItem(itemShowList[numRows()-1]);
        flushKeys();
        break;
    case Key_Enter:
    case Key_Return:
        emit selected(current, -1);
        break;
    case Key_Space:
        emit highlighted(current, -1);
        break;
    default:
        e->ignore();
        break;
    }
}


//-----------------------------------------------------------------------------
void KTabListBox::resizeEvent(QResizeEvent* e)
{
  int i, w,t;

  KTabListBoxInherited::resizeEvent(e);

  for (i=0, w=0; i<numCols(); i++)
  {
    colList[i]->setWidth(colList[i]->defaultWidth());
    w += colList[i]->defaultWidth();
  }
  t=lbox.width()-lbox.viewWidth();
  if (w < e->size().width()-t)
    colList[colShowList[numCols()-1]]->setWidth(
    e->size().width()-w-t+colList[colShowList[numCols()-1]]->defaultWidth() );

  lbox.setGeometry(0, labelHeight, e->size().width(),
		    e->size().height()-labelHeight);
  lbox.reconnectSBSignals();
  repaint();
}


//-----------------------------------------------------------------------------
void KTabListBox::paintEvent(QPaintEvent*)
{
  int i, ih, x, w;
  QPainter paint;
  QWMatrix matrix;
  QRect    clipR;
  QFont    font, oldFont;

  ih = numCols();
  x  = -lbox.xOffset();
  matrix.translate(x, 0);

  paint.begin(this);
  for (i=0; i<ih; i++)
  {
    w = colList[colShowList[i]]->width();

    if (w + x >= 0)
    {
      clipR.setRect(x, 0, w, labelHeight);
      paint.setWorldMatrix(matrix, FALSE);
      paint.setClipRect(clipR);
      colList[colShowList[i]]->paint(&paint);
      qDrawShadeLine(&paint, w, 0, w, labelHeight,
                     KTabListBoxInherited::colorGroup());
    }
    else colList[colShowList[i]]->hideCheckButton();
    matrix.translate(w, 0);
    x += w;
  }
  paint.resetXForm();
  paint.setClipping (false);
  if ( style() == MotifStyle )
    qDrawShadePanel(&paint, 0, 0, width(), height(),
                    KTabListBoxInherited::colorGroup(), false, 1);
  else
    qDrawShadeRect (&paint, 0, 0, width(), height(),
                    KTabListBoxInherited::colorGroup(), true, 1);

  paint.end();
}


//-----------------------------------------------------------------------------
void KTabListBox::mouseMoveEvent(QMouseEvent* e)
{
  register int i, x, ex, ey;
  bool mayResize = FALSE;

  ex = e->pos().x();
  ey = e->pos().y();

  if ((e->state() & LeftButton))
  {
    if (!drag)
    {
      if(mMouseDragColumn) doMouseMoveCol(e);

      if (mResizeCol && abs(mMouseStart.x() - ex) > 4)
	doMouseResizeCol(e);

      else if (!mResizeCol &&
	       (ex < mMouseColLeft ||
		ex > (mMouseColLeft+mMouseColWidth)))
      {
        mMouseDragColumn=true;
        doMouseMoveCol(e);
      }
    }

    KTabListBoxInherited::mouseMoveEvent(e);
    return;
  }

  for (i=1, x=colList[colShowList[0]]->width()-lbox.xOffset(); ; i++)
  {
    if (ex >= x-4 && ex <= x+4)
    {
      mayResize = TRUE;
      mMouseCol   = colShowList[i-1];
      mMouseColLeft = ex;
      mMouseColWidth = lbox.cellWidth(i-1);
      break;
    }
    if (i >= numColumns) break;
    x += colList[colShowList[i]]->width();
  }

  if (mayResize)
  {
    if (!mResizeCol)
    {
      mResizeCol = TRUE;
      setCursor(sizeHorCursor);
    }
  }
  else
  {
    if (mResizeCol)
    {
      mResizeCol = FALSE;
      mMouseCol = -1;
      setCursor(arrowCursor);
    }
  }
}


//-----------------------------------------------------------------------------
void KTabListBox::mousePressEvent(QMouseEvent* e)
{
  if (!mResizeCol && e->button() == LeftButton)
  {
    mMouseStart = e->pos();
    mMouseCol = lbox.findRealCol(e->pos().x());
    if (mMouseCol < 0) return;
    mMouseColWidth = colList[mMouseCol]->width();
    mMouseColLeft = 0;
    int t;
    colPosList(mMouseCol);
    for(t=0;t<colPosList(mMouseCol);t++)
        mMouseColLeft+=colList[colShowList[t]]->width();
    mLastX=mMouseColLeft;

  }
  KTabListBoxInherited::mousePressEvent(e);
}


//-----------------------------------------------------------------------------
void KTabListBox::mouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() == LeftButton)
  {
    if (!mMouseAction)
    {
      // user wants to select the column rather than drag or move it
      if (mMouseCol >= 0)
      {
        emit headerClicked(mMouseCol);
        // changeMode knows if to do a repaint or not.
        changeMode(mMouseCol);
      }
    }
    else if(mMouseDragColumn)
    {
      // The user had dragged a column! We need to change the order?
      mMouseDragColumn=false;
      mMouseCol=colPosList(mMouseCol);
      int actualColumn=colPosList(lbox.findRealCol(e->pos().x()));
      if(actualColumn>=0 && actualColumn!=mMouseCol)
      {
        int tmp, t;
        tmp=colShowList[mMouseCol];

        if(actualColumn>mMouseCol)
          for(t=mMouseCol;t<actualColumn;t++)
              colShowList[t]=colShowList[t+1];
        else
          for(t=mMouseCol;t>=actualColumn;t--)
              colShowList[t]=colShowList[t-1];

        colShowList[actualColumn]=tmp;
      }
      repaint();
    }
    else if(mResizeCol) // The user had resized the column, make a resize:
      resize(width(),height());

    mMouseCol = -1;
    mMouseAction = FALSE;
  }
  KTabListBoxInherited::mouseReleaseEvent(e);
}


//-----------------------------------------------------------------------------
void KTabListBox::doMouseResizeCol(QMouseEvent* e)
{
  int w, x,fixedmin=MINIMUM_SPACE;

  if (!mMouseAction) mMouseAction = true;
  if (mMouseCol >= 0)
  {
    x=e->pos().x();
    w = mMouseColWidth + (x - mMouseColLeft);
    if (colList[mMouseCol]->orderType()==ComplexOrder)
       fixedmin+=ARROW_SPACE+labelHeight-BUTTON_SPACE;
    else
       if (colList[mMouseCol]->orderType()==SimpleOrder)
          fixedmin+=ARROW_SPACE;
    if (w < fixedmin) w = fixedmin;
    if(w!=colList[mMouseCol]->width())
    {
      colList[mMouseCol]->setDefaultWidth(w);
      colList[mMouseCol]->setWidth(w);
      repaint();
      if(x>width())
      {
        QScrollBar *sX=(QScrollBar*)lbox.horizontalScrollBar();
        if(sX->isVisible())
          sX->setValue( sX->value() +x -width() );
      }
    }
  }
}


//-----------------------------------------------------------------------------
void KTabListBox::doMouseMoveCol(QMouseEvent* e)
{
  int x,old;
  QPainter paint;

  if (!mMouseAction) mMouseAction = true;
  x = e->pos().x();

  if(x<0)
  {
    QScrollBar *sX=(QScrollBar*)lbox.horizontalScrollBar();
    if(sX->isVisible())
    {
      old=sX->value();
      sX->setValue( sX->value() + x );
      if(old!=sX->value()) mLastX=-9999;
    }
  }
  else if(x+mMouseColWidth>width())
  {
    QScrollBar *sX=(QScrollBar*)lbox.horizontalScrollBar();
    if(sX->isVisible())
    {
      old=sX->value();
      sX->setValue( sX->value() + x+mMouseColWidth-width() );
      if(old!=sX->value()) mLastX=-9999;
    }
  }
  paint.begin(this);
  if(mLastX!=-9999)
    paint.drawWinFocusRect(mLastX+lbox.xOffset(),0,mMouseColWidth,labelHeight);
  paint.drawWinFocusRect(x,0,mMouseColWidth,labelHeight);
  paint.end();
  mLastX=x-lbox.xOffset();
}


//-----------------------------------------------------------------------------
bool KTabListBox::startDrag(int aCol, int aRow, const QPoint& p)
{
  int       dx = -(dndDefaultPixmap.width() >> 1);
  int       dy = -(dndDefaultPixmap.height() >> 1);
  KDNDIcon* icon;
  int       size, type;
  char*	    data;

  if (!prepareForDrag(aCol,aRow, &data, &size, &type)) return FALSE;

  icon = new KDNDIcon(dndDefaultPixmap, p.x()+dx, p.y()+dy);

  KTabListBoxInherited::startDrag(icon, data, size, type, dx, dy);
  return TRUE;
}


//-----------------------------------------------------------------------------
bool KTabListBox::prepareForDrag(int /*aCol*/, int /*aRow*/, char** /*data*/,
				 int* /*size*/, int* /*type*/)
{
  return FALSE;
}


//-----------------------------------------------------------------------------
void KTabListBox::horSbValue(int)
{
    // This update generates two paintEvent, why?
    update();
}


//-----------------------------------------------------------------------------
void KTabListBox::horSbSlidingDone()
{
}



//=============================================================================
//
//   C L A S S   KTabListBoxTable
//
//=============================================================================

KTabListBoxTable::KTabListBoxTable(KTabListBox *parent):
  KTabListBoxTableInherited(parent)
{
  QFontMetrics fm = fontMetrics();

  initMetaObject();

  dragging = FALSE;

  dragCol = -1;
  dragRow = -1;
  selIdx  = -1;


  setTableFlags(Tbl_autoVScrollBar|Tbl_autoHScrollBar|Tbl_smoothVScrolling|
		 Tbl_clipCellPainting);

  switch(style())
  {
  case MotifStyle:
  case WindowsStyle:
    setBackgroundColor(colorGroup().base());
    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
    break;
  default:
    setLineWidth(1);
    setFrameStyle(QFrame::Panel|QFrame::Plain);
  }

  setCellWidth(0);
  setCellHeight(fm.lineSpacing() + 1);
  setNumRows(0);

  setCursor(arrowCursor);
  setMouseTracking(FALSE);

  setFocusPolicy(NoFocus);
  // You can enable it with enableKey();
  //setFocusPolicy(StrongFocus);
}


//-----------------------------------------------------------------------------
KTabListBoxTable::~KTabListBoxTable()
{
}

//----------------------------------------------------------------------------
void KTabListBoxTable::enableKey()
{
  setFocusPolicy(StrongFocus);
}

//-----------------------------------------------------------------------------
int KTabListBoxTable::findRealCol(int xPos)
{
 int column = -1;
 int xPos1;
 KTabListBox*owner =(KTabListBox*)parentWidget();
 int maxCol=owner->numColumns;

 if ( xPos >= minViewX() && xPos <= maxViewX() )
 {
   column = 0;
   xPos1=xPos+xOffset();
   int cumWidth= 2;
   while ( column < maxCol )
   {
     cumWidth += cellWidth( column );
     if ( xPos1 < cumWidth ) break;
     column++;
   }
 }
 if ( column >=maxCol  )  return -1;
 if(column==-1) return -1;
 return owner->colShowList[column];
}


//-----------------------------------------------------------------------------
void KTabListBoxTable::paintCell(QPainter* p, int row, int col)
{
  KTabListBox*     owner =(KTabListBox*)parentWidget();
  KTabListBoxItem* item  = owner->getItem(owner->itemShowList[row]);

  if (!item) return;
  p->setPen(item->foreground());
  int t=owner->colShowList[col];
  owner->colList[t]->paintCell(p, row, item->text(t),(item->marked()==-1));
}


//-----------------------------------------------------------------------------
int KTabListBoxTable::cellWidth(int col)
{
  KTabListBox* owner =(KTabListBox*)parentWidget();

  return(owner->colList ? owner->colList[owner->colShowList[col]]->width() : 0);
}


//-----------------------------------------------------------------------------
void KTabListBoxTable::mouseDoubleClickEvent(QMouseEvent* e)
{
  KTabListBox* owner =(KTabListBox*)parentWidget();
  int idx, colnr;

  if(findRow(e->pos().y())<0) return;
  idx = owner->currentItem();
  colnr = findRealCol(e->pos().x());
  if (idx >= 0) emit owner->selected(idx,colnr);
}


//-----------------------------------------------------------------------------
void KTabListBoxTable::doItemSelection(QMouseEvent* e, int idx)
{
  KTabListBox* owner =(KTabListBox*)parentWidget();
  int i, di, colnr;

  owner->unmarkAll();
  if ((e->state()&ShiftButton)!=0 && owner->currentItem()>=0)
  {
    i  = owner->itemPosList(owner->currentItem());
    idx= owner->itemPosList(idx);
    di =(i>idx ? -1 : 1);
    while(1)
    {
      owner->markItem(owner->itemShowList[i]);
      if (i == idx) break;
      i += di;
    }
  }
  else
  {
    colnr = findRealCol(e->pos().x());
    owner->setCurrentItem(idx,colnr);
  }
}


//-----------------------------------------------------------------------------
void KTabListBoxTable::mousePressEvent(QMouseEvent* e)
{
  KTabListBox* owner =(KTabListBox*)parentWidget();
  int row, col;
  row=findRow(e->pos().y());
  if(row<0) return;
  row = owner->itemShowList[row];
  col = findRealCol(e->pos().x());

  if (e->button() == RightButton)
  {
    // handle popup menu
    if (row >= 0 && col >= 0)
      emit owner->popupMenu(row, col);
    return;
  }
  else if (e->button() == MidButton)
  {
    // handle middle click
    if (row >= 0 && col >= 0)
      emit owner->midClick(row, col);
    return;
  }

  // arm for possible dragging
  dragStartPos = e->pos();
  dragCol = col;
  dragRow = row;

  // handle item highlighting
  if (row >= 0 && owner->getItem(row)->marked() < -1)
  {
    doItemSelection(e, row);
    selIdx = row;
  }
  else selIdx = -1;
}


//-----------------------------------------------------------------------------
void KTabListBoxTable::mouseReleaseEvent(QMouseEvent* e)
{
  KTabListBox* owner =(KTabListBox*)parentWidget();
  int idx;

  if (e->button() != LeftButton) return;

  if (dragging)
  {
    owner->mouseReleaseEvent(e);
    dragRow = dragCol = -1;
    dragging = FALSE;
  }
  else
  {
    idx = findRow(e->pos().y());
    if(idx<0) return;
    idx = owner->itemShowList[idx];
    if (idx >= 0 && selIdx < 0)
      doItemSelection(e, idx);
  }
}


//-----------------------------------------------------------------------------
void KTabListBoxTable::mouseMoveEvent(QMouseEvent* e)
{
  KTabListBox* owner =(KTabListBox*)parentWidget();

  if (dragging)
  {
    owner->mouseMoveEvent(e);
    return;
  }

  if ((e->state() &(RightButton|LeftButton|MidButton)) != 0)
  {
    if (dragRow >= 0 && dragCol >= 0 &&
	(abs(e->pos().x()-dragStartPos.x()) >= 5 ||
	 abs(e->pos().y()-dragStartPos.y()) >= 5))
    {
      // we have a liftoff !
      dragging = owner->startDrag(dragCol, dragRow, mapToGlobal(e->pos()));
      return;
    }
  }
}


//-----------------------------------------------------------------------------
void KTabListBoxTable::reconnectSBSignals(void)
{
  QScrollBar* hsb = (QScrollBar*)horizontalScrollBar();
  KTabListBox* owner =(KTabListBox*)parentWidget();

  if (!hsb) return;

  hsb->setTracking(TRUE);
  connect(hsb, SIGNAL(valueChanged(int)), owner, SLOT(horSbValue(int)));
  connect(hsb, SIGNAL(sliderReleased()), owner, SLOT(horSbSlidingDone()));
}


//-----------------------------------------------------------------------------
void KTabListBoxTable::focusInEvent(QFocusEvent*)
{
  // Just do nothing here to avoid the annoying flicker whick happens due
  // to a redraw() call per default.
}


//-----------------------------------------------------------------------------
void KTabListBoxTable::focusOutEvent(QFocusEvent*)
{
  // Just do nothing here to avoid the annoying flicker whick happens due
  // to a redraw() call per default.
}


//=============================================================================
//
//      C L A S S  KNumCheckButton
//
//=============================================================================
KNumCheckButton::KNumCheckButton( QWidget *_parent, const char *name )
    : QWidget(_parent, name)
{
  int t=fontMetrics().height();
  resize(t,t);
  setText(" ");
  raised = FALSE;
  setFocusPolicy( NoFocus );
}

void KNumCheckButton::setText( const char *text )
{
  btext=text;
  repaint();
}

void KNumCheckButton::enterEvent( QEvent* )
{
      raised = TRUE;
      repaint(FALSE);
}

void KNumCheckButton::leaveEvent( QEvent * )
{
  if( raised != FALSE )
    {
      raised = FALSE;
      repaint();
    }
}

void KNumCheckButton::mousePressEvent( QMouseEvent *e)
{
  if(e->button() == LeftButton)
    emit selected();
  else if(e->button() == RightButton)
    emit deselected();
}

void KNumCheckButton::mouseDoubleClickEvent (QMouseEvent *e)
{
  if(e->button() == LeftButton)
    emit doubleclick(true);
  else if(e->button() == RightButton)
    emit doubleclick(false);
}

void KNumCheckButton::paintEvent( QPaintEvent *event )
{
  QPainter painter;
  QFont font,oldfont;

  painter.begin( this );
  painter.setClipRect( event->rect() );

  if ( raised )
    {
      QBrush   brush( white );
      qDrawShadeRect( &painter, 0, 0, width(), height(), colorGroup(),
                      TRUE, 1,1, &brush );
    }
  else
      qDrawShadeRect( &painter, 0, 0, width(), height(), colorGroup(),
                      TRUE, 1,1, 0L );
  int tf = AlignCenter;
  oldfont=painter.font();
  font=oldfont;
  font.setPointSize(height()-7);
  painter.setFont(font);
  painter.drawText(0, 0, width(), height(), tf, btext);
  painter.setFont(oldfont);
  painter.end();
}

#include "ktablistbox.moc"
