/*
  $Id$

   Copyright (C) 1998, 1999 Jochen Wilhelmy
                            digisnap@cs.tu-berlin.de

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
#include <stdio.h>

#include <qkeycode.h>
#include <qobject.h>
#include <qcombobox.h>

#include <klocale.h>

#include "kckey.h"

#include "kwrite_keys.h"

#undef KeyPress


/*
The following things i have discovered:
- on a pc with num lock unset the key "8" is Up, but Shift-"8" is 8.
- on a pc with num lock set, cursor keys have the X11 modifier 2 set.
- on old Sun workstations without cursor keys the key "8" is Up,
  but Shift-"8" is Shift-F28.
- on old Sun workstations with cursor keys the X11 modifier 3 is set.
*/

const int SHIFT = Qt::SHIFT;
const int ALT = Qt::ALT;
const int CTRL = Qt::CTRL;
//const int META = 0x00010000;
//const int MOD2 = 0x00020000;
//const int MOD3 = 0x00040000;
//const int MOD5 = 0x00080000;
const int modifierMask = SHIFT | CTRL | ALT;// | META | MOD2 | MOD3 | MOD5;

//int globalModifiers;

static bool isSpecialKey(int keyCode) {
  int special[] = {Qt::Key_CapsLock, Qt::Key_NumLock, Qt::Key_ScrollLock, 
    Qt::Key_unknown, 0};
//    Key_Shift, Key_Control, Key_Meta, Key_Alt, 0};
  int *i;

  i = special;
  while (*i != 0) {
    if (*i == keyCode) return true;
    i++;
  }
  return false;
}


static int modifier2Flag(int keyCode) {
  if (keyCode == Qt::Key_Shift) return SHIFT;
  if (keyCode == Qt::Key_Control) return CTRL;
//  if (keyCode == Qt::Key_Meta) return META;
  if (keyCode == Qt::Key_Alt) return ALT;
  return 0;
}


static QString keyToLanguage(int keyCode) {
  QString s;
  int z;
 	
  if (keyCode & SHIFT) {
    s = i18n("Shift");
  }
  if (keyCode & CTRL) {
    if (!s.isEmpty()) s += '+';
    s += i18n("Ctrl");
  }
  if (keyCode & ALT) {
    if (!s.isEmpty()) s += '+';
    s += i18n("Alt");
  }
/*  if (keyCode & META) {
    if (!s.isEmpty()) s += '+';
    s += i18n("Meta");
  }
  if (keyCode & MOD2) {
    if (!s.isEmpty()) s += '+';
    s += i18n("Mod2");
  }
  if (keyCode & MOD3) {
    if (!s.isEmpty()) s += '+';
    s += i18n("Mod3");
  }
  if (keyCode & MOD5) {
    if (!s.isEmpty()) s += '+';
    s += i18n("Mod5");
  }*/

  keyCode &= ~modifierMask;
  if (keyCode != 0) {
    for (z = 0; z < NB_KEYS; z++) {
      if (keyCode == KKEYS[z].code) {
        if (!s.isEmpty()) s += '+';
        s += i18n(KKEYS[z].name);
        break;
      }
    }
  }
  return s;
}

static QString keyToString(int keyCode) {
  QString s;
  int z;
 	
  if (keyCode & SHIFT) {
    s = "Shift";
  }
  if (keyCode & CTRL) {
    if (!s.isEmpty()) s += '+';
    s += "Ctrl";
  }
  if (keyCode & ALT) {
    if (!s.isEmpty()) s += '+';
    s += "Alt";
  }
/*  if (keyCode & META) {
    if (!s.isEmpty()) s += '+';
    s += "Meta";
  }
  if (keyCode & MOD2) {
    if (!s.isEmpty()) s += '+';
    s += "Mod2";
  }
  if (keyCode & MOD3) {
    if (!s.isEmpty()) s += '+';
    s += "Mod3";
  }
  if (keyCode & MOD5) {
    if (!s.isEmpty()) s += '+';
    s += "Mod5";
  }*/

  keyCode &= ~modifierMask;
  if (keyCode != 0) {
    for (z = 0; z < NB_KEYS; z++) {
      if (keyCode == KKEYS[z].code) {
        if (!s.isEmpty()) s += '+';
        s += KKEYS[z].name;
        break;
      }
    }
  }
  return s;
}

