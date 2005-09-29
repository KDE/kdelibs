/* This file is part of the KDE project
   Copyright (C) 2002, 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2002, 2004 Christoph Cullmann <cullmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "mainwindow.h"
#include "dockcontainer.h"

#include "dockcontainer.moc"

#include "kdockwidget_private.h"

#include <qwidgetstack.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <kmultitabbar.h>
#include <kglobalsettings.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>

//TODO: Well, this is already defined in kdeui/kdockwidget.cpp
static const char* const kmdi_not_close_xpm[]={
"5 5 2 1",
"# c black",
". c None",
"#####",
"#...#",
"#...#",
"#...#",
"#####"};

namespace KMDI
{


DockContainer::DockContainer(QWidget *parent, QWidget *win, int position, int flags)
: QWidget(parent),KDockContainer()
{
  m_tabSwitching = false;
  m_block=false;
  m_inserted=-1;
  m_mainWin = win;
  oldtab=-1;
  mTabCnt=0;
  m_position = position;
  m_previousTab=-1;
  m_separatorPos = 18000;
  m_movingState=NotMoving;
  m_startEvent=0;
  kdDebug(760)<<"DockContainer created"<<endl;

  QBoxLayout *l;
  m_vertical=!((position==KDockWidget::DockTop) || (position==KDockWidget::DockBottom));

  if (!m_vertical)
    l=new QVBoxLayout(this);
  else
    l=new QHBoxLayout(this);

  l->setAutoAdd(false);

  m_tb=new KMultiTabBar(((position==KDockWidget::DockTop) || (position==KDockWidget::DockBottom))?
    KMultiTabBar::Horizontal:KMultiTabBar::Vertical,this);

  m_tb->setStyle(KMultiTabBar::KMultiTabBarStyle(flags));
  m_tb->showActiveTabTexts(true);

  m_tb->setPosition((position==KDockWidget::DockLeft)?KMultiTabBar::Left:
    (position==KDockWidget::DockBottom)?KMultiTabBar::Bottom:
    (position==KDockWidget::DockTop)?KMultiTabBar::Top:KMultiTabBar::Right);

  m_ws=new QWidgetStack(this);

  m_ws->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

  if ( (position==KDockWidget::DockLeft) || (position==KDockWidget::DockTop))
  {
    l->add(m_tb);
    l->add(m_ws);
  }
  else
  {
    l->add(m_ws);
    l->add(m_tb);
  }

  l->activate();
  m_ws->hide();
  m_tb->installEventFilter(this);
}

void DockContainer::setStyle(int style) {
        if (m_tb)  m_tb->setStyle(KMultiTabBar::KMultiTabBarStyle(style));
}

DockContainer::~DockContainer()
{
  QMap<KDockWidget*,int>::iterator it;
  while (m_map.count()) {
    it = m_map.begin();
    KDockWidget *w=it.key();
      if (m_overlapButtons.contains(w)) {
        (static_cast<KDockWidgetHeader*>(w->getHeader()->qt_cast("KDockWidgetHeader")))->removeButton(m_overlapButtons[w]);
        m_overlapButtons.remove(w);
      }
    m_map.remove(w);
    w->undock();
  }
    deactivated(this);
}


void DockContainer::init()
{
  bool overlap = isOverlapMode();
  if (m_vertical)
  {
    parentDockWidget()->setForcedFixedWidth(m_tb->width());
    activateOverlapMode(m_tb->width());
  }
  else
  {
    parentDockWidget()->setForcedFixedHeight(m_tb->height());
    activateOverlapMode(m_tb->height());
  }

  if (!overlap) deactivateOverlapMode();

  // try to restore splitter size
  if ( parentDockWidget() && parentDockWidget()->parent() )
  {
    KDockSplitter *sp= static_cast<KDockSplitter*>(parentDockWidget()->
                parent()->qt_cast("KDockSplitter"));
    if ( sp )
      sp->setSeparatorPosX( m_separatorPos );
  }
}

KDockWidget *DockContainer::parentDockWidget()
{
  return ((KDockWidget*)parent());
}

void DockContainer::insertWidget (KDockWidget *dwdg, QPixmap pixmap, const QString &text, int &)
{
  KDockWidget* w = (KDockWidget*) dwdg;
  int tab;
  bool alreadyThere=m_map.contains(w);

  if (alreadyThere)
  {
    tab=m_map[w];
    if (m_ws->addWidget(w,tab)!=tab) kdDebug(760)<<"ERROR COULDN'T READD WIDGET************"<<endl;
    kdDebug(760)<<"READDED WIDGET***********************************"<<endl;
/*JOWENN    m_tb->setTab(tab,true);
    tabClicked(tab);*/
  }
  else
  {
    tab=m_ws->addWidget(w);
    m_map.insert(w,tab);
    m_revMap.insert(tab,w);


    if (((KDockWidget*)parentWidget())->mayBeShow())
      ((KDockWidget*)parentWidget())->dockBack();

    if (w->getHeader()->qt_cast("KDockWidgetHeader"))
    {
      kdDebug(760)<<"*** KDockWidgetHeader has been found"<<endl;

      KDockWidgetHeader *hdr=static_cast<KDockWidgetHeader*>(w->getHeader()->
        qt_cast("KDockWidgetHeader"));

      KDockButton_Private *btn = new KDockButton_Private( hdr, "OverlapButton" );

      QToolTip::add( btn, i18n("Switch between overlap and side by side mode", "Overlap") );

      btn->setToggleButton( true );
      btn->setPixmap( const_cast< const char** >(kmdi_not_close_xpm) );
      hdr->addButton(btn);
      m_overlapButtons.insert(w,btn);
      btn->setOn(!isOverlapMode());

      connect(btn,SIGNAL(clicked()),this,SLOT(changeOverlapMode()));
    }

    m_tb->appendTab(pixmap.isNull()?SmallIcon("misc"):pixmap,tab,w->tabPageLabel());
    m_tb->tab(tab)->installEventFilter( this );
    kdDebug(760)<<"NAMENAMENAMENAME:===========================:"<<w->tabPageLabel()<<endl;

//FB    m_tb->setTab(tab,true);

    connect(m_tb->tab(tab),SIGNAL(clicked(int)),this,SLOT(tabClicked(int)));

    kdDebug(760)<<"DockContainer::insertWidget()"<<endl;

//FB    m_tb->setTab(oldtab,false);

    mTabCnt++;
    m_inserted=tab;
    int dummy=0;
//FB    tabClicked(tab);
    KDockContainer::insertWidget(w,pixmap,text,dummy);
    itemNames.append(w->name());
    tabCaptions.insert(w->name(),w->tabPageLabel());
    tabTooltips.insert(w->name(),w->toolTipString());


  }

