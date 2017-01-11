#!/bin/sh

file="$1"
pieces="$2"

[ -z "$pieces" ] && {
	printf "%s file pieces\n" "$0"
	printf "splits file into 'pieces' pieces.\n"
	exit 1
}

[ -e "$file" ] || {
	echo "error: file not exists"
	exit 1
}

fs=$(wc -c "$file"| cut -d " " -f 1)

chunksz=$((fs / pieces))
bdone=0
md5=$(md5sum "$file"| cut -d " " -f 1)
f=$(basename "$file")

script=$(printf "%s.assemble.sh" "$file")
echo "#!/bin/sh" > "$script"
echo "rm -f \"$f\"" >> "$script"
for i in `seq $pieces` ; do
	of=$(printf "%s.part.%.4d" "$file" "$i")
	bytes="$chunksz"
	if [ "$i" = "$pieces" ] ; then bytes=$((fs - ((pieces-1)*chunksz) )) ; fi
	dd if="$file" of="$of" bs=1 skip="$bdone" count="$bytes" >/dev/null 2>&1
	bdone=$((bdone + bytes))
	ofs=$(basename "$of")
	echo "cat \"$ofs\" >> \"$f\"" >> "$script"
	echo created "$of".
done
cat << EOF >> "$script"
test \$(md5sum "$f" | cut -d " " -f 1) = $md5 || {
echo "md5 mismatch!"
exit 1
}
echo "successfully recreated $f"
EOF
chmod +x "$script"
echo "created $script".

