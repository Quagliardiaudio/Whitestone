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
#	$pad = $5; #not working with the pad integer value here yet...
				# but eat it still
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
{NEG, 3, dB0P75},	{NEG, 3, dB0P75},	{NEG, 3, dB0P0},	{NEG, 3, dB0P0},	    { NEG, 1, dB0P0, 11 }, //0	    { NEG, 0, dB0P75, 6 },  //0
{NEG, 3, dB0P75},	{NEG, 4, dB0P0},	{NEG, 2, dB0P75},	{NEG, 3, dB0P0},	    { NEG, 0, dB0P75, 8 },  //1	    { NEG, 1, dB0P25, 2 },  //1
{NEG, 3, dB0P75},	{NEG, 3, dB0P25},	{NEG, 2, dB0P75},	{NEG, 2, dB0P75},	    { NEG, 0, dB0P75, 5 },  //2	    { NEG, 1, dB0P0, 2 },  //2
{NEG, 3, dB0P5},	{NEG, 2, dB0P75},	{NEG, 3, dB0P0},	{NEG, 2, dB0P75},	    { NEG, 0, dB0P25, 4 },  //3	    { NEG, 0, dB0P75, 2 },  //3
{NEG, 3, dB0P5},	{NEG, 2, dB0P25},	{NEG, 3, dB0P0},	{NEG, 2, dB0P25},	    { POS, 0, dB0P0, 3 },  //4	    { NEG, 0, dB0P25, 2 },  //4
{NEG, 3, dB0P5},	{NEG, 2, dB0P75},	{NEG, 3, dB0P0},	{NEG, 3, dB0P0},	    { POS, 0, dB0P0, 2 },  //5	    { NEG, 0, dB0P5, 1 },  //5
{NEG, 3, dB0P5},	{NEG, 2, dB0P5},	{NEG, 3, dB0P25},	{NEG, 2, dB0P75},	    { NEG, 0, dB0P5, 0 },  //6	    { NEG, 0, dB0P5, 1 },  //6
{NEG, 3, dB0P5},	{NEG, 2, dB0P0},	{NEG, 3, dB0P25},	{NEG, 2, dB0P25},	    { NEG, 0, dB0P25, 0 },  //7	    { NEG, 0, dB0P25, 0 },  //7
{NEG, 3, dB0P25},	{NEG, 2, dB0P25},	{NEG, 3, dB0P25},	{NEG, 2, dB0P75},	    { POS, 0, dB0P5, 0 },  //8	    { POS, 0, dB0P25, 0 },  //8
{NEG, 3, dB0P25},	{NEG, 2, dB0P0},	{NEG, 3, dB0P0},	{NEG, 2, dB0P25},	    { POS, 0, dB0P75, 0 },  //9	    { POS, 0, dB0P75, 0 },  //9
{NEG, 3, dB0P25},	{NEG, 2, dB0P5},	{NEG, 3, dB0P0},	{NEG, 2, dB0P5},	    { POS, 1, dB0P25, 0 },  //10	    { POS, 1, dB0P25, 0 },  //10
{NEG, 3, dB0P25},	{NEG, 2, dB0P0},	{NEG, 3, dB0P25},	{NEG, 2, dB0P0},	    { POS, 2, dB0P0, 0 },  //11	    { POS, 2, dB0P0, 0 },  //11
