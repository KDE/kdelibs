// -*- c++ -*-

#ifndef _KFILEFILTER_H
#define _KFILEFILTER_H

#include <qcombobox.h>

class QStrList;

class KFileFilter : public QComboBox
{
    Q_OBJECT

 public:
    KFileFilter(QWidget *parent= 0, const char *name= 0);
    ~KFileFilter();

    void setFilter(const QString& filter);
    QString currentFilter() const;

 protected:
    QStringList filters;
    QLineEdit *edit;
    
    virtual bool eventFilter( QObject *, QEvent * );

 protected slots:
    void changed(const QString &);

 signals:
    void filterChanged();
};

#endif
