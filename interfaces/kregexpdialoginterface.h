#ifndef __kregexpdialoginterface_h__
#define __kregexpdialoginterface_h__

class QWidget;

// obsolete. don't use. will disappear next friday!
class KRegExpDialogInterface
{
public:
    virtual QWidget *regExpEditor() const = 0;
};

#endif

