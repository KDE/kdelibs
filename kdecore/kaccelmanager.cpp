/*  This file is part of the KDE project
    Copyright (C) 2002 Matthias H�lzer-Kl�pfel <mhk@kde.org>

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


#include <qwidget.h>
#include <qobjectlist.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qmemarray.h>
#include <qmainwindow.h>
#include <qtabbar.h>
#include <qwidgetstack.h>
#include <qlabel.h>
#include <qptrlist.h>
#include <qmetaobject.h>
#include <kstdaction.h>
#include <kstaticdeleter.h>
#include <kdebug.h>


#include "kaccelmanager_private.h"
#include "../kdeui/kstdaction_p.h"

#include "kaccelmanager.h"

// Default control weight
const int KAccelManagerAlgorithm::DEFAULT_WEIGHT = 50;
// Additional weight for first character in string
const int KAccelManagerAlgorithm::FIRST_CHARACTER_EXTRA_WEIGHT = 50;
// Additional weight for the beginning of a word
const int KAccelManagerAlgorithm::WORD_BEGINNING_EXTRA_WEIGHT = 50;
// Additional weight for the dialog buttons (large, we basically never want these reassigned)
const int KAccelManagerAlgorithm::DIALOG_BUTTON_EXTRA_WEIGHT = 300;
// Additional weight for a 'wanted' accelerator
const int KAccelManagerAlgorithm::WANTED_ACCEL_EXTRA_WEIGHT = 150;
// Default weight for an 'action' widget (ie, pushbuttons)
const int KAccelManagerAlgorithm::ACTION_ELEMENT_WEIGHT = 50;
// Default weight for group boxes (low priority)
const int KAccelManagerAlgorithm::GROUP_BOX_WEIGHT = 0;
// Default weight for menu titles
const int KAccelManagerAlgorithm::MENU_TITLE_WEIGHT = 250;
// Additional weight for KDE standard accelerators
const int KAccelManagerAlgorithm::STANDARD_ACCEL = 300;

/*********************************************************************

 class Item - helper class containing widget information

 This class stores information about the widgets the need accelerators,
 as well as about their relationship.

 *********************************************************************/



/*********************************************************************

 class KAcceleratorManagerPrivate - internal helper class

 This class does all the work to find accelerators for a hierarchy of
 widgets.

 *********************************************************************/


class KAcceleratorManagerPrivate
{
public:

    static void manage(QWidget *widget);
    static bool programmers_mode;
    static bool standardName(const QString &str);

    static bool checkChange(const KAccelString &as)  {
        QString t2 = as.accelerated();
        QString t1 = as.originalText();
        if (t1 != t2)
        {
            if (as.accel() == -1)  {
                removed_string  += "<tr><td>" + t1 + "</td></tr>";
            } else if (as.originalAccel() == -1) {
                added_string += "<tr><td>" + t2 + "</td></tr>";
            } else {
                changed_string += "<tr><td>" + t1 + "</td>";
                changed_string += "<td>" + t2 + "</td></tr>";
            }
            return true;
        }
        return false;
    }
    static QString changed_string;
    static QString added_string;
    static QString removed_string;

private:
  class Item;
  typedef QPtrList<Item> ItemList;


  static void traverseChildren(QWidget *widget, Item *item);

  static void manageWidget(QWidget *widget, Item *item);
  static void manageMenuBar(QMenuBar *mbar, Item *item);
  static void manageTabBar(QTabBar *bar, Item *item);

  static void calculateAccelerators(Item *item, QString &used);

  class Item
  {
  public:

    Item() : m_widget(0), m_children(0), m_index(-1) {};
    ~Item();

    void addChild(Item *item);

    QWidget       *m_widget;
    KAccelString  m_content;
    ItemList      *m_children;
    int           m_index;

  };
};


bool KAcceleratorManagerPrivate::programmers_mode = false;
QString KAcceleratorManagerPrivate::changed_string;
QString KAcceleratorManagerPrivate::added_string;
QString KAcceleratorManagerPrivate::removed_string;
static QStringList *kaccmp_sns = 0;
static KStaticDeleter<QStringList> kaccmp_sns_d;

bool KAcceleratorManagerPrivate::standardName(const QString &str)
{
    if (!kaccmp_sns)
        kaccmp_sns_d.setObject(kaccmp_sns, new QStringList(KStdAction::internal_stdNames()));
        return kaccmp_sns->contains(str);
}