static int myStringToKey(QString s) {
  int keyCode, modifier, z;
  QString keyStr;

//printf("stringtokey %s\n", s);
  keyCode = 0;
  do {
    z = s.find('+');
    keyStr = (z >= 0) ? s.left(z) : s;
    modifier = 0;
    if (keyStr == "Shift") modifier = SHIFT;
    else if (keyStr == "Ctrl") modifier = CTRL;
    else if (keyStr == "Alt") modifier = ALT;
//    else if (keyStr == "Meta") modifier = META;
/*    else if (keyStr == "Mod2") modifier = MOD2;
    else if (keyStr == "Mod3") modifier = MOD3;
    else if (keyStr == "Mod5") modifier = MOD5;*/
    keyCode |= modifier;
    s.remove(0, z +1);

    if (z < 0) {
      if (!modifier) {
        for (z = 0; z < NB_KEYS; z++) {
          if (keyStr == KKEYS[z].name) {
            keyCode |= KKEYS[z].code;
            break;
          }
        }
      }
      return keyCode;
    }
  } while (true);
}


static QString removeAnd(QString s) {
  int pos;

  while ((pos = s.find('&')) != -1) s.remove(pos, 1);
  return s;
}


// KWAccel

bool KWAccel::equals(int kc1, int kc2) {
  return (keyCode1 == kc1 && (keyCode2 == kc2 || keyCode1 == 0 ));
}


// KWCommand

KWCommand::KWCommand(int id, const QString &name, KWCommandGroup *group)
  : m_id(id), m_name(name), m_group(group) {

  int z;
  for (z = 0; z < nAccels; z++) m_accels[z].keyCode1 = 0;
//  group->addCommand(this);
}

int KWCommand::accelCount() {
  int z;

  for (z = 0; z < nAccels; z++) {
    if (m_accels[z].keyCode1 == 0) break;
  }
  return z;
}


/*
int KWCommand::containsAccel(int keyCode1, int keyCode2) {
  int z;

  for (z = 0; z < nAccels; z++) {
    if (m_accels[z].keyCode1 == 0) break;
    if (m_accels[z].keyCode1 == keyCode1 && (m_accels[z].keyCode2 == keyCode2
      || m_accels[z].keyCode2 == 0 || keyCode2 == 0)) {

      return z;
    }
  }
  return -1;
}
*/
void KWCommand::addAccel(int keyCode1, int keyCode2) {
  int z;

  for (z = 0; z < nAccels; z++) {
    if (m_accels[z].keyCode1 == 0) {
      m_accels[z].keyCode1 = keyCode1;
      m_accels[z].keyCode2 = keyCode2;
      break;
    }
  }
}
/*
void KWCommand::removeAccel(int z) {
  while (z < nAccels - 1) {
    m_accels[z] = m_accels[z + 1];
    z++;
  }
  m_accels[z].keyCode1 = 0;
}

void KWCommand::removeAccel(int keyCode1, int keyCode2) {
  int n;

  while ((n = containsAccel(keyCode1, keyCode2)) >= 0) {
    removeAccel(n);
  }
}
*/

/*
void KGuiCmd::makeDefault() {
  int z;
  for (z = 0; z < nAccels; z++) defaultAccels[z] = accels[z];
}

void KGuiCmd::restoreDefault() {
  int z;
  for (z = 0; z < nAccels; z++) accels[z] = defaultAccels[z];
}

void KGuiCmd::saveAccels() {
  int z;
  for (z = 0; z < nAccels; z++) oldAccels[z] = accels[z];
}

void KGuiCmd::restoreAccels() {
  int z;
  for (z = 0; z < nAccels; z++) accels[z] = oldAccels[z];
}

void KGuiCmd::changeAccels() {
  if (!accels[0].equals(oldAccels[0].keyCode1, oldAccels[0].keyCode2)) {
    emit changed(getAccelString());
  }
}
*/

void KWCommand::getData(KWCommandData &data) {
//  data.setData(m_name, m_accels);
  int z;
  
  data.m_name = m_name;
  for (z = 0; z < nAccels; z++) {
    data.m_accels[z] = m_accels[z];
  }
}

