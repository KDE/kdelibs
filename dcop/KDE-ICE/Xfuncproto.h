/* $Xorg: Xfuncproto.h,v 1.3 2000/08/18 04:05:44 coskrey Exp $ */
/* 
 * 
Copyright 1989, 1991, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 */

/* Definitions to make function prototypes manageable */

#ifndef _XFUNCPROTO_H_
#define _XFUNCPROTO_H_

#ifndef NeedFunctionPrototypes
#if defined(FUNCPROTO) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#define NeedFunctionPrototypes 1
#else
#define NeedFunctionPrototypes 0
#endif
#endif /* NeedFunctionPrototypes */

#ifndef NeedVarargsPrototypes
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus) || (FUNCPROTO&2)
#define NeedVarargsPrototypes 1
#else
#define NeedVarargsPrototypes 0
#endif
#endif /* NeedVarargsPrototypes */

#if NeedFunctionPrototypes

#ifndef NeedNestedPrototypes
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus) || (FUNCPROTO&8)
#define NeedNestedPrototypes 1
#else
#define NeedNestedPrototypes 0
#endif
#endif /* NeedNestedPrototypes */

#ifndef _Xconst
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus) || (FUNCPROTO&4)
#define _Xconst const
#else
#define _Xconst
#endif
#endif /* _Xconst */

#ifndef NeedWidePrototypes
#ifdef NARROWPROTO
#define NeedWidePrototypes 0
#else
#define NeedWidePrototypes 1		/* default to make interropt. easier */
#endif
#endif /* NeedWidePrototypes */

#endif /* NeedFunctionPrototypes */

#ifndef _XFUNCPROTOBEGIN
#ifdef __cplusplus			/* for C++ V2.0 */
#define _XFUNCPROTOBEGIN extern "C" {	/* do not leave open across includes */
#define _XFUNCPROTOEND }
#else
#define _XFUNCPROTOBEGIN
#define _XFUNCPROTOEND
#endif
#endif /* _XFUNCPROTOBEGIN */

#endif /* _XFUNCPROTO_H_ */