KAcceleratorManagerPrivate::Item::~Item()
{
    delete m_children;
}


void KAcceleratorManagerPrivate::Item::addChild(Item *item)
{
    if (!m_children) {
        m_children = new ItemList;
	m_children->setAutoDelete(true);
    }

    m_children->append(item);
}

void KAcceleratorManagerPrivate::manage(QWidget *widget)
{
    if (widget->inherits("QPopupMenu"))
    {
        // create a popup accel manager that can deal with dynamic menus
        KPopupAccelManager::manage(static_cast<QPopupMenu*>(widget));
        return;
    }

    Item *root = new Item;

    manageWidget(widget, root);

    QString used;
    calculateAccelerators(root, used);
    delete root;
}


void KAcceleratorManagerPrivate::calculateAccelerators(Item *item, QString &used)
{
    if (!item->m_children)
        return;

    // collect the contents
    KAccelStringList contents;
    for (Item *it = item->m_children->first(); it != 0;
         it = item->m_children->next())
    {
        contents << it->m_content;
    }

    // find the right accelerators
    KAccelManagerAlgorithm::findAccelerators(contents, used);

    // write them back into the widgets
    int cnt = -1;
    for (Item *it = item->m_children->first(); it != 0;
         it = item->m_children->next())
    {
        cnt++;

        if (it->m_widget->inherits("QTabBar"))
        {
            QTabBar *bar = static_cast<QTabBar*>(it->m_widget);
            if (checkChange(contents[cnt]))
                bar->tabAt(it->m_index)->setText(contents[cnt].accelerated());
            continue;
        }
        if (it->m_widget->inherits("QMenuBar"))
        {
            QMenuBar *bar = static_cast<QMenuBar*>(it->m_widget);
            if (it->m_index >= 0)
            {
                QMenuItem *mitem = bar->findItem(bar->idAt(it->m_index));
                if (mitem)
                {
                    checkChange(contents[cnt]);
                    mitem->setText(contents[cnt].accelerated());
                }
                continue;
            }
        }
        int tprop = it->m_widget->metaObject()->findProperty("text", true);
        if (tprop != -1)  {
            if (checkChange(contents[cnt]))
                it->m_widget->setProperty("text", contents[cnt].accelerated());
        } else {
            tprop = it->m_widget->metaObject()->findProperty("title", true);
            if (tprop != -1 && checkChange(contents[cnt]))
                it->m_widget->setProperty("title", contents[cnt].accelerated());
        }
    }

    // calculate the accelerators for the children
    for (Item *it = item->m_children->first(); it != 0;
         it = item->m_children->next())
    {
        if (it->m_widget && it->m_widget->isVisibleTo( item->m_widget ))
            calculateAccelerators(it, used);
    }
}


void KAcceleratorManagerPrivate::traverseChildren(QWidget *widget, Item *item)
{
  QObjectList *childList = widget->queryList("QWidget", 0, false, false);
  for ( QObject *it = childList->first(); it; it = childList->next() )
  {
    QWidget *w = static_cast<QWidget*>(it);

    if ( !w->isVisibleTo( widget ) )
        continue;

    manageWidget(w, item);
  }
  delete childList;
}