void KWCommand::setData(const KWCommandData &data) {
  int z;
  
  for (z = 0; z < nAccels; z++) {
    m_accels[z] = data.m_accels[z];
  }
}

void KWCommand::readConfig(KConfig *config) {
  QString s;
  int z, nextAccel, next;

  s = removeAnd(m_name);
  if (!config->hasKey(s)) return;
  s = config->readEntry(s);

  while ((z = s.find(' ')) != -1) s.remove(z, 1);

  if (s == "(Default)") return;

  for (z = 0; z < nAccels; z++) {
    m_accels[z].keyCode1 = 0;
    m_accels[z].keyCode2 = 0;
  }


  z = 0;
  while (!s.isEmpty()) {
    nextAccel = s.find(';');
    if (nextAccel == -1) nextAccel = s.length();

    next = s.findRev(',', nextAccel -1);
    if (next != -1) {
      m_accels[z].keyCode2 = myStringToKey(s.mid(next +1, nextAccel - next -1));
    } else next = nextAccel;
    m_accels[z].keyCode1 = myStringToKey(s.left(next));
    s.remove(0, nextAccel +1);
    z++;
  }
}

void KWCommand::writeConfig(KConfig *config) {
  QString key;
  int count, z;
  bool def;

  key = removeAnd(m_name);
  count = accelCount();

def = true;
/*
  //are current accelerators the default accelerators?
  if (count < nAccels && defaultAccels[count].keyCode1 != 0) {
    def = false;
  } else {
    def = true;
    for (z = 0; z < count; z++) {
      if (!defaultAccels[z].equals(accels[z].keyCode1, accels[z].keyCode2)) {
        def = false;
        break;
      }
    }
  }
*/

  if (def) {
    //default
    //is there a possibility to remove a key?
    if (config->hasKey(key)) config->writeEntry(key, "(Default)");
  } else {
    //not default
    QString s;

    for (z = 0; z < count; z++) {
      if (z != 0) s += ';';
      s += keyToString(m_accels[z].keyCode1);
      if (m_accels[z].keyCode2 != 0) {
        s += ',';
        s += keyToString(m_accels[z].keyCode2);
      }
    }
    config->writeEntry(key, s);
  }
}


void KWCommand::emitAccelString() {
  emit changed(accelString());
}

QString KWCommand::accelString(KWAccel &accel) {
  QString res;

  if (accel.keyCode1 != 0) {
    res = keyToLanguage(accel.keyCode1);

    if (accel.keyCode2 != 0) {
      res += ", ";
      res += keyToLanguage(accel.keyCode2);
    }
  }
  return res;
}

void KWCommand::matchKey(int keyCode, int modifiers, KWAccelMatch &match) {
  int z;
  KWAccel *a;
  bool second;
  int kc, m;

  for (z = 0; z < KWCommand::nAccels; z++) {
    a = &m_accels[z];
    if (a->keyCode1 == 0) break;

    second = m_second[z];
    m_second[z] = false;
    kc = (second) ? a->keyCode2 : a->keyCode1;
    m = kc & modifierMask; // modifiers to match
    kc &= ~modifierMask;   // keycode to match

    // second keypress has priority, match keycode without modifiers
    if ((second || !match.second) && (kc == keyCode)) {
      bool toSecond, b;
      int unmatchedModifiers, id;

      toSecond = !second && a->keyCode2 != 0;
      b = (m == modifiers); // match modifiers

      if (toSecond) { 
        // accel switches to second keypress 
        if (b) { 
          m_second[z] = true;
          match.consumeKeyEvent = true;
        }
      } else {
        // single-key accel or second keypress
        unmatchedModifiers = modifiers & ~m;
        id = 0;
        if (!b) {
          // if the modifiers don't match try to match them with the select modifiers
          b = (m | m_group->m_selectModifiers) == modifiers;
          id = m_group->m_selectFlag;
        }
        if (!b) {
          b = (m | m_group->m_selectModifiers2) == modifiers;
          id = m_group->m_selectFlag2;
        }
        id |= m_id;
        // a key press with less unmatched modifiers has priority
        if (b && (unmatchedModifiers & ~match.unmatchedModifiers) == 0) {
          match.unmatchedModifiers = unmatchedModifiers;
          match.id = id;
          match.command = this;
          match.group = m_group;
          match.consumeKeyEvent = true;
          match.second = second;
        }
      }
    }
  }
}


