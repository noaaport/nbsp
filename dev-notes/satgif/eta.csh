   	</> 	   	   	  Data </content/data/>  |  Tools
</content/software/>  |  Community </content/community/>  |  Projects
</content/projects/>  |  Support </content/support/>  |  About
</content/about/>  

#!/bin/csh -f
setenv DISPLAY unix:0
source $NAWIPS/Gemenviron
#
# Note HDS set in Gemenviron for location of
#     Decoded HRS files


set CURRENT_ETA=`ls -t $HDS/*_eta.gem | head -1`

#
# Generate Gifs in /tmp directory
cd /tmp

if(-e eta_emsl.gif ) then
   rm -f eta_emsl.gif*
endif

gdcntr << EOF
GDATTIM  = f000-f048
GLEVEL   = 0
GVCORD   = PRES
GFUNC    = emsl
GDFILE   = $CURRENT_ETA
CINT     = 4
LINE     = 1/1/2/2
MAP      = 5
TITLE    = 1/-2/ETA Surface Pressure ~
DEVICE   = gf|eta_emsl.gif|350;300|||NP
SATFIL   = 
RADFIL   = 
PROJ     = nps
GAREA    = usnps
CLEAR    = y
PANEL    = 0
TEXT     = 1.2/23/1/hw
SCALE    = 999
LATLON   = 0
HILO     = 4;2/H;L
HLSYM    = 9;1/2/33;2//hw
CLRBAR   = 
CONTUR   = 3/3
SKIP     = 0
FINT     = 
FLINE    = 
CTYPE    = C
\$mapfil = hipowo.gsf
r

e
EOF

gpend


Please send any *my.unidata* related comments, questions, and bug
reports to plaza@unidata.ucar.edu <mailto:plaza@unidata.ucar.edu>.
Software and package support questions can be directed to
support@unidata.ucar.edu <mailto:support@unidata.ucar.edu>.
Site Map </content/about/sitemap.html> | Search </search.php> | Terms
and Conditions
<http://my.unidata.ucar.edu/content/legal/TermsOfUse.html> | Privacy
Policy </content/legal/privacy.html> | Participation Policy
<http://my.unidata.ucar.edu/content/legal/participation.html>
Sponsored by the National Science Foundation <http://www.nsf.gov>

<http://www.ucar.edu>  <http://www.nsf.gov>  <http://www.uop.ucar.edu>

