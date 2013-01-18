/* This file is part of the KDE libraries
    Copyright (c) 2002-2003 KDE Team

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

/**
 * @file kdemacros.h
 *
 * This header defines several compiler-independent macros which are used
 * throughout KDE. Most of these macros make use of GCC extensions; on other
 * compilers, they don't have any effect.
 */

#ifndef _KDE_MACROS_H_
#define _KDE_MACROS_H_

#cmakedefine __KDE_HAVE_GCC_VISIBILITY

/**
 * @def KDE_NO_EXPORT
 * @ingroup KDEMacros
 *
 * The KDE_NO_EXPORT macro marks the symbol of the given variable
 * to be hidden. A hidden symbol is stripped during the linking step,
 * so it can't be used from outside the resulting library, which is similar
 * to static. However, static limits the visibility to the current
 * compilation unit. Hidden symbols can still be used in multiple compilation
 * units.
 *
 * \code
 * int KDE_NO_EXPORT foo;
 * int KDE_EXPORT bar;
 * \endcode
 *
 * @sa KDE_EXPORT
 */

/**
 * @def KDE_EXPORT
 * @ingroup KDEMacros
 *
 * The KDE_EXPORT macro marks the symbol of the given variable
 * to be visible, so it can be used from outside the resulting library.
 *
 * \code
 * int KDE_NO_EXPORT foo;
 * int KDE_EXPORT bar;
 * \endcode
 *
 * @sa KDE_NO_EXPORT
 */

/**
 * @def KDE_IMPORT
 * @ingroup KDEMacros
 */

#ifdef __KDE_HAVE_GCC_VISIBILITY
#define KDE_NO_EXPORT __attribute__ ((visibility("hidden")))
#define KDE_EXPORT __attribute__ ((visibility("default")))
#define KDE_IMPORT __attribute__ ((visibility("default")))
#elif defined(_WIN32) || defined(_WIN64)
#define KDE_NO_EXPORT
#define KDE_EXPORT __declspec(dllexport)
#define KDE_IMPORT __declspec(dllimport)
#else
#define KDE_NO_EXPORT
#define KDE_EXPORT
#define KDE_IMPORT
#endif

/**
 * @def KDE_PACKED
 * @ingroup KDEMacros
 *
 * The KDE_PACKED macro can be used to hint the compiler that a particular
 * structure or class should not contain unnecessary paddings.
 */

#ifdef __GNUC__
#define KDE_PACKED __attribute__((__packed__))
#else
#define KDE_PACKED
#endif

/**
 * @def KDE_DEPRECATED
 * @ingroup KDEMacros
 *
 * The KDE_DEPRECATED macro can be used to trigger compile-time warnings
 * with newer compilers when deprecated functions are used.
 *
 * For non-inline functions, the macro gets inserted at front of the
 * function declaration, right before the return type:
 *
 * \code
 * KDE_DEPRECATED void deprecatedFunctionA();
 * KDE_DEPRECATED int deprecatedFunctionB() const;
 * \endcode
 *
 * For functions which are implemented inline,
 * the KDE_DEPRECATED macro is inserted at the front, right before the return
 * type, but after "static", "inline" or "virtual":
 *
 * \code
 * KDE_DEPRECATED void deprecatedInlineFunctionA() { .. }
 * virtual KDE_DEPRECATED int deprecatedInlineFunctionB() { .. }
 * static KDE_DEPRECATED bool deprecatedInlineFunctionC() { .. }
 * inline KDE_DEPRECATED bool deprecatedInlineFunctionD() { .. }
 * \endcode
 *
 * You can also mark whole structs or classes as deprecated, by inserting the
 * KDE_DEPRECATED macro after the struct/class keyword, but before the
 * name of the struct/class:
 *
 * \code
 * class KDE_DEPRECATED DeprecatedClass { };
 * struct KDE_DEPRECATED DeprecatedStruct { };
 * \endcode
 *
 * \note
 * It does not make much sense to use the KDE_DEPRECATED keyword for a Qt signal;
 * this is because usually get called by the class which they belong to,
 * and one would assume that a class author does not use deprecated methods of
 * his own class. The only exception to this are signals which are connected to
 * other signals; they get invoked from moc-generated code. In any case,
 * printing a warning message in either case is not useful.
 * For slots, it can make sense (since slots can be invoked directly) but be
 * aware that if the slots get triggered by a signal, the will get called from
 * moc code as well and thus the warnings are useless.
 *
 * \par
 * Also note that it is not possible to use KDE_DEPRECATED for classes which
 * use the k_dcop keyword (to indicate a DCOP interface declaration); this is
 * because the dcopidl program would choke on the unexpected declaration
 * syntax.
 *
 * \note
 * KDE_DEPRECATED cannot be used at the end of the declaration anymore,
 * unlike what is done for KDE3.
 *
 * \note
 * KDE_DEPRECATED cannot be used for constructors, 
 * use KDE_CONSTRUCTOR_DEPRECATED instead.
 */

