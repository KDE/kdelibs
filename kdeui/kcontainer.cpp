/* This file is part of the KDE libraries
    Copyright (C) 1998 Jorge Monteiro <jomo@casema.net>

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

#include "kcontainer.h"


KContainerLayout::KContainerLayout(QWidget * parent, const char * name, 
			int orientation, bool homogeneos,
			int spacing, WFlags f, bool allowLines)
    : QFrame(parent,name,f,allowLines)
{
    _spacing = spacing;
    _homogeneos = homogeneos;
    _orientation = orientation;
    _startOffset = 0;
    _endOffset = 0;
    //setFrameStyle(QFrame::WinPanel|QFrame::Raised);
    if (parent && !parent->inherits("KContainerLayout"))
    {
	// Install handle for parent resize, this will allow us to 
	// resize ourselves
	parent->installEventFilter(this);
    }
}

KContainerLayout::~KContainerLayout()
{
}

int KContainerLayout::packStart(QWidget *w, bool e, bool f,int p)
{
    _startWidgets.append(new KContainerLayoutItem(w,e,f,p));
    recalcLayout();
    return 0;
}

int KContainerLayout::packEnd(QWidget *w, bool e, bool f,int p)
{
    _endWidgets.append(new KContainerLayoutItem(w,e,f,p));
    recalcLayout();
    return 0;
}

void KContainerLayout::setOrientation(int i)
{
    _orientation = i;
    recalcLayout();
}

void KContainerLayout::setHomogeneos(bool b)
{
    _homogeneos = b;
    recalcLayout();
}

void KContainerLayout::setSpacing(int i)
{
    _spacing = i;
    recalcLayout();
}

void KContainerLayout::setStartOffset(int i)
{
    _startOffset = i;
    recalcLayout();
}

void KContainerLayout::setEndOffset(int i)
{
    _endOffset = i;
    recalcLayout();
}

void KContainerLayout::recalcLayout()
{
    // Calculate our hint
    QSize sz = sizeHint();
    // if our size is not enough, resize us
    if (size().width() < sz.width() || size().height() < sz.height())
	resize(sz);
    repositionWidgets();
}

void KContainerLayout::sizeToFit()
{
    if (parent()!=0L && !parent()->inherits("KContainerLayout") && parent()->inherits("QWidget"))
	((QWidget*)parent())->resize(_sizeHint);
    resize(_sizeHint);
}

int KContainerLayout::getNumberOfWidgets() const
{
   return (_startWidgets.count() + _endWidgets.count());
}

void KContainerLayout::repositionWidgets()
{
    int x,y;

    if (horizontal())
    {
	y = 0;
	x = _spacing + _startOffset;
	if (_homogeneos)
	{
	    int nr_widgets = getNumberOfWidgets();
	    int each_width=0;
	    if (nr_widgets)
		each_width = (((size().width()-_startOffset-_endOffset) - (_spacing*(nr_widgets+1)))) / nr_widgets;
	    KContainerLayoutItem *item;
	    // Now reposition one each time
	    for ( item=_startWidgets.first(); item != 0; item=_startWidgets.next() )
	    {
		int w = idealSizeOfWidget(item).width();
		int gap = each_width - w;
		if (item->fill())
		    item->widget()->setGeometry(x+item->padding(),y,
			    	each_width-item->padding()*2,size().height());
		else
		    item->widget()->setGeometry(x+item->padding()+gap/2,y,
				each_width-item->padding()*2-gap,size().height());
		x += (each_width+_spacing);
	    }
	    // ... from the end
	    warning("homo");
	    x = size().width() - _endOffset;
	    for ( item=_endWidgets.first(); item != 0; item=_endWidgets.next() )
	    {
		int w = idealSizeOfWidget(item).width();
		int gap = each_width - w;
		x -= (each_width+_spacing);
		if (item->fill())
		    item->widget()->setGeometry(x+item->padding(),y,
			    	each_width-item->padding()*2,size().height());
		else
		    item->widget()->setGeometry(x+item->padding()+gap/2,y,
				each_width-item->padding()*2-gap,size().height());
	    }
	}
	else
	{
	    int nr_expand = numberOfWidgetsWithExpand();
	    int extra_width=0;
	    if (nr_expand)
		extra_width = (size().width() - _sizeHint.width()) / nr_expand;
	    KContainerLayoutItem *item;
	    // Now reposition one each time
	    for ( item=_startWidgets.first(); item != 0; item=_startWidgets.next() )
	    {
		int w = idealSizeOfWidget(item).width();
		if (item->expand())
		{
		    w += extra_width;
		    if (item->fill())
			item->widget()->setGeometry(x+item->padding(),y,w-item->padding()*2,size().height());
		    else
			item->widget()->setGeometry(x+item->padding()+extra_width/2,y,
				w-item->padding()*2-extra_width,size().height());
		}
		else
		    item->widget()->setGeometry(x+item->padding(),y,w-item->padding()*2,size().height());
		x += (w+_spacing);
	    }
	    // ... from end
	    warning("non-homo");
	    x = size().width() - _endOffset;
	    for ( item=_endWidgets.first(); item != 0; item=_endWidgets.next() )
	    {
		int w = idealSizeOfWidget(item).width();
		if (item->expand())
		    w += extra_width;
		x -= (w+_spacing);
		if (item->expand())
		{
		    if (item->fill())
			item->widget()->setGeometry(x+item->padding(),y,w-item->padding()*2,size().height());
		    else
			item->widget()->setGeometry(x+item->padding()+extra_width/2,y,
				w-item->padding()*2-extra_width,size().height());
		}
		else
		    item->widget()->setGeometry(x+item->padding(),y,w-item->padding()*2,size().height());
	    }

	}
    }
    else
    {
	y = _spacing + _startOffset;
	x = 0;
	if (_homogeneos)
	{
	    int nr_widgets = getNumberOfWidgets();
	    int each_height=0;
	    if (nr_widgets)
		each_height = (((size().height()-_startOffset-_endOffset) - (_spacing*(nr_widgets+1)))) / nr_widgets;
	    KContainerLayoutItem *item;
	    // Now reposition one each time
	    for ( item=_startWidgets.first(); item != 0; item=_startWidgets.next() )
	    {
		int h = idealSizeOfWidget(item).height();
		int gap = each_height - h;
		if (item->fill())
		    item->widget()->setGeometry(x,y+item->padding(),
			    	size().width(),each_height-item->padding()*2);
		else
		    item->widget()->setGeometry(x,y+item->padding()+gap/2,
				size().width(),each_height-item->padding()*2-gap);
		y += (each_height+_spacing);
	    }
	    // Now reposition one each time
	    y = size().height() - _endOffset;
	    for ( item=_endWidgets.first(); item != 0; item=_endWidgets.next() )
	    {
		int h = idealSizeOfWidget(item).height();
		int gap = each_height - h;
		y -= (each_height+_spacing);
		if (item->fill())
		    item->widget()->setGeometry(x,y+item->padding(),
			    	size().width(),each_height-item->padding()*2);
		else
		    item->widget()->setGeometry(x,y+item->padding()+gap/2,
				size().width(),each_height-item->padding()*2-gap);
	    }
	}
	else
	{
	    int nr_expand = numberOfWidgetsWithExpand();
	    int extra_height=0;
	    if (nr_expand)
		extra_height = (size().height() - _sizeHint.height()) / nr_expand;
	    KContainerLayoutItem *item;
	    // Now reposition one each time
	    for ( item=_startWidgets.first(); item != 0; item=_startWidgets.next() )
	    {
		int h = idealSizeOfWidget(item).height();
		if (item->expand())
		{
		    h += extra_height;
		    if (item->fill())
			item->widget()->setGeometry(x,y+item->padding(),size().width(),h-item->padding()*2);
		    else
			item->widget()->setGeometry(x,y+item->padding()+extra_height/2,
				size().width(),h-item->padding()*2-extra_height);
		}
		else
		    item->widget()->setGeometry(x,y+item->padding(),size().width(),h-item->padding()*2);
		y += (h+_spacing);
	    }
	    // ... and the end
	    y = size().height() - _endOffset;
	    for ( item=_endWidgets.first(); item != 0; item=_endWidgets.next() )
	    {
		int h = idealSizeOfWidget(item).height();
		if (item->expand())
		    h += extra_height;
		y -= (h+_spacing);
		if (item->expand())
		{
		    if (item->fill())
			item->widget()->setGeometry(x,y+item->padding(),size().width(),h-item->padding()*2);
		    else
			item->widget()->setGeometry(x,y+item->padding()+extra_height/2,
				size().width(),h-item->padding()*2-extra_height);
		}
		else
		    item->widget()->setGeometry(x,y+item->padding(),size().width(),h-item->padding()*2);
	    }
	}
    }
}

int KContainerLayout::numberOfWidgetsWithExpand()
{
    int i = 0;
    KContainerLayoutItem *item;
    for ( item=_startWidgets.first(); item != 0; item=_startWidgets.next() )
	i += (item->expand())?1:0;
    for ( item=_endWidgets.first(); item != 0; item=_endWidgets.next() )
	i += (item->expand())?1:0;
    return i;
}

void KContainerLayout::calculateSizeHint()
{
    int nr_widgets = getNumberOfWidgets();
    if (nr_widgets == 0)
    {
	if (horizontal())
	    _sizeHint = QSize(_startOffset + _endOffset,0);
	else
	    _sizeHint = QSize(0,_startOffset + _endOffset);
	_sizeForEach = -1;
	return;
    }
    
    if (_homogeneos)
    {
	// this is easy
	QSize sz = sizeOfLargerWidget();
	// Ok, the size of each box will be the size
	// of the bigger widget plus the spacing
	if (horizontal())
	{
	    _sizeForEach = sz.width();
	    int w = (sz.width()*nr_widgets)+(_spacing * (nr_widgets + 1));
	    sz.setWidth(w+_startOffset+_endOffset);
	}
	else
	{
	    _sizeForEach = sz.height();
	    int h = (sz.height()*nr_widgets)+(_spacing * (nr_widgets + 1));
	    sz.setHeight(h+_startOffset+_endOffset);
	}
	_sizeHint = sz;
    }
    else
    {
	// We have to calculate the size based on each widget
	int w = 0, h = 0;
	KContainerLayoutItem *item;
	// First on the start widgets...
	for ( item=_startWidgets.first(); item != 0; item=_startWidgets.next() )
	{
	    QSize widgetSz = idealSizeOfWidget(item);
	    
	    if (horizontal())
	    {
		w += widgetSz.width();
		h = (widgetSz.height()>h)?widgetSz.height():h;
	    }
	    else
	    {
		h += widgetSz.height();
		w = (widgetSz.width()>h)?widgetSz.width():w;
	    }
	}
	// ...now the end widgets
	for ( item=_endWidgets.first(); item != 0; item=_endWidgets.next() )
	{
	    QSize widgetSz = idealSizeOfWidget(item);
	    
	    if (horizontal())
	    {
		w += widgetSz.width();
		h = (widgetSz.height()>h)?widgetSz.height():h;
	    }
	    else
	    {
		h += widgetSz.height();
		w = (widgetSz.width()>h)?widgetSz.width():w;
	    }
	}
	if (horizontal())
	    w += ((nr_widgets+1)*_spacing) + _startOffset + _endOffset;
	else
	    h += ((nr_widgets+1)*_spacing) + _startOffset + _endOffset;
	_sizeHint = QSize(w,h);
	_sizeForEach = -1;
    }
    if (parent()!=0L && !parent()->inherits("KContainerLayout") && parent()->inherits("QWidget"))
    {
	warning("setting minimum size for parent widget (%i,%i)",_sizeHint.width(),_sizeHint.height());
	((QWidget*)parent())->setMinimumSize(_sizeHint);
    }
    warning("minimum size (%i,%i)",_sizeHint.width(),_sizeHint.height());
    setMinimumSize(_sizeHint);
}

QSize KContainerLayout::sizeOfLargerWidget()
{
    QSize sz;
    int maxW=0,maxH=0;
    KContainerLayoutItem *item;
    // start widgets
    for ( item=_startWidgets.first(); item != 0; item=_startWidgets.next() )
    {
	sz = idealSizeOfWidget(item);
	if (sz.width()>maxW)
	    maxW = sz.width();
	if (sz.height()>maxH)
	    maxH = sz.height();	
    }
    // ent widgets
    for ( item=_endWidgets.first(); item != 0; item=_endWidgets.next() )
    {
	sz = idealSizeOfWidget(item);
	if (sz.width()>maxW)
	    maxW = sz.width();
	if (sz.height()>maxH)
	    maxH = sz.height();	
    }
    return QSize(maxW,maxH);
}

QSize KContainerLayout::idealSizeOfWidget(KContainerLayoutItem *item)
{
   // Calculate the size for the widget this will make use of 
   // sizeHint() - the widget must return a valid value here
   QSize sz = widgetSize(item);
   if (horizontal())
	sz.setWidth(sz.width()+item->padding()*2);
   else
	sz.setHeight(sz.height()+item->padding()*2);
   return sz;
}

void KContainerLayout::resizeEvent(QResizeEvent *ev)
{
    QFrame::resizeEvent(ev);

    repositionWidgets();
}

bool KContainerLayout::eventFilter(QObject *, QEvent *ev)
{
    if (ev->type() == QEvent::Resize)
    {
	// resize ourselves
	resize(((QResizeEvent*)ev)->size());
	// let parent resize normally
	return false;
    }
    else
	return false;
}

QSize KContainerLayout::sizeHint() const
{
    KContainerLayout *p = (KContainerLayout*)this;
    p->calculateSizeHint();
    return _sizeHint;
}

#include "kcontainer.moc"

template QList<KContainerLayout::KContainerLayoutItem>;
