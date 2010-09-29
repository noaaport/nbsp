#
# $Id$
#
# proc nbsp::radstations::inputdirs_bystate {dir_tmpl args}
# proc nbsp::radstations::bystate {args}
# proc nbsp::radstations::extent_bystate {args}
#
package provide nbsp::radstations 1.0;
package require textutil::split;

namespace eval nbsp::radstations {} {

    variable radstations;

    set radstations(site,abc) \
      "pabc,faa,ak,bethel,60.79278,-161.87417,162,+9";
    set radstations(site,acg) \
      "pacg,sitka,ak,sitka,56.85278,-135.52917,270,+9";
    set radstations(site,aec) \
      "paec,nome,ak,nome,64.51139,-165.295,54,+9";
    set radstations(site,ahg) \
      "pahg,anchorage,ak,kenai peninsula,60.72639,-151.34917,242,+9";
    set radstations(site,aih) \
      "paih,middleton island,ak,valdez-cordova,59.46194,-146.30111,67,+9";
    set radstations(site,akc) \
      "pakc,king salmon,ak,bristol bay,58.67944,-156.62944,63,+9";
    set radstations(site,apd) \
      "papd,fairbanks,ak,fairbanks north star,65.03556,-147.49917,2593,+9";
    set radstations(site,bmx) \
      "kbmx,birmingham,al,shelby,33.17194,-86.76972,645,+6";
    set radstations(site,eox) \
      "keox,ft rucker,al,dale,31.46028,-85.45944,434,+6";
    set radstations(site,htx) \
      "khtx,huntsville,al,jackson,34.93056,-86.08361,1760,+6";
    set radstations(site,mob) \
      "kmob,mobile,al,mobile,30.67944,-88.23972,208,+6";
    set radstations(site,mxx) \
      "kmxx,maxwell afb,al,tallapoosa,32.53667,-85.78972,400,+6";
    set radstations(site,lzk) \
      "klzk,little rock,ar,pulaski,34.83639,-92.26194,20,+6";
    set radstations(site,srx) \
      "ksrx,ft smith,ar,sebastian,35.29056,-94.36167,1516,+6";
    set radstations(site,emx) \
      "kemx,tucson,az,pima,31.89361,-110.63028,5202,+7";
    set radstations(site,fsx) \
      "kfsx,flagstaff,az,coconino,34.57444,-111.19694,1430,+7";
    set radstations(site,iwa) \
      "kiwa,phoenix,az,maricopa,33.28917,-111.66917,1353,+7";
    set radstations(site,yux) \
      "kyux,yuma,az,pima,32.49528,-114.65583,174,+7";
    set radstations(site,bbx) \
      "kbbx,beale afb,ca,butte,39.49611,-121.63167,173,+8";
    set radstations(site,bhx) \
      "kbhx,eureka,ca,humboldt,40.49833,-124.29194,2402,+8";
    set radstations(site,dax) \
      "kdax,sacramento,ca,yolo,38.50111,-121.67667,30,+8";
    set radstations(site,eyx) \
      "keyx,edwards,ca,santa barbara,35.09778,-117.56,2757,+8";
    set radstations(site,hnx) \
      "khnx,san joaquin,ca,kings,36.31417,-119.63111,243,+8";
    set radstations(site,mux) \
      "kmux,san francisco,ca,santa clara,37.15528,-121.8975,3469,+8";
    set radstations(site,nkx) \
      "knkx,san diego,ca,san diego,32.91889,-117.04194,955,+8";
    set radstations(site,sox) \
      "ksox,santa anna,ca,orange,33.81778,-117.635,3027,+8";
    set radstations(site,vbx) \
      "kvbx,vandenberg afb,ca,santa barbara,34.83806,-120.39583,1233,+8";
    set radstations(site,vtx) \
      "kvtx,los angeles,ca,ventura,34.41167,-119.17861,2726,+8";
    set radstations(site,ftg) \
      "kftg,denver front,co,arapahoe,39.78667,-104.54528,5497,+7";
    set radstations(site,gjx) \
      "kgjx,grand junction,co,mesa,39.06222,-108.21306,9992,+7";
    set radstations(site,pux) \
      "kpux,pueblo,co,pueblo,38.45944,-104.18139,5249,+7";
    set radstations(site,dox) \
      "kdox,dover afb,de,sussex,38.82556,-75.44,50,+5";
    set radstations(site,amx) \
      "kamx,miami,fl,dade,25.61056,-80.41306,14,+5";
    set radstations(site,byx) \
      "kbyx,key west,fl,monroe,24.59694,-81.70333,8,+5";
    set radstations(site,evx) \
      "kevx,eglin afb,fl,walton,30.56417,-85.92139,140,+5";
    set radstations(site,jax) \
      "kjax,jacksonville,fl,duval,30.48444,-81.70194,33,+5";
    set radstations(site,mlb) \
      "kmlb,melbourne,fl,brevard,28.11306,-80.65444,35,+5";
    set radstations(site,tbw) \
      "ktbw,tampa,fl,hillsborough,27.70528,-82.40194,41,+5";
    set radstations(site,tlh) \
      "ktlh,tallahassee,fl,leon,30.3975,-84.32889,63,+5";
    set radstations(site,ffc) \
      "kffc,atlanta,ga,fayette,33.36333,-84.56583,858,+5";
    set radstations(site,jgx) \
      "kjgx,robins afb,ga,twiggs,32.675,-83.35111,521,+5";
    set radstations(site,vax) \
      "kvax,moody afb,ga,lanier,30.89,-83.00194,178,+5";
    set radstations(site,hki) \
      "phki,south kauai,hi,kauai,21.89417,-159.55222,179,+10";
    set radstations(site,hkm) \
      "phkm,kamuela,hi,hawaii,20.12556,-155.77778,3812,+10";
    set radstations(site,hmo) \
      "phmo,molokai,hi,hawaii,21.13278,-157.18,1363,+8";
    set radstations(site,hwa) \
      "phwa,south shore,hi,hawaii,19.095,-155.56889,1370,+10";
    set radstations(site,dmx) \
      "kdmx,des moines,ia,polk,41.73111,-93.72278,981,+6";
    set radstations(site,dvn) \
      "kdvn,davenport,ia,scott,41.61167,-90.58083,754,+6";
    set radstations(site,cbx) \
      "kcbx,boise,id,ada,43.49083,-116.23444,3061,+7";
    set radstations(site,sfx) \
      "ksfx,pocatello,id,bingham,43.10583,-112.68528,4474,+7";
    set radstations(site,ilx) \
      "kilx,lincoln,il,logan,40.15056,-89.33667,582,+6";
    set radstations(site,lot) \
      "klot,chicago,il,will,41.60444,-88.08472,663,+6";
    set radstations(site,ind) \
      "kind,indianapolis,in,marion,39.7075,-86.28028,790,+5";
    set radstations(site,iwx) \
      "kiwx,ft wayne,in,kosciusko,41.40861,-85.7,960,+5";
    set radstations(site,vwx) \
      "kvwx,evansville,in,gibson,38.26,-87.72472,-99999,+6";
    set radstations(site,ddc) \
      "kddc,dodge city,ks,ford,37.76083,-99.96833,2590,+6";
    set radstations(site,gld) \
      "kgld,goodland,ks,sherman,39.36694,-101.7,3651,+6";
    set radstations(site,ict) \
      "kict,wichita,ks,sedgwick,37.65444,-97.4425,1335,+6";
    set radstations(site,twx) \
      "ktwx,topeka,ks,wabaunsee,38.99694,-96.2325,1367,+6";
    set radstations(site,hpx) \
      "khpx,ft campbell,ky,todd,36.73667,-87.285,576,+6";
    set radstations(site,jkl) \
      "kjkl,jackson,ky,breathitt,37.59083,-83.31306,1364,+5";
    set radstations(site,lvx) \
      "klvx,louisville,ky,hardin,37.97528,-85.94389,719,+5";
    set radstations(site,pah) \
      "kpah,paducah,ky,mccracken,37.06833,-88.77194,392,+6";
    set radstations(site,lch) \
      "klch,lake charles,la,calcasieu,30.125,-93.21583,13,+6";
    set radstations(site,lix) \
      "klix,new orleans,la,st. tammany,30.33667,-89.82528,24,+6";
    set radstations(site,poe) \
      "kpoe,ft polk,la,vernon,31.15528,-92.97583,408,+6";
    set radstations(site,shv) \
      "kshv,shreveport,la,caddo,32.45056,-93.84111,273,+6";
    set radstations(site,box) \
      "kbox,boston,ma,bristol,41.95583,-71.1375,118,+5";
    set radstations(site,cbw) \
      "kcbw,houlton,me,aroostook,46.03917,-67.80694,746,+5";
    set radstations(site,gyx) \
      "kgyx,portland,me,cumberland,43.89139,-70.25694,409,+5";
    set radstations(site,apx) \
      "kapx,gaylord,mi,antrim,44.90722,-84.71972,1464,+5";
    set radstations(site,dtx) \
      "kdtx,detroit,mi,oakland,42.69972,-83.47167,1072,+5";
    set radstations(site,grr) \
      "kgrr,grand rapids,mi,kent,42.89389,-85.54472,778,+5";
    set radstations(site,mqt) \
      "kmqt,marquette,mi,marquette,46.53111,-87.54833,1411,+5";
    set radstations(site,dlh) \
      "kdlh,duluth,mn,st. louis,46.83694,-92.20972,1428,+6";
    set radstations(site,mpx) \
      "kmpx,minneapolis,mn,carver,44.84889,-93.56528,946,+6";
    set radstations(site,eax) \
      "keax,kansas city,mo,cass,38.81028,-94.26417,995,+6";
    set radstations(site,lsx) \
      "klsx,st louis,mo,st. charles,38.69889,-90.68278,608,+6";
    set radstations(site,sgf) \
      "ksgf,springfield,mo,greene,37.23528,-93.40028,1278,+6";
    set radstations(site,dgx) \
      "kdgx,brandon,ms,rankin,32.28,-89.98444,-99999,+6";
    set radstations(site,gwx) \
      "kgwx,columbus afb,ms,monroe,33.89667,-88.32889,476,+6";
    set radstations(site,blx) \
      "kblx,billings,mt,yellowstone,45.85389,-108.60611,3598,+7";
    set radstations(site,ggw) \
      "kggw,glasgow,mt,valley,48.20639,-106.62417,2276,+7";
    set radstations(site,msx) \
      "kmsx,missoula,mt,missoula,47.04111,-113.98611,7855,+7";
    set radstations(site,tfx) \
      "ktfx,great falls,mt,cascade,47.45972,-111.38444,3714,+7";
    set radstations(site,ltx) \
      "kltx,wilmington,nc,brunswick,33.98917,-78.42917,64,+5";
    set radstations(site,mhx) \
      "kmhx,morehead city,nc,carteret,34.77583,-76.87639,31,+5";
    set radstations(site,rax) \
      "krax,raleigh durham,nc,wake,35.66528,-78.49,348,+5";
    set radstations(site,bis) \
      "kbis,bismarck,nd,burleigh,46.77083,-100.76028,1658,+6";
    set radstations(site,mbx) \
      "kmbx,minot afb,nd,mchenry,48.3925,-100.86444,1493,+6";
    set radstations(site,mvx) \
      "kmvx,grand forks,nd,traill,47.52806,-97.325,986,+6";
    set radstations(site,lnx) \
      "klnx,north platte,ne,logan,41.95778,-100.57583,2970,+6";
    set radstations(site,oax) \
      "koax,omaha,ne,douglas,41.32028,-96.36639,1148,+6";
    set radstations(site,uex) \
      "kuex,hastings,ne,webster,40.32083,-98.44167,1976,+6";
    set radstations(site,dix) \
      "kdix,philadelphia,nj,burlington,39.94694,-74.41111,149,+5";
    set radstations(site,abx) \
      "kabx,albuquerque,nm,bernalillo,35.14972,-106.82333,5870,+7";
    set radstations(site,epz) \
      "kepz,el paso,nm,dona ana,31.87306,-106.6975,4104,+7";
    set radstations(site,fdx) \
      "kfdx,cannon afb,nm,curry,34.63528,-103.62944,4650,+7";
    set radstations(site,hdx) \
      "khdx,holloman afb,nm,dona ana,33.07639,-106.12222,4222,+7";
    set radstations(site,esx) \
      "kesx,las vegas,nv,clark,35.70111,-114.89139,4867,+8";
    set radstations(site,lrx) \
      "klrx,elko,nv,lander,40.73972,-116.80278,6744,+8";
    set radstations(site,rgx) \
      "krgx,reno,nv,washoe,39.75417,-119.46111,8299,+8";
    set radstations(site,bgm) \
      "kbgm,binghamton,ny,broome,42.19972,-75.985,1606,+5";
    set radstations(site,buf) \
      "kbuf,buffalo,ny,erie,42.94861,-78.73694,693,+5";
    set radstations(site,enx) \
      "kenx,albany,ny,albany,42.58639,-74.06444,1826,+5";
    set radstations(site,okx) \
      "kokx,new york,ny,suffolk,40.86556,-72.86444,85,+5";
    set radstations(site,tyx) \
      "ktyx,ft drum,ny,lewis,43.75583,-75.68,1846,+5";
    set radstations(site,cle) \
      "kcle,cleveland,oh,cuyahoga,41.41306,-81.86,763,+5";
    set radstations(site,iln) \
      "kiln,cincinnati,oh,clinton,39.42028,-83.82167,1056,+5";
    set radstations(site,fdr) \
      "kfdr,altus afb,ok,tillman,34.36222,-98.97611,1267,+6";
    set radstations(site,inx) \
      "kinx,tulsa,ok,rogers,36.175,-95.56444,668,+6";
    set radstations(site,tlx) \
      "ktlx,oklahoma city,ok,oklahoma,35.33306,-97.2775,1213,+6";
    set radstations(site,vnx) \
      "kvnx,vance afb,ok,alfalfa,36.74083,-98.1275,1210,+6";
    set radstations(site,max) \
      "kmax,medford,or,jackson,42.08111,-122.71611,7513,+8";
    set radstations(site,pdt) \
      "kpdt,pendleton,or,umatilla,45.69056,-118.85278,1515,+8";
    set radstations(site,rmx) \
      "krmx,portland,or,washington,45.715,-122.96417,1572,+8";
    set radstations(site,ccx) \
      "kccx,state college,pa,centre,40.92306,-78.00389,2405,+5";
    set radstations(site,pbz) \
      "kpbz,pittsburgh,pa,allegheny,40.53167,-80.21833,1185,+5";
    set radstations(site,jua) \
      "tjua,san juan,pr,san juan,18.1175,-66.07861,2794,+4";
    set radstations(site,cae) \
      "kcae,columbia,sc,lexington,33.94861,-81.11861,231,+5";
    set radstations(site,clx) \
      "kclx,charleston,sc,beaufort,32.65556,-81.04222,97,+5";
    set radstations(site,gsp) \
      "kgsp,greer,sc,spartanburg,34.88306,-82.22028,940,+5";
    set radstations(site,abr) \
      "kabr,aberdeen,sd,brown,45.45583,-98.41306,1302,+6";
    set radstations(site,fsd) \
      "kfsd,sioux falls,sd,minnehaha,43.58778,-96.72889,1430,+6";
    set radstations(site,udx) \
      "kudx,rapid city,sd,pennington,44.125,-102.82944,3016,+6";
    set radstations(site,mrx) \
      "kmrx,knoxville,tn,hamblen,36.16833,-83.40194,1337,+5";
    set radstations(site,nqa) \
      "knqa,memphis,tn,shelby,35.34472,-89.87333,282,+5";
    set radstations(site,ohx) \
      "kohx,nashville,tn,wilson,36.24722,-86.5625,579,+5";
    set radstations(site,ama) \
      "kama,amarillo,tx,potter,35.23333,-101.70889,3587,+6";
    set radstations(site,bro) \
      "kbro,brownsville,tx,cameron,25.91556,-97.41861,23,+6";
    set radstations(site,crp) \
      "kcrp,corpus christi,tx,nueces,27.78389,-97.51083,45,+6";
    set radstations(site,dfx) \
      "kdfx,laughlin afb,tx,kinney,29.2725,-100.28028,1131,+6";
    set radstations(site,dyx) \
      "kdyx,dyess afb,tx,shackelford,32.53833,-99.25417,1517,+6";
    set radstations(site,ewx) \
      "kewx,austin san,tx,comal,29.70361,-98.02806,633,+6";
    set radstations(site,fws) \
      "kfws,dallas,tx,tarrant,32.57278,-97.30278,683,+6";
    set radstations(site,grk) \
      "kgrk,ft hood,tx,bell,30.72167,-97.38278,538,+6";
    set radstations(site,hgx) \
      "khgx,houston,tx,galveston,29.47194,-95.07889,18,+6";
    set radstations(site,lbb) \
      "klbb,lubbock,tx,lubbock,33.65417,-101.81361,3259,+6";
    set radstations(site,maf) \
      "kmaf,midland odessa,tx,midland,31.94333,-102.18889,2868,+6";
    set radstations(site,sjt) \
      "ksjt,san angelo,tx,tom green,31.37111,-100.49222,1890,+6";
    set radstations(site,icx) \
      "kicx,cedar city,ut,iron,37.59083,-112.86222,10600,+7";
    set radstations(site,mtx) \
      "kmtx,salt lake,ut,salt lake,41.26278,-112.44694,6460,+7";
    set radstations(site,akq) \
      "kakq,norfolk rich,va,sussex,36.98389,-77.0075,112,+5";
    set radstations(site,fcx) \
      "kfcx,roanoke,va,floyd,37.02417,-80.27417,2868,+5";
    set radstations(site,lwx) \
      "klwx,sterling,va,loudoun,38.97528,-77.47806,272,+5";
    set radstations(site,cxx) \
      "kcxx,burlington,vt,chittenden,44.51111,-73.16639,317,+5";
    set radstations(site,atx) \
      "katx,seattle,wa,island,48.19472,-122.49444,494,+8";
    set radstations(site,otx) \
      "kotx,spokane,wa,spokane,47.68056,-117.62583,2384,+8";
    set radstations(site,arx) \
      "karx,la crosse,wi,la crosse,43.82278,-91.19111,1276,+6";
    set radstations(site,grb) \
      "kgrb,green bay,wi,brown,44.49833,-88.11111,682,+6";
    set radstations(site,mkx) \
      "kmkx,milwaukee,wi,waukesha,42.96778,-88.55056,958,+6";
    set radstations(site,rlx) \
      "krlx,charleston,wv,kanawha,38.31194,-81.72389,1080,+5";
    set radstations(site,cys) \
      "kcys,cheyenne,wy,laramie,41.15194,-104.80611,6128,+7";
    set radstations(site,riw) \
      "kriw,riverton,wy,fremont,43.06611,-108.47667,5568,+7";
    set radstations(state,ak) abc,acg,aec,ahg,aih,akc,apd;
    set radstations(state,al) bmx,eox,htx,mob,mxx;
    set radstations(state,ar) lzk,srx;
    set radstations(state,az) emx,fsx,iwa,yux;
    set radstations(state,ca) bbx,bhx,dax,eyx,hnx,mux,nkx,sox,vbx,vtx;
    set radstations(state,co) ftg,gjx,pux;
    set radstations(state,de) dox;
    set radstations(state,fl) amx,byx,evx,jax,mlb,tbw,tlh;
    set radstations(state,ga) ffc,jgx,vax;
    set radstations(state,hi) hki,hkm,hmo,hwa;
    set radstations(state,ia) dmx,dvn;
    set radstations(state,id) cbx,sfx;
    set radstations(state,il) ilx,lot;
    set radstations(state,in) ind,iwx,vwx;
    set radstations(state,ks) ddc,gld,ict,twx;
    set radstations(state,ky) hpx,jkl,lvx,pah;
    set radstations(state,la) lch,lix,poe,shv;
    set radstations(state,ma) box;
    set radstations(state,me) cbw,gyx;
    set radstations(state,mi) apx,dtx,grr,mqt;
    set radstations(state,mn) dlh,mpx;
    set radstations(state,mo) eax,lsx,sgf;
    set radstations(state,ms) dgx,gwx;
    set radstations(state,mt) blx,ggw,msx,tfx;
    set radstations(state,nc) ltx,mhx,rax;
    set radstations(state,nd) bis,mbx,mvx;
    set radstations(state,ne) lnx,oax,uex;
    set radstations(state,nj) dix;
    set radstations(state,nm) abx,epz,fdx,hdx;
    set radstations(state,nv) esx,lrx,rgx;
    set radstations(state,ny) bgm,buf,enx,okx,tyx;
    set radstations(state,oh) cle,iln;
    set radstations(state,ok) fdr,inx,tlx,vnx;
    set radstations(state,or) max,pdt,rmx;
    set radstations(state,pa) ccx,pbz;
    set radstations(state,pr) jua;
    set radstations(state,sc) cae,clx,gsp;
    set radstations(state,sd) abr,fsd,udx;
    set radstations(state,tn) mrx,nqa,ohx;
    set radstations(state,tx) ama,bro,crp,dfx,dyx,ewx,fws,grk,hgx,lbb,maf,sjt;
    set radstations(state,ut) icx,mtx;
    set radstations(state,va) akq,fcx,lwx;
    set radstations(state,vt) cxx;
    set radstations(state,wa) atx,otx;
    set radstations(state,wi) arx,grb,mkx;
    set radstations(state,wv) rlx;
    set radstations(state,wy) cys,riw;
}