#ifdef __cplusplus
# include <QtCore/qglobal.h>
# ifndef KDE_DEPRECATED
#  ifdef KDE_DEPRECATED_WARNINGS
#   define KDE_DEPRECATED Q_DECL_DEPRECATED
#  else
#   define KDE_DEPRECATED
#  endif
# endif
#endif

/**
 * @def KDE_CONSTRUCTOR_DEPRECATED
 * @ingroup KDEMacros
 *
 * The KDE_CONSTRUCTOR_DEPRECATED macro can be used to trigger compile-time
 * warnings with newer compilers when deprecated constructors are used.
 *
 * For non-inline constructors, the macro gets inserted at front of the
 * constructor declaration, right before the return type:
 *
 * \code
 * KDE_CONSTRUCTOR_DEPRECATED classA();
 * \endcode
 *
 * For constructors which are implemented inline,
 * the KDE_CONSTRUCTOR_DEPRECATED macro is inserted at the front,
 * but after the "inline" keyword:
 *
 * \code
 * KDE_CONSTRUCTOR_DEPRECATED classA() { .. }
 * \endcode
 *
 * \note Do not forget that inlined constructors are not allowed in public
 * headers for KDE.
 */

#ifndef KDE_CONSTRUCTOR_DEPRECATED
# ifdef __GNUC__
#  if __GNUC__ == 3 && __GNUC_MINOR__ <= 3 
    /* GCC 3.3.x cannot handle Qt 4.1.2's definition of Q_DECL_CONSTRUCTOR_DEPRECATED */
#   define KDE_CONSTRUCTOR_DEPRECATED
#  else
#   define KDE_CONSTRUCTOR_DEPRECATED Q_DECL_CONSTRUCTOR_DEPRECATED
#  endif
# else
#  define KDE_CONSTRUCTOR_DEPRECATED Q_DECL_CONSTRUCTOR_DEPRECATED
# endif
#endif

/**
 * @def KDE_NO_DEPRECATED
 * @ingroup KDEMacros
 *
 * The KDE_NO_DEPRECATED indicates if the deprecated symbols of the platform
 * have been compiled out.
 */
#cmakedefine KDE_NO_DEPRECATED

/**
 * @def KDE_ISLIKELY
 * @ingroup KDEMacros
 *
 * The KDE_ISLIKELY macro tags a boolean expression as likely to evaluate to
 * @c true. When used in an <tt>if ( )</tt> statement, it gives a hint to the compiler
 * that the following codeblock is likely to get executed. Providing this
 * information helps the compiler to optimize the code for better performance.
 * Using the macro has an insignificant code size or runtime memory footprint impact.
 * The code semantics is not affected.
 *
 * Example:
 *
 * \code
 * if ( KDE_ISLIKELY( testsomething() ) )
 *     abort();     // assume its likely that the application aborts
 * \endcode
 *
 * \note
 * Providing wrong information ( like marking a condition that almost never
 * passes as 'likely' ) will cause a significant runtime slowdown. Therefore only
 * use it for cases where you can be sure about the odds of the expression to pass
 * in all cases ( independent from e.g. user configuration ).
 *
 * \note
 * Do NOT use ( !KDE_ISLIKELY(foo) ) as an replacement for KDE_ISUNLIKELY() !
 *
 * @sa KDE_ISUNLIKELY
 */

