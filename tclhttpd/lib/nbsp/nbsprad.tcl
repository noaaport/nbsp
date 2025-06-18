#
# $Id$
#
Direct_Url /nbsprad nbsprad

package require html

proc nbsprad/latest {nidsdir imgdir outputname maxage {loopflag 0}} {

    global Config;

    # Actually these will be overriden dynamically below
    set types {n0r,n0s,n0v,n0z,\
		   n1p,n1r,n1s,n1v,\
		   n2r,n2s,\
		   n3r,n3s,\
		   ncr,net,nmd,ntp,nvl,nvw};

    set sites {abc,abr,abx,acg,aec,ahg,aih,akc,akq,ama,amx,apd,apx,arx,atx,\
		   bbx,bgm,bhx,bis,blx,bmx,box,bro,buf,byx,\
		   cae,cbw,cbx,ccx,cle,clx,crp,cxx,cys,\
		   dax,ddc,dfx,dgx,dix,dlh,dmx,dox,dtx,dvn,dyx,\
		   eax,emx,enx,eox,epz,esx,evx,ewx,eyx,\
		   fcx,fdr,fdx,ffc,fll,fsd,fsx,ftg,fws,\
		   ggw,gjx,gld,grb,grk,grr,gsp,gua,gwx,gyx,\
		   hdx,hgx,hki,hkm,hmo,hnx,hpx,htx,hwa,\
		   ict,icx,iln,ilx,ind,inx,iwa,iwx,\
		   jax,jgx,jkl,jua,\
		   lbb,lch,lix,lnx,lot,lrx,lsx,ltx,lve,lvx,lwx,lzk,\
		   maf,max,mbx,mhx,mia,mkx,mlb,mob,\
		   mpx,mqt,mrx,msx,mtx,mux,mvx,mxx,\
		   nkx,nqa,\
		   oax,ohx,okx,otx,\
		   pah,pbi,pbz,pdt,poe,pux,\
		   rax,rgx,riw,rlx,rtx,\
		   sfx,sgf,shv,sjt,sox,srx,tbw,\
		   tfx,tlh,tlx,twx,tyx,\
		   udx,uex,\
		   vax,vbx,vnx,vtx,vwx,\
		   yux};
    set sites [split $sites ","];
    set types [split $types ","];

    set savedir [pwd];
    cd $Config(docRoot);

    # Construct the list of sites and types dynamically (because the user
    # can configure the format of the directory names.
    set sites [lsort [glob -directory $nidsdir -nocomplain -tails "*"]];

    ::html::init;
    append result [::html::head "Latest radar"] "\n";
    append result [::html::bodyTag] "\n";

    append result "<h1>Latest radar</h1>\n";

    foreach s $sites {
	set s [string trim $s];
	set types [lsort [glob -directory [file join $nidsdir $s] \
			      -nocomplain -tails "*"]];
	append result "<h3>[string toupper $s]</h3>\n";
	foreach t $types {
	    set t [string trim $t];
	    # Include only the nxx
	    if {[regexp {^(N|n)} $t] == 0} {
		continue;
	    }
	    if {[file isdirectory [file join $nidsdir $s $t]]} {
		append result "<a href=display_radmap"\
		    "?site=$s"\
		    "&type=$t"\
		    "&imgdir=$imgdir"\
		    "&outputname=$outputname"\
		    "&maxage=$maxage"\
		    "&loopflag=$loopflag>"\
		    "$t</a>\n";
	    }
	}
	append result "<br>\n";
    }
    append result [::html::end];
    cd $savedir;
    
    return $result;
}

proc nbsprad/display_radmap {site type imgdir outputname maxage {loopflag 0}} {

    global Config;

    ::html::init;
    append result [::html::head "Latest radar"] "\n";
    append result [::html::bodyTag] "\n";

    set savedir [pwd];
    cd $Config(docRoot);

    if {[file isdirectory $imgdir] == 0} {
	append "$imgdir not a directory. Configuration error.";
	append result [::html::end];

	return $result;
    }

    set outputdir [file join $imgdir $site $type];
    file mkdir $outputdir;
    # Output is in $outputname in $outputdir
    set fpathout [file join $outputdir $outputname];

    # Lock file
    set fpathlock ${fpathout}.lock;
    set lockF [_nbspsat_open_lock $fpathlock 1];

    # If the file already exists and it is younger than maxage,
    # then it is not re-created.
    set recreate 1;

    if {[file exists $fpathout]} {
	set status [catch {
	    set now [clock seconds];
	    set mtime [file mtime $fpathout];
	    set age [expr $now - $mtime];
	}];

	if {($status == 0) && ($age < $maxage)} {
	    set recreate 0;
	}
    }

    if {$recreate == 1} {
	set status [catch {
	    if {$loopflag <= 0} {
		exec nbspradmapc -d $outputdir -o $outputname $site/$type;
	    } else {
		exec nbspradmapc -L -d $outputdir -o $outputname $site/$type;
	    }
	} errmsg];
    }

    _nbspsat_close_lockfile $fpathlock $lockF;

    if {[file exists $fpathout] == 0} {
	append result "Could not generate image for $site/$type.";
    } else {
	set fpathout_url [file join "/" $fpathout]; 
	append result [::html::h1 "Latest $type for [string toupper $site]"];
	append result "<img src=$fpathout_url>\n";
    }
    append result [::html::end];
    cd $savedir;

    return $result;
}


proc _nbsprad_open_lock {lockfile lock_sleep_s} {

    set locked 0; ;

    while {$locked == 0} {
	# Assume that the OS guarantees that this is an atomic operation
	set status [catch {
	    set F [open $lockfile {WRONLY CREAT EXCL}];
	} errmsg];
	
	if {$status == 0} {
	    set locked 1;
	} else {
	    # puts "Waiting for $lockfile";
	    after [expr $lock_sleep_s * 1000];
	}
    }

    return $F;
}

proc _nbsprad_close_lockfile {lockfile F} {

    catch {close $F};
    file delete $lockfile;
}
