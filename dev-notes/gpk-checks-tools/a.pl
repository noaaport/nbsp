#!/usr/bin/perl

use Compress::Zlib;

binmode STDIN;
binmode STDOUT;

my $x = deflateInit(-Bufsize=>4000, -Level=>9) 
    or die "Cannot create a deflation stream\n" ;

my ($output, $status) ;

while (<>)
{
    ($output, $status) = $x->deflate($_) ;
    
    $status == Z_OK
	or die "deflation failed\n" ;
    
    print $output ;
}

($output, $status) = $x->flush() ;

$status == Z_OK
    or die "deflation failed\n" ;

print $output ;
