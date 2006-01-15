#include <kapplication.h>
#include <kcmdlineargs.h>

#include <QWidget>
#include <QTimerEvent>

#include "kprogressbar.h"

class MyWidget : public QWidget {
public:
    MyWidget() : QWidget()
    {
        setFixedSize(440, 80);
        Progress = new KProgressBar(this);
        Progress->resize(400, 40);
        Progress->move(20, 20);
        startTimer(50);
    }

private:
    KProgressBar *Progress;

    int steps;

    void timerEvent(QTimerEvent *);
};

void MyWidget::timerEvent(QTimerEvent *)
{
    static enum { fwd, back } direction = fwd;
    if (direction == fwd)
    {
        Progress->setFormat("Forward - %p% - Value: %v - Maximum: %m");
        if (Progress->value() == Progress->maximum())
        {
            steps = rand() % 200;
            Progress->setRange(0, steps);
            Progress->setValue(Progress->maximum());
            direction = back;
        }
        else
            Progress->advance(1);
    }
    else
    {
        Progress->setFormat("Back - %p% - Value: %v - Maximum: %m");
        if (Progress->value() == Progress->minimum())
        {
            steps = rand() % 200;
            Progress->setRange(0, steps);
            Progress->setValue(Progress->minimum());
            direction = fwd;
        }
        else
            Progress->advance(-1);
    }
}

int main(int argc, char *argv[])
{
    KCmdLineArgs::init( argc, argv, "test", "Test" ,"test app" ,"1.0" );
    KApplication app;
    MyWidget w;

    app.setMainWidget(&w);

    w.show();

    int ret = app.exec();
    return ret;
}
