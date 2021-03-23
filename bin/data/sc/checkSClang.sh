#!/bin/bash

#check a value written in the FLAG env.var. supposed to be set at 0 every second by Skipjack
#restart SClang if this value is not equal to > 30.
#tip : restart network service : sudo ifconfig en0 down
#sudo ifconfig en0 up // ifconfig to list network devices (en0, en1...lo0=local)

pushd `dirname $0` > /dev/null
ABSPATH=`pwd`
popd > /dev/null

timer= 0
tempo= 300
state= "booting"

while true
do

	if [ $FLAG < 0 ]
	then
		state= "booting"
		tempo= 10
	fi

	if [ [$FLAG == 0] || [$FLAG == 1] ]
	then
		state= "running"
		tempo= 300
	fi


	if [ [$FLAG > 0] && [$FLAG < 30] ]
	then
		state= "late"
		tempo= 10
	fi



	if [ $FLAG >= 30 ]
	then
		state= "frozen"

		echo -e $(date -jnu)"\n\n\n\n___From Shell : *_*_*_*_SCLANG HAS FROZEN RESTARTING LANGAGE_*_*_*_*\n\n\n\n" >> $ABSPATH/logs/$(date "+%d-%m-%Y").txt;

		killall sclang
		sleep 2

		$ABSPATH/./startInstall.sh
	
		export FLAG= -30

	fi
	
	
	export FLAG=[ $FLAG+1 ]	
	
	if [ $timer == $tempo ]
	then
		echo -e $(date -jnu)"\n\n___From shell : sclang is " $state "___\n\n" >> $ABSPATH/logs/$(date "+%d-%m-%Y").txt
		timer= 0
	else
		timer= timer + 1
	fi
	
	sleep 1

	
done
