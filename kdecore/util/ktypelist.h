//krazy:excludeall=license (minor variation on MIT license)
/*
 This work is derived from:
 ----
 The Loki Library
 Copyright (c) 2001 by Andrei Alexandrescu                                  //krazy:exclude=copyright
 This code accompanies the book:
 Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design
     Patterns Applied". Copyright (c) 2001. Addison-Wesley.                 //krazy:exclude=copyright
 Permission to use, copy, modify, distribute and sell this software for any
     purpose is hereby granted without fee, provided that the above copyright
     notice appear in all copies and that both that copyright notice and this
     permission notice appear in supporting documentation.
 The author or Addison-Welsey Longman make no representations about the
     suitability of this software for any purpose. It is provided "as is"
     without express or implied warranty.
 ----

 Simon: Actually we could put a lot more of typelist stuff in here, like
        real list management (append, erase, ...) or other things, but
	for now I just added the basic typelist and a length template,
	to keep compile time at a minimum. If we really need more we can
	still add it :)
 Holger: Now we add a Template to create the TypeList
*/

/**
 * @file ktypelist.h
 *
 * This file defines typelist structures as well as convenience macros
 * to create typelists. Additionally, a few typelist compile-time
 * algorithms are provided.
 */

/**
 * @defgroup ktypelist Typelist classes, algorithms and macros
 *
 * Typelists are lists of C++ types of arbitrary length. They are used
 * to carry type information at compile-time.
 *
 * Internally, typelists are represented by the KTypeList template
 * class. The KTypeList class uses the recursive structure of
 * singly-linked lists which is common in functional programming
 * languages:
 *
 * - an empty list is of type KDE::NullType (the terminal marker)
 * - a one-element list with element @c T is of type
 *   KTypeList\<T, KDE::NullType\>.
 * - a two-element list with elements @c T and @c U is of type
 *   KTypeList\<T, KTypeList\<U, KDE::NullType\> \>.
 * - an N-Element list with the first element @c T and the remaining
 *   elements @c Rest is of type KTypeList\<T, Rest\>.
 *
 * Note that the last element of a typelist is always KDE::NullType.
 * Also note that this is only a convention, it is not enforced by
 * anything. But if these rules are broken, the compile-time algorithms
 * defined for typelists don't work.
 *
 * To ease the definition of typelists, there are some macros which
 * expand to nested KTypeList definitions. These macros have the form
 *
 * @code
 *   K_TYPELIST_N(T1, T2, ..., TN)
 * @endcode
 *
 * where @c N is the number of types in the list (e.g. K_TYPELIST_3())
 *
 * In addition to that, and also as the preferred way, there is the
 * KMakeTypeList template which takes an arbitrary number of type
 * arguments (up to 18) and exports a nested typedef called @c Result
 * which equals a KTypeList with the list of provided arguments.
 *
 * To work with typelists, several compile-time algorithms are provided:
 *
 * - KTypeListLength: determine the number of elements in a typelist
 * - KTypeListIndexOf: find a given type in a typelist
 *
 * For a detailed discussion about typelists, see the book "Modern C++
 * Design: Generic Programming and Design Patterns Applied" by Andrei
 * Alexandrescu, and/or the Loki Library at
 * <a href="http://sourceforge.net/projects/loki-lib/">http://sourceforge.net/projects/loki-lib/</a>
 */

#ifndef ktypelist_h
#define ktypelist_h

/**
 * @name Typelist macros
 *
 * Convenience macros for transforming flat type enumerations into the
 * recursive typelist structure. For a typelist with @c N items, the
 * @c K_TYPELIST_N macro is used. For example:
 *
 * @code
 *  typedef K_TYPELIST_4(char, short, int, long) IntegralTypes;
 * @endcode
 *
 * However, the preferred way is to use the KMakeTypeList template.
 *
 * @ingroup ktypelist
 */
//@{
#define K_TYPELIST_1(T1) KTypeList<T1, ::KDE::NullType>

#define K_TYPELIST_2(T1, T2) KTypeList<T1, K_TYPELIST_1(T2) >

#define K_TYPELIST_3(T1, T2, T3) KTypeList<T1, K_TYPELIST_2(T2, T3) >

