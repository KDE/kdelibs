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

#include <cmath>

#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QResizeEvent>
#include <QtGui/QSlider>

#include <kdebug.h>
#include <kdialog.h>
#include <klocalizedstring.h>

static inline int calcDiffByTen(int x, int y)
{
    // calculate ( x - y ) / 10 without overflowing ints:
    return (x / 10) - (y / 10)  + (x % 10 - y % 10) / 10;
}

// ----------------------------------------------------------------------------

class KNumInputPrivate
{
public:
    KNumInputPrivate(KNumInput *q, KNumInput *below = 0) :
        q(q),
        previousNumInput(0),
        nextNumInput(0),
        column1Width(0),
        column2Width(0),
        label(0),
        slider(0),
        labelAlignment(0)
    {
        if (below) {
            nextNumInput = below->d->nextNumInput;
            previousNumInput = below;
            below->d->nextNumInput = q;
            if (nextNumInput) {
                nextNumInput->d->previousNumInput = q;
            }
        }
    }

    static KNumInputPrivate *get(const KNumInput *i) {
        return i->d;
    }

    KNumInput *q;
    KNumInput* previousNumInput, *nextNumInput;
    int column1Width, column2Width;

    QLabel*  label;
    QSlider* slider;
    QSize    sliderSize, labelSize;

    Qt::Alignment labelAlignment;
};


#define K_USING_KNUMINPUT_P(_d) KNumInputPrivate *_d = KNumInputPrivate::get(this)

KNumInput::KNumInput(QWidget* parent)
    : QWidget(parent), d(new KNumInputPrivate(this))
{
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    setFocusPolicy(Qt::StrongFocus);
}

KNumInput::KNumInput(QWidget* parent, KNumInput* below)
    : QWidget(parent), d(new KNumInputPrivate(this, below))
{
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    setFocusPolicy(Qt::StrongFocus);
}

KNumInput::~KNumInput()
{
    if (d->previousNumInput) {
        d->previousNumInput->d->nextNumInput = d->nextNumInput;
    }

    if (d->nextNumInput) {
        d->nextNumInput->d->previousNumInput = d->previousNumInput;
    }

    delete d;
}

QSlider *KNumInput::slider() const
{
    return d->slider;
}

bool KNumInput::showSlider() const
{
    return d->slider;
}

void KNumInput::setLabel(const QString & label, Qt::Alignment a)
{
    if (label.isEmpty()) {
        delete d->label;
        d->label = 0;
        d->labelAlignment = 0;
    } else {
        if (!d->label) {
            d->label = new QLabel(this);
        }
        d->label->setText(label);
        d->label->setObjectName("KNumInput::QLabel");
        d->label->setAlignment(a);
        // if no vertical alignment set, use Top alignment
        if (!(a & (Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter))) {
            a |= Qt::AlignTop;
        }
        d->labelAlignment = a;
    }

    layout(true);
}

QString KNumInput::label() const
{
    return d->label ? d->label->text() : QString();
}

void KNumInput::layout(bool deep)
{
    int w1 = d->column1Width;
    int w2 = d->column2Width;

    // label sizeHint
    d->labelSize = (d->label ? d->label->sizeHint() : QSize(0, 0));

    if (d->label && (d->labelAlignment & Qt::AlignVCenter)) {
        d->column1Width = d->labelSize.width() + 4;
    } else {
        d->column1Width = 0;
    }

    // slider sizeHint
    d->sliderSize = (d->slider ? d->slider->sizeHint() : QSize(0, 0));

    doLayout();

    if (!deep) {
        d->column1Width = w1;
        d->column2Width = w2;
        return;
    }

    KNumInput* p = this;
    while (p) {
        p->doLayout();
        w1 = qMax(w1, p->d->column1Width);
        w2 = qMax(w2, p->d->column2Width);
        p = p->d->previousNumInput;
    }

    p = d->nextNumInput;
    while (p) {
        p->doLayout();
        w1 = qMax(w1, p->d->column1Width);
        w2 = qMax(w2, p->d->column2Width);
        p = p->d->nextNumInput;
    }

    p = this;
    while (p) {
        p->d->column1Width = w1;
        p->d->column2Width = w2;
        p = p->d->previousNumInput;
    }

    p = d->nextNumInput;
    while (p) {
        p->d->column1Width = w1;
        p->d->column2Width = w2;
        p = p->d->nextNumInput;
    }

//    kDebug() << "w1 " << w1 << " w2 " << w2;
}