// KWCommandGroup

KWCommandGroup::KWCommandGroup(const QString &name)
  : m_name(name), m_selectModifiers(0), m_selectFlag(0),
  m_selectModifiers2(0), m_selectFlag2(0) {

  m_commandList.setAutoDelete(true);
}

void KWCommandGroup::setSelectModifiers(int selectModifiers, int selectFlag,
  int selectModifiers2, int selectFlag2) {

  m_selectModifiers = selectModifiers;
  m_selectFlag = selectFlag;
  m_selectModifiers2 = selectModifiers2;
  m_selectFlag2 = selectFlag2;
}
/*
void KWCommandGroup::addCommand(KWCommand *command) {
  m_commandList.append(command);
}
*/
KWCommand *KWCommandGroup::addCommand(int id, const QString &name, int keyCode01,
  int keyCode11, int keyCode21) {
  
  KWCommand *command = new KWCommand(id, name, this);
  command->addAccel(keyCode01, 0);
  command->addAccel(keyCode11, 0);
  command->addAccel(keyCode21, 0);
  m_commandList.append(command);
  return command;
}

void KWCommandGroup::getData(KWCommandGroupData &data) {
  int z;
  int count = m_commandList.count();
  KWCommandData *commandList = new KWCommandData[count];
  
  for (z = 0; z < count; z++) {
    m_commandList.at(z)->getData(commandList[z]);
  }
  data.m_name = m_name;
  data.m_commandList = commandList;
  data.m_count = count;
}

void KWCommandGroup::setData(const KWCommandGroupData &data) {
  int z;
  
  for (z = 0; z < data.m_count; z++) {
    m_commandList.at(z)->setData(data.m_commandList[z]);
  }
}

void KWCommandGroup::readConfig(KConfig *config) {
  int z;
  config->setGroup(m_name);
  for (z = 0; z < (int) m_commandList.count(); z++) {
    m_commandList.at(z)->readConfig(config);
  }
}

void KWCommandGroup::writeConfig(KConfig *config) {
  int z;
  config->setGroup(m_name);
  for (z = 0; z < (int) m_commandList.count(); z++) {
    m_commandList.at(z)->writeConfig(config);
  }
}

void KWCommandGroup::matchKey(int keyCode, int modifiers, KWAccelMatch &match) {
  int z;
  for (z = 0; z < (int) m_commandList.count(); z++) {
    m_commandList.at(z)->matchKey(keyCode, modifiers, match);
  }
}


// KWCommandDispatcher

KWCommandDispatcher::KWCommandDispatcher(QObject *host) {
  host->installEventFilter(this);
//  connect(host, SIGNAL(destroyed()), this, SLOT(destroy()));
  m_groupList.setAutoDelete(true);
  m_enabled = true;
}
/*
void KWCommandDispatcher::addGroup(KWCommandGroup *group) {
  m_groupList.append(group);
}
*/
KWCommandGroup *KWCommandDispatcher::addGroup(const QString &name) {
  KWCommandGroup *group = new KWCommandGroup(name);
  m_groupList.append(group);
  return group;
}

void KWCommandDispatcher::getData(KWKeyData &data) {
  int z;
  int count = m_groupList.count();
  KWCommandGroupData *groupList = new KWCommandGroupData[count];
  
  for (z = 0; z < count; z++) {
    m_groupList.at(z)->getData(groupList[z]);
  }
  data.m_groupList = groupList;
  data.m_count = count;
}

void KWCommandDispatcher::setData(const KWKeyData &data) {
  int z;
  
  for (z = 0; z < data.m_count; z++) {
    m_groupList.at(z)->setData(data.m_groupList[z]);
  }
}

void KWCommandDispatcher::readConfig(KConfig *config) {
  int z;
  for (z = 0; z < (int) m_groupList.count(); z++) {
    m_groupList.at(z)->readConfig(config);
  }
}

void KWCommandDispatcher::writeConfig(KConfig *config) {
  int z;
  for (z = 0; z < (int) m_groupList.count(); z++) {
    m_groupList.at(z)->writeConfig(config);
  }
}

