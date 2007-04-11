/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "effectwidget.h"
#include "effectwidget_p.h"

#include <QtAlgorithms>
#include <QList>

#include "../effect.h"
#include "../effectparameter.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>

namespace Phonon
{

EffectWidget::EffectWidget(Effect *effect, QWidget *parent)
    : QWidget(parent),
    d_ptr(new EffectWidgetPrivate(effect))
{
    Q_D(EffectWidget);
    d->q_ptr = this;
    d->autogenerateUi();
}

EffectWidget::~EffectWidget()
{
    delete d_ptr;
    d_ptr = 0;
}

/*
EffectWidget::EffectWidget(EffectWidgetPrivate &dd, QWidget *parent)
    : QWidget(parent)
    , d_ptr(&dd)
{
    Q_D(EffectWidget);
    d->q_ptr = this;
    d->autogenerateUi();
}
*/

EffectWidgetPrivate::EffectWidgetPrivate(Effect *e)
    : effect(e)
{
    //TODO: look up whether there is a specialized widget for this effect. This
    //could be a DSO or a Designer ui file found via KTrader.
    //
    //if no specialized widget is available:
}

void EffectWidgetPrivate::autogenerateUi()
{
    Q_Q(EffectWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    QList<EffectParameter> plist = effect->parameterList();
    qSort(plist);
    foreach (EffectParameter para, plist)
    {
        QHBoxLayout *pLayout = new QHBoxLayout;
        mainLayout->addLayout(pLayout);

        QLabel *label = new QLabel(q);
        pLayout->addWidget(label);
        label->setText(para.name());
        label->setToolTip(para.description());

        QWidget *control;
        if (para.isToggleControl())
        {
            QCheckBox *cb = new QCheckBox(q);
            control = cb;
            cb->setChecked(para.value().toBool());
            QObject::connect(cb, SIGNAL(toggled(bool)), q, SLOT(_k_setToggleParameter(bool)));
        }
        else if (para.minimumValue().isValid() && para.maximumValue().isValid())
        {
            if (para.isIntegerControl())
            {
                QSpinBox *sb = new QSpinBox(q);
                control = sb;
                sb->setRange(para.minimumValue().toInt(),
                        para.maximumValue().toInt());
                sb->setValue(para.value().toInt());
                QObject::connect(sb, SIGNAL(valueChanged(int)), q, SLOT(_k_setIntParameter(int)));
            }
            else
            {
                QDoubleSpinBox *sb = new QDoubleSpinBox(q);
                control = sb;
                sb->setRange(para.minimumValue().toDouble(),
                        para.maximumValue().toDouble());
                sb->setValue(para.value().toDouble());
                sb->setSingleStep((para.maximumValue().toDouble() - para.minimumValue().toDouble()) / 20);
                QObject::connect(sb, SIGNAL(valueChanged(double)), q,
                        SLOT(_k_setDoubleParameter(double)));
            }
        }
        else
        {
            QDoubleSpinBox *sb = new QDoubleSpinBox(q);
            control = sb;
            sb->setDecimals(7);
            sb->setRange(-1e100, 1e100);
            QObject::connect(sb, SIGNAL(valueChanged(double)), q,
                    SLOT(_k_setDoubleParameter(double)));
        }
        control->setToolTip(para.description());
        label->setBuddy(control);
        pLayout->addWidget(control);
        parameterForObject.insert(control, para);
    }
}

void EffectWidgetPrivate::_k_setToggleParameter(bool checked)
{
    Q_Q(EffectWidget);
    EffectParameter p = parameterForObject.value(q->sender());
    if (p.isValid())
        p.setValue(checked);
}

void EffectWidgetPrivate::_k_setIntParameter(int value)
{
    Q_Q(EffectWidget);
    EffectParameter p = parameterForObject.value(q->sender());
    if (p.isValid())
        p.setValue(value);
}

void EffectWidgetPrivate::_k_setDoubleParameter(double value)
{
    Q_Q(EffectWidget);
    EffectParameter p = parameterForObject.value(q->sender());
    if (p.isValid())
        p.setValue(value);
}

} // namespace Phonon

#include "effectwidget.moc"

// vim: sw=4 ts=4