//FB  m_ws->raiseWidget(tab);
}


bool DockContainer::eventFilter( QObject *obj, QEvent *event )
{
    if (obj==m_tb) {
	if ( (event->type()==QEvent::Resize)  && (m_ws->isHidden()) ) {
		QSize size=((QResizeEvent*)event)->size();
		  if (m_vertical)
			  parentDockWidget()->setForcedFixedWidth(size.width());
		  else
			  parentDockWidget()->setForcedFixedHeight(size.height());
	}
    }
    else
    switch ( event->type() ){
      case QEvent::MouseButtonPress:
	{
		    KDockWidget * w=m_revMap[dynamic_cast<KMultiTabBarTab*>(obj)->id()];
		    if (!w)  {
			kdDebug()<<"NoWidget"<<endl;
			break;
		    }
		    if (!w->getHeader()) {
				kdDebug()<<"NO HEADER"<<endl;
				break;
			}
		      KDockWidgetHeader *hdr=static_cast<KDockWidgetHeader*>(w->getHeader()->
	        	qt_cast("KDockWidgetHeader"));
			if (!hdr) {
				kdDebug()<<"Wrong header type in DockContainer::eventFilter"<<endl;
				break;
			}
			m_dockManager=w->dockManager();
			m_dragPanel=hdr->dragPanel();
			if (m_dragPanel) m_movingState=WaitingForMoveStart;
			delete m_startEvent;
			m_startEvent=new QMouseEvent(* ((QMouseEvent*)event));
	}
		break;
      case QEvent::MouseButtonRelease:
		if (m_movingState==Moving) {
			m_movingState=NotMoving;
			QApplication::postEvent(m_dragPanel,new QMouseEvent(* ( (QMouseEvent*)event)));
			delete m_startEvent;
			m_startEvent=0;
		}
		break;
      case QEvent::MouseMove:
		if (m_movingState==WaitingForMoveStart) {
			QPoint p( ((QMouseEvent*)event)->pos() - m_startEvent->pos() );
		        if( p.manhattanLength() > KGlobalSettings::dndEventDelay()) {
				m_dockManager->eventFilter(m_dragPanel,m_startEvent);
				m_dockManager->eventFilter(m_dragPanel,event);
				m_movingState=Moving;
			}
		} else  if (m_movingState==Moving) {
			m_dockManager->eventFilter(m_dragPanel,event);
		}
		break;
	default:
		break;

	}
    return false;

}

