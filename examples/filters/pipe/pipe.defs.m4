define(match,
	if($1 =~ /$2/){
		$pipe_cmd = "$3";
		$pipe_options = "$4";
		$5

		filter($seq, $fpath, $fname, $pipe_cmd, $pipe_options);
		return;	
	}
)

define(matchmore,
	if($1 =~ /$2/){
		$pipe_cmd = "$3";
		$pipe_options = "$4";
		$5

		filter($seq, $fpath, $fname, $pipe_cmd, $pipe_options);
	}
)

