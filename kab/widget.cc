/* -*- C++ -*-
 * This file implements the "main view" widget for the 
 * addressbook
 * 
 * the KDE addressbook
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, Qt > 1.40
 *             NANA (for debugging)
 * $Revision$
 */

#include "stl_headers.h" 
#include <fnmatch.h>
#include <qprinter.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qbitmap.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qstring.h>
#include <qfiledialog.h>
#include <qpaintdevicemetrics.h> 
#include <qpainter.h>
#include <qprogressdialog.h>
#include <qclipboard.h>
#include <qtimer.h>
#include <kbutton.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include "kab.h"
#include "debug.h"
#include <kprocess.h>
#include "editentry.h"
#include "functions.h"
#include "SearchDialog.h"
#include "businesscard.h"
#include "searchresults.h"
#include "StringListSelectSetDialog.h"
#include "StringListSelectOneDialog.h"
#include "StringListSelectAndReorderSet.h"
#include "PrintDialog.h"
// the bitmaps
#include "arrow_left.xbm"
#include "arrow_right.xbm"
#include "dbarr_previous.xbm"
#include "dbarr_next.xbm"
#include "magnify2.xbm"
#include "trash_open.xbm"
#include "text.xbm"
#include "select.xbm"
#include <kapp.h>
#include <klocale.h>
#include <kstddirs.h>

const unsigned int AddressWidget::Grid=3;
const unsigned int AddressWidget::ButtonSize=24;

AddressWidget::AddressWidget(QWidget* parent,  const char* name, bool readonly_)
  : QWidget(parent, name),
    AddressBook(readonly_),
    showSearchResults(false)
{
  register bool GUARD; GUARD=true;
  // ############################################################################
  LG(GUARD, "AddressWidget constructor: creating object.\n");
  string bgFilename="background_1.jpg";
  string dlgBackground="dialog_background.jpg";
  string path;
  string lastCurrentKey;
  KeyValueMap* keys;
  // ----- 
  if(!readonly_ && isRO())
    { // if r/w requested, but file locked
      readonlyGUI=true;
    }
  // -----
  QBitmap first(16, 16, (unsigned char*)previous_xbm_bits, true);
  QBitmap previous(16, 16, (unsigned char*)arrow_left_bits, true);
  QBitmap next(16, 16, (unsigned char*)arrow_right_bits, true);
  QBitmap last(16, 16, (unsigned char*)next_xbm_bits, true);
  QBitmap search(16, 16, (unsigned char*)binoculars_bits, true);
  QBitmap erase(16, 16, (unsigned char*)trashcan_bits, true);
  QBitmap create_new(16, 16, (unsigned char*)textrect0_bits, true);
  QBitmap change_entry(16, 16, (unsigned char*)select_bits, true);
  // -----
  comboSelector=new QComboBox(FALSE, this); 
  CHECK(comboSelector!=0);
  frameSeparator1=new QFrame(this); 
  CHECK(frameSeparator1!=0);
  frameSeparator1->setFrameStyle(QFrame::HLine | QFrame::Raised);
  frameSeparator2=new QFrame(this); 
  CHECK(frameSeparator2!=0);
  frameSeparator2->setFrameStyle(QFrame::HLine | QFrame::Raised);
  buttonFirst=new KButton(this); 
  CHECK(buttonFirst!=0);
  buttonFirst->setPixmap(first);
  buttonPrevious=new KButton(this);
  CHECK(buttonPrevious!=0);
  buttonPrevious->setPixmap(previous);
  buttonNext=new KButton(this); 
  CHECK(buttonNext!=0);
  buttonNext->setPixmap(next);
  buttonLast=new KButton(this); 
  CHECK(buttonLast!=0);
  buttonLast->setPixmap(last);
  buttonAdd=new KButton(this); 
  CHECK(buttonAdd!=0);
  buttonAdd->setPixmap(create_new);
  buttonChange=new KButton(this); 
  CHECK(buttonChange!=0);
  buttonChange->setPixmap(change_entry);
  buttonRemove=new KButton(this); 
  CHECK(buttonRemove!=0);
  buttonRemove->setPixmap(erase);
  buttonSearch=new KButton(this);
  CHECK(buttonSearch!=0);
  buttonSearch->setPixmap(search);
  // -----
  card=new BusinessCard(this);
  connect(card, SIGNAL(mailURLActivated()), SLOT(mail()));
  connect(card, SIGNAL(homeURLActivated()), SLOT(browse()));
  // -----
  searchResults=new SearchResults(this);
  connect(searchResults, SIGNAL(closed()), SLOT(searchResultsClose()));
  connect(searchResults, SIGNAL(entrySelected(const char*)),
	  SLOT(selectEntry(const char*)));
  // -----
  setAcceptDrops(TRUE);
  initializeGeometry(); // sets fixed size
  createConnections(); 
  createTooltips();
  // AddressBook constructor has to ensure this:
  load(); CHECK(clear()); CHECK(load());
  if(!updateDB())
    {
      QMessageBox::information
	(this, i18n("Conversion problem"), 
	 i18n("Kab could not convert your database to the\n"
	      "current file format.\n"
	      "You can use it, but not all features will be\n"
	      "supported."));
    }
  // get configuration keys for initialization:
  keys=configSection()->getKeys();
  CHECK(keys!=0);  
  // do sessionmanagement "light":
  if(keys->get("LastCurrentKey", lastCurrentKey))
    {
      LG(GUARD, "AddressWidget constructor: last current key was %s.\n",
	 lastCurrentKey.c_str());
      if(!setCurrent(lastCurrentKey))
	{
	  QMessageBox::information
	    (this,
	     i18n("Error"),
	     i18n("The application saved the last "
		  "current entry,\nbut this entry "
		  "does not exist anymore."));
	}
    } else {
      LG(GUARD, "AddressWidget constructor: "
	 "last current key has not been saved.\n");
    }
  // -----
  keys->get("Background", bgFilename);
  CHECK(keys->get("Background", bgFilename));
  path=(string) KGlobal::iconLoader()->getIconPath(QString(bgFilename.c_str()) ).data();
  LG(GUARD, "AddressWidget constructor: loading widget background "
     "from file \n             \"%s\".\n", path.c_str());
  card->setBackground(path.c_str());

  QPixmap tmppix = Icon( QString( dlgBackground.c_str());
  DialogBase::setBackgroundTile( &tmppix );

  // -----
  timer=new QTimer(this);
  CHECK(timer!=0);
  connect(timer, SIGNAL(timeout()), SLOT(checkFile()));
  timer->start(1000);
  // -----
  comboSelector->setFocus();
  // ############################################################################
}

bool AddressWidget::updateDB()
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressWidget::updateDB: updating database.\n");
  // ############################################################################
  int format=0;
  double kabVersion=0.0;
  KeyValueMap* keys;
  Section::StringSectionMap::iterator pos;
  Entry entry;
  // versions:
  const int NewEmailsStyle=1,
    NewAddressFields=2;
  // -----
  if(configSection()==0 || noOfEntries()==0)
    {
      LG(GUARD, "AddressWidget::updateDB: clean DB.\n");
      return true;
    }
  // ----- retrieve version:
  keys=configSection()->getKeys();
  if(!keys->get("FileFormat", format))
    {
      LG(GUARD, "AddressWidget::updateDB: "
	 "no file format information in database.\n");
    }
  // ----- warn about a known bug for kab until 0.9:
  keys->get("Version", kabVersion); // possibly undefined key
  if(kabVersion<0.9)
    {
      LG(GUARD, "AddressWidget::updateDB: used old version "
	 "with update bug before.\n");
      QMessageBox::information
	(this, i18n("kab: Removed bug warning"), 
	 i18n("The version of kab you used before had an\n"
	      "error regarding the upgrading of the database\n"
	      "for new versions. This error has been removed\n"
	      "now and will not affect you anymore.\n"
	      "All configuration values have been reset to\n"
	      "its default settings."));
    }          
  if((format==KAB_FILE_FORMAT)&&(kabVersion>=0.9))
    {
      LG(GUARD, "AddressWidget::updateDB: same format.\n");
      return true;
    }
  // ----- do the updates:
  if(isRO())
    {
      if(!setFileName(fileName(), true, false))
	{
	  QMessageBox::information
	    (this, i18n("File handling error"), 
	     i18n("Unable to open database file r/w."));
	  exit(-1);
	}
    }
  CHECK(!isRO());
  if(format<NewEmailsStyle)
    {
      QMessageBox::information
	(this, i18n("Email storage conversion"),
	 i18n("Kab will move all your email addresses to\n"
	      "the new style supporting unlimited numbers\n"
	      "of email addresses per entry."));
      for(pos=entrySection()->sectionsBegin();
	  pos!=entrySection()->sectionsEnd(); 
	  pos++)
	{
	  makeEntryFromSection(*(*pos).second, entry);
	  if(!entry.email.empty())
	    {
	      entry.emails.push_back(entry.email);
	      entry.email="";
	    }
	  if(!entry.email2.empty())
	    {
	      entry.emails.push_back(entry.email2);
	      entry.email2="";
	    }
	  if(!entry.email3.empty())
	    {
	      entry.emails.push_back(entry.email3);
	      entry.email3="";
	    }
	  if(!change((*pos).first, entry))
	    {
	      LG(GUARD, "AddressWidget::updateDB: failed to change entry %s.\n",
		 (*pos).first.c_str());
	    }
	}
    }
  if(format<NewAddressFields)
    {
      QMessageBox::information
	(this, i18n("kab database update"),
	 i18n("Please note that kab now supports some new\n"
	      "fields in its addresses. These new fields\n"
	      "are:\n"
	      "° the state field,\n"
	      "° the country field and\n"
	      "° the postal code field.\n"
	      "You will probably need to edit some of your\n"
	      "entries to make use of it."));
    }
  // ----- set the new version:
  if(!keys->insert("FileFormat", KAB_FILE_FORMAT, true))
    {
      LG(GUARD, "AddressWidget::updateDB: "
	 "could not set new file format number.\n");
      return false;
    }
  // ----- save DB:
  if(!ConfigDB::save())
    {
      QMessageBox::information
	(this, i18n("File handling error"), 
	 i18n("Could not save database after update."));
      exit(-1);
    }
  // ----- switch back to r/o:
  if(!setFileName(fileName(), true, true))
    {
      QMessageBox::information
	(this, i18n("File handling error"), 
	 i18n("Unable to reopen database file r/o."));
      exit(-1);
    }      
  CHECK(isRO());
  updateEntriesMap();
  LG(GUARD, "AddressWidget::updateDB: done.\n");
  // ----- reset database configuration to the defaults:
  restoreDefaults();
  save();
  return true;
  // ############################################################################
}