void KAcceleratorManagerPrivate::manageWidget(QWidget *w, Item *item)
{
  // first treat the special cases

  if (w->inherits("QTabBar"))
  {
      manageTabBar(static_cast<QTabBar*>(w), item);
      return;
  }
  
  if (w->inherits("QPopupMenu"))
  {
      // create a popup accel manager that can deal with dynamic menus
      KPopupAccelManager::manage(static_cast<QPopupMenu*>(w));
      return;
  }

  if (w->inherits("QMenuBar"))
  {
      manageMenuBar(static_cast<QMenuBar*>(w), item);
      return;
  }

  if (w->inherits("QComboBox") || w->inherits("QLineEdit") ||
      w->inherits("QTextEdit") || w->inherits("QTextView") ||
      w->inherits("QSpinBox"))
      return;

  // now treat 'ordinary' widgets
  if (w->isFocusEnabled() || (w->inherits("QLabel") && static_cast<QLabel*>(w)->buddy()) || w->inherits("QGroupBox"))
  {
    QString content;
    QVariant variant;
    int tprop = w->metaObject()->findProperty("text", true);
    if (tprop != -1)  {
        const QMetaProperty* p = w->metaObject()->property( tprop, true );
        if ( p && p->isValid() )
            w->qt_property( tprop, 1, &variant );
        else
            tprop = -1;
    }

    if (tprop == -1)  {
        tprop = w->metaObject()->findProperty("title", true);
        if (tprop != -1)  {
            const QMetaProperty* p = w->metaObject()->property( tprop, true );
            if ( p && p->isValid() )
                w->qt_property( tprop, 1, &variant );
        }
    }

    if (variant.isValid())
        content = variant.toString();

    if (!content.isEmpty())
    {
        Item *i = new Item;
        i->m_widget = w;

        // put some more weight on the usual action elements
        int weight = KAccelManagerAlgorithm::DEFAULT_WEIGHT;
        if (w->inherits("QPushButton") || w->inherits("QCheckBox") || w->inherits("QRadioButton") || w->inherits("QLabel"))
            weight = KAccelManagerAlgorithm::ACTION_ELEMENT_WEIGHT;

        // don't put weight on group boxes, as usually the contents are more important
        if (w->inherits("QGroupBox"))
            weight = KAccelManagerAlgorithm::GROUP_BOX_WEIGHT;

        // put a lot of extra weight on the KDialogBaseButton's
        if (w->inherits("KDialogBaseButton"))
            weight += KAccelManagerAlgorithm::DIALOG_BUTTON_EXTRA_WEIGHT;

        i->m_content = KAccelString(content, weight);
        item->addChild(i);
    }
  }
  traverseChildren(w, item);
}

void KAcceleratorManagerPrivate::manageTabBar(QTabBar *bar, Item *item)
{
  for (int i=0; i<bar->count(); i++)
  {
    QString content = bar->tabAt(i)->text();
    if (content.isEmpty())
      continue;

    Item *it = new Item;
    item->addChild(it);
    it->m_widget = bar;
    it->m_index = i;
    it->m_content = KAccelString(content);
  }
}


void KAcceleratorManagerPrivate::manageMenuBar(QMenuBar *mbar, Item *item)
{
    QMenuItem *mitem;
    QString s;

    for (uint i=0; i<mbar->count(); ++i)
    {
        mitem = mbar->findItem(mbar->idAt(i));
        if (!mitem)
            continue;

        // nothing to do for separators
        if (mitem->isSeparator())
            continue;

        s = mitem->text();
        if (!s.isEmpty())
        {
            Item *it = new Item;
            item->addChild(it);
            it->m_content =
                KAccelString(s,
                             // menu titles are important, so raise the weight
                             KAccelManagerAlgorithm::MENU_TITLE_WEIGHT);

            it->m_widget = mbar;
            it->m_index = i;
        }

        // have a look at the popup as well, if present
        if (mitem->popup())
            KPopupAccelManager::manage(mitem->popup());
    }
}


/*********************************************************************

 class KAcceleratorManager - main entry point

 This class is just here to provide a clean public API...

 *********************************************************************/

void KAcceleratorManager::manage(QWidget *widget)
{
    KAcceleratorManager::manage(widget, false);
}

void KAcceleratorManager::manage(QWidget *widget, bool programmers_mode)
{
    KAcceleratorManagerPrivate::changed_string = QString::null;
    KAcceleratorManagerPrivate::added_string = QString::null;
    KAcceleratorManagerPrivate::removed_string = QString::null;
    KAcceleratorManagerPrivate::programmers_mode = programmers_mode;
    KAcceleratorManagerPrivate::manage(widget);
}

void KAcceleratorManager::last_manage(QString &added,  QString &changed, QString &removed)
{
    added = KAcceleratorManagerPrivate::added_string;
    changed = KAcceleratorManagerPrivate::changed_string;
    removed = KAcceleratorManagerPrivate::removed_string;
}


/*********************************************************************

 class KAccelString - a string with weighted characters

 *********************************************************************/

