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

var a = new Array(11, 22, 33, 44);
a.length = 2;
a[4] = 'apple';
for(i = 0; i != a.length; i++)
  print("a[" + i + "] = " + a[i]);

var b = new Boolean(1==1);
print("b = " + b);
b.toString=Object.prototype.toString;
print("b = " + b.toString());
