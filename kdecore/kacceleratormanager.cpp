/*  This file is part of the KDE project
    Copyright (C) 2002 Matthias Hölzer-Klüpfel <mhk@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kacceleratormanager.h"

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qmenudata.h>
#include <q3memarray.h>
#include <qmetaobject.h>
#include <q3mainwindow.h>
#include <qobject.h>
#include <q3popupmenu.h>
#include <QList>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtabbar.h>
#include <q3textview.h>
#include <qwidget.h>
#include <QStackedWidget>

#include <kstdaction.h>
#include <kstaticdeleter.h>
#include <kdebug.h>


#include "kacceleratormanager_private.h"
#include "../kdeui/kstdaction_p.h"


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
                removed_string  += "<tr><td>" + Q3StyleSheet::escape(t1) + "</td></tr>";
            } else if (as.originalAccel() == -1) {
                added_string += "<tr><td>" + Q3StyleSheet::escape(t2) + "</td></tr>";
            } else {
                changed_string += "<tr><td>" + Q3StyleSheet::escape(t1) + "</td>";
                changed_string += "<td>" + Q3StyleSheet::escape(t2) + "</td></tr>";
            }
            return true;
        }
        return false;
    }
    static QString changed_string;
    static QString added_string;
    static QString removed_string;
    static QMap<QWidget *, int> ignored_widgets;

private:
  class Item;
public:
  typedef QList<Item *> ItemList;

private:
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
QMap<QWidget*, int> KAcceleratorManagerPrivate::ignored_widgets;

bool KAcceleratorManagerPrivate::standardName(const QString &str)
{
    if (!kaccmp_sns)
        kaccmp_sns_d.setObject(kaccmp_sns, new QStringList(KStdAction::internal_stdNames()));
        return kaccmp_sns->contains(str);
}

KAcceleratorManagerPrivate::Item::~Item()
{
    if (m_children)
        while (!m_children->isEmpty())
            delete m_children->takeFirst();

    delete m_children;
}


void KAcceleratorManagerPrivate::Item::addChild(Item *item)
{
    if (!m_children) {
        m_children = new ItemList;
    }

    m_children->append(item);
}

void KAcceleratorManagerPrivate::manage(QWidget *widget)
{
    if (!widget)
    {
        kdDebug(125) << "null pointer given to manage" << endl;
        return;
    }

    if (dynamic_cast<QMenu*>(widget))
    {
        // create a popup accel manager that can deal with dynamic menus
        KPopupAccelManager::manage(static_cast<QMenu*>(widget));
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
    foreach(Item *it, *item->m_children)
    {
        contents << it->m_content;
    }

    // find the right accelerators
    KAccelManagerAlgorithm::findAccelerators(contents, used);

    // write them back into the widgets
    int cnt = -1;
    foreach(Item *it, *item->m_children)
    {
        cnt++;

        QTabBar *tabBar = dynamic_cast<QTabBar*>(it->m_widget);
        if (tabBar)
        {
            if (checkChange(contents[cnt]))
                tabBar->setTabText(it->m_index, contents[cnt].accelerated());
            continue;
        }
        QMenuBar *menuBar = dynamic_cast<QMenuBar*>(it->m_widget);
        if (menuBar)
        {
            if (it->m_index >= 0)
            {
                QMenuItem *mitem = menuBar->findItem(menuBar->idAt(it->m_index));
                if (mitem)
                {
                    checkChange(contents[cnt]);
                    mitem->setText(contents[cnt].accelerated());
                }
                continue;
            }
        }
        // we possibly reserved an accel, but we won't set it as it looks silly
        if ( dynamic_cast<Q3GroupBox*>( it->m_widget ) )
             continue;

        kdDebug(125) << "write " << cnt << " " << it->m_widget->className() << " " <<contents[cnt].accelerated() << endl;

        int tprop = it->m_widget->metaObject()->indexOfProperty("text");
        if (tprop != -1)  {
            if (checkChange(contents[cnt]))
                it->m_widget->setProperty("text", contents[cnt].accelerated());
        } else {
            tprop = it->m_widget->metaObject()->indexOfProperty("title");
            if (tprop != -1 && checkChange(contents[cnt]))
                it->m_widget->setProperty("title", contents[cnt].accelerated());
        }
    }

    // calculate the accelerators for the children
    foreach(Item *it, *item->m_children)
    {
        kdDebug(125) << "children " << it->m_widget->className() << endl;
        if (it->m_widget && it->m_widget->isVisibleTo( item->m_widget ) )
            calculateAccelerators(it, used);
    }
}


void KAcceleratorManagerPrivate::traverseChildren(QWidget *widget, Item *item)
{
  QList<QWidget*> childList = widget->findChildren<QWidget*>();
  foreach ( QWidget *w , childList ) {
    // Ignore unless we have the direct parent
    if(qobject_cast<QWidget *>(w->parent()) != widget) continue;

    if ( !w->isVisibleTo( widget ) || w->isTopLevel() )
        continue;

    if ( KAcceleratorManagerPrivate::ignored_widgets.find( w ) != KAcceleratorManagerPrivate::ignored_widgets.end() )
        continue;

    manageWidget(w, item);
  }
}

void KAcceleratorManagerPrivate::manageWidget(QWidget *w, Item *item)
{
  // first treat the special cases

  QTabBar *tabBar = dynamic_cast<QTabBar*>(w);
  if (tabBar)
  {
      manageTabBar(tabBar, item);
      return;
  }

  QStackedWidget *wds = dynamic_cast<QStackedWidget*>( w );
  if ( wds )
  {
      QWidgetStackAccelManager::manage( wds );
      // return;
  }

  QMenu *popupMenu = dynamic_cast<QMenu*>(w);
  if (popupMenu)
  {
      // create a popup accel manager that can deal with dynamic menus
      KPopupAccelManager::manage(popupMenu);
      return;
  }

  QStackedWidget *wdst = dynamic_cast<QStackedWidget*>( w );
  if ( wdst )
  {
      QWidgetStackAccelManager::manage( wdst );
      // return;
  }

  QMenuBar *menuBar = dynamic_cast<QMenuBar*>(w);
  if (menuBar)
  {
      manageMenuBar(menuBar, item);
      return;
  }

  if (dynamic_cast<QComboBox*>(w) || dynamic_cast<QLineEdit*>(w) ||
      dynamic_cast<Q3TextEdit*>(w) || dynamic_cast<Q3TextView*>(w) ||
      dynamic_cast<QSpinBox*>(w) || w->inherits( "KMultiTabBar" ) )
      return;

  // now treat 'ordinary' widgets
  QLabel *label =  dynamic_cast<QLabel*>(w);
  if ( label  ) {
      if ( !label->buddy() )
          label = 0;
      else {
          if ( label->textFormat() == Qt::RichText ||
               ( label->textFormat() == Qt::AutoText &&
                 Q3StyleSheet::mightBeRichText( label->text() ) ) )
              label = 0;
      }
  }

  if (w->focusPolicy() != Qt::NoFocus || label || dynamic_cast<Q3GroupBox*>(w) || dynamic_cast<QRadioButton*>( w ))
  {
    QString content;
    QVariant variant;
    int tprop = w->metaObject()->indexOfProperty("text");
    if (tprop != -1)  {
        QMetaProperty p = w->metaObject()->property( tprop );
        if ( p.isValid() )
            variant = p.read (w);
        else
            tprop = -1;
    }

    if (tprop == -1)  {
        tprop = w->metaObject()->indexOfProperty("title");
        if (tprop != -1)  {
            QMetaProperty p = w->metaObject()->property( tprop );
            if ( p.isValid() )
                variant = p.read (w);
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
        if (dynamic_cast<QPushButton*>(w) || dynamic_cast<QCheckBox*>(w) || dynamic_cast<QRadioButton*>(w) || dynamic_cast<QLabel*>(w))
            weight = KAccelManagerAlgorithm::ACTION_ELEMENT_WEIGHT;

        // don't put weight on group boxes, as usually the contents are more important
        if (dynamic_cast<Q3GroupBox*>(w))
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
    QString content = bar->tabText(i);
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
        if (mitem->menu ())
            KPopupAccelManager::manage(mitem->menu ());
    }
}


/*********************************************************************

 class KAcceleratorManager - main entry point

 This class is just here to provide a clean public API...

 *********************************************************************/

