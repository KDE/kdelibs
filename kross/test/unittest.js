#!/usr/bin/env krosstest

// Print something on the console.
println("Let's start the KjsEmbed Unittest :-)");

function UnitTest()
{
    var numpassed = 0;
    var numfailed = 0;

    this.passed = function() {
        numpassed = numpassed + 1;
    }

    this.failed = function(actual, expected) {
        println("FAILED actual=" + actual + " expected=" + expected);
        numfailed = numfailed + 1;
    }

    this.missingException = function(message) {
        println("MISSING EXCEPTION: " + message);
        numfailed = numfailed + 1;
    }

    this.unexpectedException = function(message, exception) {
        println("UNEXPECTED EXCEPTION: " + message + "\n" + exception);
        numfailed = numfailed + 1;
    }

    this.assert = function(actual, expected) {
        if(actual instanceof Array)
            this.assertArray(actual, expected);
        else if(actual == expected)
            this.passed();
        else
            this.failed(actual, expected);
    }

    this.assertArray = function(actual, expected) {
        if(actual.length != expected.length) {
            this.failed(actual, expected);
            println("Array-Length does not match. Actual=" + actual.length + " Expected=" + expected.length);
        }
        else {
            var failed = false;
            for(i=0;i<actual.length;i++) {
                if(actual[i] != expected[i]) {
                    failed = true;
                    this.failed(actual, expected);
                    println("Array-Item actual[i]=" + actual[i] + " expected[i]=" + expected[i]);
                    break;
                }
            }
            if(! failed)
                this.passed();
        }
    }

    this.assertMap = function(actual, expected) {
        var failed = false
        for(i in actual) {
            if(expected[i] == undefined) {
                failed = true;
                this.failed(actual, expected);
                println("Map-Item i=" + i + " is unexpected");
                break;
            }
            if(actual[i] != expected[i]) {
                failed = true;
                this.failed(actual, expected);
                println("Map-Item i=" + i + " actual[i]=" + actual[i] + " expected[i]=" + expected[i]);
                break;
            }
        }
        if(! failed) {
            for(i in expected) {
                if(actual[i] == undefined) {
                    failed = true;
                    this.failed(actual, expected);
                    println("Map-Item i=" + i + " is missing");
                    break;
                }
            }
            if(! failed)
                this.passed();
        }
    }

    this.printResult = function() {
        println("--------------------------------------------");
        println("Tests passed: " + numpassed);
        println("Tests failed: " + numfailed);
        println("--------------------------------------------");
    }
}

tester = new UnitTest();

// We have 2 instances of TestObject which inherit QObject.
var testobj1 = TestObject1
var testobj2 = TestObject2

// object
{
    tester.assert(testobj1, "TestObject");
    tester.assert(testobj2, "TestObject");
    tester.assert(testobj1.name(), "TestObject1");
    tester.assert(testobj2.name(), "TestObject2");

    try {
        testobj1.noSuchMethodName()
        tester.missingException("testobj1.noSuchMethodName()");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_void_qobject(testobj2);
        tester.passed();
    } catch(error) { tester.unexpectedException("testobj1.func_void_qobject(testobj2)", error); }

    try {
        obj1 = testobj1.func_createChildTestObject("MyChildObject")
        tester.passed();
        tester.assert(obj1.name(), "MyChildObject");
    } catch(error) { tester.unexpectedException("testobj1.func_createChildTestObject(\"MyChildObject\")", error); }

    try {
        testobj3 = testobj1.func_qobject_qobject(testobj2);
        tester.passed();
        tester.assert(testobj2.name(), testobj3.name());
    } catch(error) { tester.unexpectedException("testobj1.func_qobject_qobject(testobj2)", error); }
}

// bool
{
    tester.assert(testobj1.func_bool_bool(true), true);
    tester.assert(testobj1.func_bool_bool(false), false);

    try {
        testobj1.func_bool_bool(17);
        tester.missingException("testobj1.func_bool_bool(17)");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_bool_bool("SomeString");
        tester.missingException("testobj1.func_bool_bool(\"SomeString\")");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_bool_bool([]);
        tester.missingException("testobj1.func_bool_bool([])");
    } catch(error) { tester.passed(); }
}

// int
{
    tester.assert(testobj1.func_int_int(0), 0);
    tester.assert(testobj1.func_int_int(177321), 177321);
    tester.assert(testobj1.func_int_int(-98765), -98765);

    try {
        testobj1.func_int_int("SomeString")
        tester.missingException("testobj1.func_int_int(\"SomeString\")");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_int_int("SomeString")
        tester.missingException("testobj1.func_int_int(\"SomeString\")");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_int_int(true)
        tester.missingException("testobj1.func_int_int(true)");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_int_int(false)
        tester.missingException("testobj1.func_int_int(false)");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_int_int([])
        tester.missingException("testobj1.func_int_int([])");
    } catch(error) { tester.passed(); }
}

