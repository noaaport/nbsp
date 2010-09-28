#!/usr/local/bin/tclsh8.6

set a {$map(awips1)};

set body {
   CLASS
     EXPRESSION (\[pixel\] == $level)
     STYLE
       COLOR [::nbsp::gis::radcolor $a $level]
     END
   END
}

set level 0;
while {$level <= 75} {
    puts -nonewline [subst -nobackslashes -nocommands $body];
    incr level;
}
