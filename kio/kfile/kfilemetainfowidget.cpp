/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Rolf Magnus <ramagnus@kde.org>

    library is free software; you can redistribute it and/or
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

#include "kfilemetainfowidget.h"

#include <ktextedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kstringvalidator.h>
#include <kdebug.h>

#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QDateEdit>
#include <QtGui/QPixmap>
#include <QtGui/QImage>
#include <QtGui/QLayout>
#include <QtGui/QSizePolicy>
#include <QtGui/QDoubleValidator>

class KFileMetaInfoWidgetPrivate
{
public:
    KFileMetaInfoWidgetPrivate(KFileMetaInfoWidget *qq)
        : q(qq)
    {
    }

    void init(KFileMetaInfoItem item, KFileMetaInfoWidget::Mode mode);

    KFileMetaInfoWidget *q;
    QVariant m_value;  // the value will be saved here until apply() is called
    KFileMetaInfoItem m_item;
    QWidget *m_widget;
    QValidator *m_validator;
    bool m_dirty : 1;
};

/*
  Widgets used for different types:

  bool      : QCheckBox
  int       : QSpinBox
  QString   : KComboBox if the validator is a KStringListValidator, else lineedit
  QDateTime : QDateTimeEdit

*/

KFileMetaInfoWidget::KFileMetaInfoWidget(KFileMetaInfoItem item,
                                         QValidator* val,
                                         QWidget* parent )
    : QWidget(parent), d(new KFileMetaInfoWidgetPrivate(this))
{
    d->m_value = item.value();
    d->m_item = item;
    d->m_validator = val;
    d->init(item, ReadWrite);
}

KFileMetaInfoWidget::KFileMetaInfoWidget(KFileMetaInfoItem item,
                                         Mode mode,
                                         QValidator* val,
                                         QWidget* parent)
    : QWidget(parent), d(new KFileMetaInfoWidgetPrivate(this))
{
    d->m_value = item.value();
    d->m_item = item;
    d->m_validator = val;
    d->init(item, mode);
}

void KFileMetaInfoWidgetPrivate::init(KFileMetaInfoItem item, KFileMetaInfoWidget::Mode mode)
{
    Q_UNUSED(item)
    kDebug(7033) << "*** item "  << m_item.name()
                  << " is a " << m_value.typeName() << endl;

    if (m_item.isEditable() && !(mode & KFileMetaInfoWidget::ReadOnly))
        m_widget = q->makeWidget();
    else
        switch (m_value.type())
        {
            case QVariant::Image :
                m_widget = new QLabel(q);
                m_widget->setObjectName(QLatin1String("info image"));
                static_cast<QLabel*>(m_widget)->setPixmap(QPixmap::fromImage(m_value.value<QImage>()));
                break;
            case QVariant::Pixmap :
                m_widget = new QLabel(q);
                m_widget->setObjectName(QLatin1String("info pixmap"));
                static_cast<QLabel*>(m_widget)->setPixmap(m_value.value<QPixmap>());
                break;
            default:
                m_widget = new QLabel(m_value.toString(), q);
                m_widget->setObjectName(QLatin1String("info label"));
        }

    QHBoxLayout* lay = new QHBoxLayout(q);
    lay->setMargin(0);
    lay->addWidget(m_widget);

    QSizePolicy sp = q->sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Minimum);
    q->setSizePolicy(sp);
}

KFileMetaInfoWidget::~KFileMetaInfoWidget()
{
    delete d;
}

bool KFileMetaInfoWidget::apply()
{
    return d->m_item.isEditable() && d->m_item.setValue(d->m_value);
}

void KFileMetaInfoWidget::setValue(const QVariant &value)
{
    d->m_value = value;
}

QVariant KFileMetaInfoWidget::value() const
{
    return d->m_value;
}

QValidator* KFileMetaInfoWidget::validator() const
{
    return d->m_validator;
}

KFileMetaInfoItem KFileMetaInfoWidget::item() const
{
    return d->m_item;
}

