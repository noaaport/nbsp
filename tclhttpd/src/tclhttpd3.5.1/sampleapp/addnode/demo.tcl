package provide httpd::demo 1.0

proc Demo/hello {} {
    return "<title>Hello</title><p>Hello!"
}
set demodir /home/welch/web/db

proc Demo/urlnote {url index label blurb} {
    global demodir
    set out [open $demodir/newstuff a]
    set cmd "UrlNote \{ [list $url]\n [list  $index]\n [list  $label]\n [list  $blurb]\n \}\n"
    puts $out $cmd
    close $out
    catch {source $demodir/newstuff}
    return "<title>UrlNote Accepted</title><p>$cmd"

}

proc Demo/addnode {index label blurb} {
    global demodir
    set out [open $demodir/newstuff a]
    set cmd "Node \{ [list  $index]\n [list  $label]\n [list  $blurb]\n \}\n"
    puts $out $cmd
    close $out
    catch {source $demodir/newstuff}
    return "<title>Node Accepted</title><p>$cmd"

}
