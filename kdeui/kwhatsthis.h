#ifndef KWHATSTHIS_H
#define KWHATSTHIS_H

#include <qwhatsthis.h>
#include <kdialogbase.h>
#include <qapplication.h>

class QToolButton;
class QPopupMenu;
class QStyleSheet;
class QTextEdit;

/**
 * This class is used automatically by KMainWindow and KDialog to attach a
 * What's This templates to all new widgets. If you don't use either of those
 * classes and want this functionality, call KWhatsThisManager::init () before
 * creating any widgets.
 *
 * It works by watching for child creation events and if the new objects are
 * widgets, it attachs the whatsthis template to them (unless there already is
 * What's This defined). This template will be overriden by any custom What's
 * This info.
 *
 * @see QWhatsThis
 * @author Peter Rockai (mornfall) <mornfall@danill.sk>
 **/
class KWhatsThisManager : public QObject
{
    Q_OBJECT
    public:
        static void init ();
        bool eventFilter (QObject *o, QEvent *e);
    protected:
        static KWhatsThisManager *s_instance;
    private:
        KWhatsThisManager ();
};

#endif // KWHATHSTHIS_H
