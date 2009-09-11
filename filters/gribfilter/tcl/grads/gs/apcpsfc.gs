#
# $Id$
#
set outputfile $gsplot(imgfile);
if {$outputfile eq ""} {
    set outputfile apcpsfc_$gsplot(reftime)_$gsplot(forecasttime).png;
    if {$gsplot(outputdir) ne ""} {
        set outputfile [file join $gsplot(outputdir) $outputfile];
    }
    # We check whether the file exists so that the scheduler can call
    # this script every hour without worrying about recreating the file
    # each time.
    if {[file exists $outputfile]} {
	return;
    }
}

set gsplot(script) {

    open $gsplot(ctlfile)

    set lat 10 35
    set lon -90 -50
    set gxout shaded
    d apcpsfc
    set gxout contour
    d apcpsfc
    draw title Surface Total precipitation \
	$gsplot(model)/$gsplot(reftime)/$gsplot(forecasttime)
    printim $outputfile

    quit
}
