// -*- c-basic-offset: 4 -*-
/*
 * knuminput.cpp
 *
 * Initial implementation:
 *     Copyright (c) 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
 * Rewritten and maintained by:
 *     Copyright (c) 2000 Dirk A. Mueller <mueller@kde.org>
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

#include <config.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <math.h>

#include <qlabel.h>
#include <qlineedit.h>
#include <qsize.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qstyle.h>

#include <kglobal.h>
#include <klocale.h>

#include "knumvalidator.h"
#include "knuminput.h"


// -----------------------------------------------------------------------------

KNumInput::KNumInput(QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    init();
}


// -----------------------------------------------------------------------------

KNumInput::KNumInput(KNumInput* below, QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    init();

    if(below) {
        m_next = below->m_next;
        m_prev = below;
        below->m_next = this;
        if(m_next)
            m_next->m_prev = this;
    }
}


// -----------------------------------------------------------------------------

void KNumInput::init()
{
    m_prev = m_next = 0;
    m_colw1 = m_colw2 = 0;

    m_label = 0;
    m_slider = 0;
    m_alignment = 0;
}


// -----------------------------------------------------------------------------

KNumInput::~KNumInput()
{
    if(m_prev)
        m_prev->m_next = m_next;

    if(m_next)
        m_next->m_prev = m_prev;
}


// -----------------------------------------------------------------------------

void KNumInput::setLabel(const QString & label, int a)
{
    if(label.isEmpty()) {
        delete m_label;
        m_label = 0;
        m_alignment = 0;
    }
    else {
        if (m_label) m_label->setText(label);
        else m_label = new QLabel(label, this, "KNumInput::QLabel");
        m_label->setAlignment((a & (~(AlignTop|AlignBottom|AlignVCenter)))
                              | AlignVCenter);
        // if no vertical alignment set, use Top alignment
        if(!(a & (AlignTop|AlignBottom|AlignVCenter)))
           a |= AlignTop;
        m_alignment = a;
    }

    layout(true);
}

QString KNumInput::label() const
{
    if (m_label) return m_label->text();
    return QString::null;
}

// -----------------------------------------------------------------------------

void KNumInput::layout(bool deep)
{
    int w1 = m_colw1;
    int w2 = m_colw2;

    // label sizeHint
    m_sizeLabel = (m_label ? m_label->sizeHint() : QSize(0,0));

    if(m_label && (m_alignment & AlignVCenter))
        m_colw1 = m_sizeLabel.width() + 4;
    else
        m_colw1 = 0;

    // slider sizeHint
    m_sizeSlider = (m_slider ? m_slider->sizeHint() : QSize(0, 0));

    doLayout();

    if(!deep) {
        m_colw1 = w1;
        m_colw2 = w2;
        return;
    }

    KNumInput* p = this;
    while(p) {
        p->doLayout();
        w1 = QMAX(w1, p->m_colw1);
        w2 = QMAX(w2, p->m_colw2);
        p = p->m_prev;
    }

    p = m_next;
    while(p) {
        p->doLayout();
        w1 = QMAX(w1, p->m_colw1);
        w2 = QMAX(w2, p->m_colw2);
        p = p->m_next;
    }

    p = this;
    while(p) {
        p->m_colw1 = w1;
        p->m_colw2 = w2;
        p = p->m_prev;
    }

    p = m_next;
    while(p) {
        p->m_colw1 = w1;
        p->m_colw2 = w2;
        p = p->m_next;
    }

//    kdDebug() << "w1 " << w1 << " w2 " << w2 << endl;
}


// -----------------------------------------------------------------------------

QSizePolicy KNumInput::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
}


// -----------------------------------------------------------------------------

QSize KNumInput::sizeHint() const
{
    return minimumSizeHint();
}


// -----------------------------------------------------------------------------

void KNumInput::setSteps(int minor, int major)
{
    if(m_slider)
        m_slider->setSteps( minor, major );
}


// -----------------------------------------------------------------------------

KIntSpinBox::KIntSpinBox(QWidget *parent, const char *name)
    : QSpinBox(0, 99, 1, parent, name)
{
    editor()->setAlignment(AlignRight);
    val_base = 10;
    setValue(0);
}

KIntSpinBox::~KIntSpinBox()
{
}

KIntSpinBox::KIntSpinBox(int lower, int upper, int step, int value, int base,
                         QWidget* parent, const char* name)
    : QSpinBox(lower, upper, step, parent, name)
{
    editor()->setAlignment(AlignRight);
    val_base = base;
    setValue(value);
}

void KIntSpinBox::setBase(int base)
{
    val_base = base;
}


int KIntSpinBox::base() const
{
    return val_base;
}


// -----------------------------------------------------------------------------

QString KIntSpinBox::mapValueToText(int v)
{
    return QString::number(v, val_base);
}


// -----------------------------------------------------------------------------

int KIntSpinBox::mapTextToValue(bool* ok)
{
    return cleanText().toInt(ok, val_base);
}


// -----------------------------------------------------------------------------

void KIntSpinBox::setEditFocus(bool mark)
{
    editor()->setFocus();
    if(mark)
        editor()->selectAll();
}


// -----------------------------------------------------------------------------

KIntNumInput::KIntNumInput(KNumInput* below, int val, QWidget* parent,
                           int _base, const char* name)
    : KNumInput(below, parent, name)
{
    init(val, _base);
}


// -----------------------------------------------------------------------------

KIntNumInput::KIntNumInput(QWidget *parent, const char *name)
    : KNumInput(parent, name)
{
    init(0, 10);
}


KIntNumInput::KIntNumInput(int val, QWidget *parent, int _base, const char *name)
    : KNumInput(parent, name)
{
    init(val, _base);

}


// -----------------------------------------------------------------------------

void KIntNumInput::init(int val, int _base)
{
    m_spin = new KIntSpinBox(INT_MIN, INT_MAX, 1, val, _base, this, "KIntNumInput::KIntSpinBox");
    m_spin->setValidator(new KIntValidator(this, _base, "KNumInput::KIntValidtr"));
    connect(m_spin, SIGNAL(valueChanged(int)), SLOT(spinValueChanged(int)));

    setFocusProxy(m_spin);
    layout(true);
}


// -----------------------------------------------------------------------------

void KIntNumInput::spinValueChanged(int val)
{
    if(m_slider)
        m_slider->setValue(val);

    emit valueChanged(val);
}


// -----------------------------------------------------------------------------

void KIntNumInput::setRange(int lower, int upper, int step, bool slider)
{
    m_spin->setMinValue(lower);
    m_spin->setMaxValue(upper);
    m_spin->setLineStep(step);

    if(slider) {
                if (m_slider)
                        m_slider->setRange(lower, upper);
                else {
                        m_slider = new QSlider(lower, upper, step, m_spin->value(),
                                                                   QSlider::Horizontal, this);
                        m_slider->setTickmarks(QSlider::Below);
                        connect(m_slider, SIGNAL(valueChanged(int)),
                                        m_spin, SLOT(setValue(int)));
                }

        int major = (upper-lower)/10;
                if ( major==0 ) major = step; // #### workaround Qt bug in 2.1-beta4
        m_slider->setSteps(step, major);
        m_slider->setTickInterval(major);
    }
    else {
        delete m_slider;
        m_slider = 0;
    }

    layout(true);
}

void KIntNumInput::setMinValue(int min)
{
    setRange(min, m_spin->maxValue(), m_spin->lineStep(), m_slider);
}

int KIntNumInput::minValue() const
{
    return m_spin->minValue();
}

void KIntNumInput::setMaxValue(int max)
{
    setRange(m_spin->minValue(), max, m_spin->lineStep(), m_slider);
}

int KIntNumInput::maxValue() const
{
    return m_spin->maxValue();
}


// -----------------------------------------------------------------------------

void KIntNumInput::setSuffix(const QString &suffix)
{
    m_spin->setSuffix(suffix);

    layout(true);
}


QString KIntNumInput::suffix() const
{
    return m_spin->suffix();
}


// -----------------------------------------------------------------------------

void KIntNumInput::setPrefix(const QString &prefix)
{
    m_spin->setPrefix(prefix);

    layout(true);
}


QString KIntNumInput::prefix() const
{
    return m_spin->prefix();
}


// -----------------------------------------------------------------------------

void KIntNumInput::setEditFocus(bool mark)
{
    m_spin->setEditFocus(mark);
}


// -----------------------------------------------------------------------------

QSize KIntNumInput::minimumSizeHint() const
{
    constPolish();

    int w;
    int h;

    h = 2 + QMAX(m_sizeSpin.height(), m_sizeSlider.height());

    // if in extra row, then count it here
    if(m_label && (m_alignment & (AlignBottom|AlignTop)))
        h += 4 + m_sizeLabel.height();
    else
        // label is in the same row as the other widgets
        h = QMAX(h, m_sizeLabel.height() + 2);

    w = m_slider ? m_slider->sizeHint().width() + 8 : 0;
    w += m_colw1 + m_colw2;

    if(m_alignment & (AlignTop|AlignBottom))
        w = QMAX(w, m_sizeLabel.width() + 4);

    return QSize(w, h);
}


// -----------------------------------------------------------------------------

void KIntNumInput::doLayout()
{
    m_sizeSpin = m_spin->sizeHint();

    m_colw2 = m_sizeSpin.width();
}


// -----------------------------------------------------------------------------

void KIntNumInput::resizeEvent(QResizeEvent* e)
{
    int w = m_colw1;
    int h = 0;

    if(m_label && (m_alignment & AlignTop)) {
        m_label->setGeometry(0, 0, e->size().width(), m_sizeLabel.height());
        h += m_sizeLabel.height() + 4;
    }

    if(m_label && (m_alignment & AlignVCenter))
        m_label->setGeometry(0, 0, w, m_sizeSpin.height());

    m_spin->setGeometry(w, h, m_slider ? m_colw2 : QMAX(m_colw2, e->size().width() - w), m_sizeSpin.height());
    w += m_colw2 + 8;

    if(m_slider)
        m_slider->setGeometry(w, h, e->size().width() - w, m_sizeSpin.height());

    h += m_sizeSpin.height() + 2;

    if(m_label && (m_alignment & AlignBottom))
        m_label->setGeometry(0, h, m_sizeLabel.width(), m_sizeLabel.height());
}


// -----------------------------------------------------------------------------

KIntNumInput::~KIntNumInput()
{
}


// -----------------------------------------------------------------------------

void KIntNumInput::setValue(int val)
{
    m_spin->setValue(val);
    // slider value is changed by spinValueChanged
}


// -----------------------------------------------------------------------------

int  KIntNumInput::value() const
{
    return m_spin->value();
}


// -----------------------------------------------------------------------------

void KIntNumInput::setSpecialValueText(const QString& text)
{
    m_spin->setSpecialValueText(text);
    layout(true);
};


QString KIntNumInput::specialValueText() const
{
    return m_spin->specialValueText();
}


// -----------------------------------------------------------------------------

void KIntNumInput::setLabel(const QString & label, int a)
{
    KNumInput::setLabel(label, a);

    if(m_label)
        m_label->setBuddy(m_spin);
}


// -----------------------------------------------------------------------------

/**
 *
 * This is just a small wrapper mainly to enable mouseWheel support
 *
 */

