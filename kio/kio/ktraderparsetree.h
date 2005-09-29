/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __parse_tree_h__
#define __parse_tree_h__

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qshared.h>

#include <kservice.h>
#include <kuserprofile.h>

#include "ktrader.h"

namespace KIO {

class ParseTreeBase;

/** \internal */
struct KIO_EXPORT PreferencesReturn
{
  enum Type { PRT_DOUBLE, PRT_ERROR };

  PreferencesReturn() { type = PRT_ERROR; }

  PreferencesReturn( const PreferencesReturn& _r )
  {
    type = _r.type;
    f = _r.f;
  }

  Type type;
  double f;
};


/**
 * @internal
 * @return 0  => Does not match
 *         1  => Does match
 *         <0 => Error
 */
KIO_EXPORT int matchConstraint( const ParseTreeBase *_tree, const KService::Ptr &,
		     const KServiceTypeProfile::OfferList& );

/**
 * @internal
 * @return 1 on success or <0 on Error
 */
KIO_EXPORT PreferencesReturn matchPreferences( const ParseTreeBase *_tree, const KService::Ptr &,
				    const KServiceTypeProfile::OfferList& );

/**
 * @internal
 */
struct KIO_EXPORT PreferencesMaxima
{
  enum Type { PM_ERROR, PM_INVALID_INT, PM_INVALID_DOUBLE, PM_DOUBLE, PM_INT };

  Type type;
  int iMax;
  int iMin;
  double fMax;
  double fMin;
};

/**
 * @internal
 */
class KIO_EXPORT ParseContext
{
public:
  /**
   * This is NOT a copy constructor.
   */
  ParseContext( const ParseContext* _ctx ) : service( _ctx->service ), maxima( _ctx->maxima ),
    offers( _ctx->offers ) {}
  ParseContext( const KService::Ptr & _service, const KServiceTypeProfile::OfferList& _offers,
		QMap<QString,PreferencesMaxima>& _m )
    : service( _service ), maxima( _m ), offers( _offers ) {}

  bool initMaxima( const QString& _prop);

  enum Type { T_STRING = 1, T_DOUBLE = 2, T_NUM = 3, T_BOOL = 4,
	      T_STR_SEQ = 5, T_SEQ = 6 };

  QString str;
  int i;
  double f;
  bool b;
  QValueList<QVariant> seq;
  QStringList strSeq;
  Type type;

  KService::Ptr service;

  QMap<QString,PreferencesMaxima>& maxima;
  const KServiceTypeProfile::OfferList& offers;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeBase : public KShared
{
public:
  typedef KSharedPtr<ParseTreeBase> Ptr;
  ParseTreeBase() { }

  virtual bool eval( ParseContext *_context ) const = 0;
protected:
  virtual ~ParseTreeBase() { };
};

KIO_EXPORT ParseTreeBase::Ptr parseConstraints( const QString& _constr );
KIO_EXPORT ParseTreeBase::Ptr parsePreferences( const QString& _prefs );

/**
 * @internal
 */
class KIO_EXPORT ParseTreeOR : public ParseTreeBase
{
public:
  ParseTreeOR( ParseTreeBase *_ptr1, ParseTreeBase *_ptr2 ) { m_pLeft = _ptr1; m_pRight = _ptr2; }

