/*
 * knuminput.cpp
 *
 * Initial implementation:
 *     Copyright (c) 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
 *
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

#include <math.h>

#include <qlabel.h>
#include <qlineedit.h>
#include <qsize.h>
#include <qslider.h>
#include <qspinbox.h>

#include "knumvalidator.h"
#include "knuminput.h"


// -----------------------------------------------------------------------------

KNumInput::KNumInput(QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    m_prev = m_next = 0;
}


KNumInput::KNumInput(KNumInput* below, QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    m_prev = m_next = 0;

    if(below) {
        m_next = below->m_next;
        m_prev = below;
        below->m_next = this;
        if(m_next)
            m_next->m_prev = this;
    }
}


KNumInput::~KNumInput()
{
    if(m_prev)
        m_prev->m_next = m_next;

    if(m_next)
        m_next->m_prev = m_prev;
}


// -----------------------------------------------------------------------------

KIntSpinBox::KIntSpinBox(int lower, int upper, int step, int value, int base,
                         QWidget* parent, const char* name)
    : QSpinBox(lower, upper, step, parent, name)
{
    editor()->setAlignment(AlignRight);
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
    : KNumInput(parent, name)
{
    init(label, lower, upper, step, val, units, _base, use_slider);

}



// -----------------------------------------------------------------------------

KIntNumInput::KIntNumInput(int lower, int upper, int step, int value, QWidget* parent,
                           const QString& label, const QString& units, bool use_slider,
                           int _base, const char* name)
    : KNumInput(parent, name)
{
    init(label, lower, upper, step, value, units, _base, use_slider);
}


// -----------------------------------------------------------------------------

void KIntNumInput::init(const QString& label, int lower, int upper, int step, int val,
                        const QString& units, int _base, bool use_slider)
{
    int_value = val;

    spin = new KIntSpinBox(lower, upper, step, int_value, _base, this, "KIntSpinBox");
    spin->setValidator(new KIntValidator(this, _base, "KNumInput::KIntValidtr"));
    connect(spin, SIGNAL(valueChanged(int)), SLOT(setValue(int)));

    main_label = new QLabel( spin, label, this, "KNumInput::QLabel" );

	if ( label.isEmpty() )
		main_label->hide();

    if(!units.isEmpty())
        spin->setSuffix(' ' + units);

    if(use_slider) {
        slider = new QSlider(lower, upper, step, int_value, QSlider::Horizontal, this);
        slider->setTickmarks(QSlider::Below);

        connect(slider, SIGNAL(valueChanged(int)), SLOT(setValue(int)));

        // default values
        int major = (upper-lower)/_base;
        slider->setSteps( 1, major );
        slider->setTickInterval(major);
    }
    else
        slider = 0;

    setLabelAlignment( AlignLeft );
    setSpinBoxSize(33);
    spin_size = spin->sizeHint();
}

// -----------------------------------------------------------------------------
QSize KIntNumInput::minimumSize() const
{
    QSize qs;

    main_label->adjustSize();
//    spin_size = spin->sizeHint();

    int w, h;

    if(slider) {
        w = QMAX((spin_size.width()*100)/spin_frac, main_label->width());
        h = QMAX(slider->sizeHint().height(), spin_size.height());
    }
    else {
        w = QMAX(spin_size.width(), main_label->width());
        // for some reason, spin_size.height() doesn't work here
        h = spin->sizeHint().height();
    }

    h += (main_label->text().isEmpty() ? 0 : main_label->height() + 5);

    qs.setWidth(w);
    qs.setHeight(h);

    return qs;
}

// -----------------------------------------------------------------------------
QSize KIntNumInput::sizeHint() const
{
    return minimumSize();
}

// -----------------------------------------------------------------------------

void KIntNumInput::resizeEvent(QResizeEvent* e)
{
    int left_frac = QMAX(spin_size.width(), (width()*spin_frac)/100);
    int label_height = (main_label->text().isEmpty() ? 0 : main_label->height() + 4);

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

        spin->resize(left_frac, spin_size.height());
    }
    else
        spin->resize(width(), spin_size.height());
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
    if (val != int_value)
    {
        int_value = val;
        spin->setValue(val);
        if (slider)
            slider->setValue(val);
        emit valueChanged(val);
    }
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

void KIntNumInput::setSpecialValueText(const QString& text)
{
    spin->setSpecialValueText(text);
};

// -----------------------------------------------------------------------------






// -----------------------------------------------------------------------------


KDoubleNumInput::KDoubleNumInput(double value, QWidget *parent, const char *name)
    : KNumInput(parent, name)
{
    init(value);
}


// -----------------------------------------------------------------------------

KDoubleNumInput::KDoubleNumInput(KNumInput* below, double value, QWidget* parent,
                                 const char* name)
    : KNumInput(below, parent, name)
{
    init(value);
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::init(double value)
{
    m_value = value;
    m_format = "%.02g";

    edit = new QLineEdit(this, "KDoubleNumInput::QLineEdit");
    edit->setAlignment(AlignRight);
    edit->setValidator(new KFloatValidator(this, "KDoubleNumInput::KFloatValidator"));

    main_label = 0;
    m_slider = 0;
    m_suffix = m_prefix = "";

    resetEditBox();
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::sliderMoved(int val)
{
    m_value = m_lower + val*m_step;

    resetEditBox();
    emit valueChanged(m_value);
}


// -----------------------------------------------------------------------------

QSize KDoubleNumInput::minimumSize() const
{
    int w = 0;
    int h = 0;

    // if in extra row, then count it here
    if(main_label && (m_alignment & (AlignBottom|AlignTop)))
        h += 4 + m_sizeLabel.height();
    else
        // no extra frame space
        h += m_sizeLabel.height();

    h += 2 + QMAX(m_sizeEdit.height(), m_sizeSlider.height());

    w += m_slider ? m_slider->sizeHint().width() + 8 : 0;
    w += m_sizeEdit.width();
    w += main_label && (m_alignment & AlignVCenter) ? m_sizeLabel.width() + 2 : 0;

    if(m_alignment & (AlignTop|AlignBottom))
        w = QMAX(w, m_sizeLabel.width() + 4);

    return QSize(w, h);
}

// -----------------------------------------------------------------------------

QSize KDoubleNumInput::sizeHint() const
{
    return minimumSize();
}

// -----------------------------------------------------------------------------

void KDoubleNumInput::resizeEvent(QResizeEvent* e)
{
    int w = 0;
    int h = 0;

    if(main_label && (m_alignment & AlignTop)) {
        main_label->setGeometry(0, 0, e->size().width(), m_sizeLabel.height());
        h += m_sizeLabel.height() + 4;
    }

    if(main_label && (m_alignment & AlignVCenter)) {
        main_label->setGeometry(0, 0, m_sizeLabel.width(), m_sizeLabel.height());
        w += m_sizeLabel.width() + 2;
    }

    edit->setGeometry(w, h, m_slider ? m_sizeEdit.width() : e->size().width(), m_sizeEdit.height());
    w += m_sizeEdit.width() + 8;

    if(m_slider)
        m_slider->setGeometry(w, h, e->size().width() - w, m_sizeEdit.height());

    h += m_sizeLabel.height() + 2;

    if(main_label && (m_alignment & AlignBottom))
        main_label->setGeometry(0, h, m_sizeLabel.width(), m_sizeLabel.height());
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::doLayout()
{
    // edit sizeHint
    edit->constPolish();
    QFontMetrics fm( edit->font() );
    QString s;
    int h = fm.height();
    int w  = fm.width(m_prefix) + fm.width(m_suffix);
    s.sprintf(m_format.latin1(), m_value);
    w = QMAX(w, QMAX(fm.width(s), fm.width(m_specialvalue)));
    if(m_range) {
        s.sprintf(m_format.latin1(), m_lower);
        w = QMAX(w, fm.width(s));
        s.sprintf(m_format.latin1(), m_upper);
        w = QMAX(w, fm.width(s));
        // something inbetween
        s.sprintf(m_format.latin1(), m_lower + m_step);
        w = QMAX(w, fm.width(s));
    }

    if ( edit->frame() ) {
        h += 8;
        if ( edit->style() == WindowsStyle && h < 26 )
            h = 22;
        m_sizeEdit.setWidth(w + 8);
        m_sizeEdit.setHeight(h);
    } else {
        m_sizeEdit.setWidth(w + 4);
        m_sizeEdit.setHeight(h + 4);
    }

    // label sizeHint
    m_sizeLabel = (main_label ? main_label->sizeHint() : QSize(0,0));

    // slider sizeHint
    m_sizeSlider = (m_slider ? m_slider->sizeHint() : QSize(0, 0));
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::setValue(double val)
{
    m_value = val;

    if(m_value < m_lower) m_value = m_lower;
    if(m_upper < m_value) m_value = m_upper;

    if(m_slider)  m_slider->setValue(val);

    resetEditBox();
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::setRange(double lower, double upper, double step, bool slider)
{
    m_lower = lower;
    m_upper = upper;
    m_step  = step;
    m_range = (m_lower < m_upper);

    // bounds checking the values
    if(m_value < m_lower)  m_value = m_lower;
    if(m_upper < m_lower)  m_upper = m_value;
    if(m_upper < m_value)  m_value = m_upper;

    // make m_value a multiple of step
    m_value = floor(m_value / m_step) * m_step;

    if(slider) {
        m_slider = new QSlider(0, floor((m_upper - m_lower)/m_step), 1,
                               floor(m_value/m_step), QSlider::Horizontal, this);
        m_slider->setTickmarks(QSlider::Below);
        m_slider->setTickInterval((m_slider->maxValue() - m_slider->minValue()) / 14);
        connect(m_slider, SIGNAL(valueChanged(int)), SLOT(sliderMoved(int)));
    }
    else {
        delete m_slider;
        m_slider = 0;
    }

    resetEditBox();
    doLayout();
}


// -----------------------------------------------------------------------------

double  KDoubleNumInput::value()
{
    return m_value;
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::setLabel(QString label, int a)
{
    if(label.isEmpty()) {
        delete main_label;
        main_label = 0;
        m_alignment = 0;
    }
    else {
        main_label = new QLabel(edit, label, this, "KDoubleNumInput::QLabel");
        main_label->setAlignment((a & (~(AlignTop|AlignBottom|AlignVCenter))) | AlignVCenter);
        // if no vertical alignment set, use Top alignment
        if(!(a & (AlignTop|AlignBottom|AlignVCenter)))
           a |= AlignTop;
        m_alignment = a;
    }

    doLayout();
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::setSuffix(QString suffix)
{
    m_suffix = suffix;

    resetEditBox();
    doLayout();
}

// -----------------------------------------------------------------------------

void KDoubleNumInput::setPrefix(QString prefix)
{
    m_prefix = prefix;

    resetEditBox();
    doLayout();
}

// -----------------------------------------------------------------------------

void KDoubleNumInput::setFormat(const char* fmt)
{
    m_format = fmt;

    resetEditBox();
    doLayout();
}

// -----------------------------------------------------------------------------

void KDoubleNumInput::resetEditBox()
{
    if(!m_specialvalue.isEmpty() && (fabs(m_value - m_lower) < 1e-10))
        edit->setText(m_specialvalue);
    else {
        QString s;
        s.sprintf(m_format.latin1(), m_value);
        edit->setText(m_prefix + s + m_suffix);
    }
}

// -----------------------------------------------------------------------------

QSizePolicy KDoubleNumInput::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::setSpecialValueText(const QString& text)
{
    m_specialvalue = text;

    resetEditBox();
    doLayout();
};


// -----------------------------------------------------------------------------

#include "knuminput.moc"