/**
 * @def KDE_ISUNLIKELY
 * @ingroup KDEMacros
 *
 * The KDE_ISUNLIKELY macro tags a boolean expression as likely to evaluate to
 * @c false. When used in an <tt>if ( )</tt> statement, it gives a hint to the compiler
 * that the following codeblock is unlikely to get executed. Providing this
 * information helps the compiler to optimize the code for better performance.
 * Using the macro has an insignificant code size or runtime memory footprint impact.
 * The code semantics is not affected.
 *
 * Example:
 *
 * \code
 * if ( KDE_ISUNLIKELY( testsomething() ) )
 *     abort();     // assume its unlikely that the application aborts
 * \endcode
 *
 * \note
 * Providing wrong information ( like marking a condition that almost never
 * passes as 'unlikely' ) will cause a significant runtime slowdown. Therefore only
 * use it for cases where you can be sure about the odds of the expression to pass
 * in all cases ( independent from e.g. user configuration ).
 *
 * \note
 * Do NOT use ( !KDE_ISUNLIKELY(foo) ) as an replacement for KDE_ISLIKELY() !
 *
 * @sa KDE_ISLIKELY
 */

#if defined(__GNUC__) && __GNUC__ - 0 >= 3
# define KDE_ISLIKELY( x )    __builtin_expect(!!(x),1)
# define KDE_ISUNLIKELY( x )  __builtin_expect(!!(x),0)
#else
# define KDE_ISLIKELY( x )   ( x )
# define KDE_ISUNLIKELY( x )  ( x )
#endif


/**
 * @ingroup KDEMacros
 * This macro, and it's friends going up to 10 reserve a fixed number of virtual
 * functions in a class.  Because adding virtual functions to a class changes the
 * size of the vtable, adding virtual functions to a class breaks binary
 * compatibility.  However, by using this macro, and decrementing it as new
 * virtual methods are added, binary compatibility can still be preserved.
 *
 * \note The added functions must be added to the header at the same location
 * as the macro; changing the order of virtual functions in a header is also
 * binary incompatible as it breaks the layout of the vtable.
 */
#define RESERVE_VIRTUAL_1 \
    virtual void reservedVirtual1() {}
/**
 * @ingroup KDEMacros
 */
#define RESERVE_VIRTUAL_2 \
    virtual void reservedVirtual2() {} \
    RESERVE_VIRTUAL_1
/**
 * @ingroup KDEMacros
 */
#define RESERVE_VIRTUAL_3 \
    virtual void reservedVirtual3() {} \
    RESERVE_VIRTUAL_2
/**
 * @ingroup KDEMacros
 */
#define RESERVE_VIRTUAL_4 \
    virtual void reservedVirtual4() {} \
    RESERVE_VIRTUAL_3
/**
 * @ingroup KDEMacros
 */
#define RESERVE_VIRTUAL_5 \
    virtual void reservedVirtual5() {} \
    RESERVE_VIRTUAL_4
/**
 * @ingroup KDEMacros
 */
#define RESERVE_VIRTUAL_6 \
    virtual void reservedVirtual6() {} \
    RESERVE_VIRTUAL_5
/**
 * @ingroup KDEMacros
 */
#define RESERVE_VIRTUAL_7 \
    virtual void reservedVirtual7() {} \
    RESERVE_VIRTUAL_6
/**
 * @ingroup KDEMacros
 */
#define RESERVE_VIRTUAL_8 \
    virtual void reservedVirtual8() {} \
    RESERVE_VIRTUAL_7
/**
 * @ingroup KDEMacros
 */
#define RESERVE_VIRTUAL_9 \
    virtual void reservedVirtual9() {} \
    RESERVE_VIRTUAL_8
#define RESERVE_VIRTUAL_10 \
    virtual void reservedVirtual10() {} \
    RESERVE_VIRTUAL_9