#define K_TYPELIST_4(T1, T2, T3, T4) \
    KTypeList<T1, K_TYPELIST_3(T2, T3, T4) >

#define K_TYPELIST_5(T1, T2, T3, T4, T5) \
    KTypeList<T1, K_TYPELIST_4(T2, T3, T4, T5) >

#define K_TYPELIST_6(T1, T2, T3, T4, T5, T6) \
    KTypeList<T1, K_TYPELIST_5(T2, T3, T4, T5, T6) >

#define K_TYPELIST_7(T1, T2, T3, T4, T5, T6, T7) \
    KTypeList<T1, K_TYPELIST_6(T2, T3, T4, T5, T6, T7) >

#define K_TYPELIST_8(T1, T2, T3, T4, T5, T6, T7, T8) \
    KTypeList<T1, K_TYPELIST_7(T2, T3, T4, T5, T6, T7, T8) >

#define K_TYPELIST_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) \
    KTypeList<T1, K_TYPELIST_8(T2, T3, T4, T5, T6, T7, T8, T9) >

#define K_TYPELIST_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) \
    KTypeList<T1, K_TYPELIST_9(T2, T3, T4, T5, T6, T7, T8, T9, T10) >

#define K_TYPELIST_11(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) \
    KTypeList<T1, K_TYPELIST_10(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) >

#define K_TYPELIST_12(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) \
    KTypeList<T1, K_TYPELIST_11(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12) >

#define K_TYPELIST_13(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) \
    KTypeList<T1, K_TYPELIST_12(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13) >

#define K_TYPELIST_14(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14) \
    KTypeList<T1, K_TYPELIST_13(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14) >

#define K_TYPELIST_15(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15) \
    KTypeList<T1, K_TYPELIST_14(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15) >

#define K_TYPELIST_16(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16) \
    KTypeList<T1, K_TYPELIST_15(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16) >

#define K_TYPELIST_17(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17) \
    KTypeList<T1, K_TYPELIST_16(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17) >

#define K_TYPELIST_18(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18) \
    KTypeList<T1, K_TYPELIST_17(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18) >

#define K_TYPELIST_19(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19) \
    KTypeList<T1, K_TYPELIST_18(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19) >

#define K_TYPELIST_20(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20) \
    KTypeList<T1, K_TYPELIST_19(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20) >

#define K_TYPELIST_21(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21) \
    KTypeList<T1, K_TYPELIST_20(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21) >

#define K_TYPELIST_22(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22) \
    KTypeList<T1, K_TYPELIST_21(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22) >

#define K_TYPELIST_23(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23) \
    KTypeList<T1, K_TYPELIST_22(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23) >

#define K_TYPELIST_24(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24) \
    KTypeList<T1, K_TYPELIST_23(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24) >

#define K_TYPELIST_25(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25) \
    KTypeList<T1, K_TYPELIST_24(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25) >

#define K_TYPELIST_26(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26) \
    KTypeList<T1, K_TYPELIST_25(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26) >

#define K_TYPELIST_27(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27) \
    KTypeList<T1, K_TYPELIST_26(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27) >

#define K_TYPELIST_28(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28) \
    KTypeList<T1, K_TYPELIST_27(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28) >

#define K_TYPELIST_29(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29) \
    KTypeList<T1, K_TYPELIST_28(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29) >

#define K_TYPELIST_30(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30) \
    KTypeList<T1, K_TYPELIST_29(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30) >

#define K_TYPELIST_31(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31) \
    KTypeList<T1, K_TYPELIST_30(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31) >

#define K_TYPELIST_32(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32) \
    KTypeList<T1, K_TYPELIST_31(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32) >

#define K_TYPELIST_33(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32, T33) \
    KTypeList<T1, K_TYPELIST_32(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32, T33) >

#define K_TYPELIST_34(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32, T33, T34) \
    KTypeList<T1, K_TYPELIST_33(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32, T33, T34) >

#define K_TYPELIST_35(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35) \
    KTypeList<T1, K_TYPELIST_34(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35) >

