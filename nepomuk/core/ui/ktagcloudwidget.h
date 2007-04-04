/* This file is part of the KDE libraries
   Copyright (C) 2007 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KTAGCLOUDWIDGET_H
#define KTAGCLOUDWIDGET_H

#include <QWidget>
#include <QMap>

#include <kmetadata/kmetadata_export.h>

class KMETADATA_EXPORT KTagCloudWidget : public QWidget
{
    Q_OBJECT

 public:
    KTagCloudWidget( QWidget* parent = 0 );
    ~KTagCloudWidget();

    /**
     * Retrieve the weight for a certain tag
     */
    int tagWeight( const QString& tag ) const;

 public Q_SLOTS:
    /**
     * Add \a tag to the cloud using the weight factor \a weight. 
     * Existing tags with the same name will be overwritten.
     */
    void addTag( const QString& tag, int weight );

    /**
     * Add a set of tags with weight factors
     */
    void addTags( const QMap<QString,int>& tags );

    /**
     * Remove all tags from the cloud.
     */
    void clear();

    /**
     * Set the maximum font size to be used for the most popular tags.
     * Default is 22.
     */
    void setMaxFontSize( int pointSize );

    /**
     * Set the minimum font size to be used for the most unpopular tags.
     * Default is 8.
     */
    void setMinFontSize( int pointSize );
  
 signals:
    void tagClicked( const QString& tag );

 protected:
    void resizeEvent( QResizeEvent* e );

 private:
    void buildTagCloud();

    class Private;
    Private* d;
};

#endif
