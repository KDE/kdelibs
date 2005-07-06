/* This file is part of the KDE libraries
   Copyright (C) 2001,2005 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __ktexteditor_codecompletioninterface_h__
#define __ktexteditor_codecompletioninterface_h__

#include <qstring.h>
#include <qstringlist.h>
#include <Q3ValueList>
#include <q3cstring.h>
#include <kdelibs_export.h>
#include <QVariant>
#include <QIcon>

namespace KTextEditor
{

class View;
class Cursor;

/**
 * An item for the completion popup. <code>text</code> is the completed string,
 * <code>prefix</code> appears in front of it, <code>suffix</code> appears after it.
 * <code>type</code> does not appear in the completion list.
 * <code>prefix</code>, <code>suffix</code>, and <code>type</code> are not part of the
 * inserted text if a completion takes place. <code>comment</code> appears in a tooltip right of
 * the completion list for the currently selected item. <code>userdata</code> can be
 * free formed data, which the user of this interface can use in
 * CodeCompletionInterface::filterInsertString().
 *
 *
 */

class KTEXTEDITOR_EXPORT CompletionEntry
{
  public:
    QIcon   icon;
    QString type;
    QString text;
    QString prefix;
    QString postfix;
    QString comment;

    QVariant userdata;

    bool operator==( const CompletionEntry &c ) const {
      return ( c.type == type &&
	       c.text == text &&
	       c.postfix == postfix &&
	       c.prefix == prefix &&
	       c.comment == comment &&
               c.userdata == userdata &&
               c.icon.serialNumber()==icon.serialNumber());
    }
};


class CompletionProvider;

class KTEXTEDITOR_EXPORT CompletionItem
{
  public:
    QIcon icon;
    QString type;
    QString text;
    QString prefix;
    QString postfix;
    QString comment;

    QVariant userdata;
    CompletionProvider *provider; //only needs to be set if userdata should be handled

    bool operator==( const CompletionItem &c ) const {
      return ( c.type == type &&
	       c.text == text &&
	       c.postfix == postfix &&
	       c.prefix == prefix &&
	       c.comment == comment &&
               c.userdata == userdata &&
               c.provider==provider &&
               c.icon.serialNumber()==icon.serialNumber());

    }
};




class KTEXTEDITOR_EXPORT CompletionData {
  public:
     CompletionData():m_id(0) {} //You should never use that yourself
     CompletionData(QList<CompletionItem> items,int offset,bool casesensitive):
       m_items(items),m_offset(offset),m_casesensitive(casesensitive),m_id(((++s_id)==0)?(++s_id):s_id){ }
     inline const QList<CompletionItem>& items()const {return m_items;}
     inline int offset() const {return m_offset;}
     inline bool casesensitive() const {return m_casesensitive;}
     inline bool operator==( const CompletionData &d ) const { return m_id==d.m_id;}
     inline static const CompletionData Null() {return CompletionData();}
  private:
     QList<CompletionItem> m_items;
     int m_offset;
     bool m_casesensitive;
     long m_id;
     static long s_id;
  };


class KTEXTEDITOR_EXPORT ArgHintData {
  public:
    ArgHintData():m_id(0) {} //You should never use that yourself
    ArgHintData(const QString& wrapping, const QString& delimiter, const QStringList& items) :
        m_wrapping(wrapping),m_delimiter(delimiter),m_items(items),m_id(((++s_id)==0)?(++s_id):s_id) {}
    inline const QString& wrapping() const {return m_wrapping;}
    inline const QString& delimiter() const {return m_delimiter;}
    inline const QStringList& items() const {return m_items;}
    inline bool operator==( const ArgHintData &d ) const { return m_id==d.m_id;}
    inline static const ArgHintData Null() {return ArgHintData();}
  private:
    QString m_wrapping;
    QString m_delimiter;
    QStringList m_items;
    long m_id;
    static long s_id;
};

    enum CompletionType
    {
      CompletionType0 =0x00000000,
      CompletionType1 =0x00000001,
      CompletionType2 =0x00000002,
      CompletionType3 =0x00000004,
      CompletionType4 =0x00000008,
      CompletionType5 =0x00000010,
      CompletionType6 =0x00000020,
      CompletionType7 =0x00000040,
      CompletionType8 =0x00000080,
      CompletionType9=0x00000100,
      CompletionType10=0x00000200,
      CompletionType11=0x00000400,
      CompletionType12=0x00000800,
      CompletionType13=0x00001000,
      CompletionType14=0x00002000,
      CompletionType15=0x00004000,
      CompletionType16=0x00008000,
      CompletionType17=0x00010000,
      CompletionType18=0x00020000,
      CompletionType19=0x00040000,
      CompletionType20=0x00080000,
      CompletionType21=0x00100000,
      CompletionType22=0x00200000,
      CompletionType23=0x00400000,
      CompletionType24=0x00800000,
      CompletionType25=0x01000000,
      CompletionType26=0x02000000,
      CompletionType27=0x04000000,
      CompletionType28=0x08000000,
      CompletionType29=0x10000000,
      CompletionType30=0x20000000,
      CompletionType31=0x40000000,
      CompletionType32=0x80000000,

      CompletionNone=CompletionType0,
      CompletionAsYouType=CompletionType1,
      CompletionReinvokeAsYouType=CompletionType2,
      CompletionContextIndependent=CompletionType3,
      CompletionContextDependent=CompletionType4
    };

class KTEXTEDITOR_EXPORT CompletionProvider
{
  public:
    virtual ~CompletionProvider(){;}
  public:

    
    virtual const CompletionData completionData(View*,enum CompletionType, const Cursor&, const QString&)=0;
    virtual const ArgHintData argHintData(View *,const Cursor&, const QString&)=0;
    /*this function is called if a valid userdata is set for the chosen completion item*/
    virtual void filterInsertString(View*,const CompletionItem&,QString*)=0;

};



/**
 * This is an interface for the KTextEditor::View class. It can be used
 * to show completion lists, i.e. lists that pop up while a user is typing.
 * The user can then choose from the list or he can just keep typing. The
 * completion list will disappear if an item is chosen, if no completion
 * is available or if the user presses Esc etc. The contents of the list
 * is automatically adapted to the string the user types.
 *
 * There are other signals, which may be implmemented, but aren't documented here, because
 * it would have been a BIC change...:
 *
 * void completionExtendedComment(CompletionEntry)
 *
 * This is emitted when the user has completed the argument entry (ie. enters the wrapping symbol(s)
 * void argHintCompleted()
 *
 * This is emitted when there is a reason other than completion for the hint being hidden.
 * void argHintAborted()
 *
 * This is emitted when a code completion box is about to be displayed
 * void aboutToShowCompletionBox()
 *
 */
class KTEXTEDITOR_EXPORT CodeCompletionInterface
{
  public:
  	virtual ~CodeCompletionInterface() {}

        virtual bool registerCompletionProvider(CompletionProvider*)=0;
        virtual bool unregisterCompletionProvider(CompletionProvider*)=0;

};

KTEXTEDITOR_EXPORT CodeCompletionInterface *codeCompletionInterface (class View *view);

}

#endif
