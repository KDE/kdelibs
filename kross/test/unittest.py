#!/usr/bin/env krosstest
# -*- coding: utf-8 -*-
# coding: utf-8

"""
  This Python script is used to test the Kross scripting framework.
"""

import unittest
import TestObject1
from TestObject2 import *

KrossAction = self

def testFunction(*args):
    return args

class TestKross(unittest.TestCase):
	""" Testcases to test the Kross python functionality for regressions. """

	def setUp(self):
		self.object1 = TestObject1
		self.object2 = TestObject2

	def testBool(self):
		self.assert_( self.object1.func_bool_bool(True) == True )
		self.assert_( self.object1.func_bool_bool(False) == False )

	def testInt(self):
		self.assert_( self.object1.func_int_int(0) == 0 )
		self.assert_( self.object1.func_int_int(177321) == 177321 )
		self.assert_( self.object1.func_int_int(-98765) == -98765 )

	def testUInt(self):
		self.assert_( self.object1.func_uint_uint(0) == 0 )
		self.assert_( self.object1.func_uint_uint(177321) == 177321 )
		#self.assertRaises(OverflowError, self.object1.func_uint_uint, -1)

	def testDouble(self):
		self.assert_( self.object1.func_double_double(0.0) == 0.0 )
		self.assert_( self.object1.func_double_double(1773.2177) == 1773.2177 )
		self.assert_( self.object1.func_double_double(-548993.271993) == -548993.271993 )

	def testLongLong(self):
		self.assert_( self.object1.func_qlonglong_qlonglong(0) == 0 )
		self.assert_( self.object1.func_qlonglong_qlonglong(7379) == 7379 )
		self.assert_( self.object1.func_qlonglong_qlonglong(-6384673) == -6384673 )
		#self.assert_( self.object1.func_qlonglong_qlonglong(678324787843223472165) == 678324787843223472165 )

	def testULongLong(self):
		self.assert_( self.object1.func_qulonglong_qulonglong(0) == 0 )
		self.assert_( self.object1.func_qulonglong_qulonglong(378972) == 378972 )
		#self.assert_( self.object1.func_qulonglong_qulonglong(-8540276823902375665225676321823) == -8540276823902375665225676321823 )

	def testByteArray(self):
		self.assert_( self.object1.func_qbytearray_qbytearray("  Some String as ByteArray  ") == "  Some String as ByteArray  " )
		self.assert_( self.object1.func_qbytearray_qbytearray(" \0\n\r\t\s\0 test ") == " \0\n\r\t\s\0 test " )

	def testString(self):
		self.assert_( self.object1.func_qstring_qstring(None) == None )
		self.assert_( self.object1.func_qstring_qstring("") == "" )
		self.assert_( self.object1.func_qstring_qstring(" ") == " " )
		self.assert_( self.object1.func_qstring_qstring(" Another \n\r Test!   $%&\" ") == " Another \n\r Test!   $%&\" " )
		self.assert_( self.object1.func_qstring_qstring("áêïòñα") == "áêïòñα" )
		self.assert_( self.object1.func_qstring_qstring(unicode("abcdef")) == "abcdef" )
		self.assert_( self.object1.func_qstring_qstring(u"\u03B1") == u"\u03B1".encode('utf8') )

	def testStringList(self):
		self.assert_( self.object1.func_qstringlist_qstringlist( [] ) == [] )
		self.assert_( self.object1.func_qstringlist_qstringlist( ["string1"] ) == ["string1"] )
		self.assert_( self.object1.func_qstringlist_qstringlist( [" string1","string2 "] ) == [" string1","string2 "] )

	def testVariantList(self):
		self.assert_( self.object1.func_qvariantlist_qvariantlist( [] ) == [] )
		self.assert_( self.object1.func_qvariantlist_qvariantlist( [[[[]],[]]] ) == [[[[]],[]]] )
		self.assert_( self.object1.func_qvariantlist_qvariantlist( ["A string",[17539,-8591],[5.32,-842.775]] ) == ["A string",[17539,-8591],[5.32,-842.775]] )
		self.assert_( self.object1.func_qvariantlist_qvariantlist( [[True,[],False,"Other String"],"test"] ) == [[True,[],False,"Other String"],"test"] )
		self.assert_( self.object1.func_qvariantlist_qvariantlist( [[17,52],[],{},[84.21,936.821]] ) == [[17,52],[],{},[84.21,936.821]] )

		l = TestObject1.func_qobject2qvariantlist(TestObject2)
		self.assert_( l[0] == TestObject2 )
		self.assert_( TestObject1.func_qvariantlist2qobject(l) == TestObject2 )

	def testVariantMap(self):

		def doTestVariantMap(vmap):
			rmap = self.object1.func_qvariantmap_qvariantmap( vmap )
			self.assert_( len(rmap) == len(vmap) )
			for k in vmap:
				#self.assert_( rmap.has_key(k) )
				self.assert_( rmap[k] == vmap[k] )

		doTestVariantMap( {} )
		doTestVariantMap( {"3":-837,"1":73682,"2":285} )
		doTestVariantMap( {"c":84.24,"a":-6892.957,"d":2.02,"b":692.66} )
		doTestVariantMap( {"key1":True,"key2":False} )
		doTestVariantMap( {"key 2":"  Some String  ","key 3":"oThEr StRiNg"} )
		doTestVariantMap( {" key4 ":[12.5,True]," key5 ":[83.002,"test"]} )
		doTestVariantMap( {'  key7  ':{},'  key6  ':{}} )

	def testSize(self):
		self.assert_( self.object1.func_qsize_qsize( [0,0] ) == [0,0] )
		self.assert_( self.object1.func_qsize_qsize( [12,-94] ) == [12,-94] )
		self.assert_( self.object1.func_qsize_qsize( [-7264,6224] ) == [-7264,6224] )

	def testSizeF(self):
		self.assert_( self.object1.func_qsizef_qsizef( [0.0,0.0] ) == [0.0,0.0] )
		self.assert_( self.object1.func_qsizef_qsizef( [-956.0,75.0] ) == [-956.0,75.0] )
		self.assert_( self.object1.func_qsizef_qsizef( [-14.21,-535.0] ) == [-14.21,-535.0] )
		self.assert_( self.object1.func_qsizef_qsizef( [26,-525] ) == [26,-525] )
		self.assert_( self.object1.func_qsizef_qsizef( [-956.0,75.21] ) == [-956.0,75.21] )

	def testPoint(self):
		self.assert_( self.object1.func_qpoint_qpoint( [0,0] ) == [0,0] )
		self.assert_( self.object1.func_qpoint_qpoint( [12,-94] ) == [12,-94] )
		self.assert_( self.object1.func_qpoint_qpoint( [-7264,6224] ) == [-7264,6224] )

	def testPointF(self):
		self.assert_( self.object1.func_qpointf_qpointf( [0.0,0.0] ) == [0.0,0.0] )
		self.assert_( self.object1.func_qpointf_qpointf( [-956.0,751.0] ) == [-956.0,751.0] )
		self.assert_( self.object1.func_qpointf_qpointf( [-82.3172,17.0] ) == [-82.3172,17.0] )
		self.assert_( self.object1.func_qpointf_qpointf( [1.2,2.3] ) == [1.2,2.3] )
		self.assert_( self.object1.func_qpointf_qpointf( [-956.03,751.4165] ) == [-956.03,751.4165] )

	def testRect(self):
		self.assert_( self.object1.func_qrect_qrect( [0,0,0,0] ) == [0,0,0,0] )
		self.assert_( self.object1.func_qrect_qrect( [-1,-2,3,4] ) == [-1,-2,3,4] )
		self.assert_( self.object1.func_qrect_qrect( [1,2,-3,-4] ) == [1,2,-3,-4] )
		self.assert_( self.object1.func_qrect_qrect( [-10,-20,30,40] ) == [-10,-20,30,40] )
		self.assert_( self.object1.func_qrect_qrect( [10,20,30,40] ) == [10,20,30,40] )
		self.assert_( self.object1.func_qrect_qrect( [10,20,-30,-40] ) == [10,20,-30,-40] )

	def testRectF(self):
		self.assert_( self.object1.func_qrectf_qrectf( [0.0,0.0,0.0,0.0] ) == [0.0,0.0,0.0,0.0] )
		self.assert_( self.object1.func_qrectf_qrectf( [-1.0,-2.0,3.0,4.0] ) == [-1.0,-2.0,3.0,4.0] )
		self.assert_( self.object1.func_qrectf_qrectf( [-1.1,-2.2,3.3,4.4] ) == [-1.1,-2.2,3.3,4.4] )

	def testUrl(self):
		self.assert_( self.object1.func_qurl_qurl(None) == None )
		#self.assert_( self.object1.func_qurl_qurl("") == "" )
		self.assert_( self.object1.func_qurl_qurl("/home/myuser") == "/home/myuser" )
		self.assert_( self.object1.func_qurl_qurl("file:///home/myuser/myfile.txt") == "file:///home/myuser/myfile.txt" )
		self.assert_( self.object1.func_qurl_qurl("http://myname:mypass@kross.dipe.org?404.cgi?test=123&test2=456") == "http://myname:mypass@kross.dipe.org?404.cgi?test=123&test2=456" )
		self.assert_( self.object1.func_kurl_kurl("http://www.kde.org/whatiskde/") == "http://www.kde.org/whatiskde/" )
		self.assert_( self.object1.func_kurl_qstring("http://www.kde.org/whatiskde/") == None )

	def testColor(self):
		self.assert_( self.object1.func_qcolor_qcolor(None) == None )
		self.assert_( self.object1.func_qcolor_qcolor("") == None )
		self.assert_( self.object1.func_qcolor_qcolor("#0066ff") == "#0066ff" )
		#self.assert_( self.object1.func_qcolor_qcolor((0,0,255)) == "#0000ff" )
		#self.assert_( self.object1.func_qcolor_qcolor((0.0,0.0,1.0)) == "#0000ff" )

		##self.object1.func_qtime_qtime
		##self.object1.func_qdate_qdate

	def testVariant(self):
		self.assert_( self.object1.func_qvariant_qvariant(0.0) == 0.0 )
		self.assert_( self.object1.func_qvariant_qvariant(True) == True )
		self.assert_( self.object1.func_qvariant_qvariant(False) == False )
		self.assert_( self.object1.func_qvariant_qvariant(187937) == 187937 )
		self.assert_( self.object1.func_qvariant_qvariant(-69825) == -69825 )
		self.assert_( self.object1.func_qvariant_qvariant(8632.274) == 8632.274 )
		self.assert_( self.object1.func_qvariant_qvariant(-8632.351) == -8632.351 )
		self.assert_( self.object1.func_qvariant_qvariant(" Test \n\r This String $%&\"") == " Test \n\r This String $%&\"")

	def testFunctions(self):
		self.assert_( "testFunction" in KrossAction.functionNames() )
		self.assert_( KrossAction.callFunction("testFunction",[]) == [] )
		self.assert_( KrossAction.callFunction("testFunction",[True,False]) == [True,False] )
		self.assert_( KrossAction.callFunction("testFunction",[524,-958]) == [524,-958] )
		self.assert_( KrossAction.callFunction("testFunction",[524.98,-958.1257]) == [524.98,-958.1257] )
		self.assert_( KrossAction.callFunction("testFunction",["","  Some\nString  "]) == ["","  Some\nString  "] )
		self.assert_( KrossAction.callFunction("testFunction",[["one",23,"two",None,False]]) == [["one",23,"two",None,False]] )
		self.assert_( KrossAction.callFunction("testFunction",[KrossAction]) == [KrossAction] )

	def testObject(self):
		self.assert_( self.object1.name() == "TestObject1" and self.object2.name() == "TestObject2" )
		self.assert_( self.object1.objectName == "TestObject1" and self.object2.objectName == "TestObject2" )
		self.assert_( self.object1.className() == "TestObject" and self.object2.className() == "TestObject" )
		self.assert_( self.object1.__toPointer__() )
		self.assert_( self.object1 * 2 == self.object1.__toPointer__() * 2 )

		self.assert_( self.object1.func_qobject_qobject(True) == None )
		self.assert_( self.object1.func_qobject_qobject("String") == None )
		self.assert_( self.object1.func_qobject_qobject(None) == None )

		self.assert_( self.object1["TestTimer"].objectName == "TestTimer" )

	def testPropertyMembers(self):
		self.object1.boolProperty = True
		self.assert_( self.object1.boolProperty == True )
		self.object1.boolProperty = False
		self.assert_( self.object1.boolProperty == False )

		self.object1.intProperty = 20
		self.assert_( self.object1.intProperty == 20 )

		self.object1.doubleProperty = 7436.671
		self.assert_( self.object1.doubleProperty == 7436.671 )

		self.object1.stringProperty = " SoMe StRiNg "
		self.assert_( self.object1.stringProperty == " SoMe StRiNg " )

		self.object1.stringListProperty = [ "TestString", " Other String " ]
		self.assert_( self.object1.stringListProperty == [ "TestString", " Other String " ] )

		self.object1.listProperty = [ True, [2464, -8295], -572.07516, "test", [] ]
		self.assert_( self.object1.listProperty == [ True, [2464, -8295], -572.07516, "test", [] ] )

	def testPropertyMethods(self):
		self.object1.setProperty("boolProperty", False)
		self.assert_( self.object1.property("boolProperty") == False )
		self.object1.setProperty("boolProperty", True)
		self.assert_( self.object1.property("boolProperty") == True )

		self.object1.setProperty("intProperty", -75825)
		self.assert_( self.object1.property("intProperty") == -75825 )

		self.object1.setProperty("doubleProperty", -9373.8721)
		self.assert_( self.object1.property("doubleProperty") == -9373.8721 )

		self.object1.setProperty("stringProperty", " AnOtHeR sTrInG ")
		self.assert_( self.object1.property("stringProperty") == " AnOtHeR sTrInG " )

		self.object1.setProperty("stringListProperty", ["s1","s2",""])
		self.assert_( self.object1.property("stringListProperty") == ["s1","s2",""] )

		self.object1.setProperty("listProperty", [[True,False],"abc",[846,-573.02],[]])
		self.assert_( self.object1.property("listProperty") == [[True,False],"abc",[846,-573.02],[]] )

	def testDynamicProperties(self):
		self.assert_( self.object1.property("dynamicProperty") == ["Some String",99] )
		self.object1.setProperty("newDynamicProperty","New Dynamic Property")
		self.assert_( self.object1.property("newDynamicProperty") == "New Dynamic Property" )

	def testEnumerationMembers(self):
		self.assert_( self.object1.TESTENUM1 == 1 )
		self.assert_( self.object1.TESTENUM2 == 2 )
		self.assert_( self.object1.TESTENUM3 == 4 )
		self.assert_( self.object1.TESTENUM4 == 8 )
		#self.assert_( self.object1.testEnum( self.object1.TESTENUM3 ) == 4 )

	def testSignalsSlots(self):
		self.assert_( self.object1.connect("signalBool(bool)", "func_bool_bool(bool)") )
		self.assert_( self.object1.connect("signalInt(int)", self.object2, "func_int_int(int)") )

		def callback1(obj):
			self.assert_(obj.name() == "TestObject2")
			return "CALLBACK1!"
		self.assert_( self.object1.connect("signalObject(QObject*)", callback1) )
		self.object1.signalObject(self.object2)

		def callback2(s):
			self.assert_(s == " The Argument String ")
			return "CALLBACK2!"
		self.assert_( self.object1.connect("signalString(const QString&)", callback2) )
		self.object1.signalString(" The Argument String ")

		self.assert_( self.object1.name() == "TestObject1" )
		self.assert_( self.object2.name() == "TestObject2" )

	#def testExpectedFailures(self):
		# to less arguments
		#self.assertRaises(ValueError, self.pluginobject1.uintfunc)
		#self.assert_( self.pluginobject1.uintfunc() != 8465 )

	#def testPyQt(self):
		#pyqtextension = TestObject3.toPyQt()
		#print "pyqtextension=%s" % pyqtextension
		#import PyQt4, sip
		#qobj = pyqtextension.getQObject()
		#qo = sip.wrapinstance (qobj, QObject)
		#print ">>>>>>>>>>>>>>>>>>> %s" % qo

	def testClass(self):
		class MyClass:
			def __init__(self, result):
				self.result = result
			def myMethod(self):
				return self.result
		s = "my string"
		myclass = MyClass(s)
		self.assert_( self.object1.call_krossobject_method(myclass, "myMethod") == s )

	def testOtherObject(self):
		otherobj1 = self.object1.func_otherobject("OtherObject1")
		otherobj2 = self.object1.func_otherobject("OtherObject2")
		#print "otherobj1=%s otherobj1.objectName=%s dir(otherobj1)=%s" % (otherobj1, otherobj1.objectName, dir(otherobj1))

		self.assert_( otherobj1.objectName == "OtherObject1_wrapper" )
		self.assert_( otherobj2.parentObject().objectName == "OtherObject2" )
		self.assert_( self.object1.func_otherobject_otherobject(otherobj1).objectName == otherobj1.objectName )

		l = self.object1.func_otherobjectlist_otherobjectlist( [otherobj1,otherobj2] )
		self.assert_( len(l) == 2 )
		self.assert_( l[0].objectName == "OtherObject1_wrapper" )
		self.assert_( l[1].parentObject().objectName == "OtherObject2" )

	def testEvaluation(self):
		self.assert_( KrossAction.evaluate("1+2") == 3 )
		self.assert_( KrossAction.evaluate("testFunction(None,99)") == [None,99] )

print "__name__ = %s" % __name__
#print "__main__ = %s %s" % (__main__,dir(__main__))
#print "TestObject3.name = %s" % TestObject3.name()

suite = unittest.makeSuite(TestKross)
unittest.TextTestRunner(verbosity=2).run(suite)

#import Kross, TestObject1
#print "===========> %s" % TestObject1.func_qsize_qsize( [12,-94] )
#print "===========> %s" % TestObject1.func_qsizef_qsizef( [12.2,-94.2] )
