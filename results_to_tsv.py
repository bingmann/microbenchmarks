#!/usr/bin/env python3
# Script to parse RESULT lines and dump as a TSV file
# Copyright (C) 2020 Timo Bingmann <tb@panthema.net>
# All rights reserved. Published under the MIT License in the LICENSE file.

import sys

fields = {}
fields_order = []
rows = []

# process all lines individually
def process_line(line):
    if not line.startswith("RESULT\t"):
        return

    line = line.rstrip()

    r = {}
    kv_list = line.split('\t')
    kv_list.pop(0)

    for kv in kv_list:
        kv = kv.split('=', 2)
        if len(kv) == 2:
            r[kv[0]] = kv[1]
            if kv[0] not in fields.keys():
                fields[kv[0]] = 1
                fields_order.append(kv[0])
    rows.append(r)

# read all input data
for filename in sys.argv:
    with open(filename) as f:
        for line in f:
            process_line(line)

# dump data as tab-separated value TSVs
print(*fields_order, sep='\t')
for r in rows:
    print(*[r[f] for f in fields_order], sep='\t')