/**
 * @def KDE_FULL_TEMPLATE_EXPORT_INSTANTIATION
 * @ingroup KDEMacros
 *
 * From Qt's global.h:
 * Compilers which follow outdated template instantiation rules
 * require a class to have a comparison operator to exist when
 * a QList of this type is instantiated. It's not actually
 * used in the list, though. Hence the dummy implementation.
 * Just in case other code relies on it we better trigger a warning
 * mandating a real implementation.
 *
 * In KDE we need this for classes which are exported in a shared
 * lib because some compilers need a full instantiated class then.
 *
 * @sa KDE_DUMMY_COMPARISON_OPERATOR
 * @sa KDE_DUMMY_QHASH_FUNCTION
 */

/**
 * @def KDE_DUMMY_COMPARISON_OPERATOR
 * @ingroup KDEMacros
 *
 * The KDE_DUMMY_COMPARISON_OPERATOR defines a simple
 * compare operator for classes.
 *
 * @sa KDE_FULL_TEMPLATE_EXPORT_INSTANTIATION
 * @sa KDE_DUMMY_QHASH_FUNCTION
 */

/**
 * @def KDE_DUMMY_QHASH_FUNCTION
 * @ingroup KDEMacros
 *
 * The KDE_DUMMY_QHASH_FUNCTION defines a simple
 * hash-function for classes. 
 *
 * @sa KDE_FULL_TEMPLATE_EXPORT_INSTANTIATION
 * @sa KDE_DUMMY_COMPARISON_OPERATOR
 */

#ifdef KDE_FULL_TEMPLATE_EXPORT_INSTANTIATION
# define KDE_DUMMY_COMPARISON_OPERATOR(C) \
    bool operator==(const C&) const { \
        qWarning(#C"::operator==(const "#C"&) was called"); \
        return false; \
    }
# define KDE_DUMMY_QHASH_FUNCTION(C) \
    inline uint qHash(const C) { \
        qWarning("inline uint qHash(const "#C") was called"); \
        return 0; \
    }
#else
# define KDE_DUMMY_COMPARISON_OPERATOR(C)
# define KDE_DUMMY_QHASH_FUNCTION(C)
#endif

/**
 * @def KDE_BF_ENUM
 * @ingroup KDEMacros
 *
 * The KDE_BF_ENUM is used when storing an enum
 * in a bitfield, to ensure correct conversion
 * by all compilers.
 *
 * @sa KDE_CAST_BF_ENUM
 */

/**
 * @def KDE_CAST_BF_ENUM
 * @ingroup KDEMacros
 *
 * The KDE_CAST_BF_ENUM is used when retrieving an
 * enum from a bitfield, to ensure correct conversion
 * by all compilers.
 *
 * @sa KDE_BF_ENUM
 */

#ifdef Q_CC_MSVC
# define KDE_BF_ENUM(a) unsigned int
# define KDE_CAST_BF_ENUM(a,b) static_cast<a>(b)
#else
# define KDE_BF_ENUM(a) a
# define KDE_CAST_BF_ENUM(a,b) b
#endif

/**
 * @def KDE_WEAK_SYMBOL
 * @ingroup KDEMacros
 *
 * The KDE_WEAK_SYMBOL macro can be used to tell the compiler that
 * a particular function should be a weak symbol (that e.g. may be overridden
 * in another library, -Bdirect will not bind this symbol directly)
 */

#ifdef __GNUC__
#define KDE_WEAK_SYMBOL __attribute__((__weak__))
#else
#define KDE_WEAK_SYMBOL
#endif


/**
 * @def KDE_MUST_USE_RESULT
 * @ingroup KDEMacros
 *
 * The KDE_MUST_USE_RESULT macro can be used to tell the compiler that
 * a particular functions return value must be checked.
 */

#ifdef __GNUC__
#define KDE_MUST_USE_RESULT __attribute__((__warn_unused_result__))
#else
#define KDE_MUST_USE_RESULT
#endif



#endif /* _KDE_MACROS_H_ */
