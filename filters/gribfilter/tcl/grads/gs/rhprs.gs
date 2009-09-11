#
# $Id$
#

set outputfile $gsplot(imgfile);
if {$outputfile eq ""} {
    set outputfile rhprs_$gsplot(reftime)_$gsplot(forecasttime).png;
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

    set gxout shaded
    d rhprs
    set gxout contour
    d rhprs
    draw title Rel Humidity \
	$gsplot(model)/$gsplot(reftime)/$gsplot(forecasttime)
    printim $outputfile

    quit
}
