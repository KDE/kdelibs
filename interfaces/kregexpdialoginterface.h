#ifndef __kregexpdialoginterface_h__
#define __kregexpdialoginterface_h__

class QWidget;

class KRegExpDialogInterface
{
public:
    virtual QWidget *regExpEditor() const = 0;
};

#endif