void KAcceleratorManager::manage(QWidget *widget, bool programmers_mode)
{
    kdDebug(125) << "KAcceleratorManager::manage\n";
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
    m_orig_accel = m_pureText.find("(!)&");
    if (m_orig_accel != -1)
	m_pureText.remove(m_orig_accel, 4);

    m_orig_accel = m_pureText.find("(&&)");
    if (m_orig_accel != -1)
        m_pureText.replace(m_orig_accel, 4, "&");

    m_origText = m_pureText;

    if (m_pureText.contains('\t'))
        m_pureText = m_pureText.left(m_pureText.find('\t'));

    m_orig_accel = m_accel = stripAccelerator(m_pureText);

    kdDebug(125) << input << " " << m_orig_accel << " " << m_accel << " " << m_pureText << endl;
    if (initialWeight == -1)
        initialWeight = KAccelManagerAlgorithm::DEFAULT_WEIGHT;

    calculateWeights(initialWeight);

    // dump();
}


QString KAccelString::accelerated() const
{
  QString result = m_origText;
  if (result.isEmpty())
      return result;

  if (KAcceleratorManagerPrivate::programmers_mode)
  {
    if (m_accel != m_orig_accel) {
      int oa = m_orig_accel;

      if (m_accel >= 0) {
              result.insert(m_accel, "(!)&");
              if (m_accel < m_orig_accel)
                  oa += 4;
      }
      if (m_orig_accel >= 0)
	  result.replace(oa, 1, "(&&)");
    }
  } else {
      if (m_accel >= 0 && m_orig_accel != m_accel) {
          if (m_orig_accel != -1)
              result.remove(m_orig_accel, 1);
          result.insert(m_accel, "&");
      }
  }
  return result;
}


