#ifndef _KFILEFILTER_H
#define _KFILEFILTER_H

#include <qcombo.h>

class QStrList;

class KFileFilter : public QComboBox 
{
    Q_OBJECT

 public:
    KFileFilter(QWidget *parent= 0, const char *name= 0);
    ~KFileFilter();

    void setFilter(const char *filter);
    QString currentFilter();

 protected:
    QStrList *filters;

 protected slots:
    void changed(const char*);

 signals:
    void filterChanged();
};

#endif
