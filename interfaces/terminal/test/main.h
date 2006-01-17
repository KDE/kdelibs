#ifndef MAIN_H
#define MAIN_H


class Win
    : public KMainWindow
{
    Q_OBJECT
    KParts::Part* p;
public:
    Win();
public Q_SLOTS:
    void pythonExited( int status );
};

#endif
