#!/bin/sh
usage() {
	echo "$0 app dir - packs a musl dynamically linked app and all"
	echo "library dependencies into a dir and creates a wrapper to run it"
	exit 1
}
app="$1"
dir="$2"
if test -z "$dir" ; then
	usage
fi
mkdir -p "$dir"
for lib in `ldd "$app" |awk '/=>/ { print $3 }'` ; do
	cp "$lib" "$dir"/
done
new_app="$(basename "$app")"
cp "$app" "$dir"/"$new_app".app
cat << EOF > "$dir"/"$new_app"
#!/bin/sh
export LD_LIBRARY_PATH=.
exec ./ld-musl-*.so.1 "$new_app".app "\$@"
EOF
chmod +x "$dir"/"$new_app"
