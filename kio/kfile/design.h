/* This file is part of the KDE project

   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/**
 * @libdoc KDE Filedialog widget and associated classes.
 *
 * This library provides the KDE file selector widget, its building blocks and
 * some other widgets, making use of the file dialog. 
 *
 * The file dialog provides different views; there is a vertically scrolling 
 * view based on @ref KListView, showing things like filename, file size, 
 * permissions etc. in separate columns. And there is a horizontally scrolling
 * view based on @ref KIconView. Additionally, there are some compound views, 
 * like a view using the icon-view for files on the right side and another 
 * view for directories on the left. A view, that shows a preview for the 
 * currently selected file (using @ref KIO::PreviewJob to generate previews)
 * and any other view to show the files is also available.
 *
 * All those views share a common baseclass, named @ref KFileView, which 
 * defines the interface for inserting files into a view, removing them, 
 * selecting etc.
 *
 * The one class encapsulating all those views and adding browsing capabilities
 * to them is @ref KDirOperator. It allows the user to switch between different
 * views.
 *
 * @ref KFileTreeView is a KListView based widget that displays files and/or 
 * directories as a tree. It does not implement the KFileView interface, 
 * however, so it can't be used with KDirOperator.
 *
 * Besides the filebrowsing widgets, there is the @ref KPropertiesDialog class,
 * implementing a dialog showing the name, permissions, icons, meta 
 * information and all kinds of properties of a file, as well as providing a 
 * means to modify them.
 * 
 * The KPropertiesDialog is extensible with plugin-pages via the 
 * @ref KPropsDlgPlugin class.
 *
 * The @ref KIconDialog class shows a list of icons installed on the system (as
 * accessible via @ref KIconLoader) and allows the user to select one.
 *
 * @ref KOpenWithDlg implements a dialog to choose an application from, that is
 * to be run, e.g. to let the user choose an application to open a file/url 
 * with.
 *
 * 
 * @ref KFileDialog :
 *   The class providing the file selector dialog. It combines a KDirOperator,
 *   KURLBar and several other widgets.
 *
 * @ref KDirOperator :
 *   The class encapsulating different KFileViews, offering file browsing and
 *   file selection. Asynchronous, network transparent reading of directories
 *   is performed via the @ref KIO library.
 *
 * @ref KURLRequester :
 *   A widget to be used for asking for a filename/path/URL. It consists of a 
 *   @ref KLineEdit (or @ref KComboBox) and a button. Activating the button
 *   will open a @ref KFileDialog. Typing in the lineedit is aided with 
 *   filename completion.
 *
 * @ref KURLRequesterDlg  :
 *   A dialog to ask for a filename/path/URL, using KURLRequester.
 *
 * @ref KFileView  :
 *   The base class for all views to be used with KDirOperator.
 *
 * @ref KFileIconView :
 *   The KFileView based on KIconView.
 *
 * @ref KFileDetailView :
 *   The KFileView based on KListView.
 *
 * @ref KFilePreview :
 *   The KFileView, combining a widget showing preview for a selected file
 *   and another KFileView for browsing.
 *
 * @ref KURLBar :
 *   A widget offering a number of clickable entries which represent a URL,
 *   aligned horizontally or vertically. The entries are customizable by the 
 *   user both on a per application basis or for all applications (URLs, their
 *   icon and the description can be added, removed or edited by the user).
 *   This is the widget used as "sidebar" in the @ref KFileDialog.
 *
 * @ref KFileMetaInfoWidget :
 *   A widget that allows viewing and editing of meta data of a file, utilizing
 *   @ref KFileMetaInfo.
 *
 */
