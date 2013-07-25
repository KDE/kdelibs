#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Takes given zone.tab file, or looks for expected locations on the system,
# and uses it to update dummy TIMEZONES file for i18n message extraction.
#
# Timezones read from zone.tab are only added into TIMEZONES,
# no timezone (or timezone comment) is removed from TIMEZONES.
# This in order not to lose i18n for timezones and comments
# found on various newer and older systems.
#
# Chusslove Illich (Часлав Илић) <caslav.ilic@gmx.net>

import sys
import os
import codecs
import re

cmdname = os.path.basename(sys.argv[0])
def error (msg):
    print >>sys.stderr, "%s: %s" % (cmdname, msg)
    sys.exit(1)

system_zonetab_paths = [
    "/usr/share/zoneinfo/zone.tab",
]

if len(sys.argv) not in (2, 3):
    print >>sys.stderr, "usage: %s TIMEZONES_PATH [ZONETAB_PATH]" % cmdname
    sys.exit(1)

timezone_path = sys.argv[1]
if not os.path.isfile(timezone_path):
    error("TIMEZONES file '%s' does not exist "
          "(create manually an empty file if really starting from scratch")

if len(sys.argv) >= 3:
    zonetab_path = sys.argv[2]
    if not os.path.isfile(zonetab_path):
        error("zone.tab file '%s' does not exist" % zonetab_path)
    print "using given zone.tab file at '%s'" % zonetab_path
else:
    for system_zonetab_path in system_zonetab_paths:
        if os.path.isfile(system_zonetab_path):
            zonetab_path = system_zonetab_path
            break
    if not zonetab_path:
        error("cannot fine zone.tab file at any of known system locations")
    print "found system zone.tab file at '%s'" % zonetab_path

# Parse current timezones into dictionary zone->[comments].
ifs = codecs.open(timezone_path, "r", "UTF-8")
message_rx = re.compile(r"i18n\(\"(.*?)\"\)")
tzcomment_rx = re.compile(r"^// i18n:.*comment")
tzcomment_follows = False
current_timezones = {}
for line in ifs:
    if tzcomment_rx.search(line):
        tzcomment_follows = True
    else:
        m = message_rx.search(line)
        if m:
            message = m.group(1)
            if not tzcomment_follows:
                tzone = message
                if tzone not in current_timezones:
                    current_timezones[tzone] = []
            else:
                tzcomment = message
                if tzcomment not in current_timezones[tzone]:
                    current_timezones[tzone].append(tzcomment)
                tzcomment_follows = False
ifs.close()

# Parse system timezones into same form.
ifs = codecs.open(zonetab_path, "r", "UTF-8")
system_timezones = {}
for line in ifs:
    if line.lstrip().startswith("#"):
        continue
    line = line.rstrip("\n")
    els = line.split("\t")
    if len(els) >= 3:
        tzone = els[2]
        if tzone not in system_timezones:
            system_timezones[tzone] = []
        if len(els) >= 4:
            tzcomment = els[3]
            if tzcomment not in system_timezones[tzone]:
                system_timezones[tzone].append(tzcomment)
ifs.close()

# Compose new timezones by adding new timezones read from the system,
# and appending new comments to existing timezones.
num_new_tzones = 0
num_new_tzcomments = 0
for tzone, tzcomments in system_timezones.items():
    if tzone not in current_timezones:
        current_timezones[tzone] = tzcomments
        num_new_tzones += 1
    else:
        for tzcomment in tzcomments:
            if tzcomment not in current_timezones[tzone]:
                current_timezones[tzone].append(tzcomment)
                num_new_tzcomments += 1

if num_new_tzones or num_new_tzcomments:
    tzlines = []
    tzones = current_timezones.keys()
    tzones.sort()
    for tzone in tzones:
        tzlines.append("i18n(\"%s\");\n" % tzone);
        for tzcomment in current_timezones[tzone]:
            tzlines.append("// i18n: comment to the previous timezone\n")
            tzlines.append("i18n(\"%s\");\n" % tzcomment)
    ofs = codecs.open(timezone_path, "w", "UTF-8")
    ofs.writelines(tzlines)
    ofs.close()
    num_new = num_new_tzones + num_new_tzcomments
    print "added %d new timezones/comments to '%s'" % (num_new, timezone_path)
else:
    print "no new timezones/comments to add"