QWidget* KFileMetaInfoWidget::makeWidget()
{
    QString valClass;
    QWidget* w;

    switch (d->m_value.type()) {
        case QVariant::Invalid:     // no type
            // just make a label
            w = new QLabel(i18n("<Error>"), this);
            w->setObjectName(QLatin1String("label"));
            break;

        case QVariant::Int:         // an int
        case QVariant::UInt:        // an unsigned int
            w = makeIntWidget();
            break;

        case QVariant::Bool:        // a bool
            w = makeBoolWidget();
            break;

        case QVariant::Double:      // a double
            w = makeDoubleWidget();
            break;


        case QVariant::Date:        // a QDate
            w = makeDateWidget();
            break;

        case QVariant::Time:        // a QTime
            w = makeTimeWidget();
            break;

        case QVariant::DateTime:    // a QDateTime
            w = makeDateTimeWidget();
            break;

#if 0
        case QVariant::Size:        // a QSize
        case QVariant::String:      // a QString
        case QVariant::List:        // a QValueList
        case QVariant::Map:         // a QMap
        case QVariant::StringList:  //  a QStringList
        case QVariant::Font:        // a QFont
        case QVariant::Pixmap:      // a QPixmap
        case QVariant::Brush:       // a QBrush
        case QVariant::Rect:        // a QRect
        case QVariant::Color:       // a QColor
        case QVariant::Palette:     // a QPalette
        case QVariant::ColorGroup:  // a QColorGroup
        case QCoreVariant::Icon:     // a QIconSet
        case QVariant::Point:       // a QPoint
        case QVariant::Image:       // a QImage
        case QVariant::CString:     // a QCString
        case QVariant::PointArray:  // a QPointArray
        case QVariant::Region:      // a QRegion
        case QVariant::Bitmap:      // a QBitmap
        case QVariant::Cursor:      // a QCursor
        case QVariant::ByteArray:   // a QByteArray
        case QVariant::BitArray:    // a QBitArray
        case QVariant::SizePolicy:  // a QSizePolicy
        case QVariant::KeySequence: // a QKeySequence
#endif
        default:
            w = makeStringWidget();
    }

    kDebug(7033) << "*** item " << d->m_item.name()
                 << "is a " << d->m_item.value().typeName() << endl;
    if (d->m_validator)
        kDebug(7033) << " and validator is a "
                     << d->m_validator->metaObject()->className() << endl;

    kDebug(7033) << "*** created a " << w->metaObject()->className()
                 << " for it\n";

    return w;
}

// ****************************************************************
// now the different methods to make the widgets for specific types
// ****************************************************************

QWidget* KFileMetaInfoWidget::makeBoolWidget()
{
    QCheckBox* cb = new QCheckBox(this);
    cb->setObjectName(QLatin1String("metainfo bool widget"));
    cb->setChecked(d->m_item.value().toBool());
    connect(cb, SIGNAL(toggled(bool)), this, SLOT(slotChanged(bool)));
    return cb;
}

QWidget* KFileMetaInfoWidget::makeIntWidget()
{
    QSpinBox* sb = new QSpinBox(this);
    sb->setObjectName(QLatin1String("metainfo integer widget"));
    sb->setValue(d->m_item.value().toInt());

    if (d->m_validator) {
        if (QIntValidator* iv = qobject_cast<QIntValidator*>(d->m_validator)) {
            sb->setMinimum(iv->bottom());
            sb->setMaximum(iv->top());
        }
        //reparentValidator(sb, m_validator);
        //sb->setValidator(m_validator);
    }

    // make sure that an uint cannot be set to a value < 0
    if (d->m_item.properties().type() == QVariant::UInt)
        sb->setMinimum(qMax(sb->minimum(), 0));

    connect(sb, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));
    return sb;
}

QWidget* KFileMetaInfoWidget::makeDoubleWidget()
{
    double value = d->m_item.value().toDouble();
    KDoubleNumInput* dni = new KDoubleNumInput(qMin(0.0,value),qMax(0.0,value),value,this,0.01,2);


    if (d->m_validator) {
        if (QDoubleValidator* dv = qobject_cast<QDoubleValidator*>(d->m_validator)) {
            dni->setMinimum(dv->bottom());
            dni->setMaximum(dv->top());
        }
        reparentValidator(dni, d->m_validator);
    }

    connect(dni, SIGNAL(valueChanged(double)), this, SLOT(slotChanged(double)));
    return dni;
}