void DockContainer::showWidget(KDockWidget *w) {
    if (!m_map.contains(w)) return;

    kdDebug()<<"KMDI::DockContainer::<showWidget"<<endl;
    int id=m_map[w];
    m_tb->setTab(id,true);
    tabClicked(id);

}

void DockContainer::changeOverlapMode()
{
  const KDockButton_Private *btn=dynamic_cast<const KDockButton_Private*>(sender());

  kdDebug(760)<<"DockContainer::changeOverlapMode: button=="<<btn<<endl;

  if (!btn) return;

  if (!btn->isOn()) {
    kdDebug(760)<<"DockContainer::changeOverlapMode: activateOverlapMode"<<endl;
    if (m_vertical) {
      activateOverlapMode(m_tb->width());
    }
    else
    {
      activateOverlapMode(m_tb->height());
    }
  } else {
    kdDebug(760)<<"DockContainer::changeOverlapMode: deactivateOverlapMode"<<endl;
    deactivateOverlapMode();
  }

  for (QMap<KDockWidget*,KDockButton_Private*>::iterator it=m_overlapButtons.begin();
    it!=m_overlapButtons.end();++it)
    it.data()->setOn(!isOverlapMode());
}

void DockContainer::hideIfNeeded() {
  kdDebug(760)<<"************************* hideIfNeeded *************************"<<endl;
  if (!itemNames.count())
    ((KDockWidget*)parentWidget())->undock();
}

void DockContainer::removeWidget(KDockWidget* dwdg)
{
    KDockWidget* w = (KDockWidget*) dwdg;
  if (!m_map.contains(w)) return;
  int id=m_map[w];
  if (m_tb->isTabRaised(id)) {
    //why do we hide the tab if we're just going
    //to remove it? - mattr 2004-10-26
          m_tb->setTab(id,false);
          m_tabSwitching = false;
          tabClicked(id);
  }
  m_tb->removeTab(id);
  m_ws->removeWidget(w);
  m_map.remove(w);
  m_revMap.remove(id);
  if (m_overlapButtons.contains(w)) {
    (static_cast<KDockWidgetHeader*>(w->getHeader()->qt_cast("KDockWidgetHeader")))->removeButton(m_overlapButtons[w]);
    m_overlapButtons.remove(w);
  }
  KDockContainer::removeWidget(w);
  itemNames.remove(w->name());
  tabCaptions.remove(w->name());
  tabTooltips.remove(w->name());
  if (!itemNames.count())
    ((KDockWidget*)parentWidget())->undock();
}

void DockContainer::undockWidget(KDockWidget *dwdg)
{
  KDockWidget* w = (KDockWidget*) dwdg;

  if (!m_map.contains(w))
    return;

  int id=m_map[w];
  if (m_tb->isTabRaised(id)) {
          kdDebug(760)<<"Wiget has been undocked, setting tab down"<<endl;
          m_tb->setTab(id,false);
          tabClicked(id);
  }
}

void DockContainer::tabClicked(int t)
{
  kdDebug(760)<<"DockContainer::tabClicked()"<<endl;
  bool call_makeVisible=!m_tabSwitching;
  m_tabSwitching=true;
  if ((t!=-1) && m_tb->isTabRaised(t))
  {
    if (m_ws->isHidden())
    {
       m_ws->show ();
       parentDockWidget()->restoreFromForcedFixedSize();
    }
      if (!m_ws->widget(t))
    {
      m_revMap[t]->manualDock(parentDockWidget(),KDockWidget::DockCenter,20);
      if (call_makeVisible) m_revMap[t]->makeDockVisible();//manualDock(parentDockWidget(),KDockWidget::DockCenter,20);
      m_tabSwitching=false;
      emit activated(this);
      return;
    }
    m_ws->raiseWidget(t);
    if (m_ws->widget(t)) {
      KDockWidget *tmpDw=static_cast<KDockWidget*>(m_ws->widget(t)->qt_cast("KDockWidget"));
      if (tmpDw) {
        if (tmpDw->getWidget()) tmpDw->getWidget()->setFocus();
      } else kdDebug(760)<<"Something really weird is going on"<<endl;
    } else
      kdDebug(760)<<"DockContainer::tabClicked(int): m_ws->widget(t)==0 "<<endl;

    if (oldtab!=t) m_tb->setTab(oldtab,false);
    m_tabSwitching=true;
    oldtab=t;
    emit activated(this);
  }
  else
  {
       // try save splitter position
      if ( parentDockWidget() && parentDockWidget()->parent() )
      {
        KDockSplitter *sp= static_cast<KDockSplitter*>(parentDockWidget()->
                    parent()->qt_cast("KDockSplitter"));
        if ( sp )
          m_separatorPos = sp->separatorPos();
      }
    m_previousTab=t;
//    oldtab=-1;
    if (m_block) return;
    emit deactivated(this);
    m_block=true;
    if (m_ws->widget(t))
    {
//    ((KDockWidget*)m_ws->widget(t))->undock();
    }
    m_block=false;
    m_ws->hide ();


  kdDebug(760)<<"Fixed Width:"<<m_tb->width()<<endl;
  if (m_vertical)
  parentDockWidget()->setForcedFixedWidth(m_tb->width()); // strange why it worked before at all
  else
  parentDockWidget()->setForcedFixedHeight(m_tb->height()); // strange why it worked before at all
   }
  m_tabSwitching=false;
}

