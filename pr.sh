#!/bin/bash
FC=0
for X in `find .. -name "*.gcda"`
do
	B=$(basename $X .gcda)
	D=$(dirname $X | sed 's/CMakeFiles.*//' | sed 's/\.\/build\//\.\//')
	echo "===== ANALYZING $D$B -> $X ====="
	FNAMES[$FC]=$B
	FC=$(( $FC + 1 ))
	gcov -o "$X" "$D$B" > /dev/null
done

function hasValue() {
	local index=0
	while [ "$index" -lt "${#FNAMES[@]}" ]
	do
		if [ "${FNAMES[$index]}" = "$1" ]
		then
			return 1
		fi
		index=$(( index + 1 ))
	done
	return 0
}

echo "==== CONCATENATING ===="
rm -f total-gcov-tmp > /dev/null
rm -f total-gcov > /dev/null
touch total-gcov-tmp
for X in `find . -name "*.gcov"`
do
	GCBN=$(basename $X .gcov)
	hasValue $GCBN
	if [ $? -eq 0 ]
	then
		continue
	fi

	echo "Concatening $GCBN"
	GCBN=$(echo $GCBN | sed 's/\./\\./')
	echo " --> $GCBN"
	cat "$X" | sed 's/^.*$/& ; '$GCBN'/' >> total-gcov-tmp
done
# exit 0
echo "==== SORTING ===="
cat total-gcov-tmp | sort -nr > total-gcov
rm -f total-gcov-tmp
echo "==== DONE ===="
head -n 30 total-gcov
