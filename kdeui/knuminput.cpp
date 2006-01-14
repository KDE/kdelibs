// -*- c-basic-offset: 4 -*-
/*
 * Initial implementation:
 *     Copyright (c) 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
 * Rewritten and maintained by:
 *     Copyright (c) 2000 Dirk A. Mueller <mueller@kde.org>
 * KDoubleSpinBox:
 *     Copyright (c) 2002 Marc Mutz <mutz@kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <config.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <assert.h>
#include <math.h>
#include <algorithm>

#include <qapplication.h>
#include <qevent.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qsize.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qstyle.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "kdialog.h"
#include "knumvalidator.h"
#include "knuminput.h"

static inline int calcDiffByTen( int x, int y ) {
    // calculate ( x - y ) / 10 without overflowing ints:
    return ( x / 10 ) - ( y / 10 )  +  ( x % 10 - y % 10 ) / 10;
}

// ----------------------------------------------------------------------------

KNumInput::KNumInput(QWidget* parent)
    : QWidget(parent)
{
    init();
}

KNumInput::KNumInput(QWidget* parent, KNumInput* below)
    : QWidget(parent)
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

void KNumInput::init()
{
    m_prev = m_next = 0;
    m_colw1 = m_colw2 = 0;

    m_label = 0;
    m_slider = 0;
    m_alignment = 0;
}

KNumInput::~KNumInput()
{
    if(m_prev)
        m_prev->m_next = m_next;

    if(m_next)
        m_next->m_prev = m_prev;
}

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
        m_label->setAlignment((a & (~(Qt::AlignTop|Qt::AlignBottom|Qt::AlignVCenter)))
                              | Qt::AlignVCenter);
        // if no vertical alignment set, use Top alignment
        if(!(a & (Qt::AlignTop|Qt::AlignBottom|Qt::AlignVCenter)))
           a |= Qt::AlignTop;
        m_alignment = a;
    }

    layout(true);
}

QString KNumInput::label() const
{
    if (m_label) return m_label->text();
    return QString();
}

void KNumInput::layout(bool deep)
{
    int w1 = m_colw1;
    int w2 = m_colw2;

    // label sizeHint
    m_sizeLabel = (m_label ? m_label->sizeHint() : QSize(0,0));

    if(m_label && (m_alignment & Qt::AlignVCenter))
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
        w1 = qMax(w1, p->m_colw1);
        w2 = qMax(w2, p->m_colw2);
        p = p->m_prev;
    }

    p = m_next;
    while(p) {
        p->doLayout();
        w1 = qMax(w1, p->m_colw1);
        w2 = qMax(w2, p->m_colw2);
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

QSizePolicy KNumInput::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
}

QSize KNumInput::sizeHint() const
{
    return minimumSizeHint();
}

void KNumInput::setSteps(int minor, int major)
{
    if(m_slider)
        m_slider->setSteps( minor, major );
}


// ----------------------------------------------------------------------------

KIntSpinBox::KIntSpinBox(QWidget *parent)
    : QSpinBox(parent),
      val_base(10)
{
    setRange(0,99);
    setSingleStep(1);
    lineEdit()->setAlignment(Qt::AlignRight);
    setValue(0);
}

KIntSpinBox::~KIntSpinBox()
{
}

KIntSpinBox::KIntSpinBox(int lower, int upper, int step, int value, QWidget *parent,int base)
    : QSpinBox(parent),
      val_base(base)
{
    setRange(lower,upper);
    setSingleStep(step);
    lineEdit()->setAlignment(Qt::AlignRight);
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

QString KIntSpinBox::textFromValue(int v) const
{
    return QString::number(v, val_base);
}

int KIntSpinBox::valueFromText(const QString &text) const
{
    bool ok;
    return text.toInt(&ok, val_base);
}

void KIntSpinBox::setEditFocus(bool mark)
{
    lineEdit()->setFocus();
    if(mark)
        lineEdit()->selectAll();
}


// ----------------------------------------------------------------------------

class KIntNumInput::KIntNumInputPrivate {
public:
    int referencePoint;
    short blockRelative;
    KIntNumInputPrivate( int r )
	: referencePoint( r ),
	  blockRelative( 0 ) {}
};


KIntNumInput::KIntNumInput(KNumInput* below, int val,QWidget *parent,int _base)
    : KNumInput(parent,below)
{
    init(val, _base);
}

KIntNumInput::KIntNumInput(QWidget *parent)
    : KNumInput(parent)
{
    init(0, 10);
}

KIntNumInput::KIntNumInput(int val, QWidget *parent,int _base)
    : KNumInput(parent)
{
    init(val, _base);

}

void KIntNumInput::init(int val, int _base)
{
    d = new KIntNumInputPrivate( val );
    m_spin = new KIntSpinBox(INT_MIN, INT_MAX, 1, val, this, _base);
    m_spin->setObjectName("KIntNumInput::KIntSpinBox");
    // the KIntValidator is broken beyond believe for
    // spinboxes which have suffix or prefix texts, so
    // better don't use it unless absolutely necessary
#ifdef __GNUC__
#warning KDE4 we NEED to fix the validation of values here
#endif
//	if (_base != 10)
//        m_spin->setValidator(new KIntValidator(this, _base, "KNumInput::KIntValidtr"));

    connect(m_spin, SIGNAL(valueChanged(int)), SLOT(spinValueChanged(int)));
    connect(this, SIGNAL(valueChanged(int)),
	    SLOT(slotEmitRelativeValueChanged(int)));

    setFocusProxy(m_spin);
    layout(true);
}

void KIntNumInput::setReferencePoint( int ref ) {
    // clip to valid range:
    ref = qMin( maxValue(), qMax( minValue(),  ref ) );
    d->referencePoint = ref;
}

int KIntNumInput::referencePoint() const {
    return d->referencePoint;
}

void KIntNumInput::spinValueChanged(int val)
{
    if(m_slider)
        m_slider->setValue(val);

    emit valueChanged(val);
}

void KIntNumInput::slotEmitRelativeValueChanged( int value ) {
    if ( d->blockRelative || !d->referencePoint ) return;
    emit relativeValueChanged( double( value ) / double( d->referencePoint ) );
}

void KIntNumInput::setRange(int lower, int upper, int step, bool slider)
{
    upper = qMax(upper, lower);
    lower = qMin(upper, lower);
    m_spin->setMinValue(lower);
    m_spin->setMaxValue(upper);
    m_spin->setLineStep(step);

    step = m_spin->singleStep(); // maybe QRangeControl didn't like out lineStep?

    if(slider) {
	if (m_slider)
	    m_slider->setRange(lower, upper);
	else {
	    m_slider = new QSlider(lower, upper, step, m_spin->value(),
				   Qt::Horizontal, this);
	    m_slider->setTickmarks(QSlider::TicksBelow);
	    connect(m_slider, SIGNAL(valueChanged(int)),
		    m_spin, SLOT(setValue(int)));
	}

	// calculate (upper-lower)/10 without overflowing int's:
        int major = calcDiffByTen( upper, lower );
	if ( major==0 ) major = step; // #### workaround Qt bug in 2.1-beta4

        m_slider->setSteps(step, major);
        m_slider->setTickInterval(major);
    }
    else {
        delete m_slider;
        m_slider = 0;
    }

    // check that reference point is still inside valid range:
    setReferencePoint( referencePoint() );

    layout(true);
}

void KIntNumInput::setMinValue(int min)
{
    setRange(min, m_spin->maxValue(), m_spin->singleStep(), m_slider);
}

int KIntNumInput::minValue() const
{
    return m_spin->minValue();
}

void KIntNumInput::setMaxValue(int max)
{
    setRange(m_spin->minValue(), max, m_spin->singleStep(), m_slider);
}

int KIntNumInput::maxValue() const
{
    return m_spin->maxValue();
}

void KIntNumInput::setSuffix(const QString &suffix)
{
    m_spin->setSuffix(suffix);

    layout(true);
}

QString KIntNumInput::suffix() const
{
    return m_spin->suffix();
}

void KIntNumInput::setPrefix(const QString &prefix)
{
    m_spin->setPrefix(prefix);

    layout(true);
}

QString KIntNumInput::prefix() const
{
    return m_spin->prefix();
}

void KIntNumInput::setEditFocus(bool mark)
{
    m_spin->setEditFocus(mark);
}

QSize KIntNumInput::minimumSizeHint() const
{
    constPolish();

    int w;
    int h;

    h = 2 + qMax(m_sizeSpin.height(), m_sizeSlider.height());

    // if in extra row, then count it here
    if(m_label && (m_alignment & (Qt::AlignBottom|Qt::AlignTop)))
        h += 4 + m_sizeLabel.height();
    else
        // label is in the same row as the other widgets
        h = qMax(h, m_sizeLabel.height() + 2);

    w = m_slider ? m_slider->sizeHint().width() + 8 : 0;
    w += m_colw1 + m_colw2;

    if(m_alignment & (Qt::AlignTop|Qt::AlignBottom))
        w = qMax(w, m_sizeLabel.width() + 4);

    return QSize(w, h);
}

void KIntNumInput::doLayout()
{
    m_sizeSpin = m_spin->sizeHint();
    m_colw2 = m_sizeSpin.width();

    if (m_label)
        m_label->setBuddy(m_spin);
}

void KIntNumInput::resizeEvent(QResizeEvent* e)
{
    int w = m_colw1;
    int h = 0;

    if(m_label && (m_alignment & Qt::AlignTop)) {
        m_label->setGeometry(0, 0, e->size().width(), m_sizeLabel.height());
        h += m_sizeLabel.height() + KDialog::spacingHint();
    }

    if(m_label && (m_alignment & Qt::AlignVCenter))
        m_label->setGeometry(0, 0, w, m_sizeSpin.height());

    if (qApp->reverseLayout())
    {
        m_spin->setGeometry(w, h, m_slider ? m_colw2 : qMax(m_colw2, e->size().width() - w), m_sizeSpin.height());
        w += m_colw2 + 8;

        if(m_slider)
            m_slider->setGeometry(w, h, e->size().width() - w, m_sizeSpin.height());
    }
    else if(m_slider) {
        m_slider->setGeometry(w, h, e->size().width() - (w + m_colw2 + KDialog::spacingHint()), m_sizeSpin.height());
        m_spin->setGeometry(w + m_slider->size().width() + KDialog::spacingHint(), h, m_colw2, m_sizeSpin.height());
    }
    else {
        m_spin->setGeometry(w, h, qMax(m_colw2, e->size().width() - w), m_sizeSpin.height());
    }

    h += m_sizeSpin.height() + 2;

    if(m_label && (m_alignment & Qt::AlignBottom))
        m_label->setGeometry(0, h, m_sizeLabel.width(), m_sizeLabel.height());
}

KIntNumInput::~KIntNumInput()
{
	delete d;
}

void KIntNumInput::setValue(int val)
{
    m_spin->setValue(val);
    // slider value is changed by spinValueChanged
}

void KIntNumInput::setRelativeValue( double r ) {
    if ( !d->referencePoint ) return;
    ++d->blockRelative;
    setValue( int( d->referencePoint * r + 0.5 ) );
    --d->blockRelative;
}

double KIntNumInput::relativeValue() const {
    if ( !d->referencePoint ) return 0;
    return double( value() ) / double ( d->referencePoint );
}

int  KIntNumInput::value() const
{
    return m_spin->value();
}

void KIntNumInput::setSpecialValueText(const QString& text)
{
    m_spin->setSpecialValueText(text);
    layout(true);
}

QString KIntNumInput::specialValueText() const
{
    return m_spin->specialValueText();
}

void KIntNumInput::setLabel(const QString & label, int a)
{
    KNumInput::setLabel(label, a);

    if(m_label)
        m_label->setBuddy(m_spin);
}

// ----------------------------------------------------------------------------

class KDoubleNumInput::KDoubleNumInputPrivate {
public:
    KDoubleNumInputPrivate( double r )
	: spin( 0 ),
	  referencePoint( r ),
	  blockRelative ( 0 ) {}
    KDoubleSpinBox * spin;
    double referencePoint;
    short blockRelative;
};

KDoubleNumInput::KDoubleNumInput(QWidget *parent)
    : KNumInput(parent)
{
    init(0.0, 0.0, 9999.0, 0.01, 2);
}

KDoubleNumInput::KDoubleNumInput(double lower, double upper, double value, QWidget *parent,
				 double step, int precision)
    : KNumInput(parent)
{
    init(value, lower, upper, step, precision);
}

KDoubleNumInput::KDoubleNumInput(KNumInput *below,
				 double lower, double upper, double value, QWidget *parent,
				 double step, int precision)
    : KNumInput(parent,below)
{
    init(value, lower, upper, step, precision);
}

KDoubleNumInput::~KDoubleNumInput()
{
	delete d;
}


void KDoubleNumInput::init(double value, double lower, double upper,
			   double step, int precision )
{
    d = new KDoubleNumInputPrivate( value );

    d->spin = new KDoubleSpinBox(lower, upper, step, value, this,precision);
    d->spin->setObjectName("KDoubleNumInput::d->spin" );
    setFocusProxy(d->spin);
    connect( d->spin, SIGNAL(valueChanged(double)),
	     this, SIGNAL(valueChanged(double)) );
    connect( this, SIGNAL(valueChanged(double)),
	     this, SLOT(slotEmitRelativeValueChanged(double)) );

    updateLegacyMembers();

    layout(true);
}

void KDoubleNumInput::updateLegacyMembers() {
    m_specialvalue = specialValueText();
}


double KDoubleNumInput::mapSliderToSpin( int val ) const
{
    // map [slidemin,slidemax] to [spinmin,spinmax]
    double spinmin = d->spin->minValue();
    double spinmax = d->spin->maxValue();
    double slidemin = m_slider->minValue(); // cast int to double to avoid
    double slidemax = m_slider->maxValue(); // overflow in rel denominator
    double rel = ( double(val) - slidemin ) / ( slidemax - slidemin );
    return spinmin + rel * ( spinmax - spinmin );
}

void KDoubleNumInput::sliderMoved(int val)
{
    d->spin->setValue( mapSliderToSpin( val ) );
}

void KDoubleNumInput::slotEmitRelativeValueChanged( double value )
{
    if ( !d->referencePoint ) return;
    emit relativeValueChanged( value / d->referencePoint );
}

QSize KDoubleNumInput::minimumSizeHint() const
{
    constPolish();

    int w;
    int h;

    h = 2 + qMax(m_sizeEdit.height(), m_sizeSlider.height());

    // if in extra row, then count it here
    if(m_label && (m_alignment & (Qt::AlignBottom|Qt::AlignTop)))
        h += 4 + m_sizeLabel.height();
    else
        // label is in the same row as the other widgets
	h = qMax(h, m_sizeLabel.height() + 2);

    w = m_slider ? m_slider->sizeHint().width() + 8 : 0;
    w += m_colw1 + m_colw2;

    if(m_alignment & (Qt::AlignTop|Qt::AlignBottom))
        w = qMax(w, m_sizeLabel.width() + 4);

    return QSize(w, h);
}

void KDoubleNumInput::resizeEvent(QResizeEvent* e)
{
    int w = m_colw1;
    int h = 0;

    if(m_label && (m_alignment & Qt::AlignTop)) {
        m_label->setGeometry(0, 0, e->size().width(), m_sizeLabel.height());
        h += m_sizeLabel.height() + 4;
    }

    if(m_label && (m_alignment & Qt::AlignVCenter))
        m_label->setGeometry(0, 0, w, m_sizeEdit.height());

    if (qApp->reverseLayout())
    {
        d->spin->setGeometry(w, h, m_slider ? m_colw2
                                            : e->size().width() - w, m_sizeEdit.height());
        w += m_colw2 + KDialog::spacingHint();

        if(m_slider)
            m_slider->setGeometry(w, h, e->size().width() - w, m_sizeEdit.height());
    }
    else if(m_slider) {
        m_slider->setGeometry(w, h, e->size().width() -
                                    (m_colw1 + m_colw2 + KDialog::spacingHint()),
                              m_sizeEdit.height());
        d->spin->setGeometry(w + m_slider->width() + KDialog::spacingHint(), h,
                             m_colw2, m_sizeEdit.height());
    }
    else {
        d->spin->setGeometry(w, h, e->size().width() - w, m_sizeEdit.height());
    }

    h += m_sizeEdit.height() + 2;

    if(m_label && (m_alignment & Qt::AlignBottom))
        m_label->setGeometry(0, h, m_sizeLabel.width(), m_sizeLabel.height());
}

void KDoubleNumInput::doLayout()
{
    m_sizeEdit = d->spin->sizeHint();
    m_colw2 = m_sizeEdit.width();
}

void KDoubleNumInput::setValue(double val)
{
    d->spin->setValue( val );
}

void KDoubleNumInput::setRelativeValue( double r )
{
    if ( !d->referencePoint ) return;
    ++d->blockRelative;
    setValue( r * d->referencePoint );
    --d->blockRelative;
}

void KDoubleNumInput::setReferencePoint( double ref )
{
    // clip to valid range:
    ref = qMin( maxValue(), qMax( minValue(), ref ) );
    d->referencePoint = ref;
}

void KDoubleNumInput::setRange(double lower, double upper, double step,
                                                           bool slider)
{
    if( m_slider ) {
	// don't update the slider to avoid an endless recursion
	QSpinBox * spin = d->spin;
	disconnect(spin, SIGNAL(valueChanged(int)),
		m_slider, SLOT(setValue(int)) );
    }
    d->spin->setRange( lower, upper, step, d->spin->precision() );

    if(slider) {
	// upcast to base type to get the min/maxValue in int form:
	QSpinBox * spin = d->spin;
        int slmax = spin->maxValue();
	int slmin = spin->minValue();
        int slvalue = spin->value();
	int slstep = spin->singleStep();
        if (m_slider) {
            m_slider->setRange(slmin, slmax);
	    m_slider->setLineStep(slstep);
            m_slider->setValue(slvalue);
        } else {
            m_slider = new QSlider(slmin, slmax, slstep, slvalue,
                                   Qt::Horizontal, this);
            m_slider->setTickmarks(QSlider::TicksBelow);
	    // feedback line: when one moves, the other moves, too:
            connect(m_slider, SIGNAL(valueChanged(int)),
                    SLOT(sliderMoved(int)) );
        }
	connect(spin, SIGNAL(valueChanged(int)),
			m_slider, SLOT(setValue(int)) );
	// calculate ( slmax - slmin ) / 10 without overflowing ints:
	int major = calcDiffByTen( slmax, slmin );
	if ( !major ) major = slstep; // ### needed?
        m_slider->setTickInterval(major);
    } else {
        delete m_slider;
        m_slider = 0;
    }

    setReferencePoint( referencePoint() );

    layout(true);
    updateLegacyMembers();
}

void KDoubleNumInput::setMinValue(double min)
{
    setRange(min, maxValue(), d->spin->singleStep(), m_slider);
}

double KDoubleNumInput::minValue() const
{
    return d->spin->minValue();
}

void KDoubleNumInput::setMaxValue(double max)
{
    setRange(minValue(), max, d->spin->singleStep(), m_slider);
}

double KDoubleNumInput::maxValue() const
{
    return d->spin->maxValue();
}

double  KDoubleNumInput::value() const
{
    return d->spin->value();
}

double KDoubleNumInput::relativeValue() const
{
    if ( !d->referencePoint ) return 0;
    return value() / d->referencePoint;
}

double KDoubleNumInput::referencePoint() const
{
    return d->referencePoint;
}

QString KDoubleNumInput::suffix() const
{
    return d->spin->suffix();
}

QString KDoubleNumInput::prefix() const
{
    return d->spin->prefix();
}

void KDoubleNumInput::setSuffix(const QString &suffix)
{
    d->spin->setSuffix( suffix );

    layout(true);
}

void KDoubleNumInput::setPrefix(const QString &prefix)
{
    d->spin->setPrefix( prefix );

    layout(true);
}

void KDoubleNumInput::setPrecision(int precision)
{
    d->spin->setPrecision( precision );

    layout(true);
}

int KDoubleNumInput::precision() const
{
    return d->spin->precision();
}

void KDoubleNumInput::setSpecialValueText(const QString& text)
{
    d->spin->setSpecialValueText( text );

    layout(true);
    updateLegacyMembers();
}

void KDoubleNumInput::setLabel(const QString & label, int a)
{
    KNumInput::setLabel(label, a);

    if(m_label)
        m_label->setBuddy(d->spin);

}

// ----------------------------------------------------------------------------


class KDoubleSpinBoxValidator : public KDoubleValidator
{
public:
    KDoubleSpinBoxValidator( KDoubleSpinBox *sb,double bottom, double top, int decimals)
        : KDoubleValidator( bottom, top, decimals, sb), spinBox( sb ) { }

    virtual State validate( QString& str, int& pos ) const;

private:
    KDoubleSpinBox *spinBox;
};

QValidator::State KDoubleSpinBoxValidator::validate( QString& str, int& pos ) const
{
    QString pref = spinBox->prefix();
    QString suff = spinBox->suffix();
    QString suffStriped = suff.trimmed();
    int overhead = pref.length() + suff.length();
    State state = Invalid;

    if ( overhead == 0 ) {
        state = KDoubleValidator::validate( str, pos );
    } else {
        bool stripedVersion = false;
        if ( str.length() >= overhead && str.startsWith(pref)
             && (str.endsWith(suff)
                 || (stripedVersion = str.endsWith(suffStriped))) ) {
            if ( stripedVersion )
                overhead = pref.length() + suffStriped.length();
            QString core = str.mid( pref.length(), str.length() - overhead );
            int corePos = pos - pref.length();
            state = KDoubleValidator::validate( core, corePos );
            pos = corePos + pref.length();
            str.replace( pref.length(), str.length() - overhead, core );
        } else {
            state = KDoubleValidator::validate( str, pos );
            if ( state == Invalid ) {
                // trimmed(), cf. QSpinBox::interpretText()
                QString special = spinBox->specialValueText().trimmed();
                QString candidate = str.trimmed();

                if ( special.startsWith(candidate) ) {
                    if ( candidate.length() == special.length() ) {
                        state = Acceptable;
                    } else {
                        state = Intermediate;
                    }
                }
            }
        }
    }
    return state;
}

// We use a kind of fixed-point arithmetic to represent the range of
// doubles [mLower,mUpper] in steps of 10^(-mPrecision). Thus, the
// following relations hold:
//
// 1. factor = 10^mPrecision
// 2. basicStep = 1/factor = 10^(-mPrecision);
// 3. lowerInt = lower * factor;
// 4. upperInt = upper * factor;
// 5. lower = lowerInt * basicStep;
// 6. upper = upperInt * basicStep;
class KDoubleSpinBox::Private {
public:
  Private( int precision=1 )
    : mPrecision( precision ),
      mValidator( 0 )
  {
  }

  int factor() const {
    int f = 1;
    for ( int i = 0 ; i < mPrecision ; ++i ) f *= 10;
    return f;
  }

  double basicStep() const {
    return 1.0/double(factor());
  }

  int mapToInt( double value, bool * ok ) const {
    assert( ok );
    const double f = factor();
    if ( value > double(INT_MAX) / f ) {
      kdWarning() << "KDoubleSpinBox: can't represent value " << value
		  << "in terms of fixed-point numbers with precision "
		  << mPrecision << endl;
      *ok = false;
      return INT_MAX;
    } else if ( value < double(INT_MIN) / f ) {
      kdWarning() << "KDoubleSpinBox: can't represent value " << value
		  << "in terms of fixed-point numbers with precision "
		  << mPrecision << endl;
      *ok = false;
      return INT_MIN;
    } else {
      *ok = true;
      return int( value * f + ( value < 0 ? -0.5 : 0.5 ) );
    }
  }

  double mapToDouble( int value ) const {
    return double(value) * basicStep();
  }

  int mPrecision;
  KDoubleSpinBoxValidator * mValidator;
};

KDoubleSpinBox::KDoubleSpinBox( QWidget * parent)
  : QSpinBox( parent), d(new Private())
{
  lineEdit()->setAlignment( Qt::AlignRight );
  updateValidator();
  connect( this, SIGNAL(valueChanged(int)), SLOT(slotValueChanged(int)) );
}

KDoubleSpinBox::KDoubleSpinBox( double lower, double upper, double step,
				double value, QWidget *parent, int precision)
  : QSpinBox( parent), d(new Private())
{
  lineEdit()->setAlignment( Qt::AlignRight );
  setRange( lower, upper, step, precision );
  setValue( value );
  connect( this, SIGNAL(valueChanged(int)), SLOT(slotValueChanged(int)) );
}

KDoubleSpinBox::~KDoubleSpinBox() {
  delete d; d = 0;
}

bool KDoubleSpinBox::acceptLocalizedNumbers() const {
  if ( !d->mValidator ) return true; // we'll set one that does;
                                     // can't do it now, since we're const
  return d->mValidator->acceptLocalizedNumbers();
}

void KDoubleSpinBox::setAcceptLocalizedNumbers( bool accept ) {
  if ( !d->mValidator ) updateValidator();
  d->mValidator->setAcceptLocalizedNumbers( accept );
}

void KDoubleSpinBox::setRange( double lower, double upper, double step,
			       int precision ) {
  lower = qMin(upper, lower);
  upper = qMax(upper, lower);
  setPrecision( precision, true ); // disable bounds checking, since
  setMinValue( lower );            // it's done in set{Min,Max}Value
  setMaxValue( upper );            // anyway and we want lower, upper
  setSingleStep( step );             // and step to have the right precision
}

int KDoubleSpinBox::precision() const {
  return d->mPrecision;
}

void KDoubleSpinBox::setPrecision( int precision ) {
    setPrecision( precision, false );
}

void KDoubleSpinBox::setPrecision( int precision, bool force ) {
  if ( precision < 1 ) return;
  if ( !force ) {
    int maxPrec = maxPrecision();
    if ( precision > maxPrec )
      precision = maxPrec;
  }
  d->mPrecision = precision;
  updateValidator();
}

int KDoubleSpinBox::maxPrecision() const {
    // INT_MAX must be > maxAbsValue * 10^precision
    // ==> 10^precision < INT_MAX / maxAbsValue
    // ==> precision < log10 ( INT_MAX / maxAbsValue )
    // ==> maxPrecision = floor( log10 ( INT_MAX / maxAbsValue ) );
    double maxAbsValue = qMax( fabs(minValue()), fabs(maxValue()) );
    if ( maxAbsValue == 0 ) return 6; // return arbitrary value to avoid dbz...

    return int( floor( log10( double(INT_MAX) / maxAbsValue ) ) );
}

double KDoubleSpinBox::value() const {
  return d->mapToDouble( base::value() );
}

void KDoubleSpinBox::setValue( double value ) {
    if ( value == this->value() ) return;
    if ( value < minValue() )
	base::setValue( base::minValue() );
    else if ( value > maxValue() )
	base::setValue( base::maxValue() );
    else {
	bool ok = false;
	base::setValue( d->mapToInt( value, &ok ) );
	assert( ok );
    }
}

double KDoubleSpinBox::minValue() const {
  return d->mapToDouble( base::minValue() );
}

void KDoubleSpinBox::setMinValue( double value ) {
  bool ok = false;
  int min = d->mapToInt( value, &ok );
  if ( !ok ) return;
  base::setMinValue( min );
  updateValidator();
}


double KDoubleSpinBox::maxValue() const {
  return d->mapToDouble( base::maxValue() );
}

void KDoubleSpinBox::setMaxValue( double value ) {
  bool ok = false;
  int max = d->mapToInt( value, &ok );
  if ( !ok ) return;
  base::setMaxValue( max );
  updateValidator();
}

double KDoubleSpinBox::singleStep() const {
  return d->mapToDouble( base::singleStep() );
}

void KDoubleSpinBox::setSingleStep( double step ) {
  bool ok = false;
  if ( step > maxValue() - minValue() )
    base::setSingleStep( 1 );
  else
    base::setSingleStep( qMax( d->mapToInt( step, &ok ), 1 ) );
}

QString KDoubleSpinBox::textFromValue( int value ) const{
  if ( acceptLocalizedNumbers() )
    return KGlobal::locale()
      ->formatNumber( d->mapToDouble( value ), d->mPrecision );
  else
    return QString().setNum( d->mapToDouble( value ), 'f', d->mPrecision );
}

int KDoubleSpinBox::valueFromText( const QString &text ) const{
  double value;
  bool ok;
  if ( acceptLocalizedNumbers() )
    value = KGlobal::locale()->readNumber( text, &ok );
  else
    value = cleanText().toDouble( &ok );
  if ( !ok ) return 0;
  if ( value > maxValue() )
    value = maxValue();
  else if ( value < minValue() )
    value = minValue();
  return d->mapToInt( value, &ok );
}

void KDoubleSpinBox::setValidator( const QValidator * ) {
  // silently discard the new validator. We don't want another one ;-)
}

void KDoubleSpinBox::slotValueChanged( int value ) {
  emit valueChanged( d->mapToDouble( value ) );
}

void KDoubleSpinBox::updateValidator() {
  if ( !d->mValidator ) {
    d->mValidator =  new KDoubleSpinBoxValidator(this, minValue(), maxValue(), precision());
    d->mValidator->setObjectName("d->mValidator" );
#ifdef __GNUC__
#warning KDE4 we NEED to fix the validation of values here
#endif
//    base::setValidator( d->mValidator );
  } else
    d->mValidator->setRange( minValue(), maxValue(), precision() );
}

void KNumInput::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KIntNumInput::virtual_hook( int id, void* data )
{ KNumInput::virtual_hook( id, data ); }

void KDoubleNumInput::virtual_hook( int id, void* data )
{ KNumInput::virtual_hook( id, data ); }

void KIntSpinBox::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KDoubleSpinBox::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "knuminput.moc"
