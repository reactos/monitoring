#!/usr/bin/perl
#
# File: check_smart.pl
# Author: Pierre Schweitzer <pierre@reactos.org>
# Created: 23 Nov 2011
# Licence: GNU GPL v2 or any later version
# Purpose: Script to check for master status. In case it is down
# the slave is activated to take over.
#
$output = `sudo /usr/lib/nagios/plugins/check_smart @ARGV`;
chomp $output;
$ret = 0;
print $output;
if ($output =~ /^OK/) {
        $output =~ /(\d+)\)$/;
        $temp = $1;
        print " | temp=$temp\n";
        exit $ret;
}
elsif ($output =~ /^WARNING/) {
        $ret = 1;
}
elsif ($output =~ /^CRITICAL/) {
        $ret = 2;
}
elsif ($output =~ /^UNKNOWN/) {
        $ret = 3;
}
print " | temp=0\n";
exit $ret;

