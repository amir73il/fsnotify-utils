# loc.sh: count LoC by author

D=$1
N=$2
R=$3

[ -n "$D" -a -n "$N" ] || echo "usage: $0 <dir> <author> [rev]"
[ -n "$D" -a -n "$N" ] || exit 1

#T=$(wc -l $D/* | tail -1| while read a b ;do echo $a; done)
ls $D/*|while read f ; do
	wc -l $f
	a=$( git blame $R -- $f |grep -i $N|wc -l )
	t=$( git blame $R -- $f |wc -l )
	A=$(( $A + $a ))
	T=$(( $T + $t ))
	echo "$N: $a / $t ; $A / $T ( $(( 100 * $A / $T ))%)"
	echo ---
done

