#!/bin/sh

# TODO:
# add restart entry

#if [ `id -u` -ne 0 ]; then
#	echo "Root privilege needed"
#	exit 255
#fi

#JAVA_HOME="/usr/lib/java/jdk1.7.0_07"
TOMCAT_HOME="/home/a2di/Installs/apache-tomcat-7.0.30"

readConfig()
{
	TOM_CONF="../tomcat00"
	[ -f $TOM_CONF ] && source $TOM_CONF ||	echo "warning: missing config file [$TOM_CONF]"
	if [ -z "$JAVA_HOME" ]
	then
		echo "error: JAVA_HOME is not configured"
	elif [ -z "$TOMCAT_HOME" ]
	then
		echo "error: TOMCAT_HOME is not configured"
	else
		printf "use system default configure:\n"
		printf "JAVA_HOME=%s\nTOMCAT_HOME=%s\n" $JAVA_HOME $TOMCAT_HOME
		cd $TOMCAT_HOME/bin
		if [ $? -ne 0 ]; then
			echo "$TOMCAT_HOME/bin is unreadable or non-exist"
			exit 254
		fi
		return
	fi
	exit 1
}

usage()
{
	echo "Usage: $0 {catalina|configtest|daemon|digest|setclasspath|shutdown|startup|tool-wrapper|version}"
}

doOption()
{
	case $1 in
		catalina|configtest|daemon|digest|setclasspath|shutdown|startup|tool-wrapper|version|restart)
			cmd=${1}.sh
			if [ -x $cmd ]; then
				./$cmd
			fi
			unset cmd
			;;
		status)
			tpid=`ps -ef | grep [t]omcat | grep -v status | awk '{print $2}'`
			[ "$tpid" ] && echo "tomcat is running with pid: $tpid" || echo "tomcat is not running"
			unset tpid
			;;
		restart)
			tpid=`ps -ef | grep [t]omcat | grep -v status | awk '{print $2}'`
			[ "$tpid" ] && tomcat shutdown
			unset tpid
			tomcat startup
			;;
		*)
			usage
			echo "Unrecognized option, nothing was done"
			;;
	esac
}

readConfig
doOption
