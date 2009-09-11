#!/usr/bin/perl

$first = 1;
print "-(";
while($line = <STDIN>){

    if($line =~ /^#/){
       next;
   }

    ($code, $desc) = split(/\s+/, $line);
    if($first == 1){
	print "\L$code";
    }else{
	print "\|\L$code";
    }
    $first = 0;
}
print ")\n";