AddressWidget::~AddressWidget()
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressWidget destructor: trying to save and clear database.\n");
  // ############################################################################
  // ----- let us try to handle autosaving here:
  Section* section=entrySection();
  KeyValueMap* keys;
  bool setting=true;
  if(section==0) // DB contents have been destroyed before
    {
      LG(GUARD, "AddressWidget destructor: DB contents have "
	 "been destroyed before.\n");
      return;
    }
  if(readonlyGUI)
    {
      LG(GUARD, "AddressWidget destructor: GUI is read-only, not saving.\n");
    } else {
      LG(GUARD, "AddressWidget destructor: saving database.\n"); 
      CHECK(configSection()!=0);
      keys=configSection()->getKeys();
      if(noOfEntries()!=0)
	{
	  // save current entry position:
	  LG(GUARD, "AddressWidget destructor: saving last current key %s.\n", 
	     (*current).second.c_str());
	  keys->insert("LastCurrentKey", (*current).second, true);
	} else {
	  L("AddressWidget destructor: cannot save current entry, "
	    "no entries.\n");
	}
      keys->get("SaveOnExit", setting);
      if(setting)
	{ // Saving using the GUI save method does not work since
	  // it is unsure wether we still may use the GUI or not.
	  // That is why only text output is generated.
	  // set r/w mode:
	  if(setFileName(fileName(), true, false))
	    { // save the database:
	      CHECK(!isRO());
	      if(!ConfigDB::save())
		{
		  cerr << "AddressWidget destructor: " << endl
		       << "  Cannot save database, all changes are lost." 
		       << endl;
		}
	      if(!setFileName(fileName(), true, true))
		{
		  cerr << "AddressWidget destructor: " << endl 
		       << "  Error resetting read only mode. "
		       << "You may ignore this problem." << endl;
		}
	    } else {
	      cerr << "AddressWidget destructor: " << endl 
		   << "  Error setting r/w mode. Cannot save database."
		   << endl;
	    } 
	}
    }
  LG(GUARD, "AddressWidget destructor: clearing database.\n");
  if(!ConfigDB::clear())
    {
      cerr << "AddressWidget destructor: "
	   << "database contents could not be cleared, but "
	   << "this should not be a problem."
	   << endl;
    }
  LG(GUARD, "AddressWidget destructor: done.\n");
  // ############################################################################
}      

void AddressWidget::initializeGeometry()
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  LG(GUARD, "AddressWidget::initializeGeometry: setting widget geometry.\n");
  QButton* buttons[]= { buttonFirst, buttonPrevious, 0, 
			buttonNext, buttonLast, 0,
			buttonSearch, 0, 
			buttonAdd, buttonChange, 
			buttonRemove };
  int cx=Grid, cy=Grid;
  unsigned int count;
  int i;
  // place the selector and the first separator
  comboSelector->move(Grid, cy);
  cy+=comboSelector->sizeHint().height()+Grid;
  frameSeparator1->move(Grid, cy); 
  cy+=2*Grid; // separators have 1 grid unit width
  // changed at Feb 10 1998 to display business card:
  card->move(Grid, cy);
  cx+=card->width()+Grid;
  cy+=card->height()+Grid;
  // now place the separator
  frameSeparator2->move(Grid, cy);
  cy+=2*Grid; 
  // now place and resize the buttons
  i=Grid;
  for(count=0; count<(sizeof(buttons)/sizeof(buttons[0])); count++)
    {
      if(buttons[count]!=0)
	{
	  buttons[count]->setGeometry(i, cy, ButtonSize, ButtonSize);
	  i+=ButtonSize+Grid;
   	} else {  // this inserts a space only
   	  i+=10;
   	}
    }
  cy+=ButtonSize+Grid;
  // now resize all widgets that depend 
  // on the space needed for the others
  comboSelector->resize(cx-2*Grid, comboSelector->sizeHint().height());
  frameSeparator1->resize(cx-2*Grid, Grid);
  frameSeparator2->resize(cx-2*Grid, Grid);
  // place the search results window below if it is set
  if(showSearchResults)
    {
      LG(GUARD, "AddressWidget::initializeGeometry: "
	 "search results window exists, displaying.\n");
      searchResults->setGeometry(Grid, cy, cx-2*Grid, 
				 searchResults->sizeHint().height());
      searchResults->show();
      cy+=Grid+searchResults->sizeHint().height();
    } else {
      searchResults->hide();
    }
  // fix widget size
  setFixedSize(cx, cy);
  emit(sizeChanged());
  // ############################################################################
}

void AddressWidget::createTooltips()
{
  // ############################################################################
  QButton* buttons[]= { buttonFirst, buttonPrevious,
			buttonNext, buttonLast,
			buttonSearch, 
			buttonAdd, buttonChange, 
			buttonRemove };
  const char* tips[]= { i18n("first entry"), 
			i18n("previous entry"), 
			i18n("next entry"), 
			i18n("last entry"),
			i18n("search entries"),
			i18n("add a new entry"), 
			i18n("change this entry"), 
			i18n("remove this entry") };
  const unsigned int Size=sizeof(buttons)/sizeof(buttons[0]);
  unsigned int i;
  // -----
  for(i=0; i<Size; i++)
    {
      QToolTip::add(buttons[i], tips[i]);
    }
  // ############################################################################
}

