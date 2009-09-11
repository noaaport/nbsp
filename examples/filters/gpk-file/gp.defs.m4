define(match,
if($1 =~ /$2/){
	$savedir = "$3";
	$savename = "$4";
	$5

	filter($seq, $fpath, $fname,
		$savedir, $savename, $f_compress, $f_append);

        return;

})

define(matchmore,
if($1 =~ /$2/){
	$savedir = "$3";
	$savename = "$4";
	$5

	filter($seq, $fpath, $fname,
		$savedir, $savename, $f_compress, $f_append);
})
