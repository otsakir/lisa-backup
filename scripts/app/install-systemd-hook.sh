#!/bin/bash

# $ lb-trigger-manager.sh install media-nando-backup1.mount /home/nando/.lbackup/scripts/backup1.sh

COMMAND="$1"


help()
{
cat << EOF
Usage:	./install-systemd-hook.sh install -s <service-name> -u <systemd-mount-unit> [-n <lbackup-name>] <script path>
	./install-systemd-hook.sh remove -s <service-name>
Manages systemd configuration files for lbackup hooks

-s <service name>	Name of the systemd service file without the '.service' suffix.

-u <systemd mount unit>	systemd mount unit name as reported by 'systemctl list-units --type=mount'.

[-n <lbackup name>]	Backup name used in lbackup gui app. A friendly name for the service. Optional.

script path		Absolute path to backup script.

EOF
}

doinstall()
{
	SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
	# SCRIPT_DIR=/opt/lbackup  #TODO: calculate this when configuring app
	SERVICE_TEMPLATE="${SCRIPT_DIR}/template/lisa-backup.service"
	CURRENT_USER=`whoami`
	CURRENT_GROUP=`id -gn`
	SYSTEMD_SERVICE="lbackup-$SERVICE_NAME"
	SERVICE_FILE="/etc/systemd/system/$SYSTEMD_SERVICE.service"

	# echo "Lisa backup trigger manager"
	echo "Installing '$SERVICE_NAME' systemd backup trigger service for '$SYSTEMD_UNIT' to $SERVICE_FILE."
	echo "Backup script '$SCRIPT_PATH'  will be run as user $CURRENT_USER:$CURRENT_GROUP."

#	while true; do
#	    read -p "Is this you ? [y/n] " yn
#	    case $yn in
#		[Yy]* ) break;;
#		[Nn]* ) exit 2;;
#		* ) echo "Please answer yes or no.";;
#	    esac
#	done

	echo "moving on..."

	ok=1 # assume fault
	TEMP_SERVICE_FILE=$(mktemp)
	cat $SERVICE_TEMPLATE | USER=$CURRENT_USER GROUP=$CURRENT_GROUP BACKUP_NAME=\"$BACKUP_NAME\" SYSTEMD_UNIT=\"$SYSTEMD_UNIT\" SCRIPT_PATH=\"$SCRIPT_PATH\" envsubst '\$BACKUP_NAME \$USER \$GROUP \$SYSTEMD_UNIT \$SCRIPT_PATH' | bash -c "cat > $TEMP_SERVICE_FILE"
	# echo ${PIPESTATUS[0]} ${PIPESTATUS[1]} ${PIPESTATUS[2]}
	if [[ ${PIPESTATUS[0]} -eq 0 && ${PIPESTATUS[1]} -eq 0 && ${PIPESTATUS[2]} -eq 0 ]]
	then
		pkexec bash -c "cp $TEMP_SERVICE_FILE \"$SERVICE_FILE\" && systemctl enable \"$SYSTEMD_SERVICE\"" && ok=0
#		sudo bash -c "cp $TEMP_SERVICE_FILE \"$SERVICE_FILE\""
#		if [[ $? -eq 0 ]]
#		then
#			sudo systemctl enable "$SYSTEMD_SERVICE" && ok=0
#		fi
	fi
	if [[ ok -ne 0 ]] 
	then
		echo "Installing systemd '$SERVICE_NAME' service failed."
	fi
	return $ok
}

doremove()
{

	SYSTEMD_SERVICE="lbackup-$SERVICE_NAME"
	SERVICE_FILE="/etc/systemd/system/$SYSTEMD_SERVICE.service"
	echo "Removing '$SYSTEMD_SERVICE' systemd backup service - $SERVICE_FILE"
	pkexec bash -c "systemctl disable \"$SYSTEMD_SERVICE\" && rm \"$SERVICE_FILE\"" && return 0
	#sudo systemctl disable "$SYSTEMD_SERVICE" && sudo rm "$SERVICE_FILE" && return 0

	return 1
}

case $COMMAND in

	remove)
		shift 1
		while getopts "s:" opt
		do
			case $opt in
				s)
					SERVICE_NAME=$OPTARG
			esac
		done
		shift $((OPTIND-1))

		if [[ -z $SERVICE_NAME ]]; then 
			help 
			exit 1 
		fi
		doremove 
		if [ $? -ne 0 ]; then
			echo "Failed removing backup service" 
			exit 1
		fi
		;;
	install)
		shift 1	# get rid of command argument
		while getopts "s:u:n:" opt; do
			#echo "parsing $opt..."
			case $opt in
				s)
					SERVICE_NAME=$OPTARG
					;;
				u)
					SYSTEMD_UNIT=$OPTARG
					;;
				n)
					BACKUP_NAME=$OPTARG
					;;
			esac
		done
		shift $((OPTIND-1))
		SCRIPT_PATH=$1

		if [[ -z $SERVICE_NAME || -z $SYSTEMD_UNIT || -z $SCRIPT_PATH ]]
		then
			help
			exit 1
		fi
		if [[ -z $BACKUP_NAME ]] 
		then
			BACKUP_NAME=$SERVICE_NAME
		fi

		if doinstall
		then
			echo "Successfully installed and enabled '$SYSTEMD_SERVICE' systemd service." 
			#read -p "Press any key to continue" a
			exit 0
		else
			#read -p "Press any key to continue" a
			exit 1
		fi
		;;
	*)
		help
		exit 1
		;;
esac

#read -p "Press any key to continue" a
