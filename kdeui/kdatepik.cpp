/* This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998, 1999 Mirko Sucker (mirko@kde.org)
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

/////////////////// KDatePicker widget class //////////////////////
//
// Copyright (C) 1997 Tim D. Gilman
//           (C) 1998, 1999 Mirko Sucker
// Original header from Tim:
// This version of the class is the first release.  Please
// send comments/suggestions/bug reports to <tdgilman@best.com>
//
// Written using Qt (http://www.troll.no) for the
// KDE project (http://www.kde.org)
//
//
// Use this class to make a date picker widget
// When a date is selected by the user, it emits a signal: 
//    dateSelected(QDate)
//
// Required header for use:
//    kdatepik.h
//
// Required files for compilation:
//
//    kdatepik.h kdatepik.cpp kdatetbl.h kdatetbl.cpp
// ------------------------------------------------------------

#include <qpushbutton.h>
#include <qlabel.h>
#include <qfont.h>
#include <qdatetime.h>

#include "kdatepik.h"
#include "kdatepik.h"

#include "kdatetbl.h"

// Mirko, Feb 25 1998:
extern "C" {
#include "arrow_left.xbm"
#include "arrow_right.xbm"
  // highstick: added May 13 1998
#include "arrow_up.xbm"
#include "arrow_down.xbm"
  // ^^^^^^^^^^^^^^^^^^^
	   }
#include <qbitmap.h>
#include <kapp.h>

// Mirko, March 17 1998:
const int KDatePicker::PreferredButtonSize=22;
const int KDatePicker::MinimumFrameAroundButtons=2;
const int KDatePicker::NoOfMonth=12;
// ----- taken from kapp.h:
#ifndef i18n
#define i18n(X) KApplication::getKApplication()->getLocale()->translate(X)
#endif
// ^^^^^^^^^^^^^^^^^^^


KDatePicker::KDatePicker(QWidget *parent, QDate dt, const char *name)
  : QFrame(parent,name),
    fontsize(10)
{
  // Mirko: added Feb 25 1998
  QBitmap left // a left arrow, 32 Bytes
    (arrow_left_width, arrow_left_height, 
     (const unsigned char*)arrow_left_bits, true);
  QBitmap right // a right arrow, 32 Bytes
    (arrow_right_width, arrow_right_height, 
     (const unsigned char*)arrow_right_bits, true);
  // highstick: added May 13 1998
  QBitmap up
    (arrow_up_width, arrow_up_height,
     (const unsigned char*)arrow_up_bits, true);
  QBitmap down
    (arrow_down_width, arrow_down_height,
     (const unsigned char*)arrow_down_bits, true);
  // Mirko: added March 17 1998
  /* I added the names literally to force that they are
   * included into the translation templates.
   * I did not make it static to allow different languages,
   * altough I do not know if this is possible inside the
   * same process with locale.
   */
  Month[0]=i18n("January"); Month[1]=i18n("February"); 
  Month[2]=i18n("March"); Month[3]=i18n("April"); 
  Month[4]=i18n("May"); Month[5]=i18n("June");
  Month[6]=i18n("July"); Month[7]=i18n("August"); 
  Month[8]=i18n("September"); Month[9]=i18n("October"); 
  Month[10]=i18n("November"); Month[11]=i18n("December");
  // ^^^^^^^^^^^^^^^^^^^^^^^^
  initMetaObject();
   
  QDate dNow = QDate::currentDate();
  QString sNow;
  sNow.sprintf("Today: %i/%i/%i", 
	       dNow.month(),dNow.day(),dNow.year());
   
  m_header = new QLabel(this);
  updateHeader(dt);
   
  m_tbl = new KDateTable(this, dt);
  m_footer = new QLabel((const char*)sNow, this);
  // Mirko: changed Feb 25 1998
  m_back = new QPushButton(this);
  m_back->setPixmap(left);
  m_forward = new QPushButton(this);
  m_forward->setPixmap(right);
  // highstick: added May 13 1998
  m_up = new QPushButton(this);
  m_up->setPixmap(up);
  m_down = new QPushButton(this);
  m_down->setPixmap(down);
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^

  switch(style()) {
  case WindowsStyle:
  case MotifStyle:
    setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
    break;
  default:
    setFrameStyle(QFrame::Panel | QFrame::Plain);
    setLineWidth(1);
  }

  sizeElements();

  connect(m_forward, SIGNAL(clicked()), m_tbl, SLOT(goForward()));
  connect(m_back, SIGNAL(clicked()), m_tbl, SLOT(goBackward()));   
  connect(m_tbl, SIGNAL(monthChanged(QDate)), SLOT(updateHeader(QDate)));
  connect(m_tbl, SIGNAL(dateSelected(QDate)), SIGNAL(dateSelected(QDate)));
  // highstick: added May 13 1998
  connect(m_down, SIGNAL(clicked()), m_tbl, SLOT(goDown()));
  connect(m_up, SIGNAL(clicked()), m_tbl, SLOT(goUp()));   
  connect(m_tbl, SIGNAL(yearChanged(QDate)), SLOT(updateHeader(QDate)));
  connect(m_tbl, SIGNAL(dateSelected(QDate)), SIGNAL(dateSelected(QDate)));
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 
  m_header->setAlignment(AlignCenter);
  m_footer->setAlignment(AlignCenter);
   
  m_footer->setBackgroundColor(white);
   
  QColorGroup old_cg = m_header->colorGroup();
  QColorGroup new_cg(old_cg.foreground(), 
		     darkBlue,   // blue background
		     old_cg.light(), old_cg.dark(), old_cg.mid(),
		     white,      // white text
		     old_cg.base());
  m_header->setPalette(QPalette(new_cg,new_cg,new_cg));
   
  m_footer->setFont(QFont("Arial", fontsize, QFont::Bold));
  m_header->setFont(QFont("Times", fontsize+2/*, QFont::Bold*/));   
}