KAccelString::KAccelString(const QString &input, int initialWeight)
  : m_pureText(input), m_weight()
{
    if (m_pureText.contains('\t'))
        m_pureText = m_pureText.left(m_pureText.find('\t'));
    m_origText = m_pureText;
    m_orig_accel = m_pureText.find("(!)&");
    m_pureText.replace(m_orig_accel, 4, "");
    m_orig_accel = m_pureText.find("(&&)");
    if (m_orig_accel != -1)
        m_pureText.replace(m_orig_accel, 4, "&");
    m_orig_accel = m_accel = stripAccelerator(m_pureText);

    kdDebug(125) << input << " " << m_orig_accel << " " << m_accel << " " << m_pureText << endl;
    if (initialWeight == -1)
        initialWeight = KAccelManagerAlgorithm::DEFAULT_WEIGHT;

    calculateWeights(initialWeight);

    // dump();
}


QString KAccelString::accelerated() const
{
  QString result = m_pureText;
  if (result.isEmpty())
      return result;

  if (KAcceleratorManagerPrivate::programmers_mode)
  {
      int oa = m_orig_accel;

      if (m_accel >= 0) {
          if (m_accel != m_orig_accel) {
              result.insert(m_accel, "(!)&");
              if (m_accel < m_orig_accel)
                  oa += 4;
          } else {
              result.insert(m_accel, "&");
              if (m_accel < m_orig_accel)
                  oa++;
          }
      }

      if (m_accel != m_orig_accel && m_orig_accel >= 0)
          result.insert(oa, "(&&)");
  } else {
    if (m_accel >= 0)
        result.insert(m_accel, "&");
  }
  return result;
}


QChar KAccelString::accelerator() const
{
  if ((m_accel < 0) || (m_accel > (int)m_pureText.length()))
    return QChar();

  return m_pureText[m_accel].lower();
}


void KAccelString::calculateWeights(int initialWeight)
{
  m_weight.resize(m_pureText.length());

  uint pos = 0;
  bool start_character = true;

  while (pos<m_pureText.length())
  {
    QChar c = m_pureText[pos];

    int weight = initialWeight+1;

    // add special weight to first character
    if (pos == 0)
      weight += KAccelManagerAlgorithm::FIRST_CHARACTER_EXTRA_WEIGHT;

    // add weight to word beginnings
    if (start_character)
    {
      weight += KAccelManagerAlgorithm::WORD_BEGINNING_EXTRA_WEIGHT;
      start_character = false;
    }

    // add decreasing weight to left characters
    if (pos < 50)
      weight += (50-pos);

    // try to preserve the wanted accelarators
    if ((int)pos == accel()) {
        weight += KAccelManagerAlgorithm::WANTED_ACCEL_EXTRA_WEIGHT;
        // kdDebug() << "wanted " << m_pureText << " " << KAcceleratorManagerPrivate::standardName(m_origText) << endl;
        if (KAcceleratorManagerPrivate::standardName(m_origText))  {
            weight += KAccelManagerAlgorithm::STANDARD_ACCEL;
        }
    }

    // skip non typeable characters
    if (!c.isLetterOrNumber())
    {
      weight = 0;
      start_character = true;
    }

    m_weight[pos] = weight;

    ++pos;
  }
}


int KAccelString::stripAccelerator(QString &text)
{
  // Note: this code is derived from QAccel::shortcutKey
  int p = 0;

  while (p >= 0)
  {
    p = text.find('&', p)+1;

    if (p <= 0 || p >= (int)text.length())
      return -1;

    if (text[p] != '&')
    {
      QChar c = text[p];
      if (c.isPrint())
      {
        text.remove(p-1,1);
	return p-1;
      }
    }

    p++;
  }

  return -1;
}


int KAccelString::maxWeight(int &index, const QString &used)
{
  int max = 0;
  index = -1;

  for (uint pos=0; pos<m_pureText.length(); ++pos)
    if (used.find(m_pureText[pos], 0, FALSE) == -1 && m_pureText[pos].latin1() != 0)
      if (m_weight[pos] > max)
      {
        max = m_weight[pos];
	index = pos;
      }

  return max;
}


void KAccelString::dump()
{
  QString s;
  for (uint i=0; i<m_weight.count(); ++i)
    s += QString("%1(%2) ").arg(pure()[i]).arg(m_weight[i]);
  kdDebug() << "s " << s << endl;
}


