#!perl
while(<DATA>)
{
    s/\\\\.+$//g;
    
    @matches = m/\{([^\}]+)\}/cg;
    
    foreach $q (@matches)
    {
	
#	print $q . "||";
	
	
    }
#    print "\n";

    foreach $q (@matches)
    {
	$q =~ m/(.?)(POS|NEG)\D+(\d+).+dB(.+)/;
#	print $1 . ":" . $2 . ":" .$3 . ":" . $4 . "||";

#	print "\n";
	$sign = $2;
	$integer = $3;
	$frac = $4;
	chomp $frac;
	$sign =~ s/NEG/-1/;
	$sign =~ s/POS/+1/;
	
	$frac =~ s/P/./;
#	print $sign . ":" .$integer . ":" . $frac . "||";
	$number = $sign * ($integer + $frac);

#	print "\n" . $number . "\n";
	print $number . ",";
    }
    
    print "\n";
}


__DATA__
   {POS, 0, dB0P25},   	    {POS, 0, dB0P25},   	    {NEG, 1, dB0P25},   	    {NEG, 1, dB0P25},   
    {POS, 0, dB0P25},   	    {POS, 0, dB0P0},   	    {NEG, 1, dB0P0},   	    {NEG, 1, dB0P25},   
    {POS, 0, dB0P25},   	    {POS, 0, dB0P5},   	    {NEG, 0, dB0P25},   	    {NEG, 0, dB0P75},   
    {POS, 0, dB0P5},   	    {POS, 0, dB0P75},   	    {POS, 0, dB0P0},   	    {NEG, 0, dB0P5},   
    {POS, 0, dB0P5},   	    {POS, 1, dB0P25},   	    {POS, 0, dB0P25},   	    {POS, 0, dB0P0},   
    {POS, 0, dB0P5},   	    {POS, 0, dB0P75},   	    {POS, 0, dB0P5},   	    {NEG, 0, dB0P25},   
    {POS, 0, dB0P75},   	    {POS, 1, dB0P0},   	    {POS, 0, dB0P75},   	    {POS, 0, dB0P0},   
    {POS, 0, dB0P75},   	    {POS, 1, dB0P25},   	    {POS, 0, dB0P75},   	    {POS, 0, dB0P5},   
    {POS, 0, dB0P75},   	    {POS, 1, dB0P0},   	    {POS, 0, dB0P75},   	    {POS, 0, dB0P0},   
    {POS, 1, dB0P0},   	    {POS, 1, dB0P0},   	    {POS, 1, dB0P0},   	    {POS, 0, dB0P5},   
    {POS, 1, dB0P0},   	    {POS, 0, dB0P25},   	    {POS, 1, dB0P25},   	    {POS, 0, dB0P25},   
    {POS, 1, dB0P0},   	    {POS, 0, dB0P75},   	    {POS, 1, dB0P0},   	    {POS, 0, dB0P75},   