#define K_TYPELIST_36(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36) \
    KTypeList<T1, K_TYPELIST_35(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36) >

#define K_TYPELIST_37(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37) \
    KTypeList<T1, K_TYPELIST_36(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37) >

#define K_TYPELIST_38(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38) \
    KTypeList<T1, K_TYPELIST_37(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38) >

#define K_TYPELIST_39(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39) \
    KTypeList<T1, K_TYPELIST_38(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39) >

#define K_TYPELIST_40(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40) \
    KTypeList<T1, K_TYPELIST_39(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40) >

#define K_TYPELIST_41(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41) \
    KTypeList<T1, K_TYPELIST_40(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41) >

#define K_TYPELIST_42(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41, T42) \
    KTypeList<T1, K_TYPELIST_41(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41, T42) >

#define K_TYPELIST_43(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41, T42, T43) \
    KTypeList<T1, K_TYPELIST_42(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41, T42, T43) >

#define K_TYPELIST_44(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41, T42, T43, T44) \
    KTypeList<T1, K_TYPELIST_43(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41, T42, T43, T44) >

#define K_TYPELIST_45(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
        T41, T42, T43, T44, T45) \
    KTypeList<T1, K_TYPELIST_44(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
        T41, T42, T43, T44, T45) >

#define K_TYPELIST_46(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
        T41, T42, T43, T44, T45, T46) \
    KTypeList<T1, K_TYPELIST_45(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
        T41, T42, T43, T44, T45, T46) >

#define K_TYPELIST_47(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
        T41, T42, T43, T44, T45, T46, T47) \
    KTypeList<T1, K_TYPELIST_46(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
        T41, T42, T43, T44, T45, T46, T47) >

#define K_TYPELIST_48(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
        T41, T42, T43, T44, T45, T46, T47, T48) \
    KTypeList<T1, K_TYPELIST_47(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
        T41, T42, T43, T44, T45, T46, T47, T48) >

#define K_TYPELIST_49(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
        T41, T42, T43, T44, T45, T46, T47, T48, T49) \
    KTypeList<T1, K_TYPELIST_48(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
        T41, T42, T43, T44, T45, T46, T47, T48, T49) >

#define K_TYPELIST_50(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
        T41, T42, T43, T44, T45, T46, T47, T48, T49, T50) \
    KTypeList<T1, K_TYPELIST_49(T2, T3, T4, T5, T6, T7, T8, T9, T10, \
        T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, \
        T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, \
        T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, \
        T41, T42, T43, T44, T45, T46, T47, T48, T49, T50) >
//@}

namespace KDE
{
    /**
     * @class KDE::NullType
     *
     * This empty class serves as a terminal marker for typelists.
     * The last element in a KTypeList is always this class.
     *
     * @ingroup ktypelist
     */
    class NullType;
}

/**
 * The building block of typelists of any length.
 * Rather than using it directly, you should use it through the
 * KMakeTypeList template class or one of the K_TYPELIST_NN macros,
 * such as K_TYPELIST_3().
 *
 * This struct defines two nested types:
 *   @li Head (first element, a non-typelist type by convention),
 *       is the same as the type parameter @p T.
 *   @li Tail (second element, must be either another typelist
 *       or KDE::NullType), is the same as the type parameter @p U.
 *
 * @param T the head of the type list
 * @param U the tail of the type list
 *
 * @ingroup ktypelist
 */
template <class T, class U>
struct KTypeList
{
  /// first element, a non-typelist type by convention
   typedef T Head;
  /// second element, must be either another typelist or KDE::NullType
   typedef U Tail;
};

// forward decl.
/**
 * @class KTypeListLength
 *
 * This class template implements a compile-time algorithm
 * for processing typelists. It expects one type argument:
 * @p TList.
 *
 * KTypeListLength determines the number of elements (the
 * length) of the typelist @p TList and exports it through
 * the member @c Value. The length of KDE::NullType is 0.
 * Example:
 *
 * @code
 *   typedef KMakeTypeList<char, short, int, long>::Result IntegralTypes;
 *   assert(KTypeListLength<IntegralTypes>::Value == 4);
 *   assert(KTypeListLength<KDE::NullType>::Value == 0);
 * @endcode
 *
 * @param TList the typelist of which the length is to be
 *        calculated
 *
 * @ingroup ktypelist
 */
