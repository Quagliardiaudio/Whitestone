#perl

while(<DATA>)
{


    @xposeLUT = split /\s+/;
    print join("\\", @xposeLUT), "\\\n";
    @clean_pre = (@clean_pre, $xposeLUT[0]);
    @clean_post = (@clean_post, $xposeLUT[1]);
    @bloom_pre = (@bloom_pre, $xposeLUT[2]);
    @bloom_post = (@bloom_post, $xposeLUT[3]);

    @clean_open = (@clean_open, $xposeLUT[4]);
    @bloom_open = (@bloom_open, $xposeLUT[5]);

    @refs = (\@clean_pre, \@clean_post, \@bloom_pre, \@bloom_post, \@clean_open, \@bloom_open);
    
    
}
#print join(", " ,@clean_pre,),  "\n";
foreach $ref (@refs)
{
    $sw_pos_count = 0;
#    print join("|", @$ref), "\n";
    foreach (@$ref)
    {
	$pad = m/\[(([0-9.]+)\s*,\s*(\d+))\]/;
	if ($pad > 0)
	{
#	    print " pad value detected: $3 ";
	    $pad = $3;
	}
	else
	{
	    undef $pad;
	}
	
#	print $_, ": ";
	$sign = m/-/;
	
	if($sign)
	{
	    $sign = "NEG";
	}
	else
	{
	    $sign = "POS";
	}
	
	m/(\d+)\.?/;
	$integer = $1;

	$dec = m/\.(\d+)/;
	if($dec > 0)
	{
	    $dec = $1;
	}
	else
	{
	    $dec = 0;
	}
	
	$dec = "0P$dec"; #if dec = 0 (no matches) it is not assigned $1 then...
	
	
	print "{", $sign, ", ", $integer, ", dB", $dec;
	if (defined($pad))
	{
	    print ", ", $pad, "},\t//$sw_pos_count\n";
	}
	else
	{
	    print "},\t//$sw_pos_count\n";
	}
	
#    print $_ , "},\n";

	$sw_pos_count++;
    }
    print "\n\n";
}

#Note: for the data section below... make sure no extra \n after the table... makes extra null entries that throws us off...

__DATA__
0	0	0.25	0	[0,0]	[0,0]
0	0	0	1	[0,0]	[0,0]
0	0	0	0.75	[0,0]	[0,0]
0	0	0	1	[0,0]	[0,0]
0	0	0	1	[0,0]	[0,0]
0	0	0.25	2	[0,0]	[0,0]
0	0.25	0	2	[0,0]	[0,0]
0	0	0	2	[0,0]	[0,0]
0	0	0	3	[0,0]	[0,0]
0	0	0	2	[0,0]	[0,0]
0	0	0	1	[0,0]	[0,0]
0	0	0	1	[0,0]	[0,0]
