#ifndef __KFILEMETAINFOWIDGET_H__
#define __KFILEMETAINFOWIDGET_H__

#include <qwidget.h>
#include <qvariant.h>
#include <kfilemetainfo.h>

class KFileMetaInfoWidget: public QWidget
{
    Q_OBJECT
public:
    KFileMetaInfoWidget(KFileMetaInfoItem item, QValidator* val = 0,
                        QWidget* parent = 0, const char* name = 0);
    virtual ~KFileMetaInfoWidget();
    
    bool apply()
    {
        return m_item.isEditable() &&
               m_item.value()!= m_value && 
               m_item.setValue(m_value);
    }
    
    void setValue(const QVariant& value)  { m_value = value;    }
    QVariant value()                      { return m_value;     }
    QValidator* validator()               { return m_validator; }
    KFileMetaInfoItem item()              { return m_item;      }
  
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
    void slotComboChanged(int item);
    void slotLineEditChanged(const QString& value);
    void slotDateChanged(const QDate& value);
    void slotTimeChanged(const QTime& value);
    void slotDateTimeChanged(const QDateTime& value);

private:
    QVariant          m_value;  // the value will be saved here until apply() is called
    KFileMetaInfoItem m_item;
    QWidget*          m_widget;
    QValidator*       m_validator;
    bool              m_dirty : 1;
};

#endif