void DockContainer::setToolTip (KDockWidget *, QString &s)
{
  kdDebug(760)<<"***********************************Setting tooltip for a widget: "<<s<<endl;
}

void DockContainer::setPixmap(KDockWidget* widget ,const QPixmap& pixmap)
{
  int id=m_ws->id(widget);
  if (id==-1) return;
  KMultiTabBarTab *tab=m_tb->tab(id);
  tab->setIcon(pixmap.isNull()?SmallIcon("misc"):pixmap);
}

void DockContainer::save(KConfig* cfg,const QString& group_or_prefix)
{
  // group name
  QString grp=cfg->group();
  cfg->deleteGroup(group_or_prefix+QString("::%1").arg(parent()->name()));
  cfg->setGroup(group_or_prefix+QString("::%1").arg(parent()->name()));

  // save overlap mode
  cfg->writeEntry("overlapMode",isOverlapMode());

  // try to save the splitter position
  if ( parentDockWidget() && parentDockWidget()->parent() )
  {
    KDockSplitter *sp= static_cast<KDockSplitter*>(parentDockWidget()->
                parent()->qt_cast("KDockSplitter"));
    if ( sp )
      cfg->writeEntry( "separatorPosition", m_separatorPos );
  }

  QPtrList<KMultiTabBarTab>* tl=m_tb->tabs();
  QPtrListIterator<KMultiTabBarTab> it(*tl);
  QStringList::Iterator it2=itemNames.begin();
  int i=0;
  for (;it.current()!=0;++it,++it2)
  {
//    cfg->writeEntry(QString("widget%1").arg(i),m_ws->widget(it.current()->id())->name());
    cfg->writeEntry(QString("widget%1").arg(i),(*it2));
    QString s=tabCaptions[*it2];
    if (!s.isEmpty()) {
      cfg->writeEntry(QString("widget%1-tabCaption").arg(i),s);
    }
    s=tabTooltips[*it2];
    if (!s.isEmpty()) {
      cfg->writeEntry(QString("widget%1-tabTooltip").arg(i),s);
    }
//    kdDebug(760)<<"****************************************Saving: "<<m_ws->widget(it.current()->id())->name()<<endl;
    if (m_tb->isTabRaised(it.current()->id()))
      cfg->writeEntry(m_ws->widget(it.current()->id())->name(),true);
  ++i;
  }

  cfg->setGroup(grp);
}

