function test() {
	this.method1 = function(a) { println(a);}
	this.method2 = function(a) { println(a);}
	this.method3 = function(a) { println(a);}
}

var blah = new test();

for( x in blah )
{
	println(x);
}
