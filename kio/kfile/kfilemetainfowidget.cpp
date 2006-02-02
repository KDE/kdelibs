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

#include <keditcl.h>
#include <klocale.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kstringvalidator.h>
#include <kdebug.h>

#include <qlabel.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <q3datetimeedit.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qlayout.h>
#include <qvalidator.h>

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
    : QWidget(parent),
      m_value(item.value()),
      m_item(item),
      m_validator(val)
{
    init(item, ReadWrite);
}

KFileMetaInfoWidget::KFileMetaInfoWidget(KFileMetaInfoItem item,
                                         Mode mode,
                                         QValidator* val,
                                         QWidget* parent)
    : QWidget(parent),
      m_value(item.value()),
      m_item(item),
      m_validator(val)
{
    init(item, mode);
}

void KFileMetaInfoWidget::init(KFileMetaInfoItem item, Mode mode)
{
    kDebug(7033) << "*** item "  << m_item.key()
                  << " is a " << value().typeName() << endl;

    if (m_item.isEditable() && !(mode & ReadOnly))
        m_widget = makeWidget();
    else
        switch (m_value.type())
        {
            case QVariant::Image :
                m_widget = new QLabel(this, "info image");
                static_cast<QLabel*>(m_widget)->setPixmap(QPixmap(m_value.value<QImage>()));
                break;
            case QVariant::Pixmap :
                m_widget = new QLabel(this, "info pixmap");
                static_cast<QLabel*>(m_widget)->setPixmap(m_value.value<QPixmap>());
                break;
            default:
                m_widget = new QLabel(item.string(true), this, "info label");
        }

    (new QHBoxLayout(this))->addWidget(m_widget);
}

KFileMetaInfoWidget::~KFileMetaInfoWidget()
{
}

QWidget* KFileMetaInfoWidget::makeWidget()
{
    QString valClass;
    QWidget* w;

    switch (m_value.type())
    {
        case QVariant::Invalid:     // no type
            // just make a label
            w = new QLabel(i18n("<Error>"), this, "label");
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

    kDebug(7033) << "*** item " << m_item.key()
                  << "is a " << m_item.value().typeName() << endl;
    if (m_validator)
        kDebug(7033) << " and validator is a " << m_validator->className() << endl;

    kDebug(7033) << "*** created a " << w->className() << " for it\n";

    return w;
}

// ****************************************************************
// now the different methods to make the widgets for specific types
// ****************************************************************

QWidget* KFileMetaInfoWidget::makeBoolWidget()
{
    QCheckBox* cb = new QCheckBox(this, "metainfo bool widget");
    cb->setChecked(m_item.value().toBool());
    connect(cb, SIGNAL(toggled(bool)), this, SLOT(slotChanged(bool)));
    return cb;
}

QWidget* KFileMetaInfoWidget::makeIntWidget()
{
    QSpinBox* sb = new QSpinBox(this, "metainfo integer widget");
    sb->setValue(m_item.value().toInt());

    if (m_validator)
    {
        if (QIntValidator* iv = qobject_cast<QIntValidator*>(m_validator))
        {
            sb->setMinimum(iv->bottom());
            sb->setMaximum(iv->top());
        }
        //reparentValidator(sb, m_validator);
        //sb->setValidator(m_validator);
    }

    // make sure that an uint cannot be set to a value < 0
    if (m_item.type() == QVariant::UInt)
        sb->setMinimum(QMAX(sb->minimum(), 0));

    connect(sb, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));
    return sb;
}

QWidget* KFileMetaInfoWidget::makeDoubleWidget()
{
    double value=m_item.value().toDouble();
    KDoubleNumInput* dni = new KDoubleNumInput(qMin(0.0,value),qMax(0.0,value),value,this,0.01,2);


    if (m_validator)
    {
        if (QDoubleValidator* dv = qobject_cast<QDoubleValidator*>(m_validator))
        {
            dni->setMinimum(dv->bottom());
            dni->setMaximum(dv->top());
        }
        reparentValidator(dni, m_validator);
    }

    connect(dni, SIGNAL(valueChanged(double)), this, SLOT(slotChanged(double)));
    return dni;
}

QWidget* KFileMetaInfoWidget::makeStringWidget()
{
    if (KStringListValidator* val = qobject_cast<KStringListValidator*>(m_validator))
    {
        KComboBox* b = new KComboBox(true, this);
        b->insertStringList(val->stringList());
        b->setCurrentText(m_item.value().toString());
        connect(b, SIGNAL(activated(const QString &)), this, SLOT(slotComboChanged(const QString &)));
        b->setValidator(val);
        reparentValidator(b, val);
        return b;
    }

    if ( m_item.attributes() & KFileMimeTypeInfo::MultiLine ) {
        KEdit *edit = new KEdit( this );
        edit->setText( m_item.value().toString() );
        connect( edit, SIGNAL( textChanged() ),
                 this, SLOT( slotMultiLineEditChanged() ));
        // can't use a validator with a QTextEdit, but we may need to delete it
        if ( m_validator )
            reparentValidator( edit, m_validator );
        return edit;
    }

    KLineEdit* e = new KLineEdit(m_item.value().toString(), this);
    if (m_validator)
    {
        e->setValidator(m_validator);
        reparentValidator(e, m_validator);
    }
    connect(e,    SIGNAL(textChanged(const QString&)),
            this, SLOT(slotLineEditChanged(const QString&)));
    return e;
}

QWidget* KFileMetaInfoWidget::makeDateWidget()
{
  QWidget *e = new Q3DateEdit(m_item.value().toDate(), this);
  connect(e,    SIGNAL(valueChanged(const QDate&)),
          this, SLOT(slotDateChanged(const QDate&)));
  return e;
}

QWidget* KFileMetaInfoWidget::makeTimeWidget()
{
  return new Q3TimeEdit(m_item.value().toTime(), this);
}

QWidget* KFileMetaInfoWidget::makeDateTimeWidget()
{
  return new Q3DateTimeEdit(m_item.value().toDateTime(), this);
}

void KFileMetaInfoWidget::reparentValidator( QWidget *widget,
                                             QValidator *validator )
{
    if ( !validator->parent() )
        widget->insertChild( validator );
}

// ****************************************************************
// now the slots that let us get notified if the value changed in the child
// ****************************************************************

void KFileMetaInfoWidget::slotChanged(bool value)
{
    Q_ASSERT(qobject_cast<QComboBox*>(m_widget));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotChanged(int value)
{
    Q_ASSERT(qobject_cast<QSpinBox*>(m_widget));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotChanged(double value)
{
    Q_ASSERT(qobject_cast<KDoubleNumInput*>(m_widget));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotComboChanged(const QString &value)
{
    Q_ASSERT(qobject_cast<KComboBox*>(m_widget));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotLineEditChanged(const QString& value)
{
    Q_ASSERT(qobject_cast<KLineEdit*>(m_widget));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

// that may be a little expensive for long texts, but what can we do?
void KFileMetaInfoWidget::slotMultiLineEditChanged()
{
    Q_ASSERT(qobject_cast<Q3TextEdit*>(m_widget));
    m_value = QVariant( static_cast<const Q3TextEdit*>( sender() )->text() );
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotDateChanged(const QDate& value)
{
    Q_ASSERT(qobject_cast<Q3DateEdit*>(m_widget));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotTimeChanged(const QTime& value)
{
    Q_ASSERT(qobject_cast<Q3TimeEdit*>(m_widget));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotDateTimeChanged(const QDateTime& value)
{
    Q_ASSERT(qobject_cast<Q3DateTimeEdit*>(m_widget));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

#include "kfilemetainfowidget.moc"
