#!/usr/local/bin/tclsh8.6

set a {$map(awips1)};

set body {
   CLASS
     EXPRESSION (\[pixel\] >= $level1 && \[pixel\] < $level2)
     STYLE
       COLOR [::nbsp::gis::radcolor $a $level]
     END
   END
}

set level1 0;
set level2 1;
set level 0;
while {$level <= 76} {
    puts -nonewline [subst -nobackslashes -nocommands $body];
    incr level1;
    incr level2;
    incr level;
}