void DockContainer::load(KConfig* cfg,const QString& group_or_prefix)
{
  QString grp=cfg->group();
  cfg->setGroup(group_or_prefix+QString("::%1").arg(parent()->name()));

  if (cfg->readBoolEntry("overlapMode"))
    activateOverlapMode( m_vertical?m_tb->width():m_tb->height() );
  else
    deactivateOverlapMode();

  m_separatorPos = cfg->readNumEntry( "separatorPosition", 18000 ); // = 18%

  int i=0;
  QString raise;
  while (true)
  {
    QString dwn=cfg->readEntry(QString("widget%1").arg(i));
    if (dwn.isEmpty()) break;
    kdDebug(760)<<"*************************************************************Configuring dockwidget :"<<dwn<<endl;
    KDockWidget *dw=((KDockWidget*)parent())->dockManager()->getDockWidgetFromName(dwn);
    if (dw)
    {
      QString s=cfg->readEntry(QString("widget%1-tabCaption").arg(i));
      if (!s.isEmpty()) {
        dw->setTabPageLabel(s);
      }
      s=cfg->readEntry(QString("widget%1-tabTooltip").arg(i));
      if (!s.isEmpty()) {
        dw->setToolTipString(s);
      }
      dw->manualDock((KDockWidget*)parent(),KDockWidget::DockCenter);
    }
    if (cfg->readBoolEntry(dwn,false)) raise=dwn;
    i++;

  }

  QPtrList<KMultiTabBarTab>* tl=m_tb->tabs();
  QPtrListIterator<KMultiTabBarTab> it1(*tl);
  m_ws->hide();
  if (m_vertical)
    parentDockWidget()->setForcedFixedWidth(m_tb->width());
  else
    parentDockWidget()->setForcedFixedHeight(m_tb->height());
  for (;it1.current()!=0;++it1)
  {
    m_tb->setTab(it1.current()->id(),false);
  }
  kapp->syncX();
  m_delayedRaise=-1;

  for (QMap<KDockWidget*,KDockButton_Private*>::iterator it=m_overlapButtons.begin();
    it!=m_overlapButtons.end();++it)
    it.data()->setOn(!isOverlapMode());

  if (!raise.isEmpty())
  {
    for (QMap<KDockWidget*,int>::iterator it=m_map.begin();it!=m_map.end();++it)
    {
      if (it.key()->name()==raise)
      {
/*        tabClicked(it.data());
        m_tb->setTab(it.data(),true);
        tabClicked(it.data());
        m_ws->raiseWidget(it.key());
        kapp->sendPostedEvents();
        kapp->syncX();*/

        m_delayedRaise=it.data();
        QTimer::singleShot(0,this,SLOT(delayedRaise()));
        kdDebug(760)<<"************** raising *******: "<<it.key()->name()<<endl;
        break;
      }
    }

  }
  if (m_delayedRaise==-1)   QTimer::singleShot(0,this,SLOT(init()));
  cfg->setGroup(grp);

}

void DockContainer::delayedRaise()
{
        m_tb->setTab(m_delayedRaise,true);
        tabClicked(m_delayedRaise);
}

void DockContainer::collapseOverlapped()
{
  if (m_tabSwitching) return;
  if (isOverlapMode()){
    QPtrList<KMultiTabBarTab>* tl=m_tb->tabs();
          QPtrListIterator<KMultiTabBarTab> it(*tl);
    for(;it.current();++it) {
      if (it.current()->isOn()) {
        kdDebug(760)<<"Lowering TAB"<<endl;
        it.current()->setState(false);
        tabClicked(it.current()->id());
      }
    }
  }
}

void DockContainer::toggle() {
    kdDebug(760)<<"DockContainer:activate"<<endl;
    if (m_tb->isTabRaised(oldtab)) {
        m_tb->setTab(oldtab,false);
            tabClicked(oldtab);
        //    KMDI::MainWindow *mainFrm = dynamic_cast<KMDI::MainWindow*>(m_mainWin);
        //     if (mainFrm && mainFrm->activeWindow() ) // TODO
         //           mainFrm->activeWindow()->setFocus();

    } else {
        kdDebug(760)<<"DockContainer::toggle(): raising tab"<<endl;
        if (m_tb->tab(m_previousTab)==0) {
            if (m_tb->tabs()->count()==0) return;
            m_previousTab=m_tb->tabs()->getFirst()->id();
        }
        m_tb->setTab(m_previousTab,true);
            tabClicked(m_previousTab);
    }
}

void DockContainer::prevToolView() {
    QPtrList<KMultiTabBarTab>* tabs=m_tb->tabs();
    int pos=tabs->findRef(m_tb->tab(oldtab));
    if (pos==-1) return;
    pos--;
    if (pos<0) pos=tabs->count()-1;
    KMultiTabBarTab *tab=tabs->at(pos);
    if (!tab) return; //can never happen here, but who knows
    m_tb->setTab(tab->id(),true);
    tabClicked(tab->id());
}

void DockContainer::nextToolView() {
    QPtrList<KMultiTabBarTab>* tabs=m_tb->tabs();
    int pos=tabs->findRef(m_tb->tab(oldtab));
    if (pos==-1) return;
    pos++;
    if (pos>=(int)tabs->count()) pos=0;
    KMultiTabBarTab *tab=tabs->at(pos);
    if (!tab) return; //can never happen here, but who knows
    m_tb->setTab(tab->id(),true);
    tabClicked(tab->id());
}

}

// kate: space-indent on; indent-width 2; replace-tabs on;