QSize KNumInput::sizeHint() const
{
    return minimumSizeHint();
}

void KNumInput::setSteps(int minor, int major)
{
    if (d->slider) {
        d->slider->setSingleStep(minor);
        d->slider->setPageStep(major);
    }
}


// ----------------------------------------------------------------------------

class KIntSpinBox::KIntSpinBoxPrivate
{
public:
    KIntSpinBoxPrivate(KIntSpinBox *q, int val_base = 10): q(q), val_base(val_base)
    {
        connect(q, SIGNAL(valueChanged(int)), q, SLOT(updateSuffix(int)));
    }

    void updateSuffix(int value)
    {
        if (!pluralSuffix.isEmpty()) {
            KLocalizedString s = pluralSuffix;
            q->setSuffix(s.subs(value).toString());
        }
    }

    KIntSpinBox *q;
    int val_base;
    KLocalizedString pluralSuffix;
};

KIntSpinBox::KIntSpinBox(QWidget *parent)
    : QSpinBox(parent), d(new KIntSpinBoxPrivate(this))
{
    setValue(0);
}

KIntSpinBox::~KIntSpinBox()
{
    delete d;
}

KIntSpinBox::KIntSpinBox(int lower, int upper, int singleStep, int value, QWidget *parent, int base)
    : QSpinBox(parent), d(new KIntSpinBoxPrivate(this, base))
{
    setRange(lower, upper);
    setSingleStep(singleStep);
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
    if (theText.endsWith(suffix())) {
        theText.chop(suffix().length());
    }
    return theText.toInt(&ok, d->val_base);
}

void KIntSpinBox::setEditFocus(bool mark)
{
    lineEdit()->setFocus();
    if (mark) {
        lineEdit()->selectAll();
    }
}

void KIntSpinBox::setSuffix(const KLocalizedString& suffix)
{
    d->pluralSuffix = suffix;
    if (suffix.isEmpty())
        setSuffix(QString());
    else
        d->updateSuffix(value());
}

// ----------------------------------------------------------------------------

class KIntNumInput::KIntNumInputPrivate
{
public:
    KIntNumInput *q;
    int referencePoint;
    short blockRelative;
    KIntSpinBox* intSpinBox;
    QSize        intSpinBoxSize;

    KIntNumInputPrivate(KIntNumInput *q, int r)
            : q(q),
            referencePoint(r),
            blockRelative(0) {}
};


KIntNumInput::KIntNumInput(KNumInput* below, int val, QWidget *parent, int _base)
    : KNumInput(parent, below)
    , d(new KIntNumInputPrivate(this, val))
{
    init(val, _base);
}

KIntNumInput::KIntNumInput(QWidget *parent)
    : KNumInput(parent)
    , d(new KIntNumInputPrivate(this, 0))
{
    init(0, 10);
}

KIntNumInput::KIntNumInput(int val, QWidget *parent, int _base)
    : KNumInput(parent)
    , d(new KIntNumInputPrivate(this, val))
{
    init(val, _base);
}

QSpinBox *KIntNumInput::spinBox() const
{
    return d->intSpinBox;
}

void KIntNumInput::init(int val, int _base)
{
    d->intSpinBox = new KIntSpinBox(INT_MIN, INT_MAX, 1, val, this, _base);
    d->intSpinBox->setObjectName("KIntNumInput::KIntSpinBox");
    // the KIntValidator is broken beyond believe for
    // spinboxes which have suffix or prefix texts, so
    // better don't use it unless absolutely necessary

    if (_base != 10) {
        kWarning() << "WARNING: Validation is broken in KIntNumInput! Needs to be fixed.";
//         d->intSpinBox->setValidator(new KIntValidator(this, _base, "KNumInput::KIntValidator"));
    }

    connect(d->intSpinBox, SIGNAL(valueChanged(int)), SLOT(spinValueChanged(int)));
    connect(this, SIGNAL(valueChanged(int)),
            SLOT(slotEmitRelativeValueChanged(int)));

    setFocusProxy(d->intSpinBox);
    layout(true);
}