void AddressWidget::createConnections()
{
  // ############################################################################
  connect(buttonFirst, SIGNAL(clicked()), SLOT(first()));
  connect(buttonPrevious, SIGNAL(clicked()), SLOT(previous()));
  connect(buttonNext, SIGNAL(clicked()), SLOT(next()));
  connect(buttonLast, SIGNAL(clicked()), SLOT(last()));
  connect(buttonAdd, SIGNAL(clicked()), SLOT(add()));
  connect(buttonChange, SIGNAL(clicked()), SLOT(edit()));
  connect(buttonRemove, SIGNAL(clicked()), SLOT(remove()));
  connect(buttonSearch, SIGNAL(clicked()), SLOT(search()));
  connect(comboSelector, SIGNAL(activated(int)), SLOT(select(int)));
  // to handle appearance change events:
  connect(KApplication::getKApplication(), SIGNAL(appearanceChanged()), 
	  SLOT(initializeGeometry()));
  // ############################################################################
}

void AddressWidget::first()
{
  // ############################################################################
  if(!AddressBook::first())
    {
      qApp->beep();
    } else {
      emit(setStatus(i18n("Switched to first entry.")));
    }
  // ############################################################################
}

void AddressWidget::previous()
{
  // ############################################################################
  if(!AddressBook::previous())
    {
      qApp->beep();
    }
  // ############################################################################
}

void AddressWidget::next()
{
  // ############################################################################
  if(!AddressBook::next())
    {
      qApp->beep();
    }
  // ############################################################################
}

void AddressWidget::last()
{
  // ############################################################################
  if(!AddressBook::last())
    {
      qApp->beep();
    } else {
      emit(setStatus(i18n("Switched to last entry.")));
    }
  // ############################################################################
}

void AddressWidget::currentChanged()
{
  register bool GUARD; GUARD=false;
  LG(GUARD, "AddressWidget::currentChanged: current entry changed.\n");
  // ############################################################################
  unsigned int which=0;
  Entry entry;
  // -----
  AddressBook::currentChanged(); // keep the  chain
  currentEntry(entry);
  card->currentChanged(entry);
  enableWidgets();
  if(noOfEntries()!=0)
    { // careful! distance(..) broken for empty containers!
      // (should return zero but crashes)
      // cast needed because of ambiguosity
      distance((StringStringMap::iterator)entries.begin(), 
	       (StringStringMap::iterator)current, which);
      CHECK(which<noOfEntries());
      LG(GUARD, "AddressWidget::currentChanged: activating "
	 "item %i in selector.\n", which);
      comboSelector->setCurrentItem(which);
      ++which;
    }
  emit(entrySelected(which, noOfEntries()));
  // ############################################################################
}  

void AddressWidget::changed()
{
  // ############################################################################
  // this is needed for the consistency of the mirror map,
  // the call chain should always be kept
  // (would be easier with signals...)
  AddressBook::changed();
  // update combobox contents:
  updateSelector();
  // ############################################################################
  ENSURE(noOfEntries()==(unsigned)comboSelector->count());
}

void AddressWidget::remove()
{
  register bool GUARD; GUARD=false;
  REQUIRE(isRO());
  LG(GUARD, "AddressWidget::remove: removing entry ...");
  // ############################################################################
  if(noOfEntries()>0)
    {
      // ----- until now we only have RO access:
      if(setFileName(filename, true, false))
	{ // ----- now we have RW access:
	  if(fileChanged())
	    { // ----- switch back to RO access:
	      setFileName(filename, true, true);
	      checkFile();
	      setStatus(i18n("Sorry, file changed."));
	      qApp->beep();
	    }
	} else {
	  setStatus(i18n("Permission denied."));
	  qApp->beep();
	  return;
	}
      // ----- it is sure that we have locked the file here
      bool query=true; KeyValueMap* keys; 
      keys=configSection()->getKeys();
      keys->get("QueryOnDelete", query);
      CHECK(keys->get("QueryOnDelete", query));
      if(query 
	 ? (QMessageBox::information
	    (this, i18n("Remove entry?"),
	     i18n("Really remove this entry?"), 1, 2)==1)
	 : 1)
	{
	  LG(GUARD, " %i %s ... ", noOfEntries(),
	     noOfEntries()==1 ? "entry" : "entries");
	  bool rc; rc=AddressBook::remove(); 
	  // WORK_TO_DO: CHECK FOR ERRORS without variable
	  CHECK(rc);
	  LG(GUARD, "done.\n");
	  updateSelector();
	}
      // -----
      save();
      setFileName(filename, true, true);
    } else {
      LG(GUARD, " no entries found.\n");
      qApp->beep();
    }
  emit(setStatus(i18n("Entry deleted.")));
  ENSURE(isRO());
  // ############################################################################
}

void AddressWidget::add()
{
  register bool GUARD; GUARD=false;
  LG(GUARD, "AddressWidget::add: adding an empty entry.\n");
  // ############################################################################
  Entry dummy;
  string theNewKey;
  bool ro=isRO();
  // -----
  if(ro)
    {
      // ----- until now we only have RO access:
      if(setFileName(filename, true, false))
	{ // ----- now we have RW access:
	  if(fileChanged())
	    { // ----- switch back to RO access:
	      setFileName(filename, true, true);
	      checkFile();
	      setStatus(i18n("Sorry, file changed."));
	      qApp->beep();
	    }
	} else {
	  setStatus(i18n("Permission denied."));
	  qApp->beep();
	  return;
	}
    }
  CHECK(!isRO());
  // ----- it is sure that we have locked the file here
  if(edit(dummy))
    {
      AddressBook::add(dummy, theNewKey);
      updateSelector();
      setCurrent(theNewKey);
      emit(setStatus(i18n("Added a new entry.")));
      LG(GUARD, "AddressWidget::add: added key %s.\n", theNewKey.c_str());
    } else {
      LG(GUARD, "AddressWidget::add: failed.\n");
      qApp->beep();
    }
  // -----
  save();
  if(ro) 
    {
      setFileName(filename, true, true);
    }
  // ############################################################################
}

void AddressWidget::updateSelector()
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  LG(GUARD, "AddressWidget::updateSelector: doing it.\n");
  StringStringMap::iterator pos;
  string line;
  // -----
  comboSelector->clear();
  for(pos=entries.begin(); pos!=entries.end(); pos++)
    {
      // (one may change the appearance of the entries in the 
      // combobox here)
      // ... like I did on May 20, 1998
      // ... and again on June 14, 1998:
      description((*pos).second, line, true);
      CHECK(description((*pos).second, line, true));
      comboSelector->insertItem(line.c_str());
    } 
  // ############################################################################
}

void AddressWidget::select(int index)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  LG(GUARD, "AddressWidget::select: item %i selected.\n", index);
  setCurrent(index);
  CHECK(setCurrent(index));
  // ############################################################################
}

void AddressWidget::enableWidgets()
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  LG(GUARD, "AddressWidget::enableWidgets: doing it.\n");
  QWidget* widgets[]= {comboSelector, buttonFirst, 
   		       buttonPrevious, buttonNext, 
   		       buttonLast, buttonChange, 
   		       buttonRemove, buttonSearch };  
  int Size=(sizeof(widgets)/sizeof(widgets[0])); 
  int index; 
  // -----
  if(noOfEntries()==0)
    {
      for(index=0; index<Size; index++)
	{
	  widgets[index]->setEnabled(false);
	}
    } else {
      for(index=0; index<Size; index++)
	{
	  widgets[index]->setEnabled(true);
	}
      if(noOfEntries()!=1)
	{
	  if(isFirstEntry())
	    {
	      buttonFirst->setEnabled(false);
	      buttonPrevious->setEnabled(false);
	      buttonNext->setEnabled(true);
	      buttonLast->setEnabled(true);
	    } 
	  if(isLastEntry())
	    {
	      buttonFirst->setEnabled(true);
	      buttonPrevious->setEnabled(true);
	      buttonNext->setEnabled(false);
	      buttonLast->setEnabled(false);
	    } 
	} else {
	  buttonFirst->setEnabled(false);
	  buttonPrevious->setEnabled(false);
	  buttonNext->setEnabled(false);
	  buttonLast->setEnabled(false);
	} 
    }
  if(readonlyGUI)
    {
      buttonAdd->hide();
      buttonChange->hide();
      buttonRemove->hide();
    } else {
      buttonAdd->show();
      buttonChange->show();
      buttonRemove->show();
    }
  // -----
  emit(statusChanged(noOfEntries()));
  // ############################################################################
}

