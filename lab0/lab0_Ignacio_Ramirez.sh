#1
grep 'model name' /proc/cpuinfo

#2
grep 'model name' /proc/cpuinfo | wc -l

#3
wget -O - https://www.gutenberg.org/files/11/11-0.txt | sed 's/Alice/Ignacio/g' > ignacio_in_wonderland.txt

#4
sort -k5rn -o sorted_max_weather.txt weather_cordoba.in | cat sorted_max_weather.txt && echo max: &&  head -1 sorted_max_weather.txt | awk '{print $1,$2,$3}' && echo min: && tail -1 sorted_max_weather.txt | awk '{print $1,$2,$3}'

#5
sort -k3n atpplayers.in 

#6
echo -e "\nSuperliga\n" && awk -F " "  '{print $0 " " $7-$8}' superliga.in |sort -k2 -k9 -rn

#7
ip addr | grep ether | awk '{print $2}' | head -1

#8
for f in ./*; do mv "$f" "${f/_es/}"; done

#Opcional (a)
ffmpeg -i video.mp4 -ss 00:00:02 -to  $( echo "$(ffprobe -v 0 -show_entries format=duration -of compact=p=0:nk=1 video.mp4) - 2"  | bc) -c copy  video_cortado.mp4

#Opcional (b)
ffmpeg -i video.mp4 -i audio.mp3 -filter_complex "[0:a][1:a] amerge=inputs=2"  -shortest out.mp4

