/* This file is part of the KDE libraries
   Copyright (C) 1999 Toivo Pedaste <toivo@ucs.uwa.edu.au>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "../config.h"

#include <qregexp.h>
#include <qwhatsthis.h>

#include <kaccel.h>
#include <kaccelbase.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kaccelmenu.h"

template class QIntDict<char>;

//////////////////////////////////////////////////////////////////////////////
KAccelMenu::KAccelMenu(KAccel *k, QWidget * parent, const char * name ):
  KPopupMenu(parent,name)
{
  keys = k;
  quote = FALSE;
  connect(this,SIGNAL(highlighted (int)),SLOT(highl(int)));
  connect(this,SIGNAL( aboutToShow()),SLOT(aboutTS()));
}

KAccelMenu::~KAccelMenu()
{
  keys->basePtr()->removeDeletedMenu(this);
}

static int get_seq_id()
{
    static int seq_no = 1000;
    return seq_no++;
}

int KAccelMenu::insItem (const QPixmap & pixmap, const char * text,
			 const char * action, const QObject * receiver,
	      const char * member, const char * accel)
{
  char *txt = stripAnd(text);

  int id = insertItem(pixmap, text, receiver, member, 0, get_seq_id());

  KShortcuts cuts( accel );
  keys->insertAction(action, txt, cuts, cuts, receiver, member, id, this);
  actions.insert(id, action);
  changeMenuAccel(id, action);
  return id;
}

int KAccelMenu::insItem (const char* text, const char * action,
				     const QObject * receiver,
	      const char * member, const char * accel)
{
  char *txt = stripAnd(text);

  int id = insertItem(text, receiver, member, 0, get_seq_id());

  KShortcuts cuts( accel );
  keys->insertAction(action, txt, cuts, cuts, receiver, member, id, this);
  actions.insert(id, action);
  changeMenuAccel(id, action);
  return id;
}

int KAccelMenu::insItem (const QPixmap & pixmap, const char * text,
			 const char * action, const QObject * receiver,
	      const char * member, KStdAccel::StdAccel accel )
{
  int id = insertItem(pixmap, text, receiver, member, 0, get_seq_id());
  keys->insertAction(accel, receiver, member, id, this);
  actions.insert(id, action);
  changeMenuAccel(id, KStdAccel::action(accel).latin1());
  return id;
}

int KAccelMenu::insItem (const char * text, const char * action,
				     const QObject * receiver,
	      const char * member, KStdAccel::StdAccel accel )
{
  int id = insertItem(text, receiver, member, 0, get_seq_id());
  keys->insertAction(accel, receiver, member, id, this);
  actions.insert(id, action);
  changeMenuAccel(id, KStdAccel::action(accel).latin1());
  return id;
}

char *KAccelMenu::stripAnd(const char *str)
{
  QCString s = str;
  s.replace(QRegExp(QString::fromLatin1("&")), "");
  return strdup(s.data());
}

void KAccelMenu::changeMenuAccel (int id, const char * action)
{
  KAccelAction* pAction = keys->basePtr()->actionPtr( action );
  QString s = text( id );
  if (!pAction || s.isEmpty() )
    return;

  QString k = pAction->getShortcut(0).toString();
  if (k.isEmpty())
    return;

  int i = s.find('\t');
  if ( i >= 0 )
    s.replace(i+1, s.length()-i, k);
  else {
    s += '\t';
    s += k;
  }

  QPixmap *pp = pixmap(id);
  if (pp && !pp->isNull())
    changeItem(*pp, s, id);
  else
    changeItem(s, id);
}

void KAccelMenu::popMsg () {
  int idx, yp;
  QString msg;

  idx = indexOf(cid);

  yp = 0;
  for (int i = 0;i < idx;i++)
    yp += itemHeight(i);

  if (actions[cid]) {
    KAccelAction* pAction = keys->basePtr()->actionPtr(actions[cid]);
    if (pAction) {
      if (pAction->m_bConfigurable) {
        msg = i18n("Change shortcut for: ");
        msg += pAction->m_sDesc;
        QWhatsThis::add(this, msg);
        QWhatsThis::enterWhatsThisMode();
        QWhatsThis::remove(this);
      } else {
        msg = i18n("Global Key: cannot change shortcut");
        KMessageBox::sorry(this, msg);
      }
    }
  }
}

void KAccelMenu::keyPressEvent ( QKeyEvent * e)
{
  uint key = e->key();
  uint state = e->state()  & ~MouseButtonMask;
  uint kcode = e->key() | state<< 10;
  bool deleteKey = FALSE;
  //  bool needQuote = FALSE;
  QString stmp;

  if (key != Key_Shift && key != Key_Control && key != Key_Meta &&
      key != Key_Alt && key != Key_CapsLock && key != Key_NumLock &&
      key != Key_ScrollLock) {

    if (!quote) {
      switch ( e->key() ) {

      case Key_Apostrophe:
	quote = TRUE;
	popMsg();
	return;

      case Key_Delete:
	deleteKey = TRUE;
	break;

      case Key_Up: case Key_Down: case Key_Left: case Key_Right:
      case Key_Alt: case Key_Escape: case Key_Space:
      case Key_Return: case Key_Enter:
	  //	needQuote = TRUE;
	break;

      default:
	  ; //	needQuote = FALSE;
      }

//       if  (!e->state() && e->key() >= Key_0 && e->key() <= Key_Z ) {
// 	needQuote = TRUE;
//       }
    }

    if ( /* !needQuote ||*/  quote || deleteKey) {
      if (actions[cid]) {
	KAccelAction* pAction = keys->basePtr()->actionPtr(actions[cid]);
	if (pAction && pAction->m_bConfigurable) {
	  if (deleteKey && !quote) {
	    pAction->setShortcuts(KAccelShortcuts());
	    changeMenuAccel(cid, actions[cid]);
	  } else {
	    KKeySequence key(kcode);
	    KAccelAction* pActionOld = keys->basePtr()->actionPtr(key);
	    if (pActionOld && !pActionOld->m_bConfigurable) {
	      stmp = i18n("Key already in use by non-configurable action: \"%1\"").
	               arg(pActionOld->m_sDesc);
	      KMessageBox::sorry(this, stmp);
	    } else {
	      // FIXME: should only clear the one relevant shortcut and
	      //  leave any others in tact.
	      if (pActionOld)
	        pActionOld->setShortcuts(KAccelShortcuts());
	      pAction->setShortcuts(KAccelShortcuts(key));
	      changeMenuAccel(cid, actions[cid]);
	    }
	  }
	} else if (pAction) {
	  stmp = i18n("This shortcut is not configurable.");
	  KMessageBox::sorry(this, stmp);
	}
      }
    }
    if (!quote)
      KPopupMenu::keyPressEvent(e);
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
