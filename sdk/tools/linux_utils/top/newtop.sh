#! /bin/sh

export TERM=vt102
export TERMINFO=/usr/share/newtop/terminfo

ShowUsage()
{
	echo "newtop.sh [-b -f log_file] [-H] [-d interval] [-n number] [-h]"
	echo "options:"
	echo "-b -f log_file : batch mode that will print log to log_file"
	echo "-H : seperating thread from process when show information"
	echo "-d interval : the interval between successive log in seconds"
	echo "-n number: do number times and exit"
	echo "-h : show this help information"
}

batch_mode=0
toggle_thread=0
iter_number=0
iter_interval=0

# default log file for batch mode.
log_file=/tmp/top.txt

num_arg=$#
i=0

while :
do
	if [ $i -ge $num_arg ]
	then
	    break;
	fi
	
	case $1 in
	"-h")
        ShowUsage
        exit
        ;;
	"-b")
        batch_mode=1;
		shift;
		;;
	"-H")
		toggle_thread=1;
		shift;
		;;
	"-f")
		shift;
		log_file=$1;
		shift;
		;;
	"-d")
		shift;
		iter_interval=$1;
		shift;
		;;
	"-n")
		shift;
		iter_number=$1;
		shift;
		;;
	*) 
	    shift;
	    ;;
	esac
	
	let i=i+1;
done

if [ "$batch_mode" == "1" ]
then
    top_args="$top_args -b "
fi

if [ "$toggle_thread" == "1" ]
then
    top_args="$top_args -H "
fi

if [ $iter_number -gt 0 ]
then
    top_args="$top_args -n $iter_number "
fi

if [ $iter_interval -gt 0 ]
then
    top_args="$top_args -d $iter_interval "
fi

if [ "$batch_mode" == "1" ]
then
    #echo "$top_args > $log_file";
	./top $top_args > $log_file
else
	#echo "$top_args";
	./top $top_args
fi


