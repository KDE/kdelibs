/* This file is part of the KDE libraries
 * Initial implementation:
 *     Copyright (c) 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
 * Rewritten and maintained by:
 *     Copyright (c) 2000 Dirk Mueller <mueller@kde.org>
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

#include "knuminput.h"

#include <config.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <assert.h>
#include <math.h>

#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QResizeEvent>
#include <QtGui/QSlider>

#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <knumvalidator.h>

static inline int calcDiffByTen( int x, int y ) {
    // calculate ( x - y ) / 10 without overflowing ints:
    return ( x / 10 ) - ( y / 10 )  +  ( x % 10 - y % 10 ) / 10;
}

// ----------------------------------------------------------------------------

class KNumInputPrivate
{
public:
    KNumInputPrivate(KNumInput *q, KNumInput *below = 0):
        q(q),
        m_prev(0),
        m_next(0),
        m_colw1(0),
        m_colw2(0),
        m_label(0),
        m_slider(0),
        m_alignment(0)
    {
        if(below) {
            m_next = below->d->m_next;
            m_prev = below;
            below->d->m_next = q;
            if(m_next)
                m_next->d->m_prev = q;
        }
    }

    static KNumInputPrivate *get(const KNumInput *i)
    {
        return i->d;
    }

    KNumInput *q;
    KNumInput* m_prev, *m_next;
    int m_colw1, m_colw2;

    QLabel*  m_label;
    QSlider* m_slider;
    QSize    m_sizeSlider, m_sizeLabel;

    Qt::Alignment m_alignment;
};


#define K_USING_KNUMINPUT_P(_d) KNumInputPrivate *_d = KNumInputPrivate::get(this)

KNumInput::KNumInput(QWidget* parent)
    : QWidget(parent), d(new KNumInputPrivate(this))
{
  setSizePolicy(QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ));
}

KNumInput::KNumInput(QWidget* parent, KNumInput* below)
    : QWidget(parent), d(new KNumInputPrivate(this, below))
{
  setSizePolicy(QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ));
}

KNumInput::~KNumInput()
{
    if(d->m_prev)
        d->m_prev->d->m_next = d->m_next;

    if(d->m_next)
        d->m_next->d->m_prev = d->m_prev;

    delete d;
}

QSlider *KNumInput::slider() const
{
  return d->m_slider;
}

bool KNumInput::showSlider() const
{
    return d->m_slider;
}

void KNumInput::setLabel(const QString & label, Qt::Alignment a)
{
    if(label.isEmpty()) {
        delete d->m_label;
        d->m_label = 0;
        d->m_alignment = 0;
    }
    else {
        if (d->m_label) d->m_label->setText(label);
        else d->m_label = new QLabel(label, this);
        d->m_label->setObjectName("KNumInput::QLabel");
        d->m_label->setAlignment(a);
        // if no vertical alignment set, use Top alignment
        if(!(a & (Qt::AlignTop|Qt::AlignBottom|Qt::AlignVCenter)))
           a |= Qt::AlignTop;
        d->m_alignment = a;
    }

    layout(true);
}

QString KNumInput::label() const
{
    if (d->m_label) return d->m_label->text();
    return QString();
}

void KNumInput::layout(bool deep)
{
    int w1 = d->m_colw1;
    int w2 = d->m_colw2;

    // label sizeHint
    d->m_sizeLabel = (d->m_label ? d->m_label->sizeHint() : QSize(0,0));

    if(d->m_label && (d->m_alignment & Qt::AlignVCenter))
        d->m_colw1 = d->m_sizeLabel.width() + 4;
    else
        d->m_colw1 = 0;

    // slider sizeHint
    d->m_sizeSlider = (d->m_slider ? d->m_slider->sizeHint() : QSize(0, 0));

    doLayout();

    if(!deep) {
        d->m_colw1 = w1;
        d->m_colw2 = w2;
        return;
    }

    KNumInput* p = this;
    while(p) {
        p->doLayout();
        w1 = qMax(w1, p->d->m_colw1);
        w2 = qMax(w2, p->d->m_colw2);
        p = p->d->m_prev;
    }

    p = d->m_next;
    while(p) {
        p->doLayout();
        w1 = qMax(w1, p->d->m_colw1);
        w2 = qMax(w2, p->d->m_colw2);
        p = p->d->m_next;
    }

    p = this;
    while(p) {
        p->d->m_colw1 = w1;
        p->d->m_colw2 = w2;
        p = p->d->m_prev;
    }

    p = d->m_next;
    while(p) {
        p->d->m_colw1 = w1;
        p->d->m_colw2 = w2;
        p = p->d->m_next;
    }

//    kDebug() << "w1 " << w1 << " w2 " << w2;
}

QSize KNumInput::sizeHint() const
{
    return minimumSizeHint();
}

void KNumInput::setSteps(int minor, int major)
{
    if(d->m_slider)
    {
        d->m_slider->setSingleStep( minor );
        d->m_slider->setPageStep( major );
    }
}


// ----------------------------------------------------------------------------

class KIntSpinBox::KIntSpinBoxPrivate
{
public:
    KIntSpinBoxPrivate(KIntSpinBox *q, int val_base = 10): q(q), val_base(val_base) {}

    KIntSpinBox *q;
    int val_base;
};

KIntSpinBox::KIntSpinBox(QWidget *parent)
    : QSpinBox(parent), d(new KIntSpinBoxPrivate(this))
{
    setRange(0,99);
    setSingleStep(1);
    lineEdit()->setAlignment(Qt::AlignRight);
    setValue(0);
}

KIntSpinBox::~KIntSpinBox()
{
    delete d;
}

KIntSpinBox::KIntSpinBox(int lower, int upper, int step, int value, QWidget *parent,int base)
    : QSpinBox(parent), d(new KIntSpinBoxPrivate(this, base))
{
    setRange(lower,upper);
    setSingleStep(step);
    lineEdit()->setAlignment(Qt::AlignRight);
    setValue(value);
}

void KIntSpinBox::setBase(int base)
{
    d->val_base = base;
}


int KIntSpinBox::base() const
{
    return d->val_base;
}

QString KIntSpinBox::textFromValue(int v) const
{
    return QString::number(v, d->val_base);
}

int KIntSpinBox::valueFromText(const QString &text) const
{
    bool ok;
    QString theText = text;
    if (theText.endsWith(suffix())) theText.chop(suffix().length());
    return theText.toInt(&ok, d->val_base);
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
    KIntNumInput *q;
    int referencePoint;
    short blockRelative;
    KIntSpinBox* m_spin;
    QSize        m_sizeSpin;

    KIntNumInputPrivate( KIntNumInput *q, int r )
        : q(q),
          referencePoint( r ),
          blockRelative( 0 ) {}
};


KIntNumInput::KIntNumInput(KNumInput* below, int val,QWidget *parent,int _base)
    : KNumInput(parent,below)
    , d( new KIntNumInputPrivate( this, val ) )
{
    init(val, _base);
}

KIntNumInput::KIntNumInput(QWidget *parent)
    : KNumInput(parent)
    , d( new KIntNumInputPrivate( this, 0 ) )
{
    init(0, 10);
}

KIntNumInput::KIntNumInput(int val, QWidget *parent,int _base)
    : KNumInput(parent)
    , d( new KIntNumInputPrivate( this, val ) )
{
    init(val, _base);
}

QSpinBox *KIntNumInput::spinBox() const
{
    return d->m_spin;
}

void KIntNumInput::init(int val, int _base)
{
    d->m_spin = new KIntSpinBox(INT_MIN, INT_MAX, 1, val, this, _base);
    d->m_spin->setObjectName("KIntNumInput::KIntSpinBox");
    // the KIntValidator is broken beyond believe for
    // spinboxes which have suffix or prefix texts, so
    // better don't use it unless absolutely necessary
#ifdef __GNUC__
#warning KDE4 we NEED to fix the validation of values here
#endif
//        if (_base != 10)
//        m_spin->setValidator(new KIntValidator(this, _base, "KNumInput::KIntValidtr"));

    connect(d->m_spin, SIGNAL(valueChanged(int)), SLOT(spinValueChanged(int)));
    connect(this, SIGNAL(valueChanged(int)),
            SLOT(slotEmitRelativeValueChanged(int)));

    setFocusProxy(d->m_spin);
    layout(true);
}

void KIntNumInput::setReferencePoint( int ref ) {
    // clip to valid range:
    ref = qMin( maximum(), qMax( minimum(),  ref ) );
    d->referencePoint = ref;
}

int KIntNumInput::referencePoint() const {
    return d->referencePoint;
}

void KIntNumInput::spinValueChanged(int val)
{
    K_USING_KNUMINPUT_P(priv);

    if(priv->m_slider)
        priv->m_slider->setValue(val);

    emit valueChanged(val);
}

void KIntNumInput::slotEmitRelativeValueChanged( int value ) {
    if ( d->blockRelative || !d->referencePoint ) return;
    emit relativeValueChanged( double( value ) / double( d->referencePoint ) );
}

void KIntNumInput::setSliderEnabled(bool slider)
{
    K_USING_KNUMINPUT_P(priv);
    if(slider) {
        if (!priv->m_slider) {
            priv->m_slider = new QSlider(Qt::Horizontal, this);
            connect(priv->m_slider, SIGNAL(valueChanged(int)),
                    d->m_spin, SLOT(setValue(int)));
            priv->m_slider->setTickPosition(QSlider::TicksBelow);
        }

        int value = d->m_spin->value();
        priv->m_slider->setRange(d->m_spin->minimum(), d->m_spin->maximum());
        priv->m_slider->setPageStep(d->m_spin->singleStep());
        priv->m_slider->setValue(value);

        // calculate (upper-lower)/10 without overflowing int's:
        int major = calcDiffByTen( d->m_spin->maximum(), d->m_spin->minimum() );

        priv->m_slider->setSingleStep(d->m_spin->singleStep());
        priv->m_slider->setPageStep(major);
        priv->m_slider->setTickInterval(major);
    }
    else {
        delete priv->m_slider;
        priv->m_slider = 0;
    }
}

void KIntNumInput::setRange(int lower, int upper, int step)
{
    upper = qMax(upper, lower);
    lower = qMin(upper, lower);
    d->m_spin->setMinimum(lower);
    d->m_spin->setMaximum(upper);
    d->m_spin->setSingleStep(step);

    step = d->m_spin->singleStep(); // maybe QRangeControl didn't like out lineStep?

    // check that reference point is still inside valid range:
    setReferencePoint( referencePoint() );

    layout(true);
}

void KIntNumInput::setRange(int lower, int upper, int step, bool slider)
{
    setRange(lower, upper, step);
    setSliderEnabled(slider);
}

void KIntNumInput::setMinimum(int min)
{
    setRange(min, d->m_spin->maximum(), d->m_spin->singleStep());
}

int KIntNumInput::minimum() const
{
    return d->m_spin->minimum();
}

void KIntNumInput::setMaximum(int max)
{
    setRange(d->m_spin->minimum(), max, d->m_spin->singleStep());
}

int KIntNumInput::maximum() const
{
    return d->m_spin->maximum();
}

void KIntNumInput::setSuffix(const QString &suffix)
{
    d->m_spin->setSuffix(suffix);

    layout(true);
}

QString KIntNumInput::suffix() const
{
    return d->m_spin->suffix();
}

void KIntNumInput::setPrefix(const QString &prefix)
{
    d->m_spin->setPrefix(prefix);

    layout(true);
}

QString KIntNumInput::prefix() const
{
    return d->m_spin->prefix();
}

void KIntNumInput::setEditFocus(bool mark)
{
    d->m_spin->setEditFocus(mark);
}

QSize KIntNumInput::minimumSizeHint() const
{
    K_USING_KNUMINPUT_P(priv);
    ensurePolished();

    int w;
    int h;

    h = 2 + qMax(d->m_sizeSpin.height(), priv->m_sizeSlider.height());

    // if in extra row, then count it here
    if(priv->m_label && (priv->m_alignment & (Qt::AlignBottom|Qt::AlignTop)))
        h += 4 + priv->m_sizeLabel.height();
    else
        // label is in the same row as the other widgets
        h = qMax(h, priv->m_sizeLabel.height() + 2);

    w = priv->m_slider ? priv->m_slider->sizeHint().width() + 8 : 0;
    w += priv->m_colw1 + priv->m_colw2;

    if(priv->m_alignment & (Qt::AlignTop|Qt::AlignBottom))
        w = qMax(w, priv->m_sizeLabel.width() + 4);

    return QSize(w, h);
}

void KIntNumInput::doLayout()
{
    K_USING_KNUMINPUT_P(priv);

    d->m_sizeSpin = d->m_spin->sizeHint();
    priv->m_colw2 = d->m_sizeSpin.width();

    if (priv->m_label)
        priv->m_label->setBuddy(d->m_spin);
}

void KIntNumInput::resizeEvent(QResizeEvent* e)
{
    K_USING_KNUMINPUT_P(priv);

    int w = priv->m_colw1;
    int h = 0;

    if(priv->m_label && (priv->m_alignment & Qt::AlignTop)) {
        priv->m_label->setGeometry(0, 0, e->size().width(), priv->m_sizeLabel.height());
        h += priv->m_sizeLabel.height() + KDialog::spacingHint();
    }

    if(priv->m_label && (priv->m_alignment & Qt::AlignVCenter))
        priv->m_label->setGeometry(0, 0, w, d->m_sizeSpin.height());

    if (qApp->layoutDirection())
    {
        d->m_spin->setGeometry(w, h, priv->m_slider ? priv->m_colw2 : qMax(priv->m_colw2, e->size().width() - w), d->m_sizeSpin.height());
        w += priv->m_colw2 + 8;

        if(priv->m_slider)
            priv->m_slider->setGeometry(w, h, e->size().width() - w, d->m_sizeSpin.height() + KDialog::spacingHint());
    }
    else if(priv->m_slider) {
        priv->m_slider->setGeometry(w, h, e->size().width() - (w + priv->m_colw2 + KDialog::spacingHint()), d->m_sizeSpin.height() + KDialog::spacingHint());
        d->m_spin->setGeometry(w + priv->m_slider->size().width() + KDialog::spacingHint(), h, priv->m_colw2, d->m_sizeSpin.height());
    }
    else {
        d->m_spin->setGeometry(w, h, qMax(priv->m_colw2, e->size().width() - w), d->m_sizeSpin.height());
    }

    h += d->m_sizeSpin.height() + 2;

    if(priv->m_label && (priv->m_alignment & Qt::AlignBottom))
        priv->m_label->setGeometry(0, h, priv->m_sizeLabel.width(), priv->m_sizeLabel.height());
}

KIntNumInput::~KIntNumInput()
{
    delete d;
}

void KIntNumInput::setValue(int val)
{
    d->m_spin->setValue(val);
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

int KIntNumInput::value() const
{
    return d->m_spin->value();
}

void KIntNumInput::setSpecialValueText(const QString& text)
{
    d->m_spin->setSpecialValueText(text);
    layout(true);
}

QString KIntNumInput::specialValueText() const
{
    return d->m_spin->specialValueText();
}

void KIntNumInput::setLabel(const QString & label, Qt::Alignment a)
{
    K_USING_KNUMINPUT_P(priv);

    KNumInput::setLabel(label, a);

    if(priv->m_label)
        priv->m_label->setBuddy(d->m_spin);
}

// ----------------------------------------------------------------------------

class KDoubleNumInput::KDoubleNumInputPrivate {
public:
    KDoubleNumInputPrivate( double r )
        : spin( 0 ),
          referencePoint( r ),
          blockRelative ( 0 ) {}
    QDoubleSpinBox * spin;
    double referencePoint;
    short blockRelative;
    QSize m_sizeEdit;
    QString m_specialvalue;
};

KDoubleNumInput::KDoubleNumInput(QWidget *parent)
    : KNumInput(parent)
    , d( new KDoubleNumInputPrivate( 0.0 ) )

{
    init(0.0, 0.0, 9999.0, 0.01, 2);
}

KDoubleNumInput::KDoubleNumInput(double lower, double upper, double value, QWidget *parent,
                                 double step, int precision)
    : KNumInput(parent)
    , d( new KDoubleNumInputPrivate( value ) )
{
    init(value, lower, upper, step, precision);
}

KDoubleNumInput::KDoubleNumInput(KNumInput *below,
                                 double lower, double upper, double value, QWidget *parent,
                                 double step, int precision)
    : KNumInput(parent,below)
    , d( new KDoubleNumInputPrivate( value ) )
{
    init(value, lower, upper, step, precision);
}

KDoubleNumInput::~KDoubleNumInput()
{
    delete d;
}

QString KDoubleNumInput::specialValueText() const
{
    return d->m_specialvalue;
}


void KDoubleNumInput::init(double value, double lower, double upper,
                           double step, int precision )
{
    d->spin = new QDoubleSpinBox(this);
    d->spin->setRange(lower, upper);
    d->spin->setSingleStep(step);
    d->spin->setValue(value);
    d->spin->setDecimals(precision);
    d->spin->setAlignment(Qt::AlignRight);

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
    d->m_specialvalue = specialValueText();
}


double KDoubleNumInput::mapSliderToSpin( int val ) const
{
    K_USING_KNUMINPUT_P(priv);

    // map [slidemin,slidemax] to [spinmin,spinmax]
    double spinmin = d->spin->minimum();
    double spinmax = d->spin->maximum();
    double slidemin = priv->m_slider->minimum(); // cast int to double to avoid
    double slidemax = priv->m_slider->maximum(); // overflow in rel denominator
    double rel = ( double(val) - slidemin ) / ( slidemax - slidemin );
    return spinmin + rel * ( spinmax - spinmin );
}

void KDoubleNumInput::sliderMoved(int val)
{
    d->spin->setValue( mapSliderToSpin( val ) );
}

void KDoubleNumInput::spinBoxChanged(double val)
{
    K_USING_KNUMINPUT_P(priv);

    double spinmin = d->spin->minimum();
    double spinmax = d->spin->maximum();
    double slidemin = priv->m_slider->minimum(); // cast int to double to avoid
    double slidemax = priv->m_slider->maximum(); // overflow in rel denominator

    double rel = ( val - spinmin) / (spinmax - spinmin);

    if (priv->m_slider) {
        priv->m_slider->blockSignals(true);
        priv->m_slider->setValue(int(slidemin + rel * (slidemax - slidemin)));
        priv->m_slider->blockSignals(false);
    }
}

void KDoubleNumInput::slotEmitRelativeValueChanged( double value )
{
    if ( !d->referencePoint ) return;
    emit relativeValueChanged( value / d->referencePoint );
}

QSize KDoubleNumInput::minimumSizeHint() const
{
    K_USING_KNUMINPUT_P(priv);

    ensurePolished();

    int w;
    int h;

    h = 2 + qMax(d->m_sizeEdit.height(), priv->m_sizeSlider.height());

    // if in extra row, then count it here
    if(priv->m_label && (priv->m_alignment & (Qt::AlignBottom|Qt::AlignTop)))
        h += 4 + priv->m_sizeLabel.height();
    else
        // label is in the same row as the other widgets
        h = qMax(h, priv->m_sizeLabel.height() + 2);

    w = priv->m_slider ? priv->m_slider->sizeHint().width() + 8 : 0;
    w += priv->m_colw1 + priv->m_colw2;

    if(priv->m_alignment & (Qt::AlignTop|Qt::AlignBottom))
        w = qMax(w, priv->m_sizeLabel.width() + 4);

    return QSize(w, h);
}

void KDoubleNumInput::resizeEvent(QResizeEvent* e)
{
    K_USING_KNUMINPUT_P(priv);

    int w = priv->m_colw1;
    int h = 0;

    if(priv->m_label && (priv->m_alignment & Qt::AlignTop)) {
        priv->m_label->setGeometry(0, 0, e->size().width(), priv->m_sizeLabel.height());
        h += priv->m_sizeLabel.height() + 4;
    }

    if(priv->m_label && (priv->m_alignment & Qt::AlignVCenter))
        priv->m_label->setGeometry(0, 0, w, d->m_sizeEdit.height());

    if (qApp->layoutDirection())
    {
        d->spin->setGeometry(w, h, priv->m_slider ? priv->m_colw2
                                            : e->size().width() - w, d->m_sizeEdit.height());
        w += priv->m_colw2 + KDialog::spacingHint();

        if(priv->m_slider)
            priv->m_slider->setGeometry(w, h, e->size().width() - w, d->m_sizeEdit.height() + KDialog::spacingHint());
    }
    else if(priv->m_slider) {
        priv->m_slider->setGeometry(w, h, e->size().width() -
                                    (priv->m_colw1 + priv->m_colw2 + KDialog::spacingHint()),
                              d->m_sizeEdit.height() + KDialog::spacingHint());
        d->spin->setGeometry(w + priv->m_slider->width() + KDialog::spacingHint(), h,
                             priv->m_colw2, d->m_sizeEdit.height());
    }
    else {
        d->spin->setGeometry(w, h, e->size().width() - w, d->m_sizeEdit.height());
    }

    h += d->m_sizeEdit.height() + 2;

    if(priv->m_label && (priv->m_alignment & Qt::AlignBottom))
        priv->m_label->setGeometry(0, h, priv->m_sizeLabel.width(), priv->m_sizeLabel.height());
}

void KDoubleNumInput::doLayout()
{
    K_USING_KNUMINPUT_P(priv);

    d->m_sizeEdit = d->spin->sizeHint();
    priv->m_colw2 = d->m_sizeEdit.width();
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
    ref = qMin( maximum(), qMax( minimum(), ref ) );
    d->referencePoint = ref;
}

void KDoubleNumInput::setRange(double lower, double upper, double step,
                                                           bool slider)
{
    K_USING_KNUMINPUT_P(priv);

    if( priv->m_slider ) {
        // don't update the slider to avoid an endless recursion
        QDoubleSpinBox * spin = d->spin;
        disconnect(spin, SIGNAL(valueChanged(int)),
                priv->m_slider, SLOT(setValue(int)) );
    }
    d->spin->setRange( lower, upper);
    d->spin->setSingleStep(step);

    if(slider) {
        // upcast to base type to get the minimum/maximum in int form:
        QDoubleSpinBox * spin = d->spin;
        int slmax = spin->maximum();
        int slmin = spin->minimum();
        int slvalue = spin->value();
        int slstep = spin->singleStep();
        if (priv->m_slider) {
            priv->m_slider->setRange(slmin, slmax);
            priv->m_slider->setSingleStep(slstep);
            priv->m_slider->setValue(slvalue);
        } else {
            priv->m_slider = new QSlider(Qt::Horizontal, this);
            priv->m_slider->setMinimum(slmin);
            priv->m_slider->setMaximum(slmax);
            priv->m_slider->setSingleStep(slstep);
            priv->m_slider->setValue(slvalue);
            priv->m_slider->setTickPosition(QSlider::TicksBelow);
            // feedback line: when one moves, the other moves, too:
            connect(priv->m_slider, SIGNAL(valueChanged(int)),
                    SLOT(sliderMoved(int)) );
        }
        connect(spin, SIGNAL(valueChanged(double)), SLOT(spinBoxChanged(double)) );
        // calculate ( slmax - slmin ) / 10 without overflowing ints:
        int major = calcDiffByTen( slmax, slmin );
        if ( !major ) major = slstep; // ### needed?
        priv->m_slider->setTickInterval(major);
    } else {
        delete priv->m_slider;
        priv->m_slider = 0;
    }

    setReferencePoint( referencePoint() );

    layout(true);
    updateLegacyMembers();
}

void KDoubleNumInput::setMinimum(double min)
{
    K_USING_KNUMINPUT_P(priv);
    setRange(min, maximum(), d->spin->singleStep(), priv->m_slider);
}

double KDoubleNumInput::minimum() const
{
    return d->spin->minimum();
}

void KDoubleNumInput::setMaximum(double max)
{
    K_USING_KNUMINPUT_P(priv);
    setRange(minimum(), max, d->spin->singleStep(), priv->m_slider);
}

double KDoubleNumInput::maximum() const
{
    return d->spin->maximum();
}

double KDoubleNumInput::value() const
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

void KDoubleNumInput::setDecimals(int decimals)
{
    d->spin->setDecimals(decimals);

    layout(true);
}

int KDoubleNumInput::decimals() const
{
    return d->spin->decimals();
}

void KDoubleNumInput::setSpecialValueText(const QString& text)
{
    d->spin->setSpecialValueText( text );

    layout(true);
    updateLegacyMembers();
}

void KDoubleNumInput::setLabel(const QString & label, Qt::Alignment a)
{
    K_USING_KNUMINPUT_P(priv);

    KNumInput::setLabel(label, a);

    if(priv->m_label)
        priv->m_label->setBuddy(d->spin);

}

#include "knuminput.moc"
