
clean_up() {
	rm -f batch.stdout shell.stdout shell.returns batch.returns
}

clean_up

echo '* Running batch mode'
./dcop_test --batch >batch.stdout

echo -n '* Starting test app '
./dcop_test >shell.stdout &

while ! ../client/dcop | grep -q TestApp; do echo -n '.'; sleep 2; done

echo ' started'

echo '* Running driver mode'
./driver `../client/dcop 'TestApp-*'` >driver.stdout

echo '* Running shell mode'
source shell.generated >shell.returns

echo -n '* Comparing ... '

compare() 
{
if ! diff -q --strip-trailing-cr $1 $2; then
	echo "FAILED:"
	diff -u $1 $2
	exit 1;
fi
}

compare batch.stdout shell.stdout
compare batch.stdout driver.stdout
compare batch.returns shell.returns
compare batch.returns driver.returns

clean_up

echo "Passed"
exit 0;

