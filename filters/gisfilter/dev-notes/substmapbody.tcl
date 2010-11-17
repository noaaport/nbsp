#!/usr/local/bin/tclsh8.6

#
# Uses the output of mkmapbody.tcl to write an explicit map file such
# as the ones used in the dcnids-tests.
#
set script {

   CLASS
     EXPRESSION (\[level\] >= 0 && \[level\] < 5)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 0]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 5 && \[level\] < 10)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 5]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 10 && \[level\] < 15)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 10]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 15 && \[level\] < 20)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 15]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 20 && \[level\] < 25)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 20]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 25 && \[level\] < 30)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 25]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 30 && \[level\] < 35)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 30]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 35 && \[level\] < 40)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 35]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 40 && \[level\] < 45)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 40]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 45 && \[level\] < 50)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 45]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 50 && \[level\] < 55)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 50]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 55 && \[level\] < 60)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 55]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 60 && \[level\] < 65)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 60]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 65 && \[level\] < 70)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 65]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 70 && \[level\] < 75)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 70]
     END
   END

   CLASS
     EXPRESSION (\[level\] >= 75 && \[level\] < 80)
     STYLE
    COLOR [::nbsp::gis::radcolor $map(awips1) 75]
     END
   END
}

set map(awips1) "n0r";
source "gis.tcl";
nbsp::gis::init "gis-colors.def";
puts [subst $script];