static int convertModifiers(Qt::ButtonState state) {
  int r = 0;
  
  if (state & Qt::ShiftButton  ) r = SHIFT;
  if (state & Qt::ControlButton) r |= CTRL;
  if (state & Qt::AltButton    ) r |= ALT;
  return r;
}

bool KWCommandDispatcher::eventFilter(QObject *, QEvent *e) {

  if (m_enabled && e->type() == QEvent::KeyPress/*Accel*/) {    // key press
    int keyCode, modifiers, z;
    KWAccelMatch match;
//printf("eventFilter\n");
    keyCode = ((QKeyEvent *) e)->key();
    if (isSpecialKey(keyCode) || modifier2Flag(keyCode)) return false;
    modifiers = convertModifiers(((QKeyEvent *) e)->state()); //globalModifiers;

    match.unmatchedModifiers = -1;
    match.consumeKeyEvent = false;
    match.second = false;
//  printf("Event Filter %d %d\n", keyCode, modifiers);
    for (z = 0; z < (int) m_groupList.count(); z++) {
      m_groupList.at(z)->matchKey(keyCode, modifiers, match);
    }
    
    if (match.consumeKeyEvent) {
      if (match.unmatchedModifiers != -1) {
        emit match.command->activated(match.id);
        emit match.group->activated(match.id);
      }	
      ((QKeyEvent *) e)->accept();
      return true;
    }
  }
  return false;
}



// KWCommandData

int KWCommandData::accelCount() {
  int z;

  for (z = 0; z < KWCommand::nAccels; z++) {
    if (m_accels[z].keyCode1 == 0) break;
  }
  return z;
}

int KWCommandData::containsAccel(int keyCode1, int keyCode2) {
  int z;

  for (z = 0; z < KWCommand::nAccels; z++) {
    if (m_accels[z].keyCode1 == 0) break;
    if (m_accels[z].keyCode1 == keyCode1 && (m_accels[z].keyCode2 == keyCode2
      || m_accels[z].keyCode2 == 0 || keyCode2 == 0)) {

      return z;
    }
  }
  return -1;
}

void KWCommandData::addAccel(int keyCode1, int keyCode2) {
  int z;

  for (z = 0; z < KWCommand::nAccels; z++) {
    if (m_accels[z].keyCode1 == 0) {
      m_accels[z].keyCode1 = keyCode1;
      m_accels[z].keyCode2 = keyCode2;
      break;
    }
  }
}

void KWCommandData::removeAccel(int z) {
  while (z < KWCommand::nAccels - 1) {
    m_accels[z] = m_accels[z + 1];
    z++;
  }
  m_accels[z].keyCode1 = 0;
}

void KWCommandData::removeAccel(int keyCode1, int keyCode2) {
  int n;

  while ((n = containsAccel(keyCode1, keyCode2)) >= 0) {
    removeAccel(n);
  }
}

/*
void KWCommandData::setData(const QString &name, KWAccel *accels) {
  int z;
  
  m_name = name;
  for (z = 0; z < KWCommand::nAccels; z++) {
    m_accels[z] = accels[z];
  }
}
*/


// KWCommandGroupData

KWCommandGroupData::~KWCommandGroupData() {
  delete[] m_commandList;
}

KWCommandData *KWCommandGroupData::containsAccel(int keyCode1, int keyCode2) {
  int z;
  KWCommandData *command;

  for (z = 0; z < m_count; z++) {
    command = &m_commandList[z];
    if (command->containsAccel(keyCode1, keyCode2) != -1) return command;
  }
  return 0L;

}

void KWCommandGroupData::removeAccel(int keyCode1, int keyCode2) {
  int z;

  for (z = 0; z < m_count; z++) {
    m_commandList[z].removeAccel(keyCode1, keyCode2);
  }
}
/*
void KWCommandGroupData::setData(const QString &name, KWCommandData *commandList,
  int count) {

  m_name = name;
  m_commandList = commandList;
  m_count = count;
}
*/

// KWKeyData

KWKeyData::~KWKeyData() {
  delete[] m_groupList;
}

void KWKeyData::containsAccel(int keyCode1, int keyCode2, Contains &contains) {
  int z;
  
  for (z = 0; z < m_count; z++) {
    contains.group = &m_groupList[z];
    contains.command = contains.group->containsAccel(keyCode1, keyCode2);
    if (contains.command != 0L) return;
  }
  contains.command = 0L;
}

