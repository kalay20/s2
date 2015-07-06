rm read

for f in ./*
do
	echo "$f" >> read
	sed -n -e '/set_last/,/\n}/ p' "$f" >> read
done


