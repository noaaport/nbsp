



if($notawips =~ /grib/){
	$pipe_cmd = "$gdecdir/dcgrib2";
	$pipe_options = "-v 1 -d $glogdir/dcgrib.log -e GEMTBL=$gtabledir";
	$savename = "";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /^s[ap]/){
	$pipe_cmd = "$gdecdir/dcmetr";
	$pipe_options = "-v 2 -a 500 -m 72 -d $glogdir/dcmetr.log -e GEMTBL=$gtabledir -s sfmetar_sa.tbl";
	$savename = "surface/YYYYMMDD_sao.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /(^s(hv|hxx|s[^x]))|(^sx(vd|v.50|us(2[0-3]|08|40|82|86)))|(^y[ho]xx84)/){
	$pipe_cmd = "$gdecdir/dcmsfc";
	$pipe_options = "-b 9 -a 10000 -d $glogdir/dcmsfc.log -e GEMTBL=$gtabledir";
	$savename = "ship/YYYYMMDDHH_sb.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /(^s[imn]v[^gins])|(^s[imn]w[^kz])/){
	$pipe_cmd = "$gdecdir/dcmsfc";
	$pipe_options = "-b 9 -a 10000 -d $glogdir/dcmsfc.log -e GEMTBL=$gtabledir";
	$savename = "ship/YYYYMMDDHH_sb.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);
}
if($wmoid =~ /(^s[imn]v[^gins])|(^s[imn]w[^kz])/){
	$pipe_cmd = "$gdecdir/dcmsfc";
	$pipe_options = "-a 6 -d $glogdir/dcmsfc_6hr.log -e GEMTBL=$gtabledir";
	$savename = "ship6hr/YYYYMMDDHH_ship.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /^u[abcdefghijklmnpqrstwxy]/){
	$pipe_cmd = "$gdecdir/dcuair";
	$pipe_options = "-b 24 -m 16 -d $glogdir/dcuair.log -e GEMTBL=$gtabledir -s snstns.tbl";
	$savename = "upperair/YYYYMMDD_upa.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /^uz/){
	$pipe_cmd = "$gdecdir/dcuair";
	$pipe_options = "-a 50 -m 24 -d $glogdir/dcuair_drop.log -e GEMTBL=$gtabledir";
	$savename = "drops/YYYYMMDD_drop.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /(^s[im]v[igns])|(^snv[ins])|(^s[imn](w[kz]|[^vw]))/){
	$pipe_cmd = "$gdecdir/dclsfc";
	$pipe_options = "-v 2 -s lsystns.upc -d $glogdir/dclsfc.log -e GEMTBL=$gtabledir";
	$savename = "syn/YYYYMMDD_syn.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}

if($wmoid =~ /fo(us14|ak1[34]|ak2[5-9])/){
	$pipe_cmd = "$gdecdir/dcnmos";
	$pipe_options = "-d $glogdir/dcnmos.log -e GEMTBL=$gtabledir";
	$savename = "mos/YYYYMMDDHH_nmos.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /fous2[1-6]|foak3[7-9]|fopa20/){
	$pipe_cmd = "$gdecdir/dcgmos";
	$pipe_options = "-d $glogdir/dcgmos.log -e GEMTBL=$gtabledir -e GEMPAK=$gempak_datadir";
	$savename = "mos/YYYYMMDDHH_gmos.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /feus2[1-6]|feak3[7-9]|fepa20/){
	$pipe_cmd = "$gdecdir/dcxmos";
	$pipe_options = "-v 2 -d $glogdir/dcxmos.log -e GEMTBL=$gtabledir -e GEMPAK=$gempak_datadir";
	$savename = "mos/YYYYMMDDHH_xmos.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /fzxx41/){
	$pipe_cmd = "$gdecdir/dcidft";
	$pipe_options = "-v 2 -d $glogdir/dcidft.log -e GEMTBL=$gtabledir";
	$savename = "idft/YYYYMMDDHH.idft";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /^fzak41/){
	$pipe_cmd = "$gdecdir/dcidft";
	$pipe_options = "-v 2 -d $glogdir/dcidft.log -e GEMTBL=$gtabledir";
	$savename = "idft/YYYYMMDDHH.idak";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($fname =~ /kwns_nwus2[02]/){
	$pipe_cmd = "$gdecdir/dcstorm";
	$pipe_options = "-m 2000 -d $glogdir/dcstorm.log -e GEMTBL=$gtabledir";
	$savename = "storm/sels/YYYYMMDD_sels.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($fname =~ /(kmkc_wwus40|kwns_wwus30)/){
	$pipe_cmd = "$gdecdir/dcwatch";
	$pipe_options = "-t 30 -d $glogdir/dcwatch.log -e GEMTBL=$gtabledir";
	$savename = "storm/watches/watches_YYYY_MM.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($awips1 =~ /tor|svr|ffw/){
	$pipe_cmd = "$gdecdir/dcwarn";
	$pipe_options = "-d $glogdir/dcwarn.log -e GEMTBL=$gtabledir";
	$savename = "storm/warn/YYYYMMDDHH_warn.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /wwus(40|08)|wous20|wwus30/){
	$pipe_cmd = "$gdecdir/dcwtch";
	$pipe_options = "-t 30 -d $glogdir/dcwtch.log -e GEMTBL=$gtabledir";
	$savename = "storm/wtch/YYYYMMDDHH_wtch.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /wwus(6[1-6]|32)/){
	$pipe_cmd = "$gdecdir/dcsvrl";
	$pipe_options = "-d $glogdir/dcsvrl.log -e GEMTBL=$gtabledir";
	$savename = "storm/svrl/YYYYMMDDHH_svrl.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($awips1 =~ /ffg|ffh/){
	$pipe_cmd = "$gdecdir/dcffg";
	$pipe_options = "-d $glogdir/dcffg.log -e GEMTBL=$gtabledir";
	$savename = "storm/ffg/YYYYMMDD_ffg.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($fname =~ /kawn_xrxx84|yixx84|....u[abdr]/){
	$pipe_cmd = "$gdecdir/dcacft";
	$pipe_options = "-d $glogdir/dcacft.log -e GEMTBL=$gtabledir";
	$savename = "acft/YYYYMMDDHH_acf.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /waus01/){
	$pipe_cmd = "$gdecdir/dcairm";
	$pipe_options = "-d $glogdir/dcairm.log -e GEMTBL=$gtabledir";
	$savename = "airm/YYYYMMDDHH_airm.gem";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /^ft/){
	$pipe_cmd = "$gdecdir/dctaf";
	$pipe_options = "-d $glogdir/dctaf.log -e GEMTBL=$gtabledir";
	$savename = "taf/YYYYMMDD00.taf";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($wmoid =~ /fous5[1-5]/){
	$pipe_cmd = "$gdecdir/dcrdf";
	$pipe_options = "-v 4 -d $glogdir/dcrdf.log -e GEMTBL=$gtabledir";
	$savename = "rdf/YYYYMMDDHH.rdf";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($awips1 =~ /wou/){
	$pipe_cmd = "$gdecdir/dcwou";
	$pipe_options = "-d $glogdir/dcwou.log -e GEMTBL=$gtabledir";
	$savename = "storm/wou/YYYYMMDDHHNN.wou";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}
if($awips1 =~ /wcn/){
	$pipe_cmd = "$gdecdir/dcwcn";
	$pipe_options = "-d $glogdir/dcwcn.log -e GEMTBL=$gtabledir";
	$savename = "storm/wcn/YYYYMMDDHHNN.wcn";
	

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
}