  bool eval( ParseContext *_context ) const;

protected:
  ParseTreeBase::Ptr m_pLeft;
  ParseTreeBase::Ptr m_pRight;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeAND : public ParseTreeBase
{
public:
  ParseTreeAND( ParseTreeBase *_ptr1, ParseTreeBase *_ptr2 ) { m_pLeft = _ptr1; m_pRight = _ptr2; }

  bool eval( ParseContext *_context ) const;

protected:
  ParseTreeBase::Ptr m_pLeft;
  ParseTreeBase::Ptr m_pRight;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeCMP : public ParseTreeBase
{
public:
  ParseTreeCMP( ParseTreeBase *_ptr1, ParseTreeBase *_ptr2, int _i ) { m_pLeft = _ptr1; m_pRight = _ptr2; m_cmd = _i; }

  bool eval( ParseContext *_context ) const;

protected:
  ParseTreeBase::Ptr m_pLeft;
  ParseTreeBase::Ptr m_pRight;
  int m_cmd;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeIN : public ParseTreeBase
{
public:
  ParseTreeIN( ParseTreeBase *_ptr1, ParseTreeBase *_ptr2 ) { m_pLeft = _ptr1; m_pRight = _ptr2; }

  bool eval( ParseContext *_context ) const;

protected:
  ParseTreeBase::Ptr m_pLeft;
  ParseTreeBase::Ptr m_pRight;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeMATCH : public ParseTreeBase
{
public:
  ParseTreeMATCH( ParseTreeBase *_ptr1, ParseTreeBase *_ptr2 ) { m_pLeft = _ptr1; m_pRight = _ptr2; }

  bool eval( ParseContext *_context ) const;

protected:
  ParseTreeBase::Ptr m_pLeft;
  ParseTreeBase::Ptr m_pRight;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeCALC : public ParseTreeBase
{
public:
  ParseTreeCALC( ParseTreeBase *_ptr1, ParseTreeBase *_ptr2, int _i ) { m_pLeft = _ptr1; m_pRight = _ptr2; m_cmd = _i; }

  bool eval( ParseContext *_context ) const;

protected:
  ParseTreeBase::Ptr m_pLeft;
  ParseTreeBase::Ptr m_pRight;
  int m_cmd;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeBRACKETS : public ParseTreeBase
{
public:
  ParseTreeBRACKETS( ParseTreeBase *_ptr ) { m_pLeft = _ptr; }

  bool eval( ParseContext *_context ) const { return m_pLeft->eval( _context ); }

protected:
  ParseTreeBase::Ptr m_pLeft;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeNOT : public ParseTreeBase
{
public:
  ParseTreeNOT( ParseTreeBase *_ptr ) { m_pLeft = _ptr; }

  bool eval( ParseContext *_context ) const;

protected:
  ParseTreeBase::Ptr m_pLeft;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeEXIST : public ParseTreeBase
{
public:
  ParseTreeEXIST( const char *_id ) { m_id = _id; }

  bool eval( ParseContext *_context ) const;

protected:
  QString m_id;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeID : public ParseTreeBase
{
public:
  ParseTreeID( const char *arg ) { m_str = arg; }

  bool eval( ParseContext *_context ) const;

protected:
  QString m_str;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeSTRING : public ParseTreeBase
{
public:
  ParseTreeSTRING( const char *arg ) { m_str = arg; }

  bool eval( ParseContext *_context ) const { _context->type = ParseContext::T_STRING; _context->str = m_str; return true; }

protected:
  QString m_str;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeNUM : public ParseTreeBase
{
public:
  ParseTreeNUM( int arg ) { m_int = arg; }

  bool eval( ParseContext *_context ) const { _context->type = ParseContext::T_NUM; _context->i = m_int; return true; }

protected:
  int m_int;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeDOUBLE : public ParseTreeBase
{
public:
  ParseTreeDOUBLE( double arg ) { m_double = arg; }

  bool eval( ParseContext *_context ) const { _context->type = ParseContext::T_DOUBLE; _context->f = m_double; return true; }

protected:
  double m_double;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeBOOL : public ParseTreeBase
{
public:
  ParseTreeBOOL( bool arg ) { m_bool = arg; }

  bool eval( ParseContext *_context ) const { _context->type = ParseContext::T_BOOL; _context->b = m_bool; return true; }

protected:
  bool m_bool;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeMAX2 : public ParseTreeBase
{
public:
  ParseTreeMAX2( const char *_id ) { m_strId = _id; }

  bool eval( ParseContext *_context ) const;

protected:
  QString m_strId;
};

/**
 * @internal
 */
class KIO_EXPORT ParseTreeMIN2 : public ParseTreeBase
{
public:
  ParseTreeMIN2( const char *_id ) { m_strId = _id; }

  bool eval( ParseContext *_context ) const;

protected:
  QString m_strId;
};

}

#endif
