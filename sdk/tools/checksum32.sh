#! /bin/sh

#od -An -v -t x4 -w4 $1 | gawk '{sum+=strtonum("0x"$1)}END{printf("0x%08x",sum%0x100000000)}'
#od -An -v -tu4 -w4 $1 | gawk '{sum+=$1}END{printf("0x%08x",sum%0x100000000)}'


handle_block()
{
	local IFS OLD_IFS a b checksum

	if [ "$1" = "Used" -a "$2" = "blocks:" ]; then
		OLD_IFS="$IFS"
		IFS='-'
		read a b <<< "$3"
		checksum=$(dd if=$IMAGE_IMG skip=$a count=$((b-a+1)) bs=1K 2>/dev/null | od -An -v -tu4 -w4 | gawk '{sum+=$1}END{printf("0x%08x",sum%0x100000000)}')
		((TOT_CS += checksum))
		((TOT_CS %= 0x100000000))
		printf '%u-%u, cs1=0x%08x, cs2=0x%08x\n' $a $b $checksum $TOT_CS
		IFS="$OLDIFS"
	fi
}

IMAGE=$(echo $1 | sed 's:.\+/\([^/]\+\)\.[^/]\+:\1:')
#echo $IMAGE

IMAGE_IDX=$(dirname $1)/$IMAGE.idx
IMAGE_IMG=$1
TOT_CS=0

while read line
do
	handle_block $line
done < $IMAGE_IDX


