println('Empty URL');
url = new QUrl();
println('OK' + url);

println('Empty URL');
url = new QUrl('');
println('OK' + url);

println('Http URL');
url = new QUrl('http://www.kde.org/');
println('OK' + url);

println('URL with fragment');
url = new QUrl('http://www.kde.org/#where');
println('OK' + url);
