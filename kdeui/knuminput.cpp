/*
 * knuminput.cpp
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

#include "knumvalidator.h"
#include "knuminput.h"


// -----------------------------------------------------------------------------

KIntSpinBox::KIntSpinBox(int lower, int upper, int step, int value, int base,
                         QWidget* parent, const char* name)
    : QSpinBox(lower, upper, step, parent, name)
{
    val_base = base;
    setValue(value);
}


// -----------------------------------------------------------------------------

QString KIntSpinBox::mapValueToText(int v)
{
    QString str;

    return str.setNum(v, val_base);
}


// -----------------------------------------------------------------------------

int KIntSpinBox::mapTextToValue(bool* ok)
{
    return cleanText().toInt(ok, val_base);
}


// -----------------------------------------------------------------------------

KIntNumInput::KIntNumInput(const QString& label, int lower, int upper, int step,
                           int val, const QString& units, int _base, bool use_slider,
                           QWidget *parent, const char *name)
    : QWidget(parent, name )
{
    init(label, lower, upper, step, val, units, _base, use_slider);

}



// -----------------------------------------------------------------------------

KIntNumInput::KIntNumInput(int lower, int upper, int step, int value, QWidget* parent,
                           const QString& label, const QString& units, bool use_slider,
                           int _base, const char* name)
    : QWidget(parent, name) 
{
    init(label, lower, upper, step, value, units, _base, use_slider);
}


// -----------------------------------------------------------------------------

void KIntNumInput::init(const QString& label, int lower, int upper, int step, int val,
                        const QString& units, int _base, bool use_slider) 
{
  // Not checking the boundaries blocks the code in this constructor for
  // some reason.
  int_value = val;

  if (val > upper) int_value = upper;
  if (val < lower) int_value = lower;

    spin = new KIntSpinBox(lower, upper, step, int_value, _base, this, "KIntSpinBox");
    spin->setValidator(new KIntValidator(this, _base, "KNumInput::KIntValidtr"));

    main_label = new QLabel( spin, label, this, "KNumInput::QLabel" );
    
    if(!units.isEmpty())
        spin->setSuffix(" " + units);
    
    if(use_slider) {
        slider = new QSlider(lower, upper, step, int_value, QSlider::Horizontal, this);
        slider->setTickmarks(QSlider::Below);
        
        connect(slider, SIGNAL(valueChanged(int)), SLOT(resetValueField(int)));
        connect(spin, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));

        // default values
        int major = (upper-lower)/_base; 
        slider->setSteps( 1, major );
        slider->setTickInterval(major);
    }
    else
        slider = 0;
    
    setLabelAlignment( AlignLeft );
    setSpinBoxSize(25);
}


// -----------------------------------------------------------------------------

void KIntNumInput::resetValueField(int val)
{
    int_value = val;
    spin->setValue(val);
    emit valueChanged(val);
}


// -----------------------------------------------------------------------------

QSize KIntNumInput::minimumSize() const
{
    QSize qs;
    QSize spin_s(spin->sizeHint());

    main_label->adjustSize();

    int w, h;

    if(slider) {
        w = QMAX((spin_s.width()*(100-spin_frac))/100, main_label->width());
        h = QMAX(slider->sizeHint().height(), spin_s.height());;
    }
    else {
        w = QMAX(spin_s.width(), main_label->width());
        h = spin_s.height();
    }

    h += (main_label->text().isEmpty() ? 0 : main_label->height() + 5);
    
    qs.setWidth(w);
    qs.setHeight(h);
    
    return qs;
}


// -----------------------------------------------------------------------------

void KIntNumInput::resizeEvent(QResizeEvent* e)
{
    int left_frac = (width()*spin_frac)/100;
    int label_height = (main_label->text().isEmpty() ? 0 : main_label->height() + 5);
    
    // label gets placed according to alignment and label_frac
    int lx = 0;
    int ly = 0;

    switch(label_align) {
    case AlignCenter:
        lx = (e->size().width() - main_label->width())/2;
        break;
    case AlignRight:
        lx = e->size().width() - main_label->width();
        break;
        
    case AlignLeft:
    default:
        lx = 0;
        break;
    }
    
    main_label->move(lx, ly);

    // spinbox stays always the same
    spin->move(0, label_height);
  
    // slider gets stretched horizontally to fill remainder
    if(slider) {
        slider->setGeometry(left_frac + 10, label_height,
                            width() - left_frac - 10, slider->height());
        spin->resize(left_frac, spin->sizeHint().height());
    }
    else
        spin->resize(e->size().width(), spin->sizeHint().height());
}


// -----------------------------------------------------------------------------

KIntNumInput::~KIntNumInput()
{
    delete main_label;
    delete slider;
}


// -----------------------------------------------------------------------------

void KIntNumInput::setSteps(int minor, int major)
{
    if(slider)
        slider->setSteps( minor, major );
}


// -----------------------------------------------------------------------------

void KIntNumInput::setValue(int val)
{
    if(slider)
        slider->setValue(val);
}


// -----------------------------------------------------------------------------

int  KIntNumInput::value()
{
    return int_value;
}


// -----------------------------------------------------------------------------

void KIntNumInput::setLabelAlignment(int a)
{
    label_align = a;
}


// -----------------------------------------------------------------------------

void KIntNumInput::setSpinBoxSize(int frac)
{
    spin_frac = QMIN(QMAX(frac,1),100);
    setMinimumSize(minimumSize());
    
}


// -----------------------------------------------------------------------------

QSizePolicy KIntNumInput::sizePolicy() const 
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
}


// -----------------------------------------------------------------------------

void KIntNumInput::setEnabled(bool on)
{
    main_label->setEnabled(on);
    spin->setEnabled(on);
    if(slider)
        slider->setEnabled(on);
}


// -----------------------------------------------------------------------------

#include "knuminput.moc"
