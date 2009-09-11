define(match_pipe,
if($1 =~ /$2/){
	$pipe_cmd = "$3";
	$pipe_options = "$4";
	$savename = "$5";
	$6

    	filter_pipe($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
})dnl

define(match2_pipe,
if($1 =~ /$2/){
	if($3 =~ /$4/){
		$pipe_cmd = "$5";
		$pipe_options = "$6";
		$savename = "$7";
		$8;

    		filter_pipe($seq, $fpath, $fname,
		    $pipe_cmd, $pipe_options, $savename, $f_compress);
	}
	return;
})dnl

define(match_pipe_more,
if($1 =~ /$2/){
	$pipe_cmd = "$3";
	$pipe_options = "$4";
	$savename = "$5";
	$6

    	filter_pipe($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);
})dnl

define(match_file,
if($1 =~ /$2/){
	$savedir = "$3";
	$savename = "$4";
	$5

	filter_file($seq, $fpath, $fname,
		$savedir, $savename, $f_compress, $f_append);

        return;

})dnl

define(match_file_more,
if($1 =~ /$2/){
	$savedir = "$3";
	$savename = "$4";
	$5

	filter_file($seq, $fpath, $fname,
		$savedir, $savename, $f_compress, $f_append);
})dnl

define(match_exec,
if($1 =~ /$2/){
	$exec_cmd = "$3";
	$cmd_options = "$4";

    	filter_exec($seq, $fpath, $fname, $exec_cmd, $cmd_options);

	return;
})dnl
