/*****************************************************************

Copyright (c) 2000 Matthias Elter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __kpanelapplet_h__
#define __kpanelapplet_h__

class KConfig;

#include <qframe.h>

/**
 * Implement a panel applet.
 *
 * Panel applets
 *   @li Are small applications living in the KDE panel.
 *   @li Are implemented as DSOs (Dynamic Shared Objects).
 *
 * Note: For security and stability reasons the panel won't load
 * untrusted third party applets directly into its namespace but via an
 * external wrapper process.
 *
 * The panel locates available applets by searching for applet desktop
 * files in (ALL_KDEDIRS)/share/apps/kicker/applets. Every panel applet should
 * install a desktop file there to be recognised by the panel.
 *
 * Besides standard keys like "Name", "Comment" and "Icon" there are
 * two panel applet specific keys:
 *
 * @sect X-KDE-Library
 *
 * Used by the panel to locate the applet DSO (Dynamic Shared Object)
 * Example: X-KDE-Library=libexampleapplet
 *
 * @sect X-KDE-UniqueApplet
 *
 * Similar to @ref KApplication and @ref KUniqueApplication there are
 * two types of panel applets. Use unique applets when it makes no
 * sence to run more than one instance of a applet in the panel. A
 * good example for unique applets is the taskbar applet.  Use normal
 * applets when you need instance specific configuration. An example
 * is the koolclock applet where you might want to run two instances
 * in your panel, one configured as analog clock, the other one as
 * digital clock. X-KDE-UniqueApplet is a boolean key which defaults
 * to "false".  Example: X-KDE-UniqueApplet=true
 *
 * Back to panel applet DSOs, the following conventions are used for KDE:
 * Name:    lib<appletname>applet.la
 * LDFLAGS: -module -no-undefined
 *
 * To implement a panel applet it is not enough to write a class
 * inheriting from KPanelApplet but you also have to provide a
 * factory function in your DSO.  A sample factory function could look
 * like this:
 *
 * <pre>
 *
 * extern "C"
 * {
 *     KPanelApplet* init(QWidget *parent, const QString& configFile)
 *     {
 *         KGlobal::locale()->insertCatalogue("exampleapplet");
 *         return new ExampleApplet(configFile, KPanelApplet::Normal,
 *                       KPanelApplet::About | KPanelApplet::Help | KPanelApplet::Preferences,
 *                       parent, "exampleapplet");
 *     }
 * }
 *
 * </pre>
 *
 * Note: Don't change the factory function signature or the panel will
 * fail to load your applet.
 *
 * @author Matthias Elter <elter@kde.org>
 * @short KDE Panel Applet class
 **/
class KPanelApplet : public QFrame
{
    Q_OBJECT

public:

    enum Type { Normal = 0, Stretch };
    enum Action { About = 1, Help = 2, Preferences = 4, ReportBug = 8 };
    enum Direction { Up = 0, Down, Left, Right };

    /**
     * Constructs a KPanelApplet just like any other widget.
     *
     * @param configFile The configFile handed over in the factory function.
     * @param Type The applet @ref type().
     * @param actions Standard RMB menu actions supported by the applet (see @ref action() ).
     * @param parent The pointer to the parent widget handed over in the factory function.
     * @param name A Qt object name for your applet.
     **/
    KPanelApplet(const QString& configFile, Type t = Normal,
                 int actions = 0, QWidget *parent = 0, const char *name = 0,
                 WFlags f = 0);

    /**
     * Destructor.
     **/
    ~KPanelApplet();

    /**
     * Retrieve a suggested width for a given height.
     *
     * Every applet should reimplement this function.
     *
     * Depending on the panel orientation the height (horizontal panel) or the
     * width (vertical panel) of the applets is fixed.
     * The exact values of the fixed size component depend on the panel size.
     *
     * On a horizontal panel the applet height is fixed, the panel will
     * call @ref widthForHeight(int height) with @p height
     * equal to 'the fixed applet height'
     * when laying out the applets.
     *
     * The applet can now choose the other size component (width)
     * based on the given height.
     *
     * The width you return is granted.
     **/
    virtual int widthForHeight(int height) const { return height; }

