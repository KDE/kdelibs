#!/usr/bin/env krosstest

require 'test/unit'

require "TestObject1"
require "TestObject2"

KrossAction = self.action()

class MyClass
    def initialize(result)
        @result = result
    end
    def myMethod
        return @result
    end
end

def testFunction1(arg1)
    return arg1
end

def testFunction2(arg1,arg2)
    return [arg1,arg2]
end

class TestKross < Test::Unit::TestCase

	def setup
	end

	def testBool
		assert_raises(TypeError) { TestObject1.func_bool_bool(17) }
		assert_raises(TypeError) { TestObject1.func_bool_bool("string") }
		assert_raises(TypeError) { TestObject1.func_bool_bool([]) }

		assert( TestObject1.func_bool_bool(nil) == false )
		assert( TestObject1.func_bool_bool(true) == true )
		assert( TestObject1.func_bool_bool(false) == false )
	end

	def testInt
		assert_raises(TypeError) { TestObject1.func_int_int(false) }
		assert_raises(TypeError) { TestObject1.func_int_int("string") }
		assert_raises(TypeError) { TestObject1.func_bool_bool([]) }

		assert( TestObject1.func_int_int(nil) == 0 )
		assert( TestObject1.func_int_int(0) == 0 )
		assert( TestObject1.func_int_int(177321) == 177321 )
		assert( TestObject1.func_int_int(-98765) == -98765 )
		assert( TestObject1.func_int_int(17.111) == 17 )
		assert( TestObject1.func_int_int(42.999) == 42 )
	end

	def testUInt
		assert_raises(TypeError) { TestObject1.func_uint_uint(true) }
		assert_raises(TypeError) { TestObject1.func_uint_uint("string") }
		assert_raises(TypeError) { TestObject1.func_uint_uint([]) }

		assert( TestObject1.func_uint_uint(nil) == 0 )
		assert( TestObject1.func_uint_uint(0) == 0 )
		assert( TestObject1.func_uint_uint(177321) == 177321 )
		assert( TestObject1.func_uint_uint(17.111) == 17 )
		assert( TestObject1.func_uint_uint(42.999) == 42 )
	end

	def testDouble
		assert_raises(TypeError) { TestObject1.func_double_double(true) }
		assert_raises(TypeError) { TestObject1.func_double_double("string") }
		assert_raises(TypeError) { TestObject1.func_double_double([]) }
		#assert_raises(TypeError) { TestObject1.func_double_double(83) }

		assert( TestObject1.func_double_double(nil) == 0.0 )
		assert( TestObject1.func_double_double(0.0) == 0.0 )
		assert( TestObject1.func_double_double(1773.2177) == 1773.2177 )
		assert( TestObject1.func_double_double(-548993.271993) == -548993.271993 )
		assert( TestObject1.func_double_double(123) == 123.0 )
		assert( TestObject1.func_double_double(-456) == -456.0 )
	end

	def testLongLong
		assert_raises(TypeError) { TestObject1.func_qlonglong_qlonglong(true) }
		assert_raises(TypeError) { TestObject1.func_qlonglong_qlonglong("string") }
		assert_raises(TypeError) { TestObject1.func_qlonglong_qlonglong([]) }

		assert( TestObject1.func_qlonglong_qlonglong(nil) == 0 )
		assert( TestObject1.func_qlonglong_qlonglong(0) == 0 )
		assert( TestObject1.func_qlonglong_qlonglong(7379) == 7379 )
		assert( TestObject1.func_qlonglong_qlonglong(-6384673) == -6384673 )
		#assert( TestObject1.func_qlonglong_qlonglong(678324787843223472165) == 678324787843223472165 )
	end

	def testULongLong
		assert_raises(TypeError) { TestObject1.func_qulonglong_qulonglong(true) }
		assert_raises(TypeError) { TestObject1.func_qulonglong_qulonglong("string") }
		assert_raises(TypeError) { TestObject1.func_qulonglong_qulonglong([]) }

		assert( TestObject1.func_qulonglong_qulonglong(nil) == 0 )
		assert( TestObject1.func_qulonglong_qulonglong(0) == 0 )
		assert( TestObject1.func_qulonglong_qulonglong(378972) == 378972 )
		#assert( TestObject1.func_qulonglong_qulonglong(-8540276823902375665225676321823) == -8540276823902375665225676321823 )
	end

	def testByteArray
		assert_raises(TypeError) { TestObject1.func_qbytearray_qbytearray(false) }
		assert_raises(TypeError) { TestObject1.func_qbytearray_qbytearray(true) }
		assert_raises(TypeError) { TestObject1.func_qbytearray_qbytearray(532) }
		assert_raises(TypeError) { TestObject1.func_qbytearray_qbytearray(-2) }
		assert_raises(TypeError) { TestObject1.func_qbytearray_qbytearray(8572.45) }
		assert_raises(TypeError) { TestObject1.func_qbytearray_qbytearray([]) }

		#assert( TestObject1.func_qbytearray_qbytearray(nil) == "" )
		assert( TestObject1.func_qbytearray_qbytearray("") == "" )
		assert( TestObject1.func_qbytearray_qbytearray("  Some String as ByteArray  ") == "  Some String as ByteArray  " )
		assert( TestObject1.func_qbytearray_qbytearray(" \0\n\r\t\s\0 test ") == " \0\n\r\t\s\0 test " )
	end

	def testString
		assert_raises(TypeError) { TestObject1.func_qstring_qstring(false) }
		assert_raises(TypeError) { TestObject1.func_qstring_qstring(true) }
		assert_raises(TypeError) { TestObject1.func_qstring_qstring(927) }
		assert_raises(TypeError) { TestObject1.func_qstring_qstring(19.32) }
		assert_raises(TypeError) { TestObject1.func_qstring_qstring([]) }

		#assert( TestObject1.func_qstring_qstring(nil) == "" )
		assert( TestObject1.func_qstring_qstring("") == "" )
		assert( TestObject1.func_qstring_qstring(" ") == " " )
		assert( TestObject1.func_qstring_qstring(" Another \n\r Test!   $%&\" ") == " Another \n\r Test!   $%&\" " )
	end

	def testSize
		assert_raises(TypeError) { TestObject1.func_qsize_qsize("") }
		assert_raises(TypeError) { TestObject1.func_qsize_qsize(123) }
		assert_raises(TypeError) { TestObject1.func_qsize_qsize(456.789) }
		assert_raises(TypeError) { TestObject1.func_qsize_qsize([]) }

		#assert( TestObject1.func_qsize_qsize( nil ) == [0,0] )
		#assert( TestObject1.func_qsize_qsize( [0,0] ) == [0,0] )
		assert( TestObject1.func_qsize_qsize( [12,-94] ) == [12,-94] )
		assert( TestObject1.func_qsize_qsize( [-7264,6224] ) == [-7264,6224] )
	end

	def testSizeF
		assert_raises(TypeError) { TestObject1.func_qsizef_qsizef("") }
		assert_raises(TypeError) { TestObject1.func_qsizef_qsizef(123) }
		assert_raises(TypeError) { TestObject1.func_qsizef_qsizef(456.789) }
		assert_raises(TypeError) { TestObject1.func_qsizef_qsizef([]) }

		#assert_raises(TypeError) { TestObject1.func_qsizef_qsizef(nil) }
		#assert_raises(TypeError) { TestObject1.func_qsizef_qsizef([0.0]) }
		#assert_raises(TypeError) { TestObject1.func_qsizef_qsizef([0.0,0.0,0.0]) }
		#assert( TestObject1.func_qsizef_qsizef( [0.0,0.0] ) == [0.0,0.0] )
		assert( TestObject1.func_qsizef_qsizef( [-956.0,75.0] ) == [-956.0,75.0] )
		assert( TestObject1.func_qsizef_qsizef( [-14.21,-535.0] ) == [-14.21,-535.0] )
		assert( TestObject1.func_qsizef_qsizef( [26,-525] ) == [26,-525] )
		assert( TestObject1.func_qsizef_qsizef( [-956.0,75.21] ) == [-956.0,75.21] )
	end

	def testPoint
		assert_raises(TypeError) { TestObject1.func_qpoint_qpoint("") }
		assert_raises(TypeError) { TestObject1.func_qpoint_qpoint(123) }
		assert_raises(TypeError) { TestObject1.func_qpoint_qpoint(456.789) }
		assert_raises(TypeError) { TestObject1.func_qpoint_qpoint([]) }

		#assert_raises(TypeError) { TestObject1.func_qpoint_qpoint(nil) }
		#assert_raises(TypeError) { TestObject1.func_qpoint_qpoint([0]) }
		#assert_raises(TypeError) { TestObject1.func_qpoint_qpoint([0,0,0]) }
		#assert( TestObject1.func_qpoint_qpoint( [0,0] ) == [0,0] )
		assert( TestObject1.func_qpoint_qpoint( [12,-94] ) == [12,-94] )
		assert( TestObject1.func_qpoint_qpoint( [-7264,6224] ) == [-7264,6224] )
	end

	def testPointF
		assert_raises(TypeError) { TestObject1.func_qpointf_qpointf("") }
		assert_raises(TypeError) { TestObject1.func_qpointf_qpointf(123) }
		assert_raises(TypeError) { TestObject1.func_qpointf_qpointf(456.789) }
		assert_raises(TypeError) { TestObject1.func_qpointf_qpointf([]) }

		#assert_raises(TypeError) { TestObject1.func_qpointf_qpointf(nil) }
		#assert_raises(TypeError) { TestObject1.func_qpointf_qpointf([0.0]) }
		#assert_raises(TypeError) { TestObject1.func_qpointf_qpointf([0.0,0.0,0.0]) }
		#assert( TestObject1.func_qpointf_qpointf( [0.0,0.0] ) == [0.0,0.0] )
		assert( TestObject1.func_qpointf_qpointf( [-956.0,751.0] ) == [-956.0,751.0] )
		assert( TestObject1.func_qpointf_qpointf( [-82.3172,17.0] ) == [-82.3172,17.0] )
		assert( TestObject1.func_qpointf_qpointf( [1.2,2.3] ) == [1.2,2.3] )
		assert( TestObject1.func_qpointf_qpointf( [-956.03,751.4165] ) == [-956.03,751.4165] )
	end

	def testRect
		assert_raises(TypeError) { TestObject1.func_qrect_qrect("") }
		assert_raises(TypeError) { TestObject1.func_qrect_qrect(123) }
		assert_raises(TypeError) { TestObject1.func_qrect_qrect(456.789) }
		assert_raises(TypeError) { TestObject1.func_qrect_qrect([]) }

		#assert_raises(TypeError) { TestObject1.func_qrect_qrect(nil) }
		#assert_raises(TypeError) { TestObject1.func_qrect_qrect([0,0,0]) }
		#assert_raises(TypeError) { TestObject1.func_qrect_qrect([0,0,0,0,0]) }
		#assert( TestObject1.func_qrect_qrect( [0,0,0,0] ) == [0,0,0,0] )
		assert( TestObject1.func_qrect_qrect( [-1,-2,3,4] ) == [-1,-2,3,4] )
		assert( TestObject1.func_qrect_qrect( [1,2,-3,-4] ) == [1,2,-3,-4] )
		assert( TestObject1.func_qrect_qrect( [-10,-20,30,40] ) == [-10,-20,30,40] )
		assert( TestObject1.func_qrect_qrect( [10,20,30,40] ) == [10,20,30,40] )
		assert( TestObject1.func_qrect_qrect( [10,20,-30,-40] ) == [10,20,-30,-40] )
	end

	def testRectF
		assert_raises(TypeError) { TestObject1.func_qrectf_qrectf("") }
		assert_raises(TypeError) { TestObject1.func_qrectf_qrectf(123) }
		assert_raises(TypeError) { TestObject1.func_qrectf_qrectf(456.789) }
		assert_raises(TypeError) { TestObject1.func_qrectf_qrectf([]) }

		#assert_raises(TypeError) { TestObject1.func_qrectf_qrectf(nil) }
		#assert_raises(TypeError) { TestObject1.func_qrectf_qrectf([0.0,0.0,0.0]) }
		#assert_raises(TypeError) { TestObject1.func_qrectf_qrectf([0.0,0.0,0.0,0.0,0.0]) }
		#assert( TestObject1.func_qrectf_qrectf( [0.0,0.0,0.0,0.0] ) == [0.0,0.0,0.0,0.0] )
		assert( TestObject1.func_qrectf_qrectf( [-1.0,-2.0,3.0,4.0] ) == [-1.0,-2.0,3.0,4.0] )
		assert( TestObject1.func_qrectf_qrectf( [-1.1,-2.2,3.3,4.4] ) == [-1.1,-2.2,3.3,4.4] )
	end

	def testColor
		assert( TestObject1.func_qcolor_qcolor(nil) == nil )
		assert( TestObject1.func_qcolor_qcolor("") == nil )
		assert( TestObject1.func_qcolor_qcolor("#0066ff") == "#0066ff" )
	end

	def testUrl
		#assert( TestObject1.func_qurl_qurl(nil) == nil )
		assert( TestObject1.func_qurl_qurl("") == "" )
		assert( TestObject1.func_qurl_qurl("/home/myuser") == "/home/myuser" )
		assert( TestObject1.func_qurl_qurl("file:///home/myuser/myfile.txt") == "file:///home/myuser/myfile.txt" )
		assert( TestObject1.func_qurl_qurl("http://myname:mypass@kross.dipe.org?404.cgi?test=123&test2=456") == "http://myname:mypass@kross.dipe.org?404.cgi?test=123&test2=456" )
		assert( TestObject1.func_kurl_kurl("http://www.kde.org/whatiskde/") == "http://www.kde.org/whatiskde/" )
		assert( ! TestObject1.func_kurl_qstring("http://www.kde.org/whatiskde/") )
	end

	def testStringList
		assert_raises(TypeError) { TestObject1.func_qstringlist_qstringlist("") }
		assert_raises(TypeError) { TestObject1.func_qstringlist_qstringlist(0) }
		assert_raises(TypeError) { TestObject1.func_qstringlist_qstringlist(true) }
		assert_raises(TypeError) { TestObject1.func_qstringlist_qstringlist(false) }
		assert_raises(TypeError) { TestObject1.func_qstringlist_qstringlist(-32.85) }

		assert( TestObject1.func_qstringlist_qstringlist( nil ) == [] )
		assert( TestObject1.func_qstringlist_qstringlist( [] ) == [] )
		assert( TestObject1.func_qstringlist_qstringlist( ["string1"] ) == ["string1"] )
		assert( TestObject1.func_qstringlist_qstringlist( [" string1","string2 "] ) == [" string1","string2 "] )
	end

	def testVariantList
		assert_raises(TypeError) { TestObject1.func_qvariantlist_qvariantlist("") }
		assert_raises(TypeError) { TestObject1.func_qvariantlist_qvariantlist(0) }
		assert_raises(TypeError) { TestObject1.func_qvariantlist_qvariantlist(true) }
		assert_raises(TypeError) { TestObject1.func_qvariantlist_qvariantlist(false) }
		assert_raises(TypeError) { TestObject1.func_qvariantlist_qvariantlist(-32.85) }

		#assert_raises(TypeError) { TestObject1.func_qvariantlist_qvariantlist(nil) }
		assert( TestObject1.func_qvariantlist_qvariantlist( [] ) == [] )
		assert( TestObject1.func_qvariantlist_qvariantlist( [[[[]],[]]] ) == [[[[]],[]]] )
		assert( TestObject1.func_qvariantlist_qvariantlist( ["A string",[17539,-8591],[5.32,-842.775]] ) == ["A string",[17539,-8591],[5.32,-842.775]] )

		l = TestObject1.func_qobject2qvariantlist(TestObject2)
		assert( l[0].objectName == TestObject2.objectName )
		assert( TestObject1.func_qvariantlist2qobject(l).objectName == TestObject2.objectName )
	end
 
	def testVariantMap
		#assert_raises(TypeError) { TestObject1.func_qvariantmap_qvariantmap(nil) }
		#assert( {} )
		#assert( {"1":73682,"2":285} )
		#assert( {"a":-6892.957,"b":692.66} )
		#assert( {"key1":True,"key2":False} )
		#assert( {"key 1":"  Some String  ","key 2":"oThEr StRiNg"} )
		#assert( {" key1 ":[12.5,True]," key2 ":[83.002,"test"]} )
	end

	def testVariant
		assert( TestObject1.func_qvariant_qvariant(0.0) == 0.0 )
		assert( TestObject1.func_qvariant_qvariant(true) == true )
		assert( TestObject1.func_qvariant_qvariant(false) == false )
		assert( TestObject1.func_qvariant_qvariant(187937) == 187937 )
		assert( TestObject1.func_qvariant_qvariant(-69825) == -69825 )
		assert( TestObject1.func_qvariant_qvariant(8632.274) == 8632.274 )
		assert( TestObject1.func_qvariant_qvariant(-8632.351) == -8632.351 )
		assert( TestObject1.func_qvariant_qvariant(" Test \n\r This String $%&\"") == " Test \n\r This String $%&\"")
	end

	def testFunctions
		assert( KrossAction.functionNames().include?("testFunction1") )
		assert( KrossAction.callFunction("testFunction1",[true]) == true )
		assert( KrossAction.callFunction("testFunction1",[false]) == false )
		assert( KrossAction.callFunction("testFunction1",[524]) == 524 )
		assert( KrossAction.callFunction("testFunction1",[-958]) == -958 )
		assert( KrossAction.callFunction("testFunction1",[524.98]) == 524.98 )
		assert( KrossAction.callFunction("testFunction1",[-958.1257]) == -958.1257 )
		assert( KrossAction.callFunction("testFunction1",[""]) == "" )
		assert( KrossAction.callFunction("testFunction1",["  Some\nString  "]) == "  Some\nString  " )
		assert( KrossAction.callFunction("testFunction1",[[]]) == [] )
		assert( KrossAction.callFunction("testFunction1",[["one",23,"two",nil,false]]) == ["one",23,"two",nil,false] )
		assert( KrossAction.functionNames().include?("testFunction2") )
		assert( KrossAction.callFunction("testFunction2",[[],[]]) == [[],[]] )
		assert( KrossAction.callFunction("testFunction2",[[1,"test"],nil]) == [[1,"test"],nil] )

		#TODO this results in a crach in Ruby 1.8.6 in the callcache code. Probably related to the issue Cyrille jumped at?
		#puts KrossAction.callFunction("testFunction1",[KrossAction])

    end

	def testObject
		assert( KrossAction.name() == "MyAction" )
		assert( KrossAction.interpreter() == "ruby" )

		assert( TestObject1.name() == "TestObject1" )
		assert( TestObject2.name() == "TestObject2" )

		assert( TestObject1.func_qobject_qobject(true) == nil )
		assert( TestObject1.func_qobject_qobject("String") == nil )
		assert( TestObject1.func_qobject_qobject(nil) == nil )

		assert( TestObject1.func_bool_bool(true) == TestObject2.func_bool_bool(true) )
		assert( TestObject2.func_bool_bool(false) == TestObject2.func_bool_bool(false) )
		assert( TestObject1.func_int_int(82396) == TestObject2.func_int_int(82396) )
		assert( TestObject1.func_int_int(-672) == TestObject2.func_int_int(-672) )
		assert( TestObject1.func_qstringlist_qstringlist( ["s1","s2"] ) == TestObject2.func_qstringlist_qstringlist( ["s1","s2"] ) )

		assert_raises(NameError) { TestObject1.ThisDoesNotExist }
		assert_raises(TypeError) { TestObject1.findChild() }
		assert_raises(TypeError) { TestObject1.findChild(123) }
		assert_raises(TypeError) { TestObject1.findChild(nil,nil) }

		assert( TestObject1.TestTimer.objectName() == "TestTimer" )
		assert( TestObject1.findChild("TestTimer").objectName() == "TestTimer" )
		assert( TestObject1.findChild(nil,"TestTimer").objectName() == "TestTimer" )
	end

	def testProperties
		TestObject1.boolProperty = true
		assert( TestObject1.boolProperty == true )
		TestObject1.boolProperty = false
		assert( TestObject1.boolProperty == false )

		TestObject1.intProperty = 20
		assert( TestObject1.intProperty == 20 )

		TestObject1.doubleProperty = 7436.671
		assert( TestObject1.doubleProperty == 7436.671 )

		TestObject1.stringProperty = " SoMe StRiNg "
		assert( TestObject1.stringProperty == " SoMe StRiNg " )

		TestObject1.stringListProperty = [ "TestString", " Other String " ]
		assert( TestObject1.stringListProperty == [ "TestString", " Other String " ] )

		TestObject1.listProperty = [ true, [2464, -8295], -572.07516, "test", [] ]
		assert( TestObject1.listProperty == [ true, [2464, -8295], -572.07516, "test", [] ] )
	end

	def testPropertyMethods
		TestObject1.setProperty("boolProperty", false)
		assert( TestObject1.boolProperty == false )
		TestObject1.setProperty("boolProperty", true)
		assert( TestObject1.property("boolProperty") == true )
		TestObject1.setProperty("intProperty", -75825)
		assert( TestObject1.property("intProperty") == -75825 )
		TestObject1.setProperty("doubleProperty", -9373.8721)
		assert( TestObject1.property("doubleProperty") == -9373.8721 )
		TestObject1.setProperty("stringProperty", " AnOtHeR sTrInG ")
		assert( TestObject1.property("stringProperty") == " AnOtHeR sTrInG " )
		TestObject1.setProperty("stringListProperty", ["s1","s2",""])
		assert( TestObject1.property("stringListProperty") == ["s1","s2",""] )
		TestObject1.setProperty("listProperty", [[true,false],"abc",[846,-573.02],[]])
		assert( TestObject1.property("listProperty") == [[true,false],"abc",[846,-573.02],[]] )
	end

	def testEnumerations
		assert( TestObject1.TESTENUM1 == 1 )
		assert( TestObject1.TESTENUM2 == 2 )
		assert( TestObject1.TESTENUM3 == 4 )
		assert( TestObject1.TESTENUM4 == 8 )
		#self.assert_( self.object1.testEnum( self.object1.TESTENUM3 ) == 4 )
	end

	def testSignalString
		@mySignalString = ""
		def callbackSignalString(s)
			@mySignalString = s
		end
		TestObject1.connect("signalString(const QString&)", method("callbackSignalString"))
		TestObject1.signalString("Some string")
		assert( @mySignalString == "Some string" )
	end

	def testSignalBool
		@mySignalBool = nil
		def callbackSignalBool(b)
			@mySignalBool = b
		end
		TestObject1.connect("signalBool(bool)", method("callbackSignalBool"))
		TestObject1.signalBool(true)
		assert( @mySignalBool == true )
		TestObject1.signalBool(false)
		assert( @mySignalBool == false )
	end

	#def testSignalObject
	#	@myTestObject = nil
	#	def callbackSignalObject(obj)
	#		@myTestObject = obj
	#	end
	#	TestObject1.connect("signalObject(QObject*)", method("callbackSignalObject"))
	#	TestObject1.signalObject(TestObject2)
	#	assert( @myTestObject == TestObject2 )
	#end

	def testClass
		s = "my string"
		myclass = MyClass.new(s)
		assert(TestObject1.call_krossobject_method(myclass, "myMethod") == s)
	end

	def testOtherObject
		otherobj1 = TestObject1.func_otherobject("OtherObject1")
		otherobj2 = TestObject1.func_otherobject("OtherObject2")

		assert(otherobj1.objectName == "OtherObject1_wrapper")
		assert(otherobj2.parentObject().objectName == "OtherObject2")
		assert(TestObject1.func_otherobject_otherobject(otherobj1).objectName == otherobj1.objectName)

		l = TestObject1.func_otherobjectlist_otherobjectlist( [otherobj1,otherobj2] )
		assert( l.size == 2 )
		assert( l[0].objectName == "OtherObject1_wrapper" )
		assert( l[1].parentObject().objectName == "OtherObject2" )
	end

	def testEvaluation
		assert(KrossAction.evaluate("1+2") == 3)
		assert(KrossAction.evaluate("testFunction1(nil)") == nil)
		assert(KrossAction.evaluate("testFunction1(99)") == 99)
	end

end

require 'test/unit/ui/console/testrunner'
Test::Unit::UI::Console::TestRunner.run(TestKross)

#require "TestObject1"
#puts "=======> " + TestObject1.func_bool_bool(false).to_s()
#puts "=======> " + TestObject1.func_bool_bool(true).to_s()
#puts "=======> " + TestObject1.func_qpoint_qpoint( [0,0] ).inspect()