// uint
{
    tester.assert(testobj1.func_uint_uint(0), 0);
    tester.assert(testobj1.func_uint_uint(177321), 177321);

    try {
        testobj1.func_uint_uint("SomeString")
        tester.missingException("testobj1.func_uint_uint(\"SomeString\")");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_uint_uint(true)
        tester.missingException("testobj1.func_uint_uint(true)");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_uint_uint(false)
        tester.missingException("testobj1.func_uint_uint(false)");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_uint_uint([])
        tester.missingException("testobj1.func_uint_uint([])");
    } catch(error) { tester.passed(); }
}

// longlong
{
    tester.assert(testobj1.func_qlonglong_qlonglong(0), 0);
    tester.assert(testobj1.func_qlonglong_qlonglong(7379), 7379);
    tester.assert(testobj1.func_qlonglong_qlonglong(-6384673), -6384673);
}

// ulonglong
{
    tester.assert(testobj1.func_qulonglong_qulonglong(0), 0);
    tester.assert(testobj1.func_qulonglong_qulonglong(378972), 378972);
}

// double
{
    tester.assert(testobj1.func_double_double(0.0), 0.0);
    tester.assert(testobj1.func_double_double(1773.2177), 1773.2177);
    tester.assert(testobj1.func_double_double(-548993.271993), -548993.271993);

    try {
        testobj1.func_double_double("SomeString")
        tester.missingException("testobj1.func_double_double(\"SomeString\")");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_double_double(true)
        tester.missingException("testobj1.func_double_double(true)");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_double_double(false)
        tester.missingException("testobj1.func_double_double(false)");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_double_double([])
        tester.missingException("testobj1.func_double_double([])");
    } catch(error) { tester.passed(); }
}

// bytearray
{
    tester.assert(testobj1.func_qbytearray_qbytearray("  Some String as ByteArray  "), "  Some String as ByteArray  ");
    //tester.assert(testobj1.func_qbytearray_qbytearray(" \0\n\r\t\s\0 test "), " \0\n\r\t\s\0 test ");
}

// string
{
    tester.assert(testobj1.func_qstring_qstring(""), "");
    tester.assert(testobj1.func_qstring_qstring(" "), " ");
    tester.assert(testobj1.func_qstring_qstring(" Another \n\r Test!   $%&\" "), " Another \n\r Test!   $%&\" ");

    try {
        testobj1.func_qstring_qstring(-42.331)
        tester.missingException("testobj1.func_qstring_qstring(-42.331)");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_qstring_qstring(true)
        tester.missingException("testobj1.func_qstring_qstring(true)");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_qstring_qstring(false)
        tester.missingException("testobj1.func_qstring_qstring(false)");
    } catch(error) { tester.passed(); }

    try {
        testobj1.func_qstring_qstring([])
        tester.missingException("testobj1.func_qstring_qstring([])");
    } catch(error) { tester.passed(); }
}

// stringlist
{
    tester.assertArray(testobj1.func_qstringlist_qstringlist(new Array()), new Array());
    tester.assertArray(testobj1.func_qstringlist_qstringlist(new Array("s1","s2")), new Array("s1","s2"));
    tester.assertArray(testobj1.func_qstringlist_qstringlist([]), []);
    tester.assertArray(testobj1.func_qstringlist_qstringlist(["abc","def"]), ["abc","def"]);
}

// color
{
    tester.assert(testobj1.func_qcolor_qcolor("#0066ff"), "#0066ff");

    var c = new QColor();
    c.setNamedColor("#caffee");
    tester.assert(testobj1.func_qcolor_qcolor(c), "#caffee");
}

// size
{
    var s = new QSize(11,22);
    t = testobj1.func_qsize_qsize(s);
    tester.assert(t.width()+":"+t.height(), s.width()+":"+s.height());
}

// point
{
    var p = new QPoint(11,22);
    q = testobj1.func_qpoint_qpoint(p);
    tester.assert(q.x()+":"+q.y(), p.x()+":"+p.y());
}

// rect
{
    var r = new QRect(11,22,33,44);
    s = testobj1.func_qrect_qrect(r);
    tester.assert(r.left()+":"+r.top()+":"+r.right()+":"+r.bottom(), s.left()+":"+s.top()+":"+s.right()+":"+s.bottom());
}

