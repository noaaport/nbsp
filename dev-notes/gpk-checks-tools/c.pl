#!/usr/bin/perl

use Compress::Zlib;

my $input;
my ($output, $status) ;

while (read(STDIN, $input, 4000) > 0)
{
    ($output, $status) = compress($input, 9) ;

    if($status == Z_OK){
	print $output;
    }else{
	last;
    }
}



