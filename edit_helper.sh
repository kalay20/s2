rm read

for f in ./*
do
	echo "$f" >> read
	sed -n -e '/THROT/,/endif/ p' "$f" >> read
done