void KDatePicker::resizeEvent(QResizeEvent *)
{
  sizeElements();
}

void KDatePicker::sizeElements()
{
  QRect rec = contentsRect();
   
  // table height
  int th = rec.height()/10;
   
  m_header->setGeometry(rec.x(),
			rec.y(),
			rec.width(),
			2*th);
  m_footer->setGeometry(rec.x(),
			rec.bottom()-(th-1),
			rec.width(),
			th);
  m_tbl->setGeometry(rec.x(),
		     rec.y()+2*th,
		     rec.width(),
		     rec.height()-3*th);
  /* Mirko: changed Feb 25 1998
   * - assumes that both bitmaps have equal height
   *   buttons are set to a max height and width of 22, 
   *   that is the height of the arrow 
   *   plus 3 pixels in x and y direction
   */
  int buttonSize;
  int spacing;
  const int roomLeft=2*th-2*MinimumFrameAroundButtons;
  if(roomLeft>PreferredButtonSize)
    {
      spacing=(2*th-PreferredButtonSize)/2;
      buttonSize=PreferredButtonSize;
    } else { // the widget is smaller -> ugly buttons
      spacing=2; 
      /* This way the buttons might get very small,
       * but it is the task of the programmer to 
       * take care of the size hint.
       */
      buttonSize=2*th-2*MinimumFrameAroundButtons;
    }
  m_back->setGeometry(rec.x()+2*spacing+buttonSize,
		      rec.y()+spacing,
		      buttonSize,
		      buttonSize);
  m_forward->setGeometry(rec.right()-2*spacing-2*buttonSize,
			 rec.y()+spacing,
			 buttonSize,
			 buttonSize);
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  // highstick: added May 13 1998
  m_up->setGeometry(rec.x() + spacing,
		    rec.y() + spacing,
		    buttonSize,
		    buttonSize);
  m_down->setGeometry(rec.right() - spacing - buttonSize,
		      rec.y() + spacing,
		      buttonSize,
		      buttonSize);
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

void KDatePicker::updateHeader(QDate dt)
{
  QString sHeader;
  const char* month;
  if(dt.month()>0 && dt.month()<13)
    {
      month=Month[dt.month()-1];
    } else {
      month=i18n("(invalid)");
    }
  CHECK_PTR(month);
  sHeader.sprintf("%s %i", 
		  month,
		  // (const char*)dt.monthName(dt.month()), 
		  dt.year());
  m_header->setText((const char*)sHeader);
}

/* Mirko: Implementation of sizeHint and setDate methods.
 * March 17 1998
 */

QSize KDatePicker::sizeHint() const 
{
  int count, temp, x, y;
  QSize table, header, footer;
  CHECK_PTR(m_tbl); CHECK_PTR(m_footer);
  // ----- find recommended size for month table:
  table=m_tbl->sizeHint();
  // ----- find recommended size for headline:
  header=m_header->sizeHint();
  header.setWidth(0);
  for(count=0; count<NoOfMonth; count++)
    { // find the width of the widest month name
      temp=m_header->fontMetrics().width(Month[count]);
      if(temp>header.width())
	{ //  ^^^ this depends on the font set!
	  header.setWidth(temp);
	}
    }
  header.setWidth(header.width()
		  +m_header->fontMetrics().width(" 2984 ")+10
		  +6*MinimumFrameAroundButtons
		  +4*PreferredButtonSize);
  // ----- find recommended size for bottom label:
  // let us assume that the footer label is smaller than 
  // header label
  footer=m_footer->sizeHint();
  // ----- construct size hint from values:
  x=table.width(); 
  if(x<header.width())
    { // footer skipped
      x=header.width();
    }
  y=header.height()+table.height()+footer.height();
  debug("KDatePicker::sizeHint: "
	"recommending %ix%i pixels.\n", x, y);
  // -----
  return QSize(x, y);
}

void KDatePicker::setDate(QDate date)
{
  if(date.isValid())
    {
      m_tbl->setDate(date);
      updateHeader(date);
    } else {
      debug("KDatePicker::setDate: "
	    "date is invalid, not set.\n");
    }
}

// end of new methods from March 17 1998

/* Mirko: Aug 19 1998 */
void KDatePicker::setFontSize(int size)
{
  if(size>0)
    {
      fontsize=size;
      m_tbl->setFontSize(size);
      setHeadlineSize(size+2);
      repaint();
    }
}

int KDatePicker::fontSize()
{
  return fontsize;
}

void KDatePicker::setHeadlineSize(int size)
{
  QFont font=m_header->font();
  if(size>0)
    {
      font.setPointSize(size);
      m_header->setFont(font);
      m_header->repaint();
    }
}

int KDatePicker::headlineSize()
{
  return m_header->fontInfo().pointSize();
}

// end of new methods from Aug 19 1998

/**********************************************************/
#include "kdatepik.moc"