void AddressWidget::browse()
{
  register bool GUARD; GUARD=false;
  LG(GUARD, "AddressWidget::browse: calling kfmclient.\n");
  // ############################################################################
  Entry entry;

      if(noOfEntries()!=0)
	{
	  currentEntry(entry);
	  CHECK(currentEntry(entry));
	  if(!entry.URL.empty())
	    {
	      QString cmd = "kfmclient openURL ";
	      cmd += entry.URL.c_str();
	      system(cmd);
	      emit(setStatus(i18n("Opened browser window.")));
	    } else {
	      qApp->beep();
	      emit(setStatus(i18n("No URL.")));
	    }
	} else {
	  qApp->beep();
	  emit(setStatus(i18n("No entries.")));
	}

  // ############################################################################
} 


void AddressWidget::save()
{
  // ############################################################################
  bool wasLocked=locked;
  // -----
  if(!wasLocked)
    {
      if(!fileChanged())
	{
	  if(!setFileName(filename, true, false))
	    {
	      QMessageBox::information
		(this, i18n("Sorry"),
		 i18n("The addressbook file is currently\n"
		      "locked by another application.\n"
		      "kab cannot save it."));
	      return;
	    }
	} else {
	  checkFile();
	  setStatus(i18n("Sorry, file changed."));
	  qApp->beep();
	  return;
	}
    }
  // ----- the file is opened for reading and writing now:
  for(;;)
    {
      if(AddressBook::save())
	{
	  emit(setStatus(i18n("Database saved successfully.")));
	  break;
	}
      qApp->beep();
      switch(QMessageBox::information
	     (this, i18n("kab: File error"), i18n("Could not save database."),
	      i18n("&Retry"), i18n("&Ignore")))
	{
	case 1: { // cancel saving
	    emit(setStatus(i18n("Saving cancelled.\n")));
	    qApp->beep();
	    break;
	  }
	default:
	  break; // do nothing
	}
    }
  if(!wasLocked)
    {
      setFileName(filename, true, true);
    }
  // ############################################################################
}

void AddressWidget::dropEvent(QDropEvent *)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  LG(GUARD, "AddressWidget::dropEvent: got drop event.\n");
  // ############################################################################
}
  
void AddressWidget::edit()
{
  register bool GUARD; GUARD=true;
  // ############################################################################
  LG(GUARD, "AddressWidget::edit: called.\n");
  Entry entry;
  string temp;
  bool rejected=false;
  bool wasLocked=!isRO();
  // -----
  if(!wasLocked) 
    { // ----- until now we only have RO access:
      //       (otherwise this method has been called 
      //       from add(..))
      if(setFileName(filename, true, false))
	{ // ----- now we have RW access:
	  if(fileChanged())
	    { // ----- switch back to RO access:
	      setFileName(filename, true, true);
	      checkFile();
	      setStatus(i18n("Sorry, file changed."));
	      qApp->beep();
	    }
	} else {
	  setStatus(i18n("Permission denied."));
	  qApp->beep();
	  return;
	}
    } 
  // ----- it is sure that we have locked the file here
  CHECK(!isRO());
  if(currentEntry(entry))
    {
      if(edit(entry))
	{
	  LG(GUARD, "AddressWidget::edit: changes accepted, "
	     "storing in database.\n");
	  change(entry);
	  temp=(*current).second;
	  CHECK(!temp.empty());
	  updateSelector();
	  setCurrent(temp);
	  CHECK(setCurrent(temp));
	  emit(setStatus(i18n("Entry changed.")));
	} else {
	  LG(GUARD, "AddressWidget::edit: changes discarded.\n");
	  qApp->beep();
	  rejected=true;
	}
    } else { // ----- may not be called without entries:
      CHECK(false);
    }
  // -----
  if(!wasLocked)
    {
      if(!rejected) save();
      setFileName(filename, true, true);
    }
  ENSURE(isRO() || wasLocked);
  LG(GUARD, "AddressWidget::edit: done.\n");
  // ############################################################################
}

bool AddressWidget::edit(Entry& entry)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  LG(GUARD, "AddressWidget::edit: creating edit dialog.\n");
  EditEntryDialog dialog(this);
  dialog.setEntry(entry);
  if(dialog.exec())
    {
      LG(GUARD, "AddressWidget::edit: dialog finished with accept().\n");
      entry=dialog.getEntry();
      return true;
    } else {
      LG(GUARD, "AddressWidget::edit: dialog finished with reject().\n");
      return false;
    }
  // ############################################################################
}

void AddressWidget::search()
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  LG(GUARD, "AddressWidget::search: creating search dialog.\n");
  SearchDialog dialog(this);
  Section* section=entrySection();
  Section::StringSectionMap::iterator pos;
  string value;
  // -----
  CHECK(section!=0); // DB needs to be initialized
  if(noOfEntries()==0)
    {
      emit(setStatus(i18n("No entries.")));
      qApp->beep();
      return;
    }
  if(dialog.exec())
    {
      QApplication::setOverrideCursor(waitCursor);
      searchResults->clear();
      LG(GUARD, "AddressWidget::search: dialog finished with accept(), "
	 "searching all \"%s\" field for \"%s\".\n",
	 dialog.getKey().c_str(), dialog.getValue().c_str());
      for(pos=section->sectionsBegin(); pos!=section->sectionsEnd(); pos++)
	{
	  value="";
	  if((*pos).second ->getKeys()->get(dialog.getKey(), value))
	    { // the entry has the selected key defined
	      if(fnmatch(dialog.getValue().c_str(), value.c_str(),
			 FNM_NOESCAPE | FNM_PERIOD)==0)
		 /* value.find(dialog.getValue())!=string::npos */
		{
		  LG(GUARD, "AddressWidget::search: found match %s.\n", 
		     (*pos).first.c_str());
		  // construct name:
		  searchResults->add((*pos).first.c_str(),
				     getName((*pos).first));
		} else {
		  LG(GUARD, "AddressWidget::search: %s does not match.\n",
		     (*pos).first.c_str());
		}
	    } else {
	      LG(GUARD, "AddressWidget::search: key not defined in entry %s.\n",
		 (*pos).first.c_str());
	    }
	}
      LG(GUARD, "AddressWidget::search: %i matches.\n", 
	 searchResults->size());
      QApplication::restoreOverrideCursor();
      if(searchResults->size()!=0)
	{ // only if we found something
	  showSearchResults=true;
	  initializeGeometry();
	  searchResults->select(0);
	} else {
	  QMessageBox::information
	    (this, i18n("Results"), i18n("No entry matches this."));
	  if(showSearchResults==true)
	    {
	      showSearchResults=false;
	      initializeGeometry();
	    }
	}
    } else {
      LG(GUARD, "AddressWidget::search: dialog rejected.\n");
      emit(setStatus(i18n("Rejected.")));
      qApp->beep();
    }
  LG(GUARD, "AddressWidget::search: done.\n");
  // ############################################################################
}

void AddressWidget::searchResultsClose()
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  if(showSearchResults==true)
    {
      LG(GUARD, "AddressWidget::searchResultsClose: hiding search results.\n");
      showSearchResults=false;
      initializeGeometry();
      emit(setStatus(i18n("Search results window closed.")));
    } else {
      L("AddressWidget::searchResultsClose: "
	"called, but widget is not created.\n");
    }
  // ############################################################################
}

void AddressWidget::selectEntry(const char* key)
{
  // ############################################################################
  setCurrent(key);
  // ############################################################################
}

int AddressWidget::printHeadline(QPainter* p, QRect pageSize, const string& text)
{
  // ############################################################################
  const int Grid=5;
  QRect rect;
  int y;
  const int TopMargin=pageSize.top(),
    LeftMargin=pageSize.left(),
    PageWidth=pageSize.width(),
    PageHeight=pageSize.height();
  // ----- draw headline
  //       set large Times font, italic
  p->setFont(QFont("times", 24, QFont::Normal, true));
  p->drawText(LeftMargin, TopMargin, PageWidth, PageHeight,
	      AlignTop | AlignHCenter, text.c_str(), -1, &rect);
  y=TopMargin+rect.height()+Grid;
  p->drawLine(LeftMargin, y, PageWidth+LeftMargin, y);
  y+=Grid;
  return y;
  // ############################################################################
}