void KWKeyData::removeAccel(int keyCode1, int keyCode2) {
  int z;

  for (z = 0; z < m_count; z++) {
    m_groupList[z].removeAccel(keyCode1, keyCode2);
  }
}
/*
void KWKeyData::setData(KWCommandGroupData *groupList, int count) {
  m_groupList = groupList;
  m_count = count;
}
*/


// KAccelInput

KAccelInput::KAccelInput(QWidget *parent) : QLineEdit(parent) {
  pos = 0;
  keyCodes[0] = keyCodes[1] = 0;
}

int KAccelInput::getKeyCode1() {
  return keyCodes[0];
}

int KAccelInput::getKeyCode2() {
  return keyCodes[1];
}

void KAccelInput::clear() {
  //clear input line
  pos = 0;
  keyCodes[0] = keyCodes[1] = 0;
  setText("");
  emit valid(false);
  emit changed();
}
/*
void KAccelInput::removeModifiers() {

  keyCodes[0] &= ~modifierMask;
  keyCodes[1] &= ~modifierMask;
  setContents();
  emit changed();
}
*/
void KAccelInput::setContents() {
  QString s = keyToLanguage(keyCodes[0]);
  if (keyCodes[1] != 0) {
    s += ", ";
    s += keyToLanguage(keyCodes[1]);
  }
  setText(s);
}

void KAccelInput::keyPressEvent(QKeyEvent *e) {
  int keyCode, modifiers, m;

  keyCode = e->key();
  if (isSpecialKey(keyCode)) return;
  if ((pos != 0 && keyCode == Key_Backspace) || pos >= 2) {
    clear();
    if (keyCode == Key_Backspace) return;
  }

  modifiers = convertModifiers(e->state());//globalModifiers;
  m = modifier2Flag(keyCode);
  if (m) {
    //modifier pressed
    keyCodes[pos] = modifiers |= m;
  } else {
    //key pressed
    keyCodes[pos] = keyCode | modifiers;
    if (pos == 0) emit valid(true);
    emit changed();
    pos++;
  }

  //set contents of input line
  setContents();
}

void KAccelInput::keyReleaseEvent(QKeyEvent *e) {
  int m;

  //remove modifier flags on release
  m = modifier2Flag(e->key());
  if (m && pos < 2) {
    keyCodes[pos] &= ~m;
    setContents();
  }
}


KWKeyConfigTab::KWKeyConfigTab(QWidget *parent, KWKeyData &keyData,
  const char *name) : QWidget(parent, name), m_keyData(keyData) {
  int z;
  QRect r;
  QComboBox *combo;
  QLabel *label;
  QButton *button;


  combo = new QComboBox(false, this); //read only combo
  label = new QLabel(combo, i18n("Group:"), this);
  connect(combo, SIGNAL(activated(int)), SLOT(groupSelected(int)));
  r.setRect(10, 5, 150, 25);
  label->setGeometry(r);
  r.moveBy(0, 25);
  combo->setGeometry(r);

  for (z = 0; z < keyData.groupCount(); z++) {
    combo->insertItem(i18n(m_keyData.group(z)->name()));
  }

  m_commandList = new QListBox(this);
  label = new QLabel(m_commandList, i18n("Commands:"), this);
  connect(m_commandList, SIGNAL(highlighted(int)), SLOT(commandSelected(int)));
  r.moveBy(0, 25);
  label->setGeometry(r);
  r.moveBy(0, 25);
  r.setHeight(185);
  m_commandList->setGeometry(r);

  m_accelList = new QListBox(this);
  label = new QLabel(m_accelList, i18n("Current Keys:"), this);
  connect(m_accelList, SIGNAL(highlighted(int)), SLOT(accelSelected(int)));
  r.setRect(r.right() +10, 5, 180, 25);
  label->setGeometry(r);
  r.moveBy(0, 25);
  r.setHeight(85);
  m_accelList->setGeometry(r);

  m_accelInput = new KAccelInput(this);
  label = new QLabel(m_accelInput, i18n("Press New Shortcut Key:"), this);
  r.setRect(r.x(), r.bottom() +5, r.width(), 25);
  label->setGeometry(r);
  r.moveBy(0, 25);
  m_accelInput->setGeometry(r);

  m_currentBinding = new QLabel(this);
  r.setRect(r.x(), r.bottom() +5, r.width(), 60);
  m_currentBinding->setGeometry(r);
/*
  button = new QPushButton(i18n("OK"), this);
  connect(button, SIGNAL(clicked()), this, SLOT(assign()));
  connect(button, SIGNAL(clicked()), this, SLOT(accept()));
  r.setRect(r.x(), r.bottom() + 10, 80, 25);
  button->setGeometry(r);

  button = new QPushButton(i18n("Cancel"), this);
  connect(button, SIGNAL(clicked()), this, SLOT(reject()));
  r.moveBy(100, 0);
  button->setGeometry(r);
*/
  m_assignButton = new QPushButton(i18n("Assign"), this);
  m_assignButton->setEnabled(false);
  connect(m_accelInput, SIGNAL(valid(bool)), m_assignButton, SLOT(setEnabled(bool)));
  connect(m_accelInput, SIGNAL(changed()), this, SLOT(accelChanged()));
  connect(m_assignButton, SIGNAL(clicked()), this, SLOT(assign()));
  r.setRect(r.right() +10, 30, 80, 25);
  m_assignButton->setGeometry(r);

  m_removeButton = new QPushButton(i18n("Remove"), this);
  m_removeButton->setEnabled(false);
  connect(m_removeButton, SIGNAL(clicked()), this, SLOT(remove()));
  r.moveBy(0, 30);
  m_removeButton->setGeometry(r);

  button = new QPushButton(i18n("Defaults"), this);
  connect(button, SIGNAL(clicked()), this, SLOT(defaults()));
  r.moveBy(0, 30);
  button->setGeometry(r);

  groupSelected(0);
}

