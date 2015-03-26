#!/bin/bash

max=0
step=0

if [ $# != 2 ]; then
	echo "zio cane"
	exit
fi

max=$1
step=$2


echo "using $step"

for i in $(eval echo {0..${max}..${step}}) ; do
	./test -s r$((${i}*2)),g${i},b${i}
	echo $i
done