int AddressWidget::printFooter(QPainter* p, QRect pageSize, int pageNum,
			       string left, string right)
{
  // ############################################################################
  QRect rect;
  const int Grid=5;
  const int TopMargin=pageSize.top(),
    LeftMargin=pageSize.left(),
    PageWidth=pageSize.width(),
    PageHeight=pageSize.height();
  char buffer[64]; 
  string::size_type pos;
  int y;
  // ----- set small Times font, italic
  p->setFont(QFont("times", 12, QFont::Normal, true));
  // check strings for "<p>" that is replaced with pagenumber
  sprintf(buffer, "%i", pageNum);
  pos=left.find("<p>");
  if(pos!=string::npos)
    {
      left.ERASE(pos, 3); // remove 3 characters
      left.insert(pos, buffer);
    }
  pos=right.find("<p>");
  if(pos!=string::npos)
    {
      right.ERASE(pos, 3); // remove 3 characters
      right.insert(pos, buffer);
    }
  // -----
  p->drawText(LeftMargin, TopMargin, PageWidth, PageHeight, AlignLeft | AlignBottom,
	      left.c_str(), -1, &rect);
  p->drawText(LeftMargin, TopMargin, PageWidth, PageHeight,
	      AlignRight | AlignBottom, right.c_str());
  y=rect.height()+Grid;
  p->drawLine(LeftMargin, TopMargin+PageHeight-y, LeftMargin+PageWidth, 
	      TopMargin+PageHeight-y);
  y+=Grid;
  return y;
  // ############################################################################
}  

void AddressWidget::talk()
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressWidget::browse: calling talk client.\n");
  // ############################################################################
  KProcess proc;
  list<string>::iterator it;
  string command; 
  list<string> params;
  string address;
  string::size_type pos;
  Entry entry;
  StringListSelectOneDialog dialog(this);
  bool found=false; // true if "<person>" has been found
  KeyValueMap* keys=configSection()->getKeys();
  // -----
  if(!keys->get("TalkCommand", command))
    {
      QMessageBox::information
	(this, i18n("Error"), 
	 i18n("The talk command must be configured before!"));
    }
  if(!keys->get("TalkParameters", params))
    {
      QMessageBox::information
	(this, i18n("Talk configuration"),
	 i18n("Please configure the parameters for the talk command."));
      return;
    }
  if(!currentEntry(entry))
    {
      emit(setStatus(i18n("No entries.")));
      L("AddressWidget::talk: inconsistency!");
      return;
    }
  // ----- add talk addresses to dialog:
  if(entry.talk.empty())
    { // ----- is possible: valid lists may be empty
      LG(GUARD, "AddressWidget::talk: no talk address.\n");
      emit(setStatus(i18n("No talk address.")));
      qApp->beep();
      return;
    } else {
      LG(GUARD, "AddressWidget::talk: %i talk addresses.\n",
	 entry.talk.size());
    }
  dialog.setValues(entry.talk);
  dialog.setCaption(i18n("kab: Talk addresses"));
  dialog.setHeadline(i18n("Available talk addresses:"));
  if(dialog.exec())
    {
      dialog.getSelection(address);
      CHECK(dialog.getSelection(address));
    } else {
      emit(setStatus(i18n("Cancelled.")));
      return;
    }
  // ----- query what address to use:
  LG(GUARD, "AddressWidget::talk: parsing talk parameters.\n");
  for(it=params.begin(); it!=params.end(); it++)
    {
      LG(GUARD, "                     parsing %s.\n", (*it).c_str());
      pos=(*it).find("<person>");
      if(pos==string::npos)
	{
	  continue;
	} else {
	  found=true;
	}
      // WORK_TO_DO: use "replace" here
      (*it).ERASE(pos, 8); // 8 letters = <person>
      (*it).insert(pos, address);
      LG(GUARD, "                     changed to %s.\n", (*it).c_str());
    }
  if(!found)
    {
      QMessageBox::information
	(this, i18n("Error"), i18n("The talk command parameters are wrong."));
      return;
    }
  proc << command.c_str();
  for(it=params.begin(); it!=params.end(); it++)
    {
      proc << (*it).c_str();
    }
  if(proc.start(KProcess::DontCare)!=true)
    {
      QMessageBox::information
	(this, i18n("Error"), 
	 i18n("Talk command failed.\n"
	      "Make sure you did setup your talk command and parameter!"));
    } else {
      emit(setStatus(i18n("Talk program started.")));
    }  
  // ############################################################################
}

void AddressWidget::setReadonlyGUI(bool state)
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressWidget::setReadonlyGUI: setting GUI to %s.\n", 
     state ? "true" : "false");
  // ############################################################################
  readonlyGUI=state;
  enableWidgets();
  // ############################################################################
}  

void AddressWidget::checkFile()
{
  register bool GUARD; GUARD=true;
  // this produces too much noise:
  // LG(GUARD, "AddressWidget::checkFile: called.\n");
  // ############################################################################
  string lastKey=currentEntry();;
  // -----
  if(fileChanged())
    {
      LG(GUARD, "AddressWidget::checkFile: file has been changed.\n");
      clear();
      if(load())
	{
	  LG(GUARD, "AddressWidget::checkFile: reloaded.\n");
	} else { // ----- there should be no errors here!
	  LG(GUARD, "AddressWidget::checkFile: failure while"
	     " trying to reload.\n");
	  CHECK(false);
	}
      // ----- try to be convinient:
      setCurrent(lastKey);
    }
  // ############################################################################
}

