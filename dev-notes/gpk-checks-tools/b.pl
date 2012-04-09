#!/usr/bin/perl

use Compress::Zlib;

binmode STDIN;
binmode STDOUT;

my $x = inflateInit() 
    or die "Cannot create an inflation stream\n" ;

my $input = '';
my ($output, $status) ;

while (read(STDIN, $input, 4000))
{
    ($output, $status) = $x->inflate(\$input) ;

    print $output
	if $status == Z_OK or $status == Z_STREAM_END ;

    last if $status != Z_OK ;
}

die "inflation failed\n"
    unless $status == Z_STREAM_END ;