class KDoubleLine : public QLineEdit
{
public:
    KDoubleLine(KDoubleNumInput* parent, const char* name)
        : QLineEdit(parent, name)
        { };

    void interpretText();
protected:
    virtual void wheelEvent(QWheelEvent* e);
};

void KDoubleLine::wheelEvent(QWheelEvent* e)
{
    KDoubleNumInput* w = static_cast<KDoubleNumInput*>(parent());

    if(edited())
        interpretText();

    if(w->m_range) {
        w->setValue(w->value() + (e->delta()/120)*w->m_step);
        e->accept();
    }
    else
        e->ignore();
}


void KDoubleLine::interpretText()
{
    KDoubleNumInput* w = static_cast<KDoubleNumInput*>(parent());

    QString s = QString(text()).stripWhiteSpace();
    if ( !w->m_prefix.isEmpty() ) {
        QString px = QString(w->m_prefix).stripWhiteSpace();
        int len = px.length();
        if ( len && s.left(len) == px )
            s.remove( 0, len );
    }
    if ( !w->m_suffix.isEmpty() ) {
        QString sx = QString(w->m_suffix).stripWhiteSpace();
        int len = sx.length();
        if ( len && s.right(len) == sx )
            s.truncate( s.length() - len );
    }

    s = s.stripWhiteSpace();

    if(edited()) {
        bool ok;
        double value = KGlobal::locale()->readNumber(s, &ok);
	// kdDebug() << "input: " << s << ": read as: " << value
	// << ": ok: " << ok << endl;

        if(ok) {
            w->m_value = value;
            setEdited(false);
        }

    }
}


