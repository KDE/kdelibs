/*
	A dummy source file for documenting the library.
	Sirtaj Singh Kang <taj@kde.org>
	$Id$
	Distributed under the LGPL.
*/

/**
* @libdoc The KDE User Interface library
*
* This library provides standard user interface elements for use in
* KDE applications. If your KDE application has a GUI, you will almost
* certainly link to libkdeui.
*
* Most applications with single or multiple toplevel widgets
* should use the @ref KMainWindow class in this library, which
* automatically provides features like session management and simplified
* toolbar/menubar/statusbar creation.
*
* A spell-checker library is also provided.
* @ref KSpell offers easy access to International ISpell or ASpell
*  (at the user's option) as well as a spell-checker GUI
*  ("Add", "Replace", etc.).
*
* You can use @ref KSpell to
*  automatically spell-check an ASCII file as well as to implement
*  online spell-checking and to spell-check proprietary format and
*  marked up (e.g. HTML, TeX) documents.  The relevant methods for
*  these three procedures are @ref check(), @ref checkWord(), and
*  @ref checkList(), respectively.
*
* @ref KSpellConfig holds configuration information about @ref KSpell as well
*  as acting as an options-setting dialog.
*
* KSpell usually works asynchronously. If you do not need that, you should
* simply use @ref KSpell::modalCheck(). It won't return until the
* passed string is processed or the spell checking canceled.
* During modal spell checking your GUI is still repainted, but the user may
* only interact with the @ref KSpell dialog.
*
* @see KSpell, KSpellConfig
*/

