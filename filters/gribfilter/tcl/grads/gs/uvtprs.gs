#
# $Id$
#

set outputfile $gsplot(imgfile);
if {$outputfile eq ""} {
    set outputfile uvt_$gsplot(reftime)_$gsplot(forecasttime).png;
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

    d skip(ugrdprs,3);skip(vgrdprs,3);tmpprs
    draw title Wind/Temp \
	    $gsplot(model)/$gsplot(reftime)/$gsplot(forecasttime)
    printim $outputfile

    quit
}
