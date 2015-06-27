<?php
#
# File:        check_conntrack_syn.php
# Author:      Pierre Schweitzer <pierre@reactos.org>
# Created:     27 Jun 2015
# Licence:     GNU GPL v2 or any later version
# Purpose:     Template for pnp4nagios & check_syn
#

$opt[0] = '--title "SYN_SENT connections ratio"';
$ds_name[0] = "SYN_SENT connections ratio";

$def[0]  = rrd::def("var1", $RRDFILE[1], $DS[2], "AVERAGE");
$def[0] .= rrd::def("var2", $RRDFILE[2], $DS[1], "AVERAGE");

$def[0] .= rrd::area("var1", "#EA8F00", "Total");
$def[0] .= rrd::gprint("var1", "AVERAGE", "%3.3lf");
$def[0] .= rrd::area("var2", "#EACC00", "SYN_SENT") ;
$def[0] .= rrd::gprint("var2", "AVERAGE", "%3.3lf");
?>

