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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
  
    $Id$
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
#include <qdatetimeedit.h>
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
                                         QWidget* parent, const char* name)
    : QWidget(parent, name),
      m_value(item.value()),
      m_item(item),
      m_validator(val)
{
    kdDebug(7033) << "*** item "  << m_item.key()
                  << " is a " << value().typeName() << endl;

    if (m_item.isEditable())
        m_widget = makeWidget();
    else
        switch (m_value.type())
        {
            case QVariant::Image :
                m_widget = new QLabel(this, "info image");
                static_cast<QLabel*>(m_widget)->setPixmap(QPixmap(m_value.toImage()));
                break;
            case QVariant::Pixmap :
                m_widget = new QLabel(this, "info pixmap");
                static_cast<QLabel*>(m_widget)->setPixmap(m_value.toPixmap());
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
        case QVariant::IconSet:     // a QIconSet
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

    kdDebug(7033) << "*** item " << m_item.key()
                  << "is a " << m_item.value().typeName() << endl;
    if (m_validator)
        kdDebug(7033) << " and validator is a " << m_validator->className() << endl;

    kdDebug(7033) << "*** created a " << w->className() << " for it\n";

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
        if (m_validator->inherits("QIntValidator"))
        {
            sb->setMinValue(static_cast<QIntValidator*>(m_validator)->bottom());
            sb->setMaxValue(static_cast<QIntValidator*>(m_validator)->top());
        }
        reparentValidator(sb, m_validator);
        sb->setValidator(m_validator);
    }

    // make sure that an uint cannot be set to a value < 0
    if (m_item.type() == QVariant::UInt)
        sb->setMinValue(QMAX(sb->minValue(), 0));

    connect(sb, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));
    return sb;
}

QWidget* KFileMetaInfoWidget::makeDoubleWidget()
{
    KDoubleNumInput* dni = new KDoubleNumInput(m_item.value().toDouble(),
                                               this, "metainfo double widget");


    if (m_validator)
    {
        if (m_validator->inherits("QDoubleValidator"))
        {
            dni->setMinValue(static_cast<QDoubleValidator*>(m_validator)->bottom());
            dni->setMaxValue(static_cast<QDoubleValidator*>(m_validator)->top());
        }
        reparentValidator(dni, m_validator);
    }

    connect(dni, SIGNAL(valueChanged(double)), this, SLOT(slotChanged(double)));
    return dni;
}

QWidget* KFileMetaInfoWidget::makeStringWidget()
{
    if (m_validator && m_validator->inherits("KStringListValidator"))
    {
        KComboBox* b = new KComboBox(true, this, "metainfo combobox");
        KStringListValidator* val = static_cast<KStringListValidator*>
                                                    (m_validator);
        b->insertStringList(val->stringList());
        b->setCurrentText(m_item.value().toString());
        connect(b, SIGNAL(activated(int)), this, SLOT(slotComboChanged(int)));
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
  return new QDateEdit(m_item.value().toDate(), this);
}

QWidget* KFileMetaInfoWidget::makeTimeWidget()
{
  return new QTimeEdit(m_item.value().toTime(), this);
}

QWidget* KFileMetaInfoWidget::makeDateTimeWidget()
{
  return new QDateTimeEdit(m_item.value().toDateTime(), this);
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
    Q_ASSERT(m_widget->inherits("QComboBox"));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotChanged(int value)
{
    Q_ASSERT(m_widget->inherits("QSpinBox"));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotChanged(double value)
{
    Q_ASSERT(m_widget->inherits("KDoubleNumInput"));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotComboChanged(int /*item*/)
{
    Q_ASSERT(m_widget->inherits("KComboBox"));
//    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotLineEditChanged(const QString& value)
{
    Q_ASSERT(m_widget->inherits("KLineEdit"));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

// that may be a little expensive for long texts, but what can we do?
void KFileMetaInfoWidget::slotMultiLineEditChanged()
{
    Q_ASSERT(m_widget->inherits("QTextEdit"));
    m_value = QVariant( static_cast<const QTextEdit*>( sender() )->text() );
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotDateChanged(const QDate& value)
{
    Q_ASSERT(m_widget->inherits("QDateEdit"));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotTimeChanged(const QTime& value)
{
    Q_ASSERT(m_widget->inherits("QTimeEdit"));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotDateTimeChanged(const QDateTime& value)
{
    Q_ASSERT(m_widget->inherits("QDateTimeEdit"));
    m_value = QVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

#include "kfilemetainfowidget.moc"