void KWKeyConfigTab::updateAccelList() {
  int z, count, current;

  current = m_accelList->currentItem();
  m_accelList->clear();
  count = m_command->accelCount();
  for (z = 0; z < count; z++) {
    m_accelList->insertItem(m_command->accelString(z));
  }

//  count = m_accelList->count();
  if (current >= count) current = count -1;
  if (current >= 0) m_accelList->setCurrentItem(current);
    else m_removeButton->setEnabled(false);
}

void KWKeyConfigTab::groupSelected(int n) {
  int z;

  m_group = m_keyData.group(n);
  m_commandList->clear();
  for (z = 0; z < m_group->commandCount(); z++) {
    m_commandList->insertItem(removeAnd(i18n(m_group->command(z)->name())));
  }
  commandSelected(0);
}

void KWKeyConfigTab::commandSelected(int n) {
  m_command = m_group->command(n);
  updateAccelList();
}

void KWKeyConfigTab::accelSelected(int) {
  m_removeButton->setEnabled(true);
}

void KWKeyConfigTab::accelChanged() {
  KWKeyData::Contains contains;

  m_keyData.containsAccel(m_accelInput->getKeyCode1(),
    m_accelInput->getKeyCode2(), contains);
  if (contains.command != 0L) {
    QString s;

//    s.sprintf(i18n("Key Binding used in:\n%s\n%s"),
//      m_command->getGroupName(), removeAnd(m_command->getName()).data());
//    m_currentBinding->setText(s);
    m_currentBinding->setText(i18n("Key Binding used in:\n%1\n%2")
      .arg(contains.group->name())
      .arg(removeAnd(i18n(contains.command->name()))));
  } else {
//    if (m_accelInput->getKeyCode1() == Key_Escape+META+ALT+CTRL+SHIFT)
//      m_currentBinding->setText(i18n("Hi Emacs Fan! ;-)"));
//      else 
     m_currentBinding->setText("");
  }
}

void KWKeyConfigTab::assign() {

  m_keyData.removeAccel(m_accelInput->getKeyCode1(),
    m_accelInput->getKeyCode2());
  m_command->addAccel(m_accelInput->getKeyCode1(), m_accelInput->getKeyCode2());
  m_accelInput->clear();
  updateAccelList();
}

void KWKeyConfigTab::remove() {
  int n;

  n = m_accelList->currentItem();
  if (n >= 0) m_command->removeAccel(n);
  updateAccelList();
}

void KWKeyConfigTab::defaults() {
//  m_keyData.restoreDefault();
  updateAccelList();
}

