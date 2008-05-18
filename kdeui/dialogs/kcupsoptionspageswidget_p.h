/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2008 John Layt <john@layt.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KCUPSOPTIONSPAGESWIDGET_H
#define KCUPSOPTIONSPAGESWIDGET_H

#include <qwidget.h>

#include "kcupsoptionswidget_p.h"
#include "ui_kcupsoptionspageswidget.h"

/**
 * A widget to add to QPrintDialog to enable extra CUPS options
 * Private, not part of KDE API, not to be accessed by apps
 * Use KDEPrintDialog to create a dialog with widget included.
 *
 * Note that the Cups Options cannot be updated until the dialog is
 * closed otherwise they get overwritten.
 */

class KCupsOptionsPagesWidget : public KCupsOptionsWidget
{
    Q_OBJECT

public:

    /**
     * Enum for valid number of pages per sheet
     */
    enum PagesPerSheet
    {
        OnePagePerSheet = 0,  //CUPS Default
        TwoPagesPerSheet,
        FourPagesPerSheet,
        SixPagesPerSheet,
        NinePagesPerSheet,
        SixteenPagesPerSheet
    };

    /**
     * Enum for valid layouts of pages per sheet
     */
    enum PagesPerSheetLayout
    {
        LeftToRightTopToBottom = 0,  //CUPS Default 'lrtb'
        LeftToRightBottomToTop,
        RightToLeftTopToBottom,
        RightToLeftBottomToTop,
        BottomToTopLeftToRight,
        BottomToTopRightToLeft,
        TopToBottomLeftToRight,
        TopToBottomRightToLeft
    };

    /**
     * Enum for valid border styles for pages
     */
    enum PageBorder
    {
        NoBorder = 0,  //CUPS Default 'none'
        SingleLine,
        SingleThickLine,
        DoubleLine,
        DoubleThickLine
    };

    /**
     * Enum for valid banner pages
     */
    enum BannerPage
    {
        NoBanner = 0,  //CUPS Default 'none'
        Standard,
        Unclassified,
        Confidential,
        Classified,
        Secret,
        TopSecret
    };

    /**
     * Create a widget for extra CUPS options
     */
    explicit KCupsOptionsPagesWidget( QPrintDialog *parent );

    /**
     * Destructor
     */
    ~KCupsOptionsPagesWidget();

    /**
     * Set the number of pages per sheet
     */
    void setPagesPerSheet( PagesPerSheet pagesPerSheet = OnePagePerSheet  );

    /**
     * Returns the selected number of pages per sheet
     */
    PagesPerSheet pagesPerSheet() const;

    /**
     * Set the layout of pages per sheet
     */
    void setPagesPerSheetLayout( PagesPerSheetLayout pagesPerSheetLayout = LeftToRightTopToBottom  );

    /**
     * Returns the selected layout of pages per sheet
     */
    PagesPerSheetLayout pagesPerSheetLayout() const;

    /**
     * Returns the CUPS value of the selected layout of pages per sheet
     */
    QString pagesPerSheetLayoutCups() const;

    /**
     * Set the border style of pages
     */
    void setPageBorder( PageBorder pageBorder = NoBorder  );

    /**
     * Returns the selected page border style
     */
    PageBorder pageBorder() const;

    /**
     * Set the start banner
     */
    void setStartBannerPage( BannerPage bannerPage = NoBanner  );

    /**
     * Returns the selected start banner
     */
    BannerPage startBannerPage() const;

    /**
     * Returns the CUPS value of the selected start banner
     */
    QString startBannerPageCups() const;

    /**
     * Set the end banner
     */
    void setEndBannerPage( BannerPage bannerPage = NoBanner  );

    /**
     * Returns the selected end banner
     */
    BannerPage endBannerPage() const;

    /**
     * Returns the CUPS value of the selected end banner
     */
    QString endBannerPageCups() const;

    /**
     * Set Page Label text
     */
    void setPageLabel( QString label = ""  );

    /**
     * Returns Page Label text
     */
    QString pageLabel() const;

    /**
     * Set mirror pages selected
     */
    void setMirrorPages( bool mirror = false  );

    /**
     * Returns if mirror pages selected
     */
    bool mirrorPages() const;

protected:

    void setupCupsOptions( QStringList &cupsOptions );

private:

    void initPagesPerSheet();
    void initPagesPerSheetLayout();
    void initPageBorder();
    void initStartBannerPage();
    void initEndBannerPage();
    void initPageLabel();
    void initMirrorPages();

    Ui_KCupsOptionsPagesWidget ui;
};

#endif  // KCUPSOPTIONSPAGESWIDGET_H
