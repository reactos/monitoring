<?php
#
# File:        check_snort_session.php
# Author:      Pierre Schweitzer <pierre@reactos.org>
# Created:     05 May 2012
# Licence:     GNU GPL v2 or any later version
# Purpose:     Template for pnp4nagios & check_snort SES
#

$opt[0] = '--title "Snort: Session"';
$ds_name[0] = "Snort: Session";
$opt[1] = '--title "Snort: Session"';
$ds_name[1] = "Snort: Session";

$def[0]  = rrd::def("var1", $RRDFILE[1], $DS[1], "AVERAGE");
$def[0] .= rrd::def("var2", $RRDFILE[2], $DS[2], "AVERAGE");
$def[1]  = rrd::def("var3", $RRDFILE[3], $DS[3], "AVERAGE");
$def[1] .= rrd::def("var4", $RRDFILE[4], $DS[4], "AVERAGE");

$def[0] .= rrd::line1("var1", "#0000FF", "New");
$def[0] .= rrd::gprint("var1", "AVERAGE", "%3.3lf");
$def[0] .= rrd::line1("var2", "#00FF00", "Deleted") ;
$def[0] .= rrd::gprint("var2", "AVERAGE", "%3.3lf");
$def[1] .= rrd::line1("var3", "#0066FF", "Total");
$def[1] .= rrd::gprint("var3", "AVERAGE", "%3.3lf");
$def[1] .= rrd::line1("var4", "#FF0000", "Max");
$def[1] .= rrd::gprint("var4", "AVERAGE", "%3.3lf");

?>