QWidget* KFileMetaInfoWidget::makeStringWidget()
{
    if (KStringListValidator* val = qobject_cast<KStringListValidator*>(d->m_validator)) {
        KComboBox* b = new KComboBox(true, this);
        b->addItems(val->stringList());
        int i = b->findText(d->m_item.value().toString());
        if (i != -1)
            b->setCurrentIndex(i);
        else
            b->setEditText(d->m_item.value().toString());
        connect(b, SIGNAL(activated(const QString &)), this, SLOT(slotComboChanged(const QString &)));
        b->setValidator(val);
        reparentValidator(b, val);
        return b;
    }

    if (d->m_item.properties().attributes() & PredicateProperties::MultiLine) {
        KTextEdit *edit = new KTextEdit( this );
        edit->setAcceptRichText(false);
        edit->setPlainText(d->m_item.value().toString());
        connect( edit, SIGNAL( textChanged() ),
                 this, SLOT( slotMultiLineEditChanged() ));
        // can't use a validator with a KTextEdit, but we may need to delete it
        if (d->m_validator)
            reparentValidator(edit, d->m_validator);
        return edit;
    }

    KLineEdit* e = new KLineEdit(d->m_item.value().toString(), this);
    if (d->m_validator) {
        e->setValidator(d->m_validator);
        reparentValidator(e, d->m_validator);
    }
    connect(e,    SIGNAL(textChanged(const QString&)),
            this, SLOT(slotLineEditChanged(const QString&)));
    return e;
}

QWidget* KFileMetaInfoWidget::makeDateWidget()
{
  QWidget *e = new QDateEdit(d->m_item.value().toDate(), this);
  connect(e,    SIGNAL(valueChanged(const QDate&)),
          this, SLOT(slotDateChanged(const QDate&)));
  return e;
}

QWidget* KFileMetaInfoWidget::makeTimeWidget()
{
  return new QTimeEdit(d->m_item.value().toTime(), this);
}

QWidget* KFileMetaInfoWidget::makeDateTimeWidget()
{
  return new QDateTimeEdit(d->m_item.value().toDateTime(), this);
}

void KFileMetaInfoWidget::reparentValidator( QWidget *widget,
                                             QValidator *validator )
{
    if ( !validator->parent() )
        validator->setParent( widget );
}

// ****************************************************************
// now the slots that let us get notified if the value changed in the child
// ****************************************************************

void KFileMetaInfoWidget::slotChanged(bool value)
{
    Q_ASSERT(qobject_cast<QComboBox*>(d->m_widget));
    d->m_value = QVariant(value);
    emit valueChanged(d->m_value);
    d->m_dirty = true;
}

void KFileMetaInfoWidget::slotChanged(int value)
{
    Q_ASSERT(qobject_cast<QSpinBox*>(d->m_widget));
    d->m_value = QVariant(value);
    emit valueChanged(d->m_value);
    d->m_dirty = true;
}

void KFileMetaInfoWidget::slotChanged(double value)
{
    Q_ASSERT(qobject_cast<KDoubleNumInput*>(d->m_widget));
    d->m_value = QVariant(value);
    emit valueChanged(d->m_value);
    d->m_dirty = true;
}

void KFileMetaInfoWidget::slotComboChanged(const QString &value)
{
    Q_ASSERT(qobject_cast<KComboBox*>(d->m_widget));
    d->m_value = QVariant(value);
    emit valueChanged(d->m_value);
    d->m_dirty = true;
}

void KFileMetaInfoWidget::slotLineEditChanged(const QString& value)
{
    Q_ASSERT(qobject_cast<KLineEdit*>(d->m_widget));
    d->m_value = QVariant(value);
    emit valueChanged(d->m_value);
    d->m_dirty = true;
}

// that may be a little expensive for long texts, but what can we do?
void KFileMetaInfoWidget::slotMultiLineEditChanged()
{
    Q_ASSERT(qobject_cast<KTextEdit*>(d->m_widget));
    d->m_value = QVariant(static_cast<const KTextEdit*>(sender())->toPlainText());
    emit valueChanged(d->m_value);
    d->m_dirty = true;
}

void KFileMetaInfoWidget::slotDateChanged(const QDate& value)
{
    Q_ASSERT(qobject_cast<QDateEdit*>(d->m_widget));
    d->m_value = QVariant(value);
    emit valueChanged(d->m_value);
    d->m_dirty = true;
}

void KFileMetaInfoWidget::slotTimeChanged(const QTime& value)
{
    Q_ASSERT(qobject_cast<QTimeEdit*>(d->m_widget));
    d->m_value = QVariant(value);
    emit valueChanged(d->m_value);
    d->m_dirty = true;
}

void KFileMetaInfoWidget::slotDateTimeChanged(const QDateTime& value)
{
    Q_ASSERT(qobject_cast<QDateTimeEdit*>(d->m_widget));
    d->m_value = QVariant(value);
    emit valueChanged(d->m_value);
    d->m_dirty = true;
}

#include "kfilemetainfowidget.moc"