proc nbsp::radstations::inputdirs_bystate {dir_tmpl args} {
#
# In the last argument, each element can be a space separated list of
# states; e.g.,
#
# bundlelib_inputdirs "rad/tif/%{sss}/n0r" ar la nm ok tx ...;
# bundlelib_inputdirs "rad/tif/%{sss}/n0r" "ar la" "nm ok tx" ...;
#
    variable radstations;

    set r [list];	# the returned list of radar sites

    # Construct the (tcl) state list
    set statelist [list];
    foreach _a $args {
	set statelist [concat $statelist [::textutil::split::splitx ${_a}]];
    }

    foreach k [array names radstations "site,*"] {
	set line $radstations($k);
	set parts [split $line ","];
	set ssss [lindex $parts 0];
	set sss [string range $ssss 1 3];
	set state [lindex $parts 2];

	if {[regexp "%{sss}" $dir_tmpl]} {
	    regsub "%{sss}" $dir_tmpl $sss dir;
	} elseif {[regexp "%{SSS}" $dir_tmpl]} {
	    regsub "%{SSS}" $dir_tmpl [string toupper $sss] dir;
	} elseif {[regexp "%{ssss}" $dir_tmpl]} {
	    regsub "%{ssss}" $dir_tmpl $ssss dir;
	} elseif {[regexp "%{SSSS}" $dir_tmpl]} {
	    regsub "%{SSSS}" $dir_tmpl [string toupper $ssss] dir;
	}

	foreach _state $statelist {
	    if {$_state eq $state} {
		lappend r $dir;
		break;
	    }
	}
    }
    
    return $r;
}

