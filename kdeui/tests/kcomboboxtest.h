#ifndef somethinghere
#define somethinghere

#include <qwidget.h>
class KComboBox;

class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget();
//private slots:
//    void slotDisplay();
private:
    KComboBox * rwc;
    KComboBox * konqc;
    KComboBox * soc;
};

#endif
