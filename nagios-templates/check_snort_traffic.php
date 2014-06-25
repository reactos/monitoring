<?php
#
# File:        check_snort_traffic.php
# Author:      Pierre Schweitzer <pierre@reactos.org>
# Created:     28 Jun 2012
# Licence:     GNU GPL v2 or any later version
# Purpose:     Template for pnp4nagios & check_snort TRA
#

$opt[0] = '--title "Snort: Traffic"';
$ds_name[0] = "Snort: Traffic";

$def[0]  = rrd::def("var1", $RRDFILE[1], $DS[1], "AVERAGE");
$def[0] .= rrd::def("var2", $RRDFILE[2], $DS[2], "AVERAGE");

$def[0] .= rrd::area("var1", "#C6C6C6", "Traffic (mbits/s)");
$def[0] .= rrd::gprint("var1", "AVERAGE", "%3.3lf");
$def[0] .= rrd::line1("var2", "#003300", "Alerts per sec") ;
$def[0] .= rrd::gprint("var2", "AVERAGE", "%3.3lf");
?>

