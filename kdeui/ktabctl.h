/* This file is part of the KDE libraries
    Copyright (C) 1997 Alexander Sanda (alex@darkstar.ping.at)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#ifndef KTABCTL_H
#define KTABCTL_H

#include <QtCore/QVector>
#include <QtGui/QWidget>
#include <QtGui/QTabBar>

#include <kdelibs_export.h>

/**
 * Tabbed dialog with extended features.
 * KTabCtl is very similar to QTabDialog, with the following differences:
 *
 * @li To avoid confusion, the API is almost identical with QTabDialog.
 * @li Does not create any buttons, therefore KTabCtl is not limited
 * to dialog boxes. You can use it whereever you want.
 * @li emits the signal tabSelected(int pagenumber) when the user
 * selects one of the tabs. This gives you the chance to update the
 * widget contents of a single page. The signal is emitted _before_ the
 * page is shown.  This is very useful if the contents of some widgets
 * on page A depend on the contents of some other widgets on page B.
 *
 * @author Alexander Sanda (alex@darkstar.ping.at)
*/
class KDEUI_EXPORT KTabCtl : public QWidget
{
  Q_OBJECT

  public:
    /**
     * Creates a new tab bar widget.
     *
     * @param parent The parent widget.
     */
    explicit KTabCtl( QWidget *parent = 0 );

    /**
     * Destroys the tab widget.
     */
    ~KTabCtl();


    /**
     * Sets the @param font for the whole widget.
     */
    void setFont( const QFont &font );

    /**
     * Sets the @param font for the tab widgets.
     */
    void setTabFont( const QFont &font );

    /**
     * Adds the @param widget as new tab with the given
     * @param name.
     */
    void addTab( QWidget *widget, const QString &name );

    /**
     * Returns whether the tab with the given @param name
     * is enabled.
     */
    bool isTabEnabled( const QString &name );

    /**
     * Sets the tab with the given @param name enabled or disabled.
     */
    void setTabEnabled( const QString &name, bool enabled );

    /**
     * Sets whether the tab widget should be enclosed by
     * a border.
     */
    void showBorder( bool border );

    /**
     * Sets the shape of the tab widget.
     */
    void setShape( QTabBar::Shape shape );

    /**
     * Reimplemented from @see QWidget.
     */
    void show();

    /**
     * Reimplemented from @see QWidget.
     */
    virtual QSize sizeHint() const;

  protected:
    void paintEvent( QPaintEvent* );
    void resizeEvent( QResizeEvent* );

  Q_SIGNALS:
    void tabSelected( int index );

  private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void showTab( int ) )
};
#endif
