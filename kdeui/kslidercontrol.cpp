/*
 * kslidercontrol.cpp
 *
 * Initial implementation:
 *     Copyright (c) 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
 *
 * Enhancements:
 *     Copyright (c) 1999 Dirk A. Mueller <dmuell@gmx.net>
 *
 *  Requires the Qt widget libraries, available at no cost at
 *  http://www.troll.no/
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include <qlabel.h>
#include <qsize.h>
#include <qslider.h>
#include <qspinbox.h>

#include "kslidercontrol.h"

KSliderControl::KSliderControl(const QString& label, 
                               const int lower, const int upper,
                               const int step, const int value,
                               const QString& units,
                               QWidget *parent, const char *name)
    : QWidget( parent, name )
{
    h_spacing = v_spacing = 10; // 10 pixels of space between widgets
    
    // allocate GUI objects
    main_label = new QLabel( label, this );
    slider = new QSlider( lower, upper, step, value, QSlider::Horizontal, this );
    slider->setTickmarks(QSlider::Below);
    unit_str = units;
    connect( slider, SIGNAL( valueChanged(int) ), SLOT( resetValueField(int) ));
    spinbox = new QSpinBox(lower, upper, step, this);
    if(!units.isEmpty())
        spinbox->setSuffix(" " + units);
    connect( spinbox, SIGNAL( valueChanged(int) ), slider, SLOT( setValue(int) ));
    
    // default values
    int major = (upper-lower)/10; // default to 10 major ticks
    slider->setSteps( 1, major );
    slider->setTickInterval(major*2);
    
    setLabelAlignment( AlignLeft );
    setLabelSize(25);
}


void KSliderControl::resetValueField(int val)
{
    slider_value = val;
    spinbox->setValue(val);
    emit valueChanged(val);
}


void KSliderControl::computeMinimumSize()
{
    int tmp = spinbox->value();
    spinbox->setValue(spinbox->maxValue());
    int sp_w = spinbox->sizeHint().width();
    spinbox->setValue(tmp);

    int w = QMAX((sp_w*(100-spin_frac))/100, main_label->sizeHint().width());
    int h = slider->sizeHint().height() + main_label->height();
    qs.setWidth(w);
    qs.setHeight(h);
    setMinimumSize(w,h);
}


void KSliderControl::resizeEvent ( QResizeEvent * )
{
    int left_frac = (width()*spin_frac)/100;
    
    // label gets placed according to alignment and label_frac
    int lx = 0;
    int ly = 0;
    
    if ( label_align == AlignLeft )
        lx = 0;
    else if (label_align == AlignRight)
        lx = width() - main_label->width();
    else
        lx = ( width() - main_label->width() )/2;
    
    main_label->move(lx, ly);

    // spinbox stays always the same
    spinbox->setGeometry(0, main_label->height(), left_frac, spinbox->height());

    // slider gets stretched horizontally to fill remainder
    int sw = width() - left_frac - h_spacing;
    int sh = slider->height(); // keep height
    int sx = left_frac + h_spacing;
    int sy = main_label->height();
    
    slider->setGeometry(sx, sy, sw, sh);
}


KSliderControl::~KSliderControl()
{
    delete main_label;
    delete slider;
    delete spinbox;
}

#include "kslidercontrol.moc"