// -----------------------------------------------------------------------------

KDoubleNumInput::KDoubleNumInput(QWidget *parent, const char *name)
    : KNumInput(parent, name)
{
    init(0.0);
}


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

KDoubleNumInput::~KDoubleNumInput()
{
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::init(double value)
{
    m_value = value;
    m_precision = 2;
    m_range = false;

    edit = new KDoubleLine(this, "KDoubleNumInput::QLineEdit");
    edit->setAlignment(AlignRight);
    KFloatValidator* validator = new KFloatValidator(this, "KDoubleNumInput::KFloatValidator");
    validator->setAcceptLocalizedNumbers( true );
    edit->setValidator( validator );
    edit->installEventFilter( this );
    setFocusProxy(edit);

    m_suffix = m_prefix = "";
    layout(true);

    resetEditBox();
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::sliderMoved(int val)
{
    m_value = m_lower + (double)val * (m_upper - m_lower)/m_slider->maxValue();

    resetEditBox();
    emit valueChanged(m_value);
}


// -----------------------------------------------------------------------------

QSize KDoubleNumInput::minimumSizeHint() const
{
    constPolish();

    int w;
    int h;

    h = 2 + QMAX(m_sizeEdit.height(), m_sizeSlider.height());

    // if in extra row, then count it here
    if(m_label && (m_alignment & (AlignBottom|AlignTop)))
        h += 4 + m_sizeLabel.height();
    else
        // label is in the same row as the other widgets
	h = QMAX(h, m_sizeLabel.height() + 2);

    w = m_slider ? m_slider->sizeHint().width() + 8 : 0;
    w += m_colw1 + m_colw2;

    if(m_alignment & (AlignTop|AlignBottom))
        w = QMAX(w, m_sizeLabel.width() + 4);

    return QSize(w, h);
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::resizeEvent(QResizeEvent* e)
{
    int w = m_colw1;
    int h = 0;

    if(m_label && (m_alignment & AlignTop)) {
        m_label->setGeometry(0, 0, e->size().width(), m_sizeLabel.height());
        h += m_sizeLabel.height() + 4;
    }

    if(m_label && (m_alignment & AlignVCenter))
        m_label->setGeometry(0, 0, w, m_sizeEdit.height());

    edit->setGeometry(w, h, m_slider ? m_colw2
                                          : e->size().width() - w, m_sizeEdit.height());
    w += m_colw2 + 8;

    if(m_slider)
        m_slider->setGeometry(w, h, e->size().width() - w, m_sizeEdit.height());

    h += m_sizeEdit.height() + 2;

    if(m_label && (m_alignment & AlignBottom))
        m_label->setGeometry(0, h, m_sizeLabel.width(), m_sizeLabel.height());
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::doLayout()
{
    // edit sizeHint
    edit->constPolish();
    QFontMetrics fm( edit->font() );
    QString s;
    int h = fm.height();
    s = KGlobal::locale()->formatNumber(m_value, m_precision);
    int w0 = fm.width( m_prefix ) + fm.width( m_suffix );
    int w = w0 + fm.width(s);
    w = QMAX(w, fm.width(m_specialvalue));
    if(m_range) {
        s = KGlobal::locale()->formatNumber(m_lower, m_precision);
        w = QMAX(w, fm.width(s)+w0);
        s = KGlobal::locale()->formatNumber(m_upper, m_precision);
        w = QMAX(w, fm.width(s)+w0);
        // something inbetween
        s = KGlobal::locale()->formatNumber(m_lower + m_step, m_precision);
        w = QMAX(w, fm.width(s)+w0);
    }

    if ( edit->frame() ) {
        h += 8;
        if ( edit->style().styleHint( QStyle::SH_GUIStyle ) == WindowsStyle && h < 26 )
            h = 22;
        m_sizeEdit.setWidth(w + 8);
        m_sizeEdit.setHeight(h);
    } else {
        m_sizeEdit.setWidth(w + 4);
        m_sizeEdit.setHeight(h + 4);
    }

    m_colw2 = m_sizeEdit.width();
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::setValue(double val)
{
    double prevVal = m_value;
    m_value = val;
    
    if(m_range) {
      if(m_value < m_lower) m_value = m_lower;
      if(m_upper < m_value) m_value = m_upper;
    }

    if(m_slider) {
        int slvalue = int(m_slider->maxValue()
                            * (m_value - m_lower)/(m_upper - m_lower));
        m_slider->blockSignals(true);
        m_slider->setValue(slvalue);
        m_slider->blockSignals(false);
    }

    resetEditBox();

    if ( m_range && fabs(prevVal - m_value) > m_step )
        emit valueChanged(m_value);
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::setRange(double lower, double upper, double step,
                                                           bool slider)
{
    double prevVal = m_value;
    m_lower = lower;
    m_upper = upper;
    m_step  = step;
    m_range = (m_lower < m_upper);

    // bounds checking the values
    if(m_value < m_lower)  m_value = m_lower;
    if(m_upper < m_lower)  m_upper = m_value;
    if(m_upper < m_value)  m_value = m_upper;

    // make m_value a multiple of step
    m_value = floor(( m_value+.5*m_step ) / m_step) * m_step;

    if(slider) {
        int slmax = QMIN(INT_MAX, (int)((m_upper - m_lower)/m_step));
        int slvalue = (int)(slmax * (m_value - m_lower) / (m_upper - m_lower));
        if (m_slider) {
            m_slider->setRange(0, slmax);
            m_slider->setValue(slvalue);
        } else {
            m_slider = new QSlider(0, slmax, 1, slvalue,
                                   QSlider::Horizontal, this);
            m_slider->setTickmarks(QSlider::Below);
            connect(m_slider, SIGNAL(valueChanged(int)),
                    SLOT(sliderMoved(int)));
        }
        m_slider->setTickInterval(slmax / 10);
    } else {
        delete m_slider;
        m_slider = 0;
    }

    resetEditBox();
    layout(true);
    if ( prevVal!=m_value ) emit valueChanged(m_value);
}

void KDoubleNumInput::setMinValue(double min)
{
    setRange(min, m_upper, m_step, m_slider);
}

double KDoubleNumInput::minValue() const
{
    return m_lower;
}

void KDoubleNumInput::setMaxValue(double max)
{
    setRange(m_lower, max, m_step, m_slider);
}

double KDoubleNumInput::maxValue() const
{
    return m_upper;
}


// -----------------------------------------------------------------------------

double  KDoubleNumInput::value() const
{
    if(edit->edited())
        edit->interpretText();

    return m_value;
}


QString KDoubleNumInput::suffix() const
{
    return m_suffix;
}


QString KDoubleNumInput::prefix() const
{
    return m_prefix;
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::setSuffix(const QString &suffix)
{
    m_suffix = suffix;

    resetEditBox();
    layout(true);
}

// -----------------------------------------------------------------------------

void KDoubleNumInput::setPrefix(const QString &prefix)
{
    m_prefix = prefix;

    resetEditBox();
    layout(true);
}

// -----------------------------------------------------------------------------

void KDoubleNumInput::setPrecision(int precision)
{
    m_precision = precision;

    resetEditBox();
    layout(true);
}

int KDoubleNumInput::precision() const
{
    return m_precision;
}

// -----------------------------------------------------------------------------

void KDoubleNumInput::resetEditBox()
{
    if(!m_specialvalue.isEmpty() && (fabs(m_value - m_lower) < 1e-10)) {
        edit->setText(m_specialvalue);
        edit->home( false );
    }
    else {
        QString s;
        s = KGlobal::locale()->formatNumber(m_value, m_precision);
        edit->setText(m_prefix + s + m_suffix);
        edit->home( false );
    }
}


// -----------------------------------------------------------------------------

void KDoubleNumInput::setSpecialValueText(const QString& text)
{
    m_specialvalue = text;

    resetEditBox();
    layout(true);
};


// -----------------------------------------------------------------------------

void KDoubleNumInput::setLabel(const QString & label, int a)
{
    KNumInput::setLabel(label, a);

    if(m_label)
        m_label->setBuddy(edit);

}

// -----------------------------------------------------------------------------

bool KDoubleNumInput::eventFilter( QObject* obj, QEvent* ev )
{
    if ( obj != edit )
        return false;

    bool revalue = false;
    double old_value = m_value;

    if (ev->type() == QEvent::FocusOut || ev->type() == QEvent::Leave || ev->type() == QEvent::Hide) {
        edit->interpretText();
        revalue = false;
    } else if ( ev->type() == QEvent::KeyPress ) {
        QKeyEvent* k = (QKeyEvent*)ev;
        if ( k->key() == Key_Up || k->text() == "+" ) {
            if(m_range)
                m_value += m_step;
            revalue = true;
        } else if ( k->key() == Key_Down || k->text() == "-" ) {
            if(m_range)
                m_value -= m_step;
            revalue = true;
        } else if ( k->key() == Key_Return ) {
            edit->interpretText();
            revalue = false;
        }
    }

    if(m_value != old_value) {
        setValue(m_value);
        emit valueChanged(m_value);
    }

    return revalue;
}


// -----------------------------------------------------------------------------

void KNumInput::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KIntNumInput::virtual_hook( int id, void* data )
{ KNumInput::virtual_hook( id, data ); }

void KDoubleNumInput::virtual_hook( int id, void* data )
{ KNumInput::virtual_hook( id, data ); }

void KIntSpinBox::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "knuminput.moc"
