#!/bin/sh
# print the (N th) latest file name
no="$1"
test -z "$no" && no=1
ls -1 -t | head -n $no | tail -n 1
