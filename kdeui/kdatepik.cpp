/* This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)

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
// This version of the class is the first release.  Please
// send comments/suggestions/bug reports to <tdgilman@best.com>
//
// Written using Qt (http://www.troll.no) for the
// KDE project (http://www.kde.org)
//
//
// Use this class to make a date picker widget
// When a date is selected by the user, it emits a signal: dateSelected(QDate)
//
// Required header for use:
//    kdatepik.h
//
// Required files for compilation:
//
//    kdatepik.h kdatepik.cpp kdatetbl.h kdatetbl.cpp


#include <qpushbt.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qfont.h>
#include <qdatetm.h>

#include "kdatepik.h"
#include "kdatepik.moc"

#include "kdatetbl.h"

// Mirko, Feb 25 1998:
extern "C" {
#include "arrow_left.xbm"
#include "arrow_right.xbm"
	   }
#include <qbitmap.h>
// ^^^^^^^^^^^^^^^^^^^

KDatePicker::KDatePicker(QWidget *parent, QDate dt, const char *name)
       :QFrame(parent,name)
{
  // Mirko: added Feb 25 1998
  QBitmap left // a left arrow, 32 Bytes
    (arrow_left_width, arrow_left_height, 
     (const unsigned char*)arrow_left_bits, true);
  QBitmap right // a right arrow, 32 Bytes
    (arrow_right_width, arrow_right_height, 
     (const unsigned char*)arrow_right_bits, true);
  // ^^^^^^^^^^^^^^^^^^^^^^^^
   initMetaObject();
   
   QDate dNow = QDate::currentDate();
   QString sNow;
   sNow.sprintf("Today: %i/%i/%i", dNow.month(),dNow.day(),dNow.year());
   
   m_header = new QLabel(this);
   updateHeader(dt);
   
   m_tbl = new KDateTable(this, dt);
   m_footer = new QLabel((const char*)sNow, this);
   // Mirko: changed Feb 25 1998
   m_back = new QPushButton(this);
   m_back->setPixmap(left);
   m_forward = new QPushButton(this);
   m_forward->setPixmap(right);
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
   
   m_header->setFont(QFont("Arial", 18, QFont::Bold));
   m_footer->setFont(QFont("Arial", 12, QFont::Bold));
   
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
    * - the consts should be optimized out by the 
    *   compiler
    */
   const int PreferredButtonSize=22;
   const int MinimumFrameAroundButtons=2;
   int buttonSize;
   int spacing;
   const int roomLeft=2*th-2*MinimumFrameAroundButtons;
   if(roomLeft>PreferredButtonSize)
     {
       spacing=(2*th-PreferredButtonSize)/2;
       buttonSize=PreferredButtonSize;
     } else { // the widget is smaller
       spacing=2; 
       buttonSize=2*th-2*MinimumFrameAroundButtons;
     }
   m_back->setGeometry(rec.x()+spacing,
		       rec.y()+spacing,
		       buttonSize,
		       buttonSize);
   m_forward->setGeometry(rec.right()-spacing-buttonSize,
			  rec.y()+spacing,
			  buttonSize,
			  buttonSize);
   // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

void KDatePicker::updateHeader(QDate dt)
{
   QString sHeader;
   sHeader.sprintf("%s %i", (const char*)dt.monthName(dt.month()), dt.year());
   
   m_header->setText((const char*)sHeader);
}
