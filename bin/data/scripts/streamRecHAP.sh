#!/bin/bash

#
# Syntax : ./streamRecStdb.sh inputLink outputFilePath rectime(m)
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

page=$input

curl $page -A 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_6) AppleWebKit/602.4.8 (KHTML, like Gecko) Version/10.0.3 Safari/602.4.8' > page.txt
line=`cat page.txt | grep "m3u8"`
if [ ${#line} == 0 ]
then
echo -e "\n"
echo 'Grepping "Token"'
line=`cat page.txt | grep "token"`
token=${line#*'token='}
token=${token:0:95}
src=${line#*'src="'}
src=${src%'embed'*}
fix="index.m3u8?token="
url=$src$fix$token
else
echo -e "\n"
echo 'Grepping "m3u8"'
line=${line#*'"'}
url=${line%%'"'*}
fi

input=$url
}
###################################

#script start here
last=${input##*'.'}

if [ $last == "php" ]
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
