//////////////////////////////////////////////////////////////////////
//      $Id$
// File  : kpopupmenu.cpp
// Author: Toivo Pedaste
//
//////////////////////////////////////////////////////////////////////
#include "../config.h"

#include <qregexp.h>
#include <qwhatsthis.h>

#include <kaccel.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kaccelmenu.h"

//////////////////////////////////////////////////////////////////////////////
KAccelMenu::KAccelMenu(KAccel *k, QWidget * parent, const char * name ):
  QPopupMenu(parent,name)
{
  keys = k;
  quote = FALSE;
  connect(this,SIGNAL(highlighted (int)),SLOT(highl(int)));
  connect(this,SIGNAL( aboutToShow()),SLOT(aboutTS()));
}

KAccelMenu::~KAccelMenu()
{
  keys->removeDeletedMenu(this);
}

static int get_seq_id()
{
    static int seq_no = 1000;
    return seq_no++;
}

int KAccelMenu::insItem (const QPixmap & pixmap, const char* text,
			 const char * action, const QObject * receiver,
	      const char * member, const char * accel)
{
  char *txt = stripAnd(text);

  int id = insertItem(pixmap, text, receiver, member, 0, get_seq_id());

  if (accel)
    keys->insertItem(txt,action, accel, id, this);
  else
    keys->insertItem(txt,action, (uint)0, id, this);

  actions.insert(id,action);
  keys->connectItem(action, receiver, member);
  keys->changeMenuAccel(this,id,action);
  return id;
}

int KAccelMenu::insItem (const char* text, const char * action,
				     const QObject * receiver,
	      const char * member, const char * accel)
{
  char *txt = stripAnd(text);

  int id = insertItem(text, receiver, member, 0, get_seq_id());

  if (accel)
    keys->insertItem(txt,action, accel, id, this);
  else
    keys->insertItem(txt,action, (uint)0, id, this);

  actions.insert(id,action);
  keys->connectItem(action, receiver, member);
  keys->changeMenuAccel(this,id,action);
  return id;
}

int KAccelMenu::insItem (const QPixmap & pixmap, const char * text,
			 const char * action, const QObject * receiver,
	      const char * member, KAccel::StdAccel accel )
{
  keys->connectItem(accel, receiver, member);
  int id = insertItem(pixmap, text, receiver, member, 0, get_seq_id());
  actions.insert(id,action);
  keys->changeMenuAccel(this,id,accel);
  return id;
}

int KAccelMenu::insItem (const char * text, const char * action,
				     const QObject * receiver,
	      const char * member, KAccel::StdAccel accel )
{
  keys->connectItem(accel, receiver, member);
  int id = insertItem(text, receiver, member, 0, get_seq_id());
  actions.insert(id,action);
  keys->changeMenuAccel(this,id,accel);
  return id;
}

char *KAccelMenu::stripAnd(const char *str)
{
  QString s = str;

  s.replace(QRegExp("&"),"");
  return strdup(s.ascii());
}

void KAccelMenu::popMsg () {
  int idx, yp;
  QString msg;

  idx = indexOf(cid);

  yp = 0;
  for (int i = 0;i < idx;i++)
    yp += itemHeight(i);

  if (actions[cid]) {
    if (keys->configurable(actions[cid])) {
      msg = i18n("Change shortcut for: ");
      msg += keys->description(actions[cid]);
      QWhatsThis::add(this, msg);
      QWhatsThis::enterWhatsThisMode();
      QWhatsThis::remove(this);
    } else {
      msg = i18n("Global Key: cannot change shortcut");
      KMessageBox::sorry(this, msg, i18n("KAccelMenu Warning"));
    }
  }
}

void KAccelMenu::keyPressEvent ( QKeyEvent * e)
{
  uint key = e->key();
  uint state = e->state()  & ~MouseButtonMask;
  uint kcode = e->key() | state<< 10;
  bool deleteKey = FALSE;
  bool needQuote = FALSE;
  QString stmp;
  //printf("key=%x state=%x kcode=%x\n",e->key(),state,kcode);

  if (key != Key_Shift && key != Key_Control && key != Key_Meta &&
      key != Key_Alt && key != Key_CapsLock && key != Key_NumLock &&
      key != Key_ScrollLock) {

    if (!quote) {
      switch ( e->key() ) {

      case Key_Apostrophe:
	quote = TRUE;
	popMsg();
	return;
	break;

      case Key_Delete:
	deleteKey = TRUE;
	break;

      case Key_Up: case Key_Down: case Key_Left: case Key_Right:
      case Key_Alt: case Key_Escape: case Key_Space:
      case Key_Return: case Key_Enter:
	needQuote = TRUE;
	break;

      default:
	needQuote = FALSE;
      }
      if  (!e->state() && e->key() >= Key_0 && e->key() <= Key_Z ) {
	needQuote = TRUE;
      }
    }

    if ( /* !needQuote ||*/  quote || deleteKey) {
      if (actions[cid]) {
	if (keys->configurable(actions[cid])) {
	  if (deleteKey && !quote) {
	    keys->clearItem(actions[cid]);
	    keys->changeMenuAccel(this,cid,actions[cid]);
	  } else {
	    QString oldact = keys->findKey(kcode);
	    if (!oldact.isNull() && !keys->configurable(oldact)) {
	      stmp = i18n("Key already used as global key: ");
	      stmp += keys->description(oldact);
	      KMessageBox::sorry(this, stmp);
	    } else {
	      keys->clearItem(oldact);
	      keys->updateItem(actions[cid],kcode);
	      keys->changeMenuAccel(this,cid,actions[cid]);
	    }
	  }
	} else {
	  stmp = i18n("Global key cannot be changed");
	  KMessageBox::sorry(this, stmp);
	}
      }
    }
    if (!quote)
      QPopupMenu::keyPressEvent(e);
    quote = FALSE;
  }
}

void KAccelMenu::highl(int id) {
  cid = id;
  quote = FALSE;
}

void KAccelMenu::aboutTS() {
  cid = idAt(1);
  quote = FALSE;
}

//////////////////////////////////////////////////////////////////////////////
#include <kaccelmenu.moc>