QChar KAccelString::accelerator() const
{
  if ((m_accel < 0) || (m_accel > (int)m_pureText.length()))
    return QChar();

  return m_pureText[m_accel].toLower();
}


void KAccelString::calculateWeights(int initialWeight)
{
  m_weight.resize(m_pureText.length());

  int pos = 0;
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
        // kdDebug(125) << "wanted " << m_pureText << " " << KAcceleratorManagerPrivate::standardName(m_origText) << endl;
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

  for (int pos=0; pos<m_pureText.length(); ++pos)
    if (used.find(m_pureText[pos], 0, false) == -1 && m_pureText[pos].latin1() != 0)
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
  for (int i=0; i<m_weight.count(); ++i)
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
   * it is quite fast, O(N²)
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
    kdDebug(125) << "findAccelerators\n";
  KAccelStringList accel_strings = result;

  // initally remove all accelerators
  for (KAccelStringList::Iterator it = result.begin(); it != result.end(); ++it) {
      kdDebug(125) << "reset " << ( *it ).pure() << endl;
    (*it).setAccel(-1);
  }

  // pick the highest bids
  for (int cnt=0; cnt<accel_strings.count(); ++cnt)
  {
      kdDebug(125) << "cnt " << accel_strings[cnt].pure() << endl;
    int max = 0, index = -1, accel = -1;

    // find maximum weight
    for (int i=0; i<accel_strings.count(); ++i)
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

KPopupAccelManager::KPopupAccelManager(QMenu *popup)
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
    if (mitem->menu())
        KPopupAccelManager::manage(mitem->menu());
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


void KPopupAccelManager::manage(QMenu *popup)
{
  // don't add more than one manager to a popup
  if (popup->child(0, "KPopupAccelManager", false) == 0 )
    new KPopupAccelManager(popup);
}

void QWidgetStackAccelManager::manage( QStackedWidget *stack )
{
    if ( stack->child( 0, "QWidgetStackAccelManager", false ) == 0 )
        new QWidgetStackAccelManager( stack );
}

QWidgetStackAccelManager::QWidgetStackAccelManager(QStackedWidget *stack)
  : QObject(stack), m_stack(stack)
{
    aboutToShow(stack->currentWidget()); // do one check and then connect to show
    connect(stack, SIGNAL(aboutToShow(QWidget *)), SLOT(aboutToShow(QWidget *)));
}

bool QWidgetStackAccelManager::eventFilter ( QObject * watched, QEvent * e )
{
    if ( e->type() == QEvent::Show && qApp->activeWindow() ) {
        KAcceleratorManager::manage( qApp->activeWindow() );
        watched->removeEventFilter( this );
    }
    return false;
}

void QWidgetStackAccelManager::aboutToShow(QWidget *child)
{
    if (!child)
    {
        kdDebug(125) << "null pointer given to aboutToShow" << endl;
        return;
    }

    child->installEventFilter( this );
}

void KAcceleratorManager::setNoAccel( QWidget *widget )
{
    KAcceleratorManagerPrivate::ignored_widgets[widget] = 1;
}

#include "kacceleratormanager_private.moc"
