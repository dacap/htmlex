#! /usr/bin/env perl
# htmlex-filter.pl
# Copyright (C) 2003 by David A. Capello
#
# Usage:
#   htmlex file.htex | htmlex-filter.pl > file.html
#
# Remove whitespaces outside <pre></pre> tags.

my $c, $chr, $len, $pre, $space;

$pre = 0;

LINE: while (<STDIN>) {
    # remove trailing whitespaces
    s/[\ \t]+$//g;

    # try a fast mode
    if (!$pre) {
	# nothing in this line?
	next LINE if /^[\ \t\r\n]+$/;

	# check is this line doesn't have <pre> tag
	if (!($_ =~ /\<[pP][rR][eE]\>/)) {
	    # remove leading whitespaces
	    s/^[\ \t]+//g;

	    # remove duplicate whitespaces
	    s/[\ \t]+/ /g;

	    print $_;
	    next LINE;
	}
    }

    $len = length;
    $space = 1;

    for ($c=0; $c<$len; $c++) {
	$chr = substr ($_, $c, 1);
	if ($chr =~ /[\ \t\r\n]+/) {
	    if (!$pre) {
		if (!$space) {
		    $space = 1;
		    if ($chr eq "\t") {
			print (" ");
		    }
		    else {
			print ($chr);
		    }
		}
	    }
	    else {
		print ($chr);
	    }
	}
	else {
	    if ($space) {
		$space = 0;
	    }

	    if ($chr eq '<') {
		if (substr ($_, $c, 5) =~ /\<[pP][rR][eE]\>/) {
		    print ("<pre>");
		    $pre++;
		    $c += 4;
		}
		elsif (substr ($_, $c, 6) =~ /\<\/[pP][rR][eE]\>/) {
		    print ("</pre>");
		    $pre--;
		    $c += 5;
		}
		else {
		    print ($chr);
		}
	    }
	    else {
		print ($chr);
	    }
	}
    }
}
