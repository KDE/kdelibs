#ifndef somethinghere
#define somethinghere

#include <qwidget.h>
class KComboBox;

class Widget : public QObject
{
    Q_OBJECT
public:
    Widget();
private slots:
    void slotActivated( int );
};

#endif
