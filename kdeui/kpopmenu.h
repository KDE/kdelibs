#ifndef _KPOPUP_H
#define _KPOPUP_H

#include <qpopupmenu.h>


/// Popup menus with a title bar
/**
Here's a popup menu widget for KDE apps. It differs from Qt popup menu in
that it has a title -- thus it is *not* to be used in a menu bar. Only two
new methods are added -- setTitle(char *) and title() and a new constructor,
taking the title as a first argument. The main API difference from Qt's
QPopupMenu is that you should *not* expect the first item you insert into
KPopupMenu to have index (an ID) 0! So the following is wrong:
	
<pre>
	KPopupMenu menu("Window operations");
	menu->insertItem("Move");
	menu->insertItem("Minimize");
	menu->insertItem("Close");

	menu->connectItem(0, this, SLOT(moveWindow()));		// WRONG!!!
	menu->connectItem(1, this, SLOT(minimizeWindow()));	// WRONG!!!
	menu->connectItem(2, this, SLOT(closeWindow()));	// WRONG!!!
</pre>

The reason is that the title and a double line (actually, two separators) are
stored as menu items too, so the first item you insert has index 3. There's
a constant KPM_FirstItem so use one of those approaches instead of the above:

<pre>
[1] int move_item = menu->insertItem("Move");
    ...
    menu->connectItem(menu_item, this, SLOT(moveWindow()));

[2] menu->insertItem("Move");
    ...
    menu->connectItem(KPM_FirstItem+0, this, SLOT(moveWindow()));

[3] The best one!
     menu->insertItem("Move", this, SLOT(moveWindow()));
</pre>

* @short Popup menu with title.
* @version $Id$
*/
class KPopupMenu : public QPopupMenu {
    Q_OBJECT
public:
    KPopupMenu(QWidget *parent=0, const char *name=0);
    KPopupMenu(const QString& title, QWidget *parent=0, const char *name=0);
    ~KPopupMenu();
    
    void setTitle(const QString& );
    QString title() const;
    
private:
    void paintCell(QPainter *, int, int);
    void initialize(const QString& );

};         

const int KPM_FirstItem = 3;


#endif
