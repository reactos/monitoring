<?php
#
# File:        check_snort_syn.php
# Author:      Pierre Schweitzer <pierre@reactos.org>
# Created:     24 May 2012
# Licence:     GNU GPL v2 or any later version
# Purpose:     Template for pnp4nagios & check_snort SYN
#

$opt[0] = '--title "Snort: Syn"';
$ds_name[0] = "Snort: Syn";

$def[0]  = rrd::def("var1", $RRDFILE[1], $DS[1], "AVERAGE");
$def[0] .= rrd::def("var2", $RRDFILE[2], $DS[2], "AVERAGE");

$def[0] .= rrd::area("var1", "#EA8F00", "Syn");
$def[0] .= rrd::gprint("var1", "AVERAGE", "%3.3lf");
$def[0] .= rrd::area("var2", "#EACC00", "Synack") ;
$def[0] .= rrd::gprint("var2", "AVERAGE", "%3.3lf");
?>

