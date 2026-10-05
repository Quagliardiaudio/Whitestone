#perl

while(<DATA>)
{


    @xposeLUT = split /\s+/;
#    print join("\\", @xposeLUT), "\\\n";
    @clean_pre = (@clean_pre, $xposeLUT[0]);
    @clean_post = (@clean_post, $xposeLUT[1]);
    @bloom_pre = (@bloom_pre, $xposeLUT[2]);
    @bloom_post = (@bloom_post, $xposeLUT[3]);

    @refs = (\@clean_pre, \@clean_post, \@bloom_pre, \@bloom_post);
    
    
}
#print join(", " ,@clean_pre,),  "\n";
foreach $ref (@refs)
{
#    print join("|", @$ref), "\n";
    foreach (@$ref)
    {

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
	
	
	print "{", $sign, ", ", $integer, ", dB", $dec, "}, \n";
#    print $_ , "},\n";
    }
    print "\n\n";
}

#Note: for the data section below... make sure no extra \n after the table... makes extra null entries that throws us off...

__DATA__
-3.75	-3.75	-3	-3
-3.75	-4	-2.75	-3
-3.75	-3.25	-2.75	-2.75
-3.5	-2.75	-3	-2.75
-3.5	-2.25	-3	-2.25
-3.5	-2.75	-3	-3
-3.5	-2.5	-3	-2.75
-3.5	-2	-3	-2.25
-3.25	-2.25	-3	-2.75
-3.25	-2	-3.25	-2.25
-3.25	-2.5	-3.25	-2.5
-3.25	-2	-3.25	-2
