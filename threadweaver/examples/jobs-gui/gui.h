#ifndef GUI_H
#define GUI_H

#include <weaver.h>
#include "gui_base.h"


using namespace ThreadWeaver;

class DummyJob : public Job
{
public:
    DummyJob (QObject* parent = 0 , const char* name = 0);
protected:
    void run ();
};

const int NoOfJobs = 100;

class GUI : public GUIBase
{
    Q_OBJECT
public:
    GUI ( QWidget *parent = 0, const char *name = 0);
    ~GUI();
protected:
    void slotStart();
    void slotStop();
    void slotQuit();
    ThreadWeaver::Weaver weaver;
    QPtrList <Job> jobs;
    bool m_quit;
protected slots:
    void slotJobDone (Job*);
    void slotStopped ();
};

#endif // defined GUI_H