void KIntNumInput::setReferencePoint(int ref)
{
    // clip to valid range:
    ref = qMin(maximum(), qMax(minimum(),  ref));
    d->referencePoint = ref;
}

int KIntNumInput::referencePoint() const
{
    return d->referencePoint;
}

void KIntNumInput::spinValueChanged(int val)
{
    K_USING_KNUMINPUT_P(priv);

    if (priv->slider) {
        priv->slider->setValue(val);
    }

    emit valueChanged(val);
}

void KIntNumInput::slotEmitRelativeValueChanged(int value)
{
    if (d->blockRelative || !d->referencePoint) {
        return;
    }
    emit relativeValueChanged(double(value) / double(d->referencePoint));
}

void KIntNumInput::setSliderEnabled(bool slider)
{
    K_USING_KNUMINPUT_P(priv);
    if (slider) {
        if (!priv->slider) {
            priv->slider = new QSlider(Qt::Horizontal, this);
            connect(priv->slider, SIGNAL(valueChanged(int)),
                    d->intSpinBox, SLOT(setValue(int)));
            priv->slider->setTickPosition(QSlider::TicksBelow);
            layout(true);
        }

        const int value = d->intSpinBox->value();
        priv->slider->setRange(d->intSpinBox->minimum(), d->intSpinBox->maximum());
        priv->slider->setPageStep(d->intSpinBox->singleStep());
        priv->slider->setValue(value);

        // calculate (upper-lower)/10 without overflowing int's:
        const int major = calcDiffByTen(d->intSpinBox->maximum(), d->intSpinBox->minimum());

        priv->slider->setSingleStep(d->intSpinBox->singleStep());
        priv->slider->setPageStep(qMax(1, major));
        priv->slider->setTickInterval(major);
    } else {
        if (priv->slider) {
            layout(true);
        }
        delete priv->slider;
        priv->slider = 0;
    }
}

void KIntNumInput::setRange(int lower, int upper, int singleStep)
{
    if (upper < lower || singleStep <= 0) {
        kWarning() << "WARNING: KIntNumInput::setRange() called with bad arguments. Ignoring call...";
        return;
    }

    d->intSpinBox->setMinimum(lower);
    d->intSpinBox->setMaximum(upper);
    d->intSpinBox->setSingleStep(singleStep);

    singleStep = d->intSpinBox->singleStep(); // maybe QRangeControl didn't like our lineStep?

    // check that reference point is still inside valid range:
    setReferencePoint(referencePoint());

    layout(true);

    // update slider information if it's shown
    K_USING_KNUMINPUT_P(priv);
    setSliderEnabled(priv->slider);
}

void KIntNumInput::setRange(int lower, int upper, int singleStep, bool slider)
{
    setRange(lower, upper, singleStep);
    setSliderEnabled(slider);
}

void KIntNumInput::setMinimum(int min)
{
    setRange(min, d->intSpinBox->maximum(), d->intSpinBox->singleStep());
}

int KIntNumInput::minimum() const
{
    return d->intSpinBox->minimum();
}

void KIntNumInput::setMaximum(int max)
{
    setRange(d->intSpinBox->minimum(), max, d->intSpinBox->singleStep());
}

int KIntNumInput::maximum() const
{
    return d->intSpinBox->maximum();
}

int KIntNumInput::singleStep() const
{
    return d->intSpinBox->singleStep();
}

void KIntNumInput::setSingleStep(int singleStep)
{
    d->intSpinBox->setSingleStep(singleStep);
}

void KIntNumInput::setSuffix(const QString &suffix)
{
    d->intSpinBox->setSuffix(suffix);

    layout(true);
}