bool AddressWidget::print(QPrinter& printer, const list<string>& fields,
			  const string& header, const string& ftLeft,
			  const string& ftRight)
{
  register bool GUARD; GUARD=true;
  REQUIRE(fields.size()!=0);
  debug(i18n("Attention: printing is still experimental!"));
  // ############################################################################
  LG(GUARD, "AddressWidget::print: %i fields to be printed.\n", fields.size());
  string path;
  KeyValueMap* keys;
  Section* section=entrySection();
  CHECK(section!=0);
  StringStringMap::iterator entry;
  Section::StringSectionMap::iterator entryPos;
  list<string>::const_iterator pos;
  list<int> bdList; // for parsing birthdays
  string text;
  // Qt handles top, bottom and right margin, but uses a left margin of zero.
  QPaintDeviceMetrics metrics(&printer);
  const QFont PrintFont("Helvetica", 8);
  const int 
    LeftMargin=72, // points == 1 inch
    RightMargin=72,
    TopMargin=72,
    BottomMargin=72,
    PageWidth=metrics.width()-LeftMargin-RightMargin,
    PageHeight=metrics.height()-TopMargin-BottomMargin,
    Spacing=2; // points 
  QPainter p;
  QRect rect;
  int footerHeight, headlineHeight, count, temp, cy, cx, commentFieldWidth=0;
  int* fieldWidth=new int[fields.size()];
  bool comment=false;
  float stretch;
  int pageNum=0;
  // -----
  // draw main part of page
  p.begin(&printer);
  LG(GUARD, "AddressWidget::print: printing %i entries,"
     "\n                      stage 1: placement.\n", noOfEntries());
  count=0;
  // find place needed for columns:
  // for all fields that should be printed:
  // find place needed for printing them
  p.setFont(PrintFont);
  for(pos=fields.begin(); 
      pos!=fields.end(); pos++)
    {
      if((*pos)!="comment")
	{
	  LG(GUARD, "AddressWidget::print: measuring %s-fields.\n", 
	     (*pos).c_str());
	  fieldWidth[count]=0;
	  // iterate over all entries:
	  for(entryPos=section->sectionsBegin();
	      entryPos!=section->sectionsEnd();
	      entryPos++)
	    {
	      keys=(*entryPos).second->getKeys();
	      CHECK(keys!=0);
	      text="";
	      keys->get((*pos), text);
	      // find largest value in pixels:
	      rect=p.boundingRect(0, 0, PageWidth, PageHeight,
				  AlignLeft | AlignTop, text.c_str());
	      if(fieldWidth[count]<rect.width()+2*Spacing)
		{
		  fieldWidth[count]=rect.width()+2*Spacing;
		}
	    }
	  LG(GUARD, "AddressWidget::print: width is %i.\n", fieldWidth[count]);
	} else {
	  LG(GUARD, "AddressWidget::print: found comment field.\n");
	  fieldWidth[count]=0;
	  comment=true;
	}
      count++;
    }
  // ----- now find stretch factor:
  /* There are 2 possibilities: 
     1. the comment field will be printed
     -> the other fields will not be stretched
     2. the comment field will not be printed
     -> the other fields are stretched to fit the page
  */
  if(comment)
    { // with comment field
      stretch=1;
    } else { // without comment field
      temp=0;
      for(count=0; (unsigned)count<fields.size(); count++)
	{
	  temp+=fieldWidth[count];
	}
      stretch=(float)PageWidth/(float)temp;
      LG(GUARD, "AddressWidget::print: stretch*points is %f, "
	 "page width is %i.\n", stretch*temp, PageWidth);
    }
  if(stretch<1)
    {
      if(QMessageBox::information
	 (this, i18n("Page size problem"),
	  i18n("The fields you requested to print do\n"
	       "not fit into the page width."),
	  i18n("Continue"), i18n("Cancel"))==1)
	{
	  p.end();
	  printer.abort();	  
	  return false;
	}
    }
  // -----
  // ----- stretch the fields:
  LG(GUARD, "AddressWidget::print: calculated a stretch factor of %f.\n",
     stretch);
  if(comment)
    { // ----- comment field will be printed
      // ----- find sum of field widths
      temp=0;
      for(count=0; (unsigned)count<fields.size(); count++)
	{ // safe: comment field width is zero
	  temp+=fieldWidth[count];
	}
      CHECK(temp>0); // field widths contain spacings
      if(temp<0.80*PageWidth)
	{ // comment field gets 20%, rest gets stretched
	  stretch=(0.80*(float)PageWidth)/(float)temp;
	  pos=fields.begin();
	  for(count=0; (unsigned)count<fields.size(); count++)
	    {
	      if(*pos!="comment")
		{
		  fieldWidth[count]=(int)(stretch*(float)fieldWidth[count]+0.5);
		} else {
		  fieldWidth[count]=(int)(0.20*(float)PageWidth+0.5);
		  commentFieldWidth=fieldWidth[count];
		}
	      ++pos;
	    }
	} else { // hmmmm... how to handle this?
	  LG(GUARD, "AddressWidget::print: not enough room"
	     " to handle comment field nicely.\n");
	  if(QMessageBox::information
	     (this, i18n("Page size problem"),
	      i18n("The fields you requested to print do\n"
		   "not fit into the page width."),
	      i18n("Continue"), i18n("Cancel"))==1)
	    {
	      p.end();
	      printer.abort();	  
	      return false;
	    }
	}
    } else { // comment field will not be printed
      for(count=0; (unsigned)count<fields.size(); count++)
	{
	  fieldWidth[count]=(int)(stretch*(float)fieldWidth[count]+0.5);
	}
      commentFieldWidth=-1;
    }
  // -----
  // ----- now print the page:
  LG(GUARD, "AddressWidget::print: stage 2: actual printing.\n");
  cy=0;
  CHECK(section==entrySection());
  entry=entries.begin();
  LG(GUARD, "AddressWidget::print: page size is %ix%i dots.\n"
     "                      page size is %ix%i mm.\n",
     metrics.width(), metrics.height(), metrics.widthMM(), metrics.heightMM());
  for(;;)
    {
      CHECK(entry!=entries.end());
      ++pageNum; // starts with zero
      // -----
      // ----- draw a headline
      headlineHeight=printHeadline
	(&p, QRect(LeftMargin, TopMargin, PageWidth, PageHeight), header);
      CHECK(headlineHeight>=0);
      // -----
      // ----- draw a footer
      footerHeight=printFooter
	(&p, QRect(LeftMargin, TopMargin, PageWidth, PageHeight),
	 pageNum, ftLeft, ftRight);
      CHECK(footerHeight>=0);
      // -----
      // ----- print lines between columns:
      temp=LeftMargin;
      for(count=0; (unsigned)count<=fields.size(); count++)
	{
	  p.drawLine(temp, headlineHeight, temp, 
		     metrics.height()-BottomMargin-footerHeight);
	  if((unsigned)count!=fields.size())
	    {
	      temp+=fieldWidth[count];
	    }
	}
      // -----
      // ----- draw entries as long as they fit on the page
      p.setFont(PrintFont);
      cy=headlineHeight;
      for(;;) 
	{
	  path=EntrySection+(string)"/"+(string)(*entry).second;
	  get(path, keys);
	  CHECK(get(path, keys));
	  cy+=Spacing;
	  text="";
	  // calculate the height of the line:
	  if(comment)
	    { // height is max (commentHeight, lineHeight):
	      CHECK(commentFieldWidth!=-1);
	      // get the comment:
	      int commentHeight=0;
	      int lineHeight=p.fontMetrics().height();
	      if(keys->get("comment", text))
		{
		  rect=p.boundingRect(0, 0, commentFieldWidth, 
		     PageHeight, AlignLeft | AlignTop | WordBreak, 
		     text.c_str());
		  commentHeight=rect.height();
		}
	      commentHeight>lineHeight ? temp=commentHeight : temp=lineHeight;
	      temp+=Spacing;
	    } else {
	      temp=p.fontMetrics().height()+Spacing;
	    }
	  // break if entry does not fit on page:
	  if(cy+temp+Spacing>TopMargin+PageHeight-footerHeight)
	    { // exit if it does not fit:
	      break; // exit : if ...
	    }
	  // temp is used to store height of text:
	  count=0; cx=LeftMargin;
	  temp=p.fontMetrics().height()+Spacing;
	  for(pos=fields.begin(); pos!=fields.end(); pos++)
	    { // draw all fields:
	      text="";
	      if(*pos=="comment")
		{
		  keys->get(*pos, text);
		  if(!text.empty())
		    {
		      p.drawText(cx+Spacing, cy, fieldWidth[count], PageHeight,
				 AlignLeft | AlignTop | WordBreak, 
				 text.c_str(), -1, &rect);
		      if(temp<rect.height()+Spacing)
			{
			  temp=rect.height()+Spacing;
			}
		    }
		  cx+=fieldWidth[count];
		  count++;
		  continue;
		}
	      if(*pos=="birthday")
		{
		  CHECK(bdList.empty());
		  if(keys->get(*pos, bdList))
		    {
		      LG(GUARD, "AddressWidget::print: "
			 "read birthday list, %i elements.\n",
			 bdList.size());
		      // ----- there could be empty lists:
		      if(bdList.size()==3)
			{ 
			  int day=bdList.back(); bdList.pop_back();
			  int month=bdList.back(); bdList.pop_back();
			  int year=bdList.back(); bdList.pop_back();
			  QDate date(year, month, day);
			  if(date.isValid())
			    {
			      p.drawText(cx+Spacing, cy, fieldWidth[count], 
				 PageHeight, AlignLeft | AlignTop, 
				 date.toString());
			    }
			} else {
			  if(bdList.size()!=0)
			    {
			      list<int> dummy;
			      keys->insert("birthday", dummy, true);
			    }
			}
		    }
		  cx+=fieldWidth[count];
		  count++;
		  continue;
		}
	      keys->get(*pos, text);
	      p.drawText(cx+Spacing, cy, fieldWidth[count], PageHeight,
		 AlignLeft | AlignTop, text.c_str());
	      cx+=fieldWidth[count];
	      count++;
	    }
	  cy+=temp;
	  p.drawLine(LeftMargin, cy, PageWidth+LeftMargin, cy);
	  entry++;
	  // exit if all entries have been printed:
	  if(entry==entries.end()) break;
	}
      // do not start a new page if we are done:
      if(entry!=entries.end())
	{
	  /*
	    I have to ignore the value since it is reported wrong!
	    if(printer.newPage()!=true)
	    {
	    LG(GUARD, "AddressWidget::print: "
	    "failed to start a new page.\n");
	    p.end();
	    delete fieldWidth;
	    return false;
	    }
	  */
	  printer.newPage();
	} else {
	  // all done
	  break; 
	}
    }
  p.end();
  delete fieldWidth;
  emit(setStatus(i18n("Printing finished successfully.")));
  return true;
  // ############################################################################
}

