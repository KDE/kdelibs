#!/usr/bin/env kross

"""
  This Python script is used to test the Kross scripting framework.
"""

import unittest

class TestKross(unittest.TestCase):
	""" Testcases to test the Kross python functionality for regressions. """

	def setUp(self):
		import TestObject1, TestObject2
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
		self.assert_( self.object1.func_qstring_qstring("") == "" )
		self.assert_( self.object1.func_qstring_qstring(" ") == " " )
		self.assert_( self.object1.func_qstring_qstring(" Another \n\r Test!   $%&\" ") == " Another \n\r Test!   $%&\" " )

	def testStringList(self):
		self.assert_( self.object1.func_qstringlist_qstringlist( [] ) == [] )
		self.assert_( self.object1.func_qstringlist_qstringlist( ["string1"] ) == ["string1"] )
		self.assert_( self.object1.func_qstringlist_qstringlist( [" string1","string2 "] ) == [" string1","string2 "] )

	def testVariantList(self):
		self.assert_( self.object1.func_qvariantlist_qvariantlist( [] ) == [] )
		self.assert_( self.object1.func_qvariantlist_qvariantlist( [[[[]],[]]] ) == [[[[]],[]]] )
		self.assert_( self.object1.func_qvariantlist_qvariantlist( ["A string",[17539,-8591],[5.32,-842.775]] ) == ["A string",[17539,-8591],[5.32,-842.775]] )
		self.assert_( self.object1.func_qvariantlist_qvariantlist( [[True,[],False,"Other String"],"test"] ) == [[True,[],False,"Other String"],"test"] )

	def testVariantMap(self):

		def doTestVariantMap(vmap):
			rmap = self.object1.func_qvariantmap_qvariantmap( vmap )
			self.assert_( len(rmap) == len(vmap) )
			for k in vmap:
				self.assert_( rmap[k] == vmap[k] )

		doTestVariantMap( {} )
		doTestVariantMap( {"1":73682,"2":285} )
		doTestVariantMap( {"a":-6892.957,"b":692.66} )
		doTestVariantMap( {"key1":True,"key2":False} )
		doTestVariantMap( {"key 1":"  Some String  ","key 2":"oThEr StRiNg"} )
		doTestVariantMap( {" key1 ":[12.5,True]," key2 ":[83.002,"test"]} )

	def testSize(self):
		self.assert_( self.object1.func_qsize_qsize( [12,-94] ) == [12,-94] )
		self.assert_( self.object1.func_qsize_qsize( [-7264,6224] ) == [-7264,6224] )

		self.assert_( self.object1.func_qsizef_qsizef( [-956.0,75.0] ) == [-956.0,75.0] )
		self.assert_( self.object1.func_qsizef_qsizef( [-14.21,-535.0] ) == [-14.21,-535.0] )
		self.assert_( self.object1.func_qsizef_qsizef( [26,-525] ) == [26,-525] )

		#TODO following crashes with;
		#
		#0  ~QVariant (this=0x816218c) at ../../include/QtCore/../../src/corelib/arch/qatomic_i386.h:82
		#1  0xb63c756b in ~MetaTypeVariant (this=0x8162188) at /opt/kde4/include/kross/core/metatype.h:91
		#2  0xb63bf33f in Kross::PythonExtension::proxyhandler (_self_and_name_tuple=0xb62134b4, args=0xb61fd80c) at /home/kde4/koffice/libs/kross/python/pythonextension.cpp:574
		#
		#This seems to be a bug in Qt cause it does crash only if the second argument has something != .0 :-(
		#
		#self.assert_( self.object1.func_qsizef_qsizef( [-956.0,75.21] ) == [-956.0,75.21] )

	def testPoint(self):
		self.assert_( self.object1.func_qpoint_qpoint( [12,-94] ) == [12,-94] )
		self.assert_( self.object1.func_qpoint_qpoint( [-7264,6224] ) == [-7264,6224] )

		self.assert_( self.object1.func_qpointf_qpointf( [-956.0,751.0] ) == [-956.0,751.0] )
		self.assert_( self.object1.func_qpointf_qpointf( [-82.3172,17.0] ) == [-82.3172,17.0] )

		#TODO following cases are crashing (see note in the testSize method above);
		#self.assert_( self.object1.func_qpointf_qpointf( [1.2,2.3] ) == [1.2,2.3] )
		#self.assert_( self.object1.func_qpointf_qpointf( [-956.03,751.4165] ) == [-956.03,751.4165] )

	def testRect(self):
		self.assert_( self.object1.func_qrect_qrect( [-1,-2,3,4] ) == [-1,-2,3,4] )

		#TODO following cases are crashing (see note in the testSize method above, but compared
		#to the other crashes, here it crashes even on non-float functionality, grrrr);
		#self.assert_( self.object1.func_qrect_qrect( [1,2,-3,-4] ) == [1,2,-3,-4] )
		#self.assert_( self.object1.func_qrect_qrect( [-10,-20,30,40] ) == [-10,-20,30,40] )
		#self.assert_( self.object1.func_qrect_qrect( [10,20,30,40] ) == [10,20,30,40] )
		#self.assert_( self.object1.func_qrect_qrect( [10,20,-30,-40] ) == [10,20,-30,-40] )

		#0  0xb6aa8947 in raise () from /lib/tls/libc.so.6
		#1  0xb6aaa0c9 in abort () from /lib/tls/libc.so.6
		#2  0xb6addfda in __fsetlocking () from /lib/tls/libc.so.6
		#3  0xb6ae589f in mallopt () from /lib/tls/libc.so.6
		#4  0xb6ae5942 in free () from /lib/tls/libc.so.6
		#5  0xb6c935e1 in operator delete () from /usr/lib/libstdc++.so.6
		#6  0xb63a85b7 in ~MetaTypeVariant (this=0x816b8c8) at /opt/kde4/include/kross/core/metatype.h:91
		#7  0xb63a033f in Kross::PythonExtension::proxyhandler (_self_and_name_tuple=0xb61fb5f4, args=0xb61de7ec) at /home/kde4/koffice/libs/kross/python/pythonextension.cpp:574
		#
		#self.assert_( self.object1.func_qrectf_qrectf( [-1.0,-2.0,3.0,4.0] ) == [-1.0,-2.0,3.0,4.0] )

		#0  0xb6a4a947 in raise () from /lib/tls/libc.so.6
		#1  0xb6a4c0c9 in abort () from /lib/tls/libc.so.6
		#2  0xb6a7ffda in __fsetlocking () from /lib/tls/libc.so.6
		#3  0xb6a883f3 in free () from /lib/tls/libc.so.6
		#4  0xb6a89c4f in malloc () from /lib/tls/libc.so.6
		#5  0xb7e532bd in qMalloc (size=26) at global/qglobal.cpp:1818
		#6  0xb7e5b88e in QByteArray::realloc (this=0xbf8f2328, alloc=6) at tools/qbytearray.cpp:1358
		#7  0xb7ef1d02 in normalizeTypeInternal (t=<value optimized out>, e=<value optimized out>, fixScope=false, adjustConst=true) at ../../include/QtCore/../../src/corelib/tools/qbytearray.h:361
		#8  0xb7ef28f6 in qNormalizeType (d=<value optimized out>, templdepth=<value optimized out>, result=@0xbf8f2498) at kernel/qmetaobject.cpp:741
		#9  0xb7ef3326 in QMetaObject::normalizedType (type=0x8052026 "QRectF") at kernel/qmetaobject.cpp:774
		#10 0xb7ef401c in QMetaType::type (typeName=0x8052026 "QRectF") at kernel/qmetatype.cpp:470
		#11 0xb7f05963 in QVariant::nameToType (name=0x8052026 "QRectF") at kernel/qvariant.cpp:1682
		#12 0xb63421de in Kross::PythonExtension::proxyhandler (_self_and_name_tuple=0xb619d5f4, args=0xb61807ec) at /home/kde4/koffice/libs/kross/python/pythonextension.cpp:560
		#
		#self.assert_( self.object1.func_qrectf_qrectf( [-1.1,-2.2,3.3,4.4] ) == [-1.1,-2.2,3.3,4.4] )

	#def testColor(self):
		#import Kross
		#color = Kross.color()

		#color.setNamedColor("#ff0000");
		#self.assert_( color.name() == "#ff0000" )
		#self.assert_( color.red() == 255 and color.green() == 0 and color.blue() == 0 and color.alpha() == 255 )

		#color.setCmyk(1,2,3,4,5)
		#self.assert_( color.name() == "#fafaf9" )
		#self.assert_( color.cyan() == 1 and color.magenta() == 2 and color.yellow() == 3 and color.black() == 4 and color.alpha() == 5 )

		#self.assert_( self.object1.func_qcolor_qcolor("#0066ff").name() == "#0066ff" )
		#self.assert_( self.object1.func_qcolor_qcolor((0,0,255)).name() == "#0000ff" )
		#self.assert_( self.object1.func_qcolor_qcolor((0.0,0.0,1.0)).name() == "#0000ff" )

		##self.object1.func_qfont_qfont
		##self.object1.func_qbrush_qbrush
		##self.object1.func_qtime_qtime
		##self.object1.func_qdate_qdate
		##self.object1.func_qdatetime_qdatetime

	def testVariant(self):
		#self.assert_( self.object1.func_qvariant_qvariant(0.0) == 0.0 )
		#self.assert_( self.object1.func_qvariant_qvariant(True) == True )
		#self.assert_( self.object1.func_qvariant_qvariant(False) == False )
		#self.assert_( self.object1.func_qvariant_qvariant(187937) == 187937 )
		#self.assert_( self.object1.func_qvariant_qvariant(-69825) == -69825 )
		#self.assert_( self.object1.func_qvariant_qvariant(8632.274) == 8632.274 )
		#self.assert_( self.object1.func_qvariant_qvariant(-8632.351) == -8632.351 )
		#self.assert_( self.object1.func_qvariant_qvariant(" Test \n\r This String $%&\"") == " Test \n\r This String $%&\"")
		pass

	def testObject(self):
		self.assert_( self.object1.name() == "TestObject1" and self.object2.name() == "TestObject2" )
		self.assert_( self.object1.objectName == "TestObject1" and self.object2.objectName == "TestObject2" )
		self.assert_( self.object1.className() == "TestObject" and self.object2.className() == "TestObject" )

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

	def testEnumerationMembers(self):
		self.assert_( self.object1.TESTENUM1 == 1 )
		self.assert_( self.object1.TESTENUM2 == 2 )
		self.assert_( self.object1.TESTENUM3 == 4 )
		self.assert_( self.object1.TESTENUM4 == 8 )
		#self.assert_( self.object1.testEnum( self.object1.TESTENUM3 ) == 4 )

	def testSignalsSlots(self):
		self.assert_( self.object1.connect("signalBool(bool)", "func_bool_bool(bool)") )
		self.assert_( self.object1.connect("signalInt(int)", self.object2, "func_int_int(int)") )

		def callback(s):
			self.assert_(s == " The Argument String ")
			return "CALLBACK!"
		self.assert_( self.object1.connect("signalString(const QString&)", callback) )
		self.object1.signalString(" The Argument String ")

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

print "__name__ = %s" % __name__
#print "__main__ = %s %s" % (__main__,dir(__main__))
#print "TestObject3.name = %s" % TestObject3.name()

suite = unittest.makeSuite(TestKross)
unittest.TextTestRunner(verbosity=2).run(suite)

#import Kross, TestObject1
#print "===========> %s" % TestObject1.func_qsize_qsize( [12,-94] )