// variantlist
{
    tester.assertArray(testobj1.func_qvariantlist_qvariantlist(new Array()), new Array());
    tester.assertArray(testobj1.func_qvariantlist_qvariantlist(new Array("s1","s2",17,-95)), new Array("s1","s2",17,-95));
    tester.assertArray(testobj1.func_qvariantlist_qvariantlist([]), []);
    tester.assertArray(testobj1.func_qvariantlist_qvariantlist(["abc","def",426,-842,96.23,-275.637]), ["abc","def",426,-842,96.23,-275.637]);

    obj = TestObject1.func_qvariantlist2qobject( TestObject1.func_qobject2qvariantlist(TestObject2) )
    tester.assert(obj.name(), TestObject2.name());
    obj = TestObject1.func_qvariantlist2qobject( TestObject1.func_qobject2qvariantlist(TestObject2.myself()) )
    tester.assert(obj.name(), TestObject2.myself().name());
}

// variantmap
{
    var v = new Array;
    v["key2"] = "";
    v["key1"] = " MyValue ";
    v["key5"] = true;
    v["key6"] = false;
    v["key3"] = 1764;
    v["key4"] = -8772;
    v["key7"] = 978.216;
    v["key8"] = -692.967;
    //v["key9"] = [];
    //v["key10"] = ["one","two"];
    tester.assertMap(testobj1.func_qvariantmap_qvariantmap(v), v)
    tester.assertMap(testobj1.func_qvariantmap_qvariantmap([]), [])
}

// variant
{
    tester.assert(testobj1.func_qvariant_qvariant(true), true);
    tester.assert(testobj1.func_qvariant_qvariant(1892), 1892);
    tester.assert(testobj1.func_qvariant_qvariant(8529.54), 8529.54);
    tester.assert(testobj1.func_qvariant_qvariant(" String "), " String ");
    tester.assert(testobj1.func_qvariant_qvariant(["One","Two"]), ["One","Two"]);
    //tester.assert(testobj1.func_qvariant_qvariant(null), null);
}

// url
{
    tester.assert(testobj1.func_qurl_qurl("/home/me"), "/home/me");
    tester.assert(testobj1.func_qurl_qurl("file://home/me/myfile.txt"), "file://home/me/myfile.txt");
    tester.assert(testobj1.func_qurl_qurl("http://nowhere.anywhere/path/file?arg=val"), "http://nowhere.anywhere/path/file?arg=val");

    tester.assert(testobj1.func_qurl_kurl("/home/otheruser/"), "/home/otheruser/");

    //TODO following crashes in kjsembed;
    //tester.assert(testobj1.func_kurl_kurl("/home/otheruser/"), "/home/otheruser/");
    //tester.assert(testobj1.func_kurl_kurl("http://www.kde.org/whatiskde/test?123"), "http://www.kde.org/whatiskde/test?123");
}

// properties
{
    testobj1.intProperty = 5278;
    tester.assert(testobj1.intProperty, 5278);

    testobj1.doubleProperty = 928.581;
    tester.assert(testobj1.doubleProperty, 928.581);

    testobj1.stringProperty = "StRiNg";
    tester.assert(testobj1.stringProperty, "StRiNg");
}

// enumerations
{
    tester.assert(testobj1.TESTENUM1, 1);
    tester.assert(testobj1.TESTENUM2, 2);
}

// callback functions
{
    function myEmptyFunction() {}
    tester.assert(self.callFunction("myEmptyFunction"), null);

    function myValueFunction(arg) { return arg; }
    tester.assert(self.callFunction("myValueFunction", [true]), true);
    tester.assert(self.callFunction("myValueFunction", [1892]), 1892); // take care here, something like "new Array(12)" means array with 12 elements while "new Array(12,13)" means the 2 elements 12 and 13 :-/
    tester.assert(self.callFunction("myValueFunction", [8529.54]), 8529.54);
    tester.assert(self.callFunction("myValueFunction", ["Some String Again"]), "Some String Again");

    tester.assertArray(self.callFunction("myValueFunction", [ new Array() ]), new Array());
    tester.assertArray(self.callFunction("myValueFunction", [ [] ]), []);
    tester.assertArray(self.callFunction("myValueFunction", [ ["One","Two"] ]), ["One","Two"]);
}

// signals and slots
{
    var slottestresult = null;
    function mySlotFunction(arg) { slottestresult = arg; }

    connect(testobj1, "signalString(QString)", this, "mySlotFunction(QString)");
    testobj1.signalString("more beer");
    tester.assert(slottestresult, "more beer");

    connect(testobj1, "signalObject(QObject*)", this, "mySlotFunction(QObject*)");
    testobj1.signalObject(testobj2);
    tester.assert(slottestresult.objectName, testobj2.objectName);
}

// evaluate functionality
{
    tester.assert(self.evaluate("1+2"), 3);
    function evalCalc(arg) { return arg; }
    tester.assert(self.evaluate("evalCalc(null)"), null);
    tester.assert(self.evaluate("evalCalc(99)"), 99);
}

// print the test-results
tester.printResult();

/*
f = Kross.module('forms');
d = f.createDialog('TestGuiFormDialog');
d.setButtons('Ok|Cancel');
d.exec_loop();
*/