bool AddressWidget::emailAddress(const string& key, string& address, bool select)
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressWidget::emailAddress: called.\n");
  // ############################################################################
  Entry entry;
  StringListSelectOneDialog dialog(this);
  // -----
  if(!getEntry(key, entry))
    {
      LG(GUARD, "AddressWidget::emailAddress: no such entry.\n");
      return false;
    }
  if(entry.emails.empty())
    {
      LG(GUARD, "AddressWidget::emailAddress: no email address.\n");
      return false;
    }
  if(select)
    {
      dialog.setValues(entry.emails);
      dialog.setCaption(i18n("kab: Email addresses"));
      dialog.setHeadline(i18n("Available email addresses:"));
      if(dialog.exec())
	{
	  dialog.getSelection(address);
	  CHECK(dialog.getSelection(address));
	  return true;
	} else {
	  return false;
	}
    } else {
      address=entry.emails.front();
      return true;
    }
  // ############################################################################
}


void AddressWidget::exportPlain()
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressWidget::exportPlain: called.\n");
  // ############################################################################
  // ############################################################################
}

void AddressWidget::exportTeXTable()
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressWidget::exportTeXTable: called.\n");
  // ############################################################################
  // ############################################################################
}

void AddressWidget::exportTeXLabels()
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressWidget::exportTeXLabels: called.\n");
  // ############################################################################
  // ############################################################################
}

void AddressWidget::copy()
{
  // ############################################################################
  //       an array containing the keys for all fields:
  StringListSelectSetDialog dialog(this);
  int index;
  list<string> fields;
  list<string>::iterator fpos; // selects a field
  list<int> indizes;
  list<int>::iterator ipos;
  Section* section;
  KeyValueMap* keys;
  StringStringMap::iterator epos; // selects an entry
  string data, text, temp;
  // ----- select fields to copy:
  for(index=0; index<NoOfFields; index++)
    { // translate field keys into texts:
      if(!nameOfField(Fields[index], text))
	{
	  CHECK(false); // should not happen
	}
      fields.push_back(text);
    }
  dialog.setValues(fields);
  dialog.setCaption(i18n("kab: Select fields to copy"));
  if(!dialog.exec())
    {
      emit(setStatus(i18n("Rejected.")));
      qApp->beep();
      return;
    }
  dialog.getSelection(indizes);
  if(indizes.size()==0)
    {
      emit(setStatus(i18n("Nothing selected.")));
      qApp->beep();
      return;
    } else { // put selected keys in fields:
      fields.erase(fields.begin(), fields.end());
      for(ipos=indizes.begin(); ipos!=indizes.end(); ipos++)
	{
	  CHECK(*ipos>-1 && *ipos<NoOfFields);
	  fields.push_back(Fields[*ipos]);
	}
      CHECK(fields.size()==indizes.size());
    }
  // ----- add the headers:
  for(fpos=fields.begin(); fpos!=fields.end(); fpos++)
    {
      temp="";
      nameOfField(*fpos, temp);
      CHECK(nameOfField(*fpos, temp));
      data+=temp+"\t";
    }
  data+="\n";
  // ----- add all entries:
  for(epos=entries.begin(); epos!=entries.end(); epos++)
    { //       for all entries in current sorting order do:
      // ----- get the entry as an key-value-map
      getEntry((*epos).second, section);
      CHECK(getEntry((*epos).second, section));
      keys=section->getKeys();
      for(fpos=fields.begin(); fpos!=fields.end(); fpos++)
	{ //       for all selected fields do:
	  if(*fpos=="emails")
	    {
	      if(emailAddress((*epos).second, text, false))
		{
		  data+=text;
		}
	      data+="\t";
	      continue;
	    }
	  if(keys->get(*fpos /* the field key */, text))
	    {
	      data+=text;
	    }
	  data+="\t";
	}
      data+="\n";
    }
  // ----- copy data to the clipboard:
  QApplication::clipboard()->setText(data.c_str());
  // ############################################################################
}

void AddressWidget::exportHTML()
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressWidget::exportHTML: called.\n");
  // ############################################################################
  const string background=card->getBackground();;
  const string title=i18n("KDE addressbook overview").ascii();
  string header=
    (string)"<html>\n<head>\n"
    +(string)"<title>"+(string)title+(string)"</title>\n"
    +(string)"</head>\n"
    +(string)"<body background=\""+background+(string)"\">\n"
    +(string)"<h1>"+title+(string)"</h1>";
  string footer="</body>\n</html>";
  string logo;
  string kdelabel;
  string alignment="center";
  string body;
  string home;
  string file;
  string temp;
  QString dummy; // string objects crash on 0 pointers
  list<int> indizes;
  list<int>::iterator ipos;
  list<string> fields; // the fields in the table
  list<string>::iterator fieldPos;
  StringListSAndRSetDialog pDialog(this);
  Section* entry;
  StringStringMap::iterator pos;
  KeyValueMap* keys;
  int i;
  // ----- preparation:
  if(noOfEntries()==0)
    {
      emit(setStatus(i18n("No entries.")));
      qApp->beep();
      return;
    }
  // ----- create the table:
  body+=(string)"<"+alignment+(string)">"+(string)"<table border>\n";
  //       select what fields to add to the table:
  // fields.erase(fields.begin(), fields.end());
  for(i=0; i<NoOfFields; i++)
    {
      if(!nameOfField(Fields[i], temp))
	{
	  CHECK(false);
	}
      fields.push_back(temp.c_str());
    }
  fields.push_front(i18n("Name-Email-Link (!!)").ascii());
  pDialog.selector()->setValues(fields);
  pDialog.setCaption(i18n("Select table columns"));
  if(pDialog.exec())
    {
      if(!pDialog.selector()->getSelection(indizes))
	{
	  emit(setStatus(i18n("Nothing to export.")));
	  qApp->beep();
	  return;
	}	
      fields.erase(fields.begin(), fields.end());
      CHECK(fields.size()==0);
      for(ipos=indizes.begin(); ipos!=indizes.end(); ipos++)
	{
	  if(*ipos==0) // the name-email-link
	    {
	      // WORK_TO_DO: query settings for name style here
	      fields.push_back("name-email-link");
	    } else {
	      CHECK(*ipos<=NoOfFields && *ipos>0);
	      // remove the leading pseudo-field
	      fields.push_back(Fields[*ipos-1]);
	    }
	}
    } else {
      emit(setStatus(i18n("Rejected.")));
      qApp->beep();
      return;
    }
  //       create table headers:
  body+="<tr>\n";
  for(fieldPos=fields.begin(); fieldPos!=fields.end(); fieldPos++)
    {
      if(*fieldPos=="name-email-link")
	{
	  temp=i18n("Email link");
	} else {
	  if(!nameOfField(*fieldPos, temp))
	    {
	      L("AddressWidget::exportHTML: could not get name for field %s.\n",
		(*fieldPos).c_str());
	      temp="(unknown field name)";
	    }
	}
      body+=(string)"<th>"+temp+"\n";
    }
  body+="</tr>\n";
  //       create table, linewise:
  for(pos=entries.begin(); pos!=entries.end(); pos++)
    {
      getEntry((*pos).second, entry); // the section
      CHECK(getEntry((*pos).second, entry));
      keys=entry->getKeys();
      CHECK(keys!=0);
      body+="<tr>\n";
      for(fieldPos=fields.begin(); fieldPos!=fields.end(); fieldPos++)
	{
	  if(*fieldPos=="birthday")
	    {
	      Entry dummy;
	      getEntry((*pos).second, dummy);
	      CHECK(getEntry((*pos).second, dummy));
	      if(dummy.birthday.isValid())
		{
		  temp=dummy.birthday.toString();
		} else {
		  temp="";
		}
	      //       insert a non-breaking space - mozilla 
	      //       displays this better (hint from Thomas 
	      //       Stinner <thomas@roedgen.pop-siegen.de>
	      if(temp.empty()) temp="&nbsp;";
	      body+=(string)"<td>"+temp+"\n";
	      continue;
	    }
	  if(*fieldPos=="name-email-link")
	    {
	      Entry dummy;
	      getEntry((*pos).second, dummy);
	      CHECK(getEntry((*pos).second, dummy));
	      string mail;
	      string name;
	      emailAddress((*pos).second, mail, false);
	      literalName((*pos).second, name);
	      if(mail.empty())
		{
		  body+=(string)"<td>"+name+(string)"\n";
		} else {
		  body+=(string)"<td> <a href=mailto:\""+
		    mail+(string)"\">"+name+(string)"</a>\n";
		}
	      continue;
	    }
	  if(!keys->get(*fieldPos, temp))
	    {
	      L("AddressWidget::exportHTML: could not get data for key %s.\n",
		(*fieldPos).c_str());
	      temp="";
	    }
	  if(temp.empty()) temp="&nbsp;";
	  body+=(string)"<td>"+temp+"\n";
	}
      body+="</tr>\n";
    }
  body+="</table>\n"
    +(string)"</"+alignment+(string)">";
  // ----- get a filename:
  if(!getHomeDirectory(home))
    {
      QMessageBox::information
	(this, i18n("Sorry"), i18n("Could not find the users home directory."));
      emit(setStatus(i18n("Intern error!"))); 
      qApp->beep();
      return;
    }
  dummy=KFileDialog::getOpenFileName(home.c_str(), "*html", this);
  if(!dummy.isEmpty())
    {
      file=dummy;
      LG(GUARD, "AddressWidget::exportHTML: filename is %s.\n", file.c_str());
    } else {
      emit(setStatus(i18n("Cancelled.")));
      qApp->beep();
      return;
    }
  // ----- create HTML file:
  ofstream stream(file.c_str());
  if(!stream.good())
    {
      QMessageBox::information
	(this, i18n("Error"), 
	 i18n("Could not open the file to create the HTML table."));
    }
  LG(GUARD, "AddressWidget::exportHTML: writing the file.\n");
  //        htmlizeString is n.i., but may already be called:
  if(htmlizeString(header, temp)) header=temp;
  if(htmlizeString(body, temp)) body=temp;
  if(htmlizeString(footer, temp)) footer=temp;
  stream << header << endl 
	 << body << endl 
	 << footer << endl;
  LG(GUARD, "AddressWidget::exportHTML: done.\n");
  // ############################################################################
}
 
