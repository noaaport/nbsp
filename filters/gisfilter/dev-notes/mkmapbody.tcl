#!/usr/local/bin/tclsh8.6

set a {$map(awips1)};

set body {
   CLASS
     EXPRESSION (\[level\] >= $level1 && \[level\] < $level2)
     STYLE
    COLOR [::nbsp::gis::radcolor $a $level1]
     END
   END
}

set level1 0;
set level2 5;
set i 0;
while {$i <= 76} {
    puts -nonewline [subst -nobackslashes -nocommands $body];
    incr level1 5;
    incr level2 5;
    incr i 5;
}