/*********************************************************************

 findAccelerators - the algorithm determining the new accelerators

 The algorithm is very crude:

   * each character in each widget text is assigned a weight
   * the character with the highest weight over all is picked
   * that widget is removed from the list
   * the weights are recalculated
   * the process is repeated until no more accelerators can be found

 The algorithm has some advantages:

   * it favors 'nice' accelerators (first characters in a word, etc.)
   * it is quite fast, O(N�)
   * it is easy to understand :-)

 The disadvantages:

   * it does not try to find as many accelerators as possible

 TODO:

 * The result is always correct, but not neccesarily optimal. Perhaps
   it would be a good idea to add another algorithm with higher complexity
   that gets used when this one fails, i.e. leaves widgets without
   accelerators.

 * The weights probably need some tweaking so they make more sense.

 *********************************************************************/

void KAccelManagerAlgorithm::findAccelerators(KAccelStringList &result, QString &used)
{
  KAccelStringList accel_strings = result;

  // initally remove all accelerators
  for (KAccelStringList::Iterator it = result.begin(); it != result.end(); ++it)
    (*it).setAccel(-1);

  // pick the highest bids
  for (uint cnt=0; cnt<accel_strings.count(); ++cnt)
  {
    int max = 0, index = -1, accel = -1;

    // find maximum weight
    for (uint i=0; i<accel_strings.count(); ++i)
    {
      int a;
      int m = accel_strings[i].maxWeight(a, used);
      if (m>max)
      {
        max = m;
        index = i;
        accel = a;
      }
    }

    // stop if no more accelerators can be found
    if (index < 0)
      return;

    // insert the accelerator
    if (accel >= 0)
    {
      result[index].setAccel(accel);
      used.append(result[index].accelerator());
    }

    // make sure we don't visit this one again
    accel_strings[index] = KAccelString();
  }
}


/*********************************************************************

 class KPopupAccelManager - managing QPopupMenu widgets dynamically

 *********************************************************************/

KPopupAccelManager::KPopupAccelManager(QPopupMenu *popup)
  : QObject(popup), m_popup(popup), m_count(-1)
{
    aboutToShow(); // do one check and then connect to show
    connect(popup, SIGNAL(aboutToShow()), SLOT(aboutToShow()));
}


void KPopupAccelManager::aboutToShow()
{
  // Note: we try to be smart and avoid recalculating the accelerators
  // whenever possible. Unfortunately, there is no way to know if an
  // item has been added or removed, so we can not do much more than
  // to compare the items each time the menu is shown :-(

  if (m_count != (int)m_popup->count())
  {
    findMenuEntries(m_entries);
    calculateAccelerators();
    m_count = m_popup->count();
  }
  else
  {
    KAccelStringList entries;
    findMenuEntries(entries);
    if (entries != m_entries)
    {
      m_entries = entries;
      calculateAccelerators();
    }
  }
}


void KPopupAccelManager::calculateAccelerators()
{
  // find the new accelerators
  QString used;
  KAccelManagerAlgorithm::findAccelerators(m_entries, used);

  // change the menu entries
  setMenuEntries(m_entries);
}


void KPopupAccelManager::findMenuEntries(KAccelStringList &list)
{
  QMenuItem *mitem;
  QString s;

  list.clear();

  // read out the menu entries
  for (uint i=0; i<m_popup->count(); i++)
  {
    mitem = m_popup->findItem(m_popup->idAt(i));
    if (mitem->isSeparator())
      continue;

    s = mitem->text();

    // in full menus, look at entries with global accelerators last
    int weight = 50;
    if (s.contains('\t'))
        weight = 0;

    list.append(KAccelString(s, weight));

    // have a look at the popup as well, if present
    if (mitem->popup())
        KPopupAccelManager::manage(mitem->popup());
  }
}


void KPopupAccelManager::setMenuEntries(const KAccelStringList &list)
{
  QMenuItem *mitem;

  uint cnt = 0;
  for (uint i=0; i<m_popup->count(); i++)
  {
    mitem = m_popup->findItem(m_popup->idAt(i));
    if (mitem->isSeparator())
      continue;

    if (KAcceleratorManagerPrivate::checkChange(list[cnt]))
        mitem->setText(list[cnt].accelerated());
    cnt++;
  }
}


void KPopupAccelManager::manage(QPopupMenu *popup)
{
  // don't add more than one manager to a popup
  if (popup->child(0, "KPopupAccelManager", false) == 0 )
    new KPopupAccelManager(popup);
}


#include "kaccelmanager_private.moc"
