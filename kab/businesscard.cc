/* -*- C++ -*-
 * This file implements the businesscard-like central display
 * of the selected address.
 * 
 * the KDE addressbook.
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana for debugging
 * $Revision$
 */

#include <string>
#include <qpainter.h>
#include <qmsgbox.h>
#include <qtooltip.h>
#include "businesscard.h"
extern "C" {
#include <stdio.h>
	   }
#include "debug.h"

const int BusinessCard::Grid=5;

BusinessCard::BusinessCard(QWidget* parent, const char* name)
  : QWidget(parent, name)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "BusinessCard constructor: creating object.\n");
  background=new QPixmap(310, 160); // width x height
  background->fill(); // white
  resize(310, 160);
  // ########################################################
}

BusinessCard::~BusinessCard()
{
  ID(bool GUARD=false);
  // ########################################################
  if(background!=0) delete background;
  LG(GUARD, "BusinessCard destructor: object removed.\n");
  // ########################################################
}
  
void BusinessCard::paintEvent(QPaintEvent*)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "BusinessCard::paintEvent: repainting.\n");
  QFont original;
  QFont font;
  QRect rect;
  bool drawSeparator=false;
  int posSeparator=0;
  string temp;
  int cy;
  QPixmap pm(width(), height());
  QPainter p;     
  // ----- 
  p.begin(&pm);
  if(background->isNull())
    {
      p.setPen(darkYellow);
      p.drawRect(0, 0, width(), height());
      p.setPen(black);
    } else {
      p.drawTiledPixmap(0, 0, width(), height(), *background);
    }
  original=p.font();
  font.setFamily(original.family());
  font.setPointSize(10);
  p.setFont(font);
  // print the birthday in the upper right corner 
  // if it is set:
  if(current.birthday.isValid())
    { // by now I do not take care if there is enough 
      // space left
      p.drawText
	(width()-Grid-
	 p.fontMetrics().width(current.birthday.toString()),
	 Grid+p.fontMetrics().ascent(),
	 current.birthday.toString());
    }
  // ------				
  cy=height()-Grid;
  if(!current.URL.empty())
    {
      temp=(string)"URL: "+current.URL;
      p.drawText(Grid, cy, temp.c_str());
      cy-=p.fontMetrics().height();
      drawSeparator=true;
      temp="";
    }
  if(!current.emails.empty())
    {
      temp=(string)"email: "+current.emails.front();
      p.drawText(Grid, cy, temp.c_str());
      cy-=p.fontMetrics().height();
      drawSeparator=true;
      temp="";
    }
  if(!current.telephone.empty())
    {
      temp+=(string)"tel: "+current.telephone;
    }
  if(!current.fax.empty())
    {
      if(!temp.empty()) temp+=", ";
      temp+=(string)"fax: "+current.fax;
    }
  if(!current.modem.empty())
    {
      if(!temp.empty()) temp+=", ";
      temp+=(string)"modem: "+current.modem;
    }
  if(!temp.empty())
    {
      p.drawText(Grid, cy, temp.c_str());
      cy-=p.fontMetrics().height();
      drawSeparator=true;
      temp="";
    }
  if(drawSeparator)
    {
      posSeparator=cy;
      cy-=Grid;
    }
  // now draw the comment:
  if(!current.comment.empty())
    {
      p.drawText(Grid, cy-2*p.fontMetrics().height(), 
		 width()-2*Grid, 
		 2*p.fontMetrics().height(),
		 AlignHCenter | AlignBottom | WordBreak,
		 current.comment.c_str(), -1, &rect);
      if(rect.height()<2*p.fontMetrics().height())
	{ // only one line needed for comment
	  LG(GUARD, "BusinessCard::paintEvent: "
	     "comment fits in 1 line only.\n");
	  cy-=p.fontMetrics().height()+Grid;
	} else {
	  LG(GUARD, "BusinessCard::paintEvent: "
	     "comment needs more than 1 line.\n");	  
	  cy-=2*p.fontMetrics().height()+Grid;
	}
    }
  // ------
  font.setPointSize(12);
  p.setFont(font);
  // a space
  if(current.comment.empty())
    {
      cy-=2*p.fontMetrics().height();
    } else {
      cy-=p.fontMetrics().height();
    }
  if(!current.town.empty())
    {
      p.drawText(2*Grid, cy, current.town.c_str());
      cy-=p.fontMetrics().height();
    }      
  if(!current.address.empty())
    {
      p.drawText(2*Grid, cy, current.address.c_str());
      cy-=p.fontMetrics().height();
    }        
  if(!current.role.empty())
    {
      p.drawText(2*Grid, cy, current.role.c_str());
      cy-=p.fontMetrics().height();
    }        
  font.setItalic(true);
  p.setFont(font);
  if(!current.fn.empty())
    {
      temp=current.fn;
    } else {
      if(!current.title.empty())
	{
	  if(!temp.empty()) temp+=" ";
	  temp+=current.title;
	}
      if(!current.namePrefix.empty())
	{
	  if(!temp.empty()) temp+=" ";
	  temp+=current.namePrefix;
	}
      if(!current.firstname.empty())
	{
	  if(!temp.empty()) temp+=" ";
	  temp+=current.firstname;
	}
      if(!current.additionalName.empty())
	{
	  if(!temp.empty()) temp+=" ";
	  temp+=current.additionalName;
	}
      if(!current.name.empty())
	{
	  if(!temp.empty()) temp+=" ";
	  temp+=current.name;
	}
    }
  p.setPen(blue);
  if(!temp.empty())
    {
      p.drawText(2*Grid, cy, temp.c_str());
      cy-=p.fontMetrics().height();
    }
  if(drawSeparator)
    {
      p.drawLine(Grid, posSeparator, 
		 width()-Grid, posSeparator);
    }
  p.end();
  bitBlt(this, 0, 0, &pm);
  // ########################################################
}

void BusinessCard::currentChanged(AddressBook::Entry entry)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "BusinessCard::currentChanged: "
     "updating and repainting.\n");
  current=entry;
  repaint(false);
  // ########################################################
}

void BusinessCard::setBackground(QPixmap* pixmap)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "BusinessCard::setBackground: "
     "background pixmap changed.\n");
  *background=*pixmap;
  /*
  if(pixmap->width()!=310 && pixmap->height()!=160)
    {
      QMessageBox::information
	(this, "Size error",
	 "The background pixmap must be 310x160 pixels.");
      return;
    } else {
    }
  */
  // ########################################################
}

void BusinessCard::setBackground(const string& path)
{
  ID(bool GUARD=false);
  // ########################################################
  QPixmap pixmap;
  if(!pixmap.load(path.c_str()))
    {
      QMessageBox::information
	(this, "Image load failure",
	 "Could not load background image!");
      return;
    }
  /*
    if(pixmap.width()!=310 || pixmap.height()!=160)
    {
    QMessageBox::information
    (this, "Image size failure",
    "The background image must be \n"
    "    310 pixels wide and \n"
    "    160 pixels high.");
    return;
    }
  */
  LG(GUARD, "BusinessCard::setBackground: loaded image from "
     "file \"%s\".\n", path.c_str());
  filename=path;
  setBackground(&pixmap);
  // ########################################################
}


#include "businesscard.moc"
