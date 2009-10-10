#
# $Id$
#
# This is used to generated conf.c from conf.in
#
BEGIN {
  FS = "[ \t]+";
  counter = 1;
  
  print "/*";
  print " * This file is automatically generated from conf.c.in";
  print " */";
}

/@counter@/{
  gsub("@counter@", counter);
  ++counter;
}

{
  print $0;
}