void KIntNumInput::setSuffix(const KLocalizedString& suffix)
{
    d->intSpinBox->setSuffix(suffix);
    layout(true);
}

QString KIntNumInput::suffix() const
{
    return d->intSpinBox->suffix();
}

void KIntNumInput::setPrefix(const QString &prefix)
{
    d->intSpinBox->setPrefix(prefix);

    layout(true);
}

QString KIntNumInput::prefix() const
{
    return d->intSpinBox->prefix();
}

void KIntNumInput::setEditFocus(bool mark)
{
    d->intSpinBox->setEditFocus(mark);
}

QSize KIntNumInput::minimumSizeHint() const
{
    K_USING_KNUMINPUT_P(priv);
    ensurePolished();

    int w;
    int h;

    h = qMax(d->intSpinBoxSize.height(), priv->sliderSize.height());

    // if in extra row, then count it here
    if (priv->label && (priv->labelAlignment & (Qt::AlignBottom | Qt::AlignTop))) {
        h += 4 + priv->labelSize.height();
    } else {
        // label is in the same row as the other widgets
        h = qMax(h, priv->labelSize.height() + 2);
    }

    w = priv->slider ? priv->slider->sizeHint().width() + 8 : 0;
    w += priv->column1Width + priv->column2Width;

    if (priv->labelAlignment & (Qt::AlignTop | Qt::AlignBottom)) {
        w = qMax(w, priv->labelSize.width() + 4);
    }

    return QSize(w, h);
}

void KIntNumInput::doLayout()
{
    K_USING_KNUMINPUT_P(priv);

    d->intSpinBoxSize = d->intSpinBox->sizeHint();
    priv->column2Width = d->intSpinBoxSize.width();

    if (priv->label) {
        priv->label->setBuddy(d->intSpinBox);
    }
}

void KIntNumInput::resizeEvent(QResizeEvent* e)
{
    K_USING_KNUMINPUT_P(priv);

    int w = priv->column1Width;
    int h = 0;

    if (priv->label && (priv->labelAlignment & Qt::AlignTop)) {
        priv->label->setGeometry(0, 0, e->size().width(), priv->labelSize.height());
        h += priv->labelSize.height() + KDialog::spacingHint();
    }

    if (priv->label && (priv->labelAlignment & Qt::AlignVCenter)) {
        priv->label->setGeometry(0, 0, w, d->intSpinBoxSize.height());
    }

    if (qApp->layoutDirection()) {
        d->intSpinBox->setGeometry(w, h, priv->slider ? priv->column2Width : qMax(priv->column2Width, e->size().width() - w), d->intSpinBoxSize.height());
        w += priv->column2Width + 8;

        if (priv->slider) {
            priv->slider->setGeometry(w, h, e->size().width() - w, d->intSpinBoxSize.height() + KDialog::spacingHint());
        }
    } else if (priv->slider) {
        priv->slider->setGeometry(w, h, e->size().width() - (w + priv->column2Width + KDialog::spacingHint()), d->intSpinBoxSize.height() + KDialog::spacingHint());
        d->intSpinBox->setGeometry(w + priv->slider->size().width() + KDialog::spacingHint(), h, priv->column2Width, d->intSpinBoxSize.height());
    } else {
        d->intSpinBox->setGeometry(w, h, qMax(priv->column2Width, e->size().width() - w), d->intSpinBoxSize.height());
    }

    h += d->intSpinBoxSize.height() + 2;

    if (priv->label && (priv->labelAlignment & Qt::AlignBottom)) {
        priv->label->setGeometry(0, h, priv->labelSize.width(), priv->labelSize.height());
    }
}

KIntNumInput::~KIntNumInput()
{
    delete d;
}

void KIntNumInput::setValue(int val)
{
    d->intSpinBox->setValue(val);
    // slider value is changed by spinValueChanged
}

void KIntNumInput::setRelativeValue(double r)
{
    if (!d->referencePoint) {
        return;
    }
    ++d->blockRelative;
    setValue(qRound(d->referencePoint * r + 0.5));
    --d->blockRelative;
}

