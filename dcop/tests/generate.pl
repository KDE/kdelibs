#!/usr/bin/perl -w

use strict;


my $DERIVED_CLASS = 'Test';
my $BASE_CLASS = 'Test';
my $DCOP = '../client/dcop';
my $TEST_APP = '$TEST_APP';
my $OBJECT_NAME = 'TestInterface';


sub shell_header {
	print SHELL "TEST_APP=\`$DCOP 'TestApp-*'\`\n";
}
sub shell_footer {
	print SHELL "$DCOP $TEST_APP 'TestInterface' quit";
}

sub test_h_header {
print HEADER <<END;

#ifndef MY_INTERFACE_H
#define MY_INTERFACE_H

/* This is combined with test.cpp and run_test a test case for dcop
 * family of programms
 */

// some comment
#include <dcopobject.h>
#include <dcopref.h>

class Test2;

// this has to work too
#include <qstring.h>
#include <qapplication.h>



namespace MyNamespace {
	struct MyParentClass {
		int x;
	};
}

class Test : public MyNamespace::MyParentClass, virtual public DCOPObject
{
    K_DCOP

public:
    Test() : DCOPObject("TestInterface") {}
    ~Test();
k_dcop:
    void quit() { qApp->quit(); }

END

}

sub test_h_footer {
print HEADER <<END;

};

#endif // end

END
}



sub getline {
	local $_;
	until (eof()) {
		last unless defined($_ = <>);
		chomp;
		next if /^\s*#/;
		return $_;
	}
	return '';
}

#sub getline {
#	print STDERR "HERE2\n";
#	my $res = &getlinereal;
#	print STDERR "G->" . $res . "<-\n";
#	return $res;
#}


# main();

open(HEADER,'>test.h');
open(IMPLEMENTATION,'>definitions.generated');
open(BATCH, '>batch.generated');
open(SHELL,'>shell.generated');
open(DRIVER,'>driver.generated');

&test_h_header;
&shell_header;
my $previous_comment;
my $i = 0;
my $i_1 = 1;
until (eof()) {
	my $comment = &getline;
	next if $comment eq '';
	my $return_type = $comment;
	$previous_comment = $comment;
	if ($comment =~ m#\s*//#) { $return_type = &getline; }
	else { $comment = $previous_comment; }
	my $function_name = &getline;
	my $argument_types = &getline;
	my $function_body = '';
	my $line;
	do { 
		$line = &getline;
		$function_body .= $line;
	} until ($line =~ /^}/);
	my @shell_args = ();
	my @cpp_args = ();
	print STDERR "Working on function $function_name$argument_types\n";
	while (1) {
		local $_ = &getline;
		chomp;
		print STDERR "Looking at -$_-\n";
		die 'I/O Error' if eof();
		die 'I/O Error' unless defined($_); # should catch all previous IO errors as well

		if (/^\s*$/ || /^-$/ ) {
			if (scalar(@shell_args) == 0) {
				@shell_args = ('');
				@cpp_args = ('');
				print STDERR "Function $function_name$argument_types: No arguments\n";
			}
			last;
		}
		/^(.*);(.*)$/ or last;
		push @shell_args, ($1);
		push @cpp_args, ($2);
		print STDERR "Function $function_name$argument_types: args {-$1-} {-$2-}\n";
		print STDERR "Function $function_name$argument_types: so far ", scalar(@shell_args), "\n";
	}


	$comment =~ s#^\s*//##;

	print HEADER <<END;
	// $comment
	virtual $return_type ${function_name}_virtual $argument_types;
	 $return_type ${function_name} $argument_types;
END

	print IMPLEMENTATION <<END;
	$return_type $DERIVED_CLASS :: ${function_name}_virtual $argument_types
		$function_body

	 $return_type $BASE_CLASS :: ${function_name} $argument_types
		 $function_body
END

	my $cpp_cur = scalar(shift @cpp_args or '');
	my $shell_cur = scalar(shift @shell_args or '');
	
	if ($return_type ne 'void') {
		print BATCH <<END;
	output << "$return_type $function_name($argument_types)\\n{\\n";
	output << "// $comment\\n";
	output << object->$function_name($cpp_cur) << "\\n}\\n";
	output << "$return_type ${function_name}_virtual($argument_types)\\n{\\n";
	output << "// $comment\\n";
	output << object->${function_name}_virtual($cpp_cur) << "\\n}\\n";
END
	} else {
		print BATCH <<END;
	// Void Functions:
	object->$function_name();
	object->${function_name}_virtual();
END
	}
	
	print SHELL <<END;
	echo "$return_type $function_name($argument_types)"
	echo "{"
	echo "// $comment";
	$DCOP $TEST_APP $OBJECT_NAME $function_name $shell_cur
	echo "}"
	echo "$return_type ${function_name}_virtual($argument_types)"
	echo "{"
	echo "// $comment";
	$DCOP $TEST_APP $OBJECT_NAME ${function_name}_virtual $shell_cur
	echo "}"
END

	print DRIVER <<END;
		case $i:
			output << "$return_type $function_name($argument_types)\\n{" << endl;
			output << "// $comment" << endl;
			output << object->$function_name($cpp_cur) << endl;
			output << '}' << endl;
			break;
		case $i_1:
			output << "$return_type ${function_name}_virtual($argument_types)\\n{" << endl;
			output << "// $comment" << endl;
			output << object->${function_name}_virtual($cpp_cur) << endl;
			output << '}' << endl;
			break;
END
	$i += 2;
	$i_1 = $i + 1;
}

&test_h_footer;
&shell_footer;

close HEADER;
close IMPLEMENTATION;
close BATCH;
close SHELL;
close DRIVER;

1;
