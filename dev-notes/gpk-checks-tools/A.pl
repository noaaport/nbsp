#!/usr/bin/perl

use Compress::Zlib;

@body = <STDIN>;
$body = join("", @body);
$cbody = compress($body, 9);

print $cbody;