void AddressWidget::print()
{
  register bool GUARD; GUARD=true;
  // ############################################################################
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
      qApp->beep();
      return;
    }
  if(!prt.setup(this)) 
    {
      emit(setStatus(i18n("Printing cancelled.")));
      qApp->beep();
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
      LG(GUARD, "AddressWidget::print: could not query fields to print.\n");
      emit(setStatus(i18n("Rejected.")));
      qApp->beep();
      return;
    }
  if(!dialog.selector()->getSelection(indizes))
    {
      emit(setStatus(i18n("Nothing to print.")));
      qApp->beep();
      return;
    }
  // ----- find selected keys:
  for(pos=indizes.begin(); pos!=indizes.end(); pos++)
    {
      CHECK((*pos)<NoOfFields);
      keys.push_back(Fields[*pos]);
    }
  // ----- now configure the printing:
  printDialog.setHeadline(i18n("KDE addressbook overview").ascii());
  printDialog.setRightFooter(i18n("Page <p>").ascii());
  printDialog.setLeftFooter(i18n("KDE - the professionals choice.").ascii());
  if(!printDialog.exec())
    {
      LG(GUARD, "AddressWidget::print: printing setup rejected\n");
      emit(setStatus(i18n("Rejected.")));
      qApp->beep();
      return;
    }
  // ----- call the printing subroutines:
  if(!print(prt, keys, printDialog.getHeadline(), printDialog.getLeftFooter(),
	    printDialog.getRightFooter()))
     {
       qApp->beep();
       QMessageBox::information(this, i18n("Error"), i18n("Printing failed!"));
     }
  emit(setStatus(i18n("Printing finished successfully.")));
  // ############################################################################
}

void AddressWidget::mail()
{
  REQUIRE(configSection()!=0);
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressWidget::mail: calling mail composer.\n");
  // ############################################################################
  KeyValueMap* keys=configSection()->getKeys();
  string address;
  Entry entry;
  /* Wether to ask which mail address to use in case of multiple addresses.
   * The setting is stored in the key "MailSelectAddress" in the configuration
   * section.
   */
  bool ask=true; 
  // -----
  if(!currentEntry(entry))
    {
      emit(setStatus(i18n("No entries.")));
      qApp->beep();
      return;
    }
  if(entry.emails.empty())
    { //       complain:
      qApp->beep();
      LG(GUARD, "AddressWidget::mail: empty email address.\n");
      emit(setStatus(i18n("Empty email address.")));
      return;
    } else { //       select address:
      if(entry.emails.size()>1)
	{
	  //       what to do:
	  keys->get("MailSelectAddress", ask);
	  if(!ask)
	    { //       select address:
	      address=entry.emails.front();
	    }
	  //       query:
	  if(!emailAddress((*current).second, address, ask))
	    {
	      qApp->beep();
	      if(ask)
		{
		  emit(setStatus(i18n("Rejected.")));
		} else {
		  emit(setStatus(i18n("No email address.")));
		  CHECK(false);
		}
	      return;
	    }
	} else {
	  address=entry.emails.front();
	}
      if(!sendEmail(address))
	{
	  emit(setStatus(i18n("Error calling mailer.")));
	} else {
	  emit(setStatus(i18n("Mailer called.")));
	}
    }
  // ############################################################################
}

bool AddressWidget::sendEmail(const string& address, const string& subject)
{
  register bool GUARD; GUARD=true;
  REQUIRE(configSection()!=0);
  // ############################################################################
  KProcess proc;
  KeyValueMap* keys=configSection()->getKeys();
  bool found=false;
  list<string> params;
  list<string>::iterator it;
  string::size_type pos;
  string command; 
  // -----
  if(!keys->get("MailCommand", command))
    {
      QMessageBox::information
	(this, i18n("Error"), i18n
	 ("The mail command must be configured before!"));
    }
  if(!keys->get("MailParameters", params))
    {
      QMessageBox::information
	(this, i18n("Mail configuration"),
	 i18n("Please configure the parameters for the email command."));
      return false;
    } 
  LG(GUARD, "AddressWidget::mail: parsing mail parameters.\n");
  for(it=params.begin(); it!=params.end(); it++)
    {
      LG(GUARD, "                     parsing %s.\n", (*it).c_str());
      pos=0;
      while(pos!=string::npos)
	{
	  pos=(*it).find("<person>", pos);
	  if(pos!=string::npos)
	    {
	      found=true;
	      // WORK_TO_DO: use "replace" here
	      (*it).ERASE(pos, 8); // 8 letters = <person>
	      (*it).insert(pos, address);
	      LG(GUARD, "                     changed to %s.\n", (*it).c_str());
	    }
	}
      /* if(!subject.empty()) { */
      pos=0;
      while(pos!=string::npos)
	{
	  pos=(*it).find("<subject>");
	  if(pos!=string::npos)
	    {
	      // WORK_TO_DO: use "replace" here
	      (*it).ERASE(pos, 9); // 9 letters = <subject>
	      (*it).insert(pos, subject);
	      LG(GUARD, "                     changed to %s.\n", (*it).c_str());
	    }
	  /* } */
	}
    }
  if(!found) // subject param is mandatory
    {
      QMessageBox::information
	(this, i18n("Error"), i18n("The email command parameters are wrong."));
      return false;
    }
  proc << command.c_str();
  for(it=params.begin(); it!=params.end(); it++)
    {
      proc << (*it).c_str();
    }
  if(proc.start(KProcess::DontCare)!=true)
    {
      QMessageBox::information
	(this, i18n("Error"), 
	 i18n("Email command failed.\n"
	      "Make sure you did setup your email command and parameter!"));
      return false;
    } else {
      emit(setStatus(i18n("Mail program started.")));
      return true;
    }  
  // ############################################################################
}

void AddressWidget::mail(const char* address)
{
  // ############################################################################
  sendEmail(address);
  // ############################################################################
}

// #############################################################################
// MOC OUTPUT FILES:
#include "kab.moc"
// #############################################################################
