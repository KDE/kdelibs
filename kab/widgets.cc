/* -*- C++ -*-
 * This file contains stuff that is in development for 
 * faster compilation. Usually it is nearly empty.
 * 
 * the KDE addressbook.
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana for debugging
 * $Revision$
 */

#include "stl_headers.h"
#include "StringListSelectSetDialog.h"
#include "StringListSelectAndReorderSet.h"
#include "kab.h"
#include <kprocess.h>
#include <qpainter.h>
#include <qclipboard.h>
#include <qpaintdevicemetrics.h> 
#include <qmessagebox.h>
#include "PrintDialog.h"
#include "debug.h"
#include "functions.h"
#include "businesscard.h"
#include <kfiledialog.h>
#include <kapp.h>


void AddressWidget::print()
{
  ID(bool GUARD=true);
  // ########################################################
  LG(GUARD, "AddressWidget::print: printing database.\n");
  QPrinter prt;
  int temp;
  string text;
  list<string> keys;
  StringListSAndRSetDialog dialog(this);
  PrintDialog printDialog(this);
  list<int> indizes; // selected fields
  list<int>::iterator pos;
  // ----- setup QPrinter object:
  if(noOfEntries()==0)
    {
      setStatus(i18n("No entries."));
      return;
    }
  if(!prt.setup(this)) 
    {
      emit(setStatus(i18n("Printing cancelled.")));
      return;
    }
  prt.setCreator("KDE Addressbook");
  prt.setDocName("address database overview");
  // ----- set dialog textes, abusing "keys":
  for(temp=0; temp<NoOfFields; temp++)
    {
      if(nameOfField(Fields[temp], text))
	{
	  keys.push_back(text);
	} else {
	  CHECK(false);
	}
    }
  dialog.setCaption(i18n("kab: Select columns for printing"));
  dialog.selector()->setValues(keys);
  keys.erase(keys.begin(), keys.end());
  // ----- query fields to print:
  if(!dialog.exec())
    {
      LG(GUARD, "AddressWidget::print: "
	 "could not query fields to print.\n");
      emit(setStatus(i18n("Rejected.")));
      return;
    }
  if(!dialog.selector()->getSelection(indizes))
    {
      emit(setStatus
	   (i18n("Nothing to print.")));
      return;
    }
  // ----- find selected keys:
  for(pos=indizes.begin(); 
      pos!=indizes.end(); pos++)
    {
      CHECK((*pos)<NoOfFields);
      keys.push_back(Fields[*pos]);
    }
  // ----- now configure the printing:
  printDialog.setHeadline(i18n("KDE addressbook overview"));
  printDialog.setRightFooter(i18n("Page <p>"));
  printDialog.setLeftFooter(i18n("KDE - the professionals choice."));
  if(!printDialog.exec())
    {
      LG(GUARD, "AddressWidget::print: "
	 "printing setup rejected\n");
      emit(setStatus(i18n("Rejected.")));
      return;
    }
  // ----- call the printing subroutines:
  if(!print(prt, keys, 
	    printDialog.getHeadline(),
	    printDialog.getLeftFooter(),
	    printDialog.getRightFooter()))
     {
       QMessageBox::information
	 (this, i18n("Error"),
	  i18n("Printing failed!"));
     }
       emit(setStatus
	    (i18n("Printing finished successfully.")));
       // ########################################################
       }

#include "widgets.moc"

  
