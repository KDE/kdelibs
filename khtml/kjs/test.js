var i = 0;

function sum(a, b)
{
 print("inside test()");
 i = i + 1;
 print(a);
 print(b);
 return a + b;
}

s = sum(10, sum(20, 30));
print("s = " + s);
print("i = " + i);

var b = new Boolean(1==1);
print("b = " + b);
b.toString=Object.prototype.toString;
print("b = " + b.toString());