double KIntNumInput::relativeValue() const
{
    if (!d->referencePoint) {
        return 0;
    }
    return double(value()) / double(d->referencePoint);
}

int KIntNumInput::value() const
{
    return d->intSpinBox->value();
}

void KIntNumInput::setSpecialValueText(const QString& text)
{
    d->intSpinBox->setSpecialValueText(text);
    layout(true);
}

QString KIntNumInput::specialValueText() const
{
    return d->intSpinBox->specialValueText();
}

void KIntNumInput::setLabel(const QString & label, Qt::Alignment a)
{
    K_USING_KNUMINPUT_P(priv);

    KNumInput::setLabel(label, a);

    if (priv->label) {
        priv->label->setBuddy(d->intSpinBox);
    }
}

// ----------------------------------------------------------------------------

class KDoubleNumInput::KDoubleNumInputPrivate
{
public:
    KDoubleNumInputPrivate(double r)
            : spin(0),
            referencePoint(r),
            blockRelative(0),
            exponentRatio(1.0) {}
    QDoubleSpinBox * spin;
    double referencePoint;
    short blockRelative;
    QSize editSize;
    QString specialValue;
    double exponentRatio;
};

KDoubleNumInput::KDoubleNumInput(QWidget *parent)
    : KNumInput(parent)
    , d(new KDoubleNumInputPrivate(0.0))

{
    init(0.0, 0.0, 9999.0, 0.01, 2);
}

KDoubleNumInput::KDoubleNumInput(double lower, double upper, double value, QWidget *parent,
                                 double singleStep, int precision)
    : KNumInput(parent)
    , d(new KDoubleNumInputPrivate(value))
{
    init(value, lower, upper, singleStep, precision);
}

KDoubleNumInput::KDoubleNumInput(KNumInput *below,
                                 double lower, double upper, double value, QWidget *parent,
                                 double singleStep, int precision)
    : KNumInput(parent, below)
    , d(new KDoubleNumInputPrivate(value))
{
    init(value, lower, upper, singleStep, precision);
}

KDoubleNumInput::~KDoubleNumInput()
{
    delete d;
}

QString KDoubleNumInput::specialValueText() const
{
    return d->specialValue;
}


void KDoubleNumInput::init(double value, double lower, double upper,
                           double singleStep, int precision)
{
    d->spin = new QDoubleSpinBox(this);
    d->spin->setRange(lower, upper);
    d->spin->setSingleStep(singleStep);
    d->spin->setValue(value);
    d->spin->setDecimals(precision);

    d->spin->setObjectName("KDoubleNumInput::QDoubleSpinBox");
    setFocusProxy(d->spin);
    connect(d->spin, SIGNAL(valueChanged(double)),
            this, SIGNAL(valueChanged(double)));
    connect(this, SIGNAL(valueChanged(double)),
            this, SLOT(slotEmitRelativeValueChanged(double)));

    updateLegacyMembers();

    layout(true);
}

void KDoubleNumInput::updateLegacyMembers()
{
    d->specialValue = specialValueText();
}

double KDoubleNumInput::mapSliderToSpin(int val) const
{
    K_USING_KNUMINPUT_P(priv);

    // map [slidemin,slidemax] to [spinmin,spinmax]
    const double spinmin = d->spin->minimum();
    const double spinmax = d->spin->maximum();
    const double slidemin = priv->slider->minimum(); // cast int to double to avoid
    const double slidemax = priv->slider->maximum(); // overflow in rel denominator
    const double rel = (double(val) - slidemin) / (slidemax - slidemin);
    Q_ASSERT(d->exponentRatio > 0.0);
    return spinmin + pow(rel, d->exponentRatio ) * (spinmax - spinmin);
}

void KDoubleNumInput::sliderMoved(int val)
{
    d->spin->setValue(mapSliderToSpin(val));
}

