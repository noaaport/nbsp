define(match,
if($1 =~ /$2/){
	$pipe_cmd = "$3";
	$pipe_options = "$4";
	$savename = "$5";
	$6

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);

	return;
})

define(matchmore,
if($1 =~ /$2/){
	$pipe_cmd = "$3";
	$pipe_options = "$4";
	$savename = "$5";
	$6

    	filter($seq, $fpath, $fname,
		$pipe_cmd, $pipe_options, $savename, $f_compress);
})
