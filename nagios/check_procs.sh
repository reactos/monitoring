#!/bin/sh
#
# File:        check_procs.sh
# Author:      Pierre Schweitzer <pierre@reactos.org>
# Created:     14 Jul 2014
# Licence:     GNU GPL v2 or any later version
# Purpose:     Wrapper around check_procs to get perf data
#

OUTPUT=`/usr/lib/nagios/plugins/check_procs $*`
RES=$?
COUNT=`echo $OUTPUT | awk '{print $3}'`
echo "$OUTPUT | procs=$COUNT"
exit $RES