void KDoubleNumInput::spinBoxChanged(double val)
{
    K_USING_KNUMINPUT_P(priv);

    const double spinmin = d->spin->minimum();
    const double spinmax = d->spin->maximum();
    const double slidemin = priv->slider->minimum(); // cast int to double to avoid
    const double slidemax = priv->slider->maximum(); // overflow in rel denominator

    Q_ASSERT(d->exponentRatio > 0.0);
    const double rel = pow((val - spinmin) / (spinmax - spinmin) , 1.0 / d->exponentRatio);

    if (priv->slider) {
        priv->slider->blockSignals(true);
        priv->slider->setValue(qRound(slidemin + rel * (slidemax - slidemin)));
        priv->slider->blockSignals(false);
    }
}

void KDoubleNumInput::slotEmitRelativeValueChanged(double value)
{
    if (!d->referencePoint) {
        return;
    }
    emit relativeValueChanged(value / d->referencePoint);
}

QSize KDoubleNumInput::minimumSizeHint() const
{
    K_USING_KNUMINPUT_P(priv);

    ensurePolished();

    int w;
    int h;

    h = qMax(d->editSize.height(), priv->sliderSize.height());

    // if in extra row, then count it here
    if (priv->label && (priv->labelAlignment & (Qt::AlignBottom | Qt::AlignTop))) {
        h += 4 + priv->labelSize.height();
    } else {
        // label is in the same row as the other widgets
        h = qMax(h, priv->labelSize.height() + 2);
    }

    w = priv->slider ? priv->slider->sizeHint().width() + 8 : 0;
    w += priv->column1Width + priv->column2Width;

    if (priv->labelAlignment & (Qt::AlignTop | Qt::AlignBottom)) {
        w = qMax(w, priv->labelSize.width() + 4);
    }

    return QSize(w, h);
}

void KDoubleNumInput::resizeEvent(QResizeEvent* e)
{
    K_USING_KNUMINPUT_P(priv);

    int w = priv->column1Width;
    int h = 0;

    if (priv->label && (priv->labelAlignment & Qt::AlignTop)) {
        priv->label->setGeometry(0, 0, e->size().width(), priv->labelSize.height());
        h += priv->labelSize.height() + 4;
    }

    if (priv->label && (priv->labelAlignment & Qt::AlignVCenter)) {
        priv->label->setGeometry(0, 0, w, d->editSize.height());
    }

    if (qApp->layoutDirection()) {
        d->spin->setGeometry(w, h, priv->slider ? priv->column2Width
                             : e->size().width() - w, d->editSize.height());
        w += priv->column2Width + KDialog::spacingHint();

        if (priv->slider)
            priv->slider->setGeometry(w, h, e->size().width() - w, d->editSize.height() + KDialog::spacingHint());
    } else if (priv->slider) {
        priv->slider->setGeometry(w, h, e->size().width() -
                                    (priv->column1Width + priv->column2Width + KDialog::spacingHint()),
                                    d->editSize.height() + KDialog::spacingHint());
        d->spin->setGeometry(w + priv->slider->width() + KDialog::spacingHint(), h,
                             priv->column2Width, d->editSize.height());
    } else {
        d->spin->setGeometry(w, h, e->size().width() - w, d->editSize.height());
    }

    h += d->editSize.height() + 2;

    if (priv->label && (priv->labelAlignment & Qt::AlignBottom)) {
        priv->label->setGeometry(0, h, priv->labelSize.width(), priv->labelSize.height());
    }
}

void KDoubleNumInput::doLayout()
{
    K_USING_KNUMINPUT_P(priv);

    d->editSize = d->spin->sizeHint();
    priv->column2Width = d->editSize.width();
}

void KDoubleNumInput::setValue(double val)
{
    d->spin->setValue(val);
}

void KDoubleNumInput::setRelativeValue(double r)
{
    if (!d->referencePoint) {
        return;
    }
    ++d->blockRelative;
    setValue(r * d->referencePoint);
    --d->blockRelative;
}

void KDoubleNumInput::setReferencePoint(double ref)
{
    // clip to valid range:
    ref = qMin(maximum(), qMax(minimum(), ref));
    d->referencePoint = ref;
}

