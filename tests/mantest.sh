#!/bin/sh
buggy=0
total=0
bugs=
for p in `find /share/man/` ; do 
	total=$(($total + 1))
	i=`basename "$p" | sed 's@\.[0-9].*@@'`
	if [ -z "$SEGV" ] ; then
	if man "$i" 2>&1 | grep "Unknown formatter" > /dev/null ; then 
		local pbugs
		buggy=$(($buggy + 1))
		pbugs=$(man "$i" 2>&1 | grep "Unknown formatter")
		local pbt=$(echo "$pbugs" | wc -l)
		local pbu=$(echo "$pbugs" | sort -u | wc -l)
		printf "%s\t%s\t%s\n" "$p" "$pbu" "$pbt"
		bugs=$bugs$'\n'$pbugs
	fi
	else
		printf "%s\n" "$p"
		man "$i" 2>&1 | grep "Unknown formatter" > /dev/null
	fi
done
ft=$(echo "$bugs" | wc -l)
fu=$(echo "$bugs" | sort -u | wc -l)
bl=$(echo "$bugs" | sort | uniq -c | sort -n -r)
cat << EOF >&2
=== status report  ===
total manpages	: $total
buggy manpages	: $buggy
total failures	: $ft
unique failures	: $fu

bug list:
$bl
EOF