proc nbsp::radstations::bystate {args} {

    variable radstations;

    set r [list];

    # Construct the (tcl) state list
    set statelist [list];
    foreach _a $args {
	set statelist [concat $statelist [::textutil::split::splitx ${_a}]];
    }

    foreach state $statelist {
	set r [concat $r [split $radstations(state,$state) ","]];
    }

    return $r;
}

proc nbsp::radstations::extent_bystate {args} {

    variable radstations;

    set lat1 1000;
    set lat2 0;
    set lon1 0;
    set lon2 -1000;

    # Construct the (tcl) state list
    set statelist [list];
    foreach _a $args {
	set statelist [concat $statelist [::textutil::split::splitx ${_a}]];
    }

    foreach state $statelist {
	foreach site [split $radstations(state,$state) ","] {
	    set data [split $radstations(site,$site) ","];
	    set lat [expr int([lindex $data 4])];
	    set lon [expr int([lindex $data 5])];

	    if {$lat < $lat1} {
		set lat1 $lat;
	    }
		     
	    if {$lat > $lat2} {
		set lat2 $lat;
	    }

	    if {$lon < $lon1} {
		set lon1 $lon;
	    }

	    if {$lon > $lon2} {
		set lon2 $lon;
	    }
	}
    }

    incr lon1 -2;
    incr lat1 -2;
    incr lon2 2;
    incr lat2 2;

    return [list $lon1 $lat1 $lon2 $lat2];
}