void KDoubleNumInput::setRange(double lower, double upper, double singleStep,
                               bool slider)
{
    K_USING_KNUMINPUT_P(priv);

    if (priv->slider) {
        // don't update the slider to avoid an endless recursion
        QDoubleSpinBox * spin = d->spin;
        disconnect(spin, SIGNAL(valueChanged(double)),
                   priv->slider, SLOT(setValue(int)));
    }
    d->spin->setRange(lower, upper);
    d->spin->setSingleStep(singleStep);

    setSliderEnabled(slider);

    setReferencePoint(referencePoint());

    layout(true);
    updateLegacyMembers();
}

void KDoubleNumInput::setSliderEnabled(bool enabled)
{
    K_USING_KNUMINPUT_P(priv);
    if (enabled) {
        QDoubleSpinBox * spin = d->spin;
        const double range = spin->maximum() - spin->minimum();
        const double steps = range * pow(10.0, spin->decimals());
        if (!priv->slider) {
            priv->slider = new QSlider(Qt::Horizontal, this);
            priv->slider->setTickPosition(QSlider::TicksBelow);
            // feedback line: when one moves, the other moves, too:
            connect(priv->slider, SIGNAL(valueChanged(int)),
                    SLOT(sliderMoved(int)));
            layout(true);
        }
        if (steps > 1000 || d->exponentRatio != 1.0) {
            priv->slider->setRange(0, 1000);
            priv->slider->setSingleStep(1);
            priv->slider->setPageStep(50);
        } else {
            const int singleSteps = qRound(steps);
            priv->slider->setRange(0, singleSteps);
            priv->slider->setSingleStep(1);
            const int pageSteps = qBound(1, singleSteps / 20, 10);
            priv->slider->setPageStep(pageSteps);
        }
        spinBoxChanged(spin->value());
        connect(spin, SIGNAL(valueChanged(double)), SLOT(spinBoxChanged(double)));
    } else {
        if (priv->slider) {
            layout(true);
        }
        delete priv->slider;
        priv->slider = 0;
    }
}


void KDoubleNumInput::setMinimum(double min)
{
    K_USING_KNUMINPUT_P(priv);
    setRange(min, maximum(), d->spin->singleStep(), priv->slider);
}

double KDoubleNumInput::minimum() const
{
    return d->spin->minimum();
}

void KDoubleNumInput::setMaximum(double max)
{
    K_USING_KNUMINPUT_P(priv);
    setRange(minimum(), max, d->spin->singleStep(), priv->slider);
}

double KDoubleNumInput::maximum() const
{
    return d->spin->maximum();
}

double KDoubleNumInput::singleStep() const
{
  return d->spin->singleStep();
}

void KDoubleNumInput::setSingleStep(double singleStep)
{
  d->spin->setSingleStep(singleStep);
}

double KDoubleNumInput::value() const
{
    return d->spin->value();
}

double KDoubleNumInput::relativeValue() const
{
    if (!d->referencePoint) {
        return 0;
    }
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
    d->spin->setSuffix(suffix);

    layout(true);
}

void KDoubleNumInput::setPrefix(const QString &prefix)
{
    d->spin->setPrefix(prefix);

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
    d->spin->setSpecialValueText(text);

    layout(true);
    updateLegacyMembers();
}

void KDoubleNumInput::setLabel(const QString & label, Qt::Alignment a)
{
    K_USING_KNUMINPUT_P(priv);

    KNumInput::setLabel(label, a);

    if (priv->label) {
        priv->label->setBuddy(d->spin);
    }
}

double KDoubleNumInput::exponentRatio() const
{
    return d->exponentRatio;
}

void KDoubleNumInput::setExponentRatio(double dbl)
{
    Q_ASSERT(dbl > 0.0);
    if(dbl > 0.0) {
        d->exponentRatio = dbl;
        spinBoxChanged( d->spin->value() ); // used to reset the value of the slider
    } else {
        kError() << "ExponentRatio need to be strictly positive.";
    }
}


#include "knuminput.moc"