    /**
     * @return A suggested height for a given width.
     *
     * Every applet should reimplement this function.
     *
     * Depending on the panel orientation the height (horizontal panel) or the
     * width (vertical panel) of the applets is fixed.
     * The exact values of the fixed size component depend on the panel size.
     *
     * On a vertical panel the applet width is fixed, the panel will
     * call @ref heightForWidth(int width) with @p width
     * equal to 'the fixed applet width'
     * when layint out the applets.
     *
     * The applet can now choose the other size component (height)
     * based on the given width.
     *
     * The height you return is granted.
     **/
    virtual int heightForWidth(int width) const { return width; }

    /**
     * Always use this @ref KConfig object to save/load your applets configuration.
     *
     * For unique applets this config object will write to a config file called
     * <appletname>rc in the users local KDE directory.
     *
     * For normal applets this config object will write to a instance specific config file
     * called <appletname><instanceid>rc in the users local KDE directory.
     **/
    KConfig* config() const { return _config; }

    /**
     * @return Type indicating the applet's type.
     * @ref Type
     **/
    Type type() const { return _type; }

    /**
     * @return int indicating the supported RMB menu actions.
     * @ref Action
     **/
    int actions() const { return _actions; }

    /**
     * Generic action dispatcher. Called  when the user selects an item
     * from the applets RMB menu.
     *
     * Reimplement this function to handle actions.
     *
     * For About, Help, Preferences and ReportBug, use the convenience handlers
     * ref about(), help(), preferences(), reportBug()
     *
     **/
    virtual void action( Action a );

signals:
    /**
     * Emit this signal to make the panel relayout all applets, when
     * you want to change your width (horizontal panel) or
     * height (vertical panel).
     *
     * The panel is going to relayout all applets based on their
     * widthForHeight(int height) (horizontal panel) or
     * heightForWidth(int width) (vertical panel).
     *
     * Please note that the panel may change the applets location
     * if the new widthForHeight(int height) (horizontal panel) or
     * heightForWidth(int width) (vertical panel) does not fit into the
     * current panel layout.
     **/
    void updateLayout();

    /**
     * Request keyboard focus from the panel.
     **/
    void requestFocus();


public slots:
    /**
     * Don't reimplement, this is used internally
     **/
    void slotSetOrientation(Orientation o);

    /**
     * Don't reimplement, this is used internally
     **/
    void slotSetPopupDirection(Direction d);

protected:

    /**
     * Is called when the user selects "About" from the applet's RMB menu.
     * Reimplement this function to launch a about dialog.
     *
     * Note that this is called only when your applet supports the About action.
     * See @ref Action and KPanelApplet().
     **/
    virtual void about() {}

    /**
     * Is called when the user selects "Help" from the applet's RMB menu.
     * Reimplement this function to launch a manual or help page.
     *
     * Note that this is called only when your applet supports the Help action.
     * See @ref Action and KPanelApplet().
     **/
    virtual void help() {}

    /**
     * Is called when the user selects "Preferences" from the applet's RMB menu.
     * Reimplement this function to launch a preferences dialog or kcontrol module.
     *
     * Note that this is called only when your applet supports the preferences action.
     * See @ref Action and KPanelApplet().
     **/
    virtual void preferences() {}
 
    /**
     * Is called when the user selects "Report bug" from the applet's RMB menu.
     * Reimplement this function to launch a bug reporting dialog.
     *
     * Note that this is called only when your applet supports the ReportBug
     * action.
     * See @ref Action and KPanelApplet()
     **/
   virtual void reportBug() {}

    /**
     * @return the applet's orientation. (horizontal or vertical)
     **/
    Orientation orientation() const { return _orient; }


    /**
     * The orientation changed to @p orientation. Reimplement this
     * change handler in order to adjust the look of your applet.
     **/
    virtual void orientationChange( Orientation /* orientation*/) {}

    /**
     * You may need this if you want to popup menus at the right position.
     *
     * See @ref  popupDirectionChange()
     **/
    Direction popupDirection() const { return _dir; }


    /**
     * The popup direction changed to @p direction. Reimplement this
     * change handler in order to adjust the look of your applet.
     **/
    virtual void popupDirectionChange( Direction /*direction*/ ) {}



private:
    Type         _type;
    Orientation  _orient;
    Direction    _dir;
    KConfig*     _config;
    int          _actions;
    class Private;
    Private *d;
};

#endif
