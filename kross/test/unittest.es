#!/usr/bin/env kross

// Print something on the console.
println("Let's start the Unittest :-)");

function UnitTest()
{
    var numpassed = 0;
    var numfailed = 0;

    this.passed = function() {
        numpassed = numpassed + 1;
    }

    this.failed = function(actual, expected) {
        print("FAILED actual=" + actual + " expected=" + expected);
        numfailed = numfailed + 1;
    }

    this.missingException = function(message) {
        print("MISSING EXCEPTION: " + message);
        numfailed = numfailed + 1;
    }

    this.unexpectedException = function(message, exception) {
        print("UNEXPECTED EXCEPTION: " + message + "\n" + exception);
        numfailed = numfailed + 1;
    }

    this.assert = function(actual, expected) {
        if(actual == expected)
            this.passed();
        else
            this.failed(actual, expected);
    }

    this.assertArray = function(actual, expected) {
        if(actual.length != expected.length) {
            this.failed(actual, expected);
            print("Array-Length does not match: actual=" + actual.length + " expected=" + expected.length);
        }
        else {
            var failed = false;
            for(i=0;i<actual.length;i++) {
                if(actual[i] != expected[i]) {
                    failed = true;
                    this.failed(actual, expected);
                    print("Array-Item actual[i]=" + actual[i] + " expected[i]=" + expected[i]);
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
                print("Map-Item i=" + i + " is unexpected");
                break;
            }
            if(actual[i] != expected[i]) {
                failed = true;
                this.failed(actual, expected);
                print("Map-Item i=" + i + " actual[i]=" + actual[i] + " expected[i]=" + expected[i]);
                break;
            }
        }
        if(! failed) {
            for(i in expected) {
                if(actual[i] == undefined) {
                    failed = true;
                    this.failed(actual, expected);
                    print("Map-Item i=" + i + " is missing");
                    break;
                }
            }
            if(! failed)
                this.passed();
        }
    }

    this.printResult = function() {
        print("--------------------------------------------");
        print("Tests passed: " + numpassed);
        print("Tests failed: " + numfailed);
        print("--------------------------------------------");
    }
}

tester = new UnitTest();

// We have 2 instances of TestObject which inherit QObject.
var testobj1 = TestObject1
var testobj2 = TestObject2

// object
tester.assert(testobj1.name(), "TestObject1");
tester.assert(testobj2.name(), "TestObject2");

// bool
tester.assert(testobj1.func_bool_bool(true), true);
tester.assert(testobj1.func_bool_bool(false), false);

// int
tester.assert(testobj1.func_int_int(0), 0);
tester.assert(testobj1.func_int_int(177321), 177321);
tester.assert(testobj1.func_int_int(-98765), -98765);

// uint
tester.assert(testobj1.func_uint_uint(0), 0);
tester.assert(testobj1.func_uint_uint(177321), 177321);

// double
tester.assert(testobj1.func_double_double(0.0), 0.0);
tester.assert(testobj1.func_double_double(1773.2177), 1773.2177);
tester.assert(testobj1.func_double_double(-548993.271993), -548993.271993);

// longlong
tester.assert(testobj1.func_qlonglong_qlonglong(0), 0);
tester.assert(testobj1.func_qlonglong_qlonglong(7379), 7379);
tester.assert(testobj1.func_qlonglong_qlonglong(-6384673), -6384673);

// ulonglong
tester.assert(testobj1.func_qulonglong_qulonglong(0), 0);
tester.assert(testobj1.func_qulonglong_qulonglong(378972), 378972);

// bytearray
//tester.assert(testobj1.func_qbytearray_qbytearray("  Some String as ByteArray  "), "  Some String as ByteArray  ");
//tester.assert(testobj1.func_qbytearray_qbytearray(" \0\n\r\t\s\0 test "), " \0\n\r\t\s\0 test ");

// string
tester.assert(testobj1.func_qstring_qstring(""), "");
tester.assert(testobj1.func_qstring_qstring(" "), " ");
tester.assert(testobj1.func_qstring_qstring(" Another \n\r Test!   $%&\" "), " Another \n\r Test!   $%&\" ");

// stringlist
tester.assertArray(testobj1.func_qstringlist_qstringlist(new Array()), new Array());
tester.assertArray(testobj1.func_qstringlist_qstringlist(new Array("s1","s2")), new Array("s1","s2"));
tester.assertArray(testobj1.func_qstringlist_qstringlist([]), []);
tester.assertArray(testobj1.func_qstringlist_qstringlist(["abc","def"]), ["abc","def"]);

// variantlist
tester.assertArray(testobj1.func_qvariantlist_qvariantlist(new Array()), new Array());
tester.assertArray(testobj1.func_qvariantlist_qvariantlist(new Array("s1","s2",17,-95)), new Array("s1","s2",17,-95));
tester.assertArray(testobj1.func_qvariantlist_qvariantlist([]), []);
tester.assertArray(testobj1.func_qvariantlist_qvariantlist(["abc","def",426,-842,96.23,-275.637]), ["abc","def",426,-842,96.23,-275.637]);

// variantmap
var v = new Array;
v["key2"] = "";
v["key1"] = " MyValue ";
v["key5"] = true;
v["key6"] = false;
v["key3"] = 1764;
v["key4"] = -8772;
v["key7"] = 978.216;
v["key8"] = -692.967;
v["key10"] = ["one","two"];
v["key9"] = [];
tester.assertMap(testobj1.func_qvariantmap_qvariantmap(v), v)
tester.assertMap(testobj1.func_qvariantmap_qvariantmap([]), [])

// misc tests
tester.assert(self.callFunction("name"), self.name());
tester.assert(self.callFunction("code"), self.code());
tester.assert(self.callFunction("isEnabled"), self.isEnabled());

//TODO work around it somehow!
var stringlist = self.callFunction("functionNames");
//println( stringlist.length ); // numbers of chars in stringlist.toString() rather then number of items
//tester.assertArray(self.callFunction("functionNames"), self.functionNames()); //this fails
//println(stringlist.prototype); //undefined
//println(stringlist instanceof Object); //false
//println(stringlist instanceof String); //false
//println(stringlist instanceof Array); //false
//println(stringlist instanceof Variant); //false
//for(a in stringlist) { println(a); } //each char, seems it's now handled as .toString()
//println(stringlist); //prints the correct output of the stringlist, just like .toString()
//println(stringlist.toString()); //prints the correct output of the stringlist as string

//println( testobj1.func_qvariant_qvariant( Variant(new Array("One","Two")) ) );
//println( Variant(new Array("One","Two")) );
//println( Variant("test"));

self.callFunction("setIconName",new Array("MyIconName"));
tester.assert(self.callFunction("iconName"), "MyIconName");

var objectCountBefore = self.qobjectNames().length;
self.addQObject(self);
tester.assert(objectCountBefore + 1, self.qobjectNames().length);
tester.assert(self.qobject(self.name()).file(), self.file());

// print the test-results
tester.printResult();
