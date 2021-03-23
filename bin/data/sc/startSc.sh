#!/bin/bash


killall sclang
sleep 1
killall scsynth
sleep 1


pushd `dirname $0` > /dev/null
ABSPATH=`pwd`
popd > /dev/null


/Applications/SuperCollider/SuperCollider.app/Contents/MacOS/sclang $ABSPATH/WatchDog.scd &> $ABSPATH/logs/$(date "+%d-%m-%Y").txt