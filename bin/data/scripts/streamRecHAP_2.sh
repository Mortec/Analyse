#!/bin/bash

#
# Syntax : ./streamRecHAP.sh inputLink outputFilePath rectime(m)
#


input="$1"
output="$2"
recTime=0
let recTime=$3*60
ext=".MOV"

###################################
function findUrl {
echo "Searching url on "$input
echo -e "\n"

page=${input%'_'*}
forma=${input##*'_'}

url=`youtube-dl -f $forma -g $page`

input=$url
}
###################################

#script start here
last=${input##*'.'}

if [ $last != "m3u8" ]
then
findUrl
fi

echo -e "\nFound : "$input"\n"

#d=`date +%A"_"%d"_"%B"_"%Y"__"%H"H"%M"-"%S"s"`

#output=$output"__"$d$ext

echo `date +%H"H"%M`
echo "NEXT RECORDING :"
echo $output
echo -e "\n"


#/usr/local/bin/ffmpeg -i "$input" -acodec copy -vcodec copy -t $recTime "$output" -y
#/usr/local/bin/ffmpeg -i "$input" -ar 44100 -acodec copy -vf scale=960:540 -t $recTime "$output" -y
#/usr/local/bin/ffmpeg -i "$input" -ar 44100 -vcodec copy -r 25 -t $recTime "$output" -y
/usr/local/bin/ffmpeg -i "$input" -ar 44100 -vcodec hap -format hap -r 25 -t $recTime "$output" -y

echo -e "\n"
echo `date +%H"H"%M`
echo "$output"
echo -e "------------------------------------------------------------------------------------------> DONE."
echo -e "\n"

exit 0
