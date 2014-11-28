#!/usr/bin/env python

# TODO merge diffdummy into this script

import sys, os, re

if len ( sys.argv ) < 3 :
    print 'Usage: ' + os.path.basename ( sys.argv[0] ) + ' <sync-log-1> <sync-log-2>'
    exit ( 0 )


f = open ( sys.argv[1] )
g = open ( sys.argv[2] )

log1 = f.readlines()
log2 = g.readlines()

g.close()
f.close()


def mismatch ( i, j ):
    print 'Line', i + 1
    print '<', log1[i]
    print 'Line', j + 1
    print '>', log2[j]
    exit ( -1 )


if log1[3] != log2[3]:
    mismatch ( 3, 3 )


i = 4
while log1[i].find ( 'CharaSelect' ) < 0:
    i += 1
i -= 1

j = 4
while log2[j].find ( 'CharaSelect' ) < 0:
    j += 1
j -= 1


count = 0

REGEX = '^[^ ]+ \[[0-9]+\] NetplayState::([A-Za-z]+) \[([0-9]+):([0-9]+)\] ([A-Za-z]+: .+)$'


while i + 1 < len ( log1 ):
    i += 1
    m = re.match ( REGEX, log1[i] )

    if not m:
        print 'Invalid line (%d) in log 1:' % ( i + 1 )
        print log1[i]
        exit ( -1 )

    # Skip Loading, Skippable, and RetryMenu states
    if ( m.group ( 1 ) == 'Loading' ) or ( m.group ( 1 ) == 'Skippable' ) or ( m.group ( 1 ) == 'RetryMenu' ):
        continue

    state1 = m.group ( 1 )
    index1 = int ( m.group ( 2 ) )
    frame1 = int ( m.group ( 3 ) )
    data1 = m.group ( 4 )

    while j + 1 < len ( log2 ):
        j += 1
        m = re.match ( REGEX, log2[j] )

        if not m:
            print 'Invalid line (%d) in log 2:' % ( j + 1 )
            print log2[j]
            exit ( -1 )

        state2 = m.group ( 1 )
        index2 = int ( m.group ( 2 ) )
        frame2 = int ( m.group ( 3 ) )
        data2 = m.group ( 4 )

        if index2 < index1:
            # Skip line
            continue

        if ( index2 == index1 ) and ( frame2 < frame1 ):
            # Skip line
            continue

        if ( index2 == index1 ) and ( frame2 == frame1 ) and ( data2 != data1 ):
            mismatch ( i, j )

        if ( index2 == index1 ) and ( frame2 == frame1 ) and ( data2 == data1 ):
            # Matched
            count += 1
            break

        print 'Missing line (%d) in log 1 from log 2:' % ( i + 1 )
        print log1[i]
        exit ( -1 )


print 'Successfully matched', count, 'lines'