template <class TList> struct KTypeListLength;

template <>
struct KTypeListLength<KDE::NullType>
{
/**
 * Zero length type list.
 */
    enum { Value = 0 };
};

template <class T, class U>
struct KTypeListLength< KTypeList<T, U> >
{
    /**
     * The length of the type list.
     */
    enum { Value = 1 + KTypeListLength<U>::Value };
};

///////////////////////////////////////////////////////////////////////////////
// class template IndexOf
// Finds the index of a type in a typelist
// Invocation (TList is a typelist and T is a type):
// IndexOf<TList, T>::value
// returns the position of T in TList, or NullType if T is not found in TList
////////////////////////////////////////////////////////////////////////////////

/**
 * @class KTypeListIndexOf
 *
 * This class template implements a compile-time algorithm
 * for processing typelists. It expects two type arguments:
 * @p TList and @p T.
 *
 * KTypeListIndexOf finds the index of @p T in @p TList
 * starting at 0 and exports it through the @c value member.
 * If @p T is not found, @c value is -1. Example:
 *
 * @code
 *   typedef KMakeTypeList<char, short, int, long>::Result IntegralTypes;
 *   assert(KTypeListIndexOf<IntegralTypes, int>::value == 3);
 *   assert(KTypeListIndexOf<IntegralTypes, double>::value == -1);
 * @endcode
 *
 * @param TList either a KTypeList or KDE::NullType
 * @param T the type to search for in the typelist
 *
 * @ingroup ktypelist
 */
template <class TList, class T> struct KTypeListIndexOf;

template <class T>
struct KTypeListIndexOf<KDE::NullType, T>
{
    enum { value = -1 };
};

template <class T, class Tail>
struct KTypeListIndexOf< KTypeList<T, Tail>, T >
{
    enum { value = 0 };
};

template <class Head, class Tail, class T>
struct KTypeListIndexOf< KTypeList<Head, Tail>, T >
{
private:
    enum { temp = KTypeListIndexOf<Tail, T>::value };
public:
    enum { value = (temp == -1 ? -1 : 1 + temp) };
};



/**
 * This class template implements a compile-time algorithm
 * for generating typelists.
 *
 * KMakeTypeList the preferred way to create a typelist for you.
 * You can specify up to 18 types for the typelist. This template
 * class calculates the desired typelist and stores it in the
 * nested @c Result typedef. Example:
 *
 * \code
 * typedef KMakeTypeList<MyType1,MyWidget,MyQobject,MyKoffice>::Result Products;
 * K_EXPORT_COMPONENT_FACTORY( libmyplugin, KGenericFactory<Products> )
 * \endcode 
 *
 * @author Holger Freyther based on the Loki library. See copyright statement at the top
 * @ingroup ktypelist
 */
template<
    typename T1  = KDE::NullType, typename T2  = KDE::NullType, typename T3  = KDE::NullType,
    typename T4  = KDE::NullType, typename T5  = KDE::NullType, typename T6  = KDE::NullType,
    typename T7  = KDE::NullType, typename T8  = KDE::NullType, typename T9  = KDE::NullType,
    typename T10 = KDE::NullType, typename T11 = KDE::NullType, typename T12 = KDE::NullType,
    typename T13 = KDE::NullType, typename T14 = KDE::NullType, typename T15 = KDE::NullType,
    typename T16 = KDE::NullType, typename T17 = KDE::NullType, typename T18 = KDE::NullType
    >
struct KMakeTypeList{
private:
typedef typename KMakeTypeList
<
  T2 , T3 , T4 ,
  T5 , T6 , T7 ,
  T8 , T9 , T10,
  T11, T12, T13,
  T14, T15, T16,
  T17, T18
>::Result TailResult;

public:
    /**
     * The resulting KTypeList calculated by this compile-time
     * algorithm.
     */
    typedef KTypeList<T1, TailResult> Result;
};

template<>
struct KMakeTypeList<>
{
    typedef KDE::NullType Result;
};


#endif

