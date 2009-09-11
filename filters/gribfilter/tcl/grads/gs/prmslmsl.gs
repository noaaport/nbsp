#
# $Id$
#
set outputfile $gsplot(imgfile);
if {$outputfile eq ""} {
    set outputfile prmslmsl_$gsplot(reftime)_$gsplot(forecasttime).png;
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
    d prmslmsl
    set gxout contour
    d prmslmsl
    draw title Mean sea-level Press \
	    $gsplot(model)/$gsplot(reftime)/$gsplot(forecasttime)
    printim $outputfile

    quit
}

# set gsplot(post) {
    # 
    # If these lines were uncommented, the file is copied to the directory
    # "/home/www/images/model". Any valid tcl code can be entered.
    #
    #  file copy $gsplot(outputdir)/prmslmsl-$gsplot(reftime)-$fcast.png \
    #	/home/www/images/model
    #
    # A variation of this using for example wget or more comprehensive tools
    # like sitecopy can be setup to transfer the file to a remote location.
# }
