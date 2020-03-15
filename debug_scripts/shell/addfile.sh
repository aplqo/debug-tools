#!/bin/bash

if [ ! -e $2.cpp -a "$4" != "true" ]
then
	touch $2.cpp
fi

i=0
while [ $i -lt $3 ]
do
	touch $2$i.in
	touch $2$i.ans
	$1 $2$i.in
	$1 $2$i.ans
	let i++
done	
