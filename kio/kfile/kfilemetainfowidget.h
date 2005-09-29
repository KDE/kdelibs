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

#ifndef __KFILEMETAINFOWIDGET_H__
#define __KFILEMETAINFOWIDGET_H__

#include <qwidget.h>
#include <qvariant.h>
#include <kfilemetainfo.h>

/*!
 * A widget to display file meta infos (like id3 for mp3)
 */
class KIO_EXPORT KFileMetaInfoWidget: public QWidget
{
    Q_OBJECT
public:
    enum Mode
    {
      ReadOnly  = 1,  ///Only display the meta infos, and do not permit the user to edit them
      ReadWrite = 0,  ///Permits user to edit the displayed meta-info
      Reserve = 0xff
    };
  
    KFileMetaInfoWidget(KFileMetaInfoItem item, QValidator* val = 0,
                        QWidget* parent = 0, const char* name = 0);

    KFileMetaInfoWidget(KFileMetaInfoItem item, Mode mode, QValidator* val = 0,
                        QWidget* parent = 0, const char* name = 0);

    virtual ~KFileMetaInfoWidget();

    bool apply()
    {
        return m_item.isEditable() && m_item.setValue(m_value);
    }

    void setValue(const QVariant& value)  { m_value = value;    }
    QVariant value()const                      { return m_value;     }
    QValidator* validator() const              { return m_validator; }
    KFileMetaInfoItem item()const              { return m_item;      }

signals:
    void valueChanged(const QVariant& value);

protected:
    void reparentValidator(QWidget *widget, QValidator *validator);
    virtual QWidget* makeWidget();

    QWidget* makeBoolWidget();
    QWidget* makeIntWidget();
    QWidget* makeDoubleWidget();
    QWidget* makeStringWidget();
    QWidget* makeDateWidget();
    QWidget* makeTimeWidget();
    QWidget* makeDateTimeWidget();

private slots:
    void slotChanged(bool value);
    void slotChanged(int value);
    void slotChanged(double value);
    void slotComboChanged(const QString &value);
    void slotLineEditChanged(const QString& value);
    void slotMultiLineEditChanged();
    void slotDateChanged(const QDate& value);
    void slotTimeChanged(const QTime& value);
    void slotDateTimeChanged(const QDateTime& value);

private:
    void init(KFileMetaInfoItem item, Mode mode);

    QVariant          m_value;  // the value will be saved here until apply() is called
    KFileMetaInfoItem m_item;
    QWidget*          m_widget;
    QValidator*       m_validator;
    bool              m_dirty : 1;
};

#endif
