#!/bin/sh

SERVICE_HOME="${GENITOR_SERVICE_HOME:-/etc/genitor/sv}"
RUN_HOME="${GENITOR_RUN_HOME:-/run/genitor/service}"
ENABLED_HOME="${GENITOR_ENABLED_HOME:-/run/genitor/service}"

usage() {
	cat <<EOF
USAGE :
	sv start <NAME>			start a service
	sv stop <NAME>			stop a service
	sv enable <NAME>		enable a service
	sv disable <NAME>		disable a service
	sv status <NAME>		show service state
	sv list				list available services
EOF
}

fail() {
	printf 'sv: %s\n' "$*" >&2
	exit 1
}

require_root() {
	if [ "$(id -u)" -ne 0 ]; then
		fail "must be run as root"
	fi
}

service_dir() {
	printf '%s/%s' "$SERVICE_HOME" "$1"
}

valid_name() {
	case "$1" in
	''|'.'|'..')
		return 1
		;;
	*[!A-Za-z0-9._-]*)
		return 1
		;;
	esac
	return 0
}

check_name() {
	[ -n "${1-}" ] || fail "missing service name"
	valid_name "$1" || fail "invalid service name: $1"
}

check_service() {
	local name=$1 dir
	dir=$(service_dir "$name")

	[ -d "$dir" ] || fail "service definition not found: $dir"
	[ -x "$dir/run" ] || fail "run script missing or not executable: $dir/run"
}

link_service() {
	local dir=$1 link=$2

	if [ -e "$link" ] && [ ! -L "$link" ]; then
		fail "refusing to replace non-symlink path: $link"
	fi

	ln -sfn -- "$dir" "$link" || fail "cannot create symlink: $link"
}

remove_service_link() {
	local link=$1 label=$2

	if [ -L "$link" ]; then
		rm -f -- "$link" || fail "cannot remove $link"
		printf '%s: %s request removed\n' "$label" "${link##*/}"
	elif [ -e "$link" ]; then
		fail "refusing to remove non-symlink path: $link"
	else
		printf '%s: %s was not requested\n' "$label" "${link##*/}"
	fi
}

cmd_status() {
	local name=$1 enabled=no requested=no
	check_name "$name"

	if [ -L "$ENABLED_HOME/$name" ]; then
		enabled=yes
	fi
	if [ -L "$RUN_HOME/$name" ]; then
		requested=yes
	fi

	printf '%s: enabled=%s runtime-request=%s\n' "$name" "$enabled" "$requested"
	if [ "$requested" = yes ]; then
		printf '  Note: runtime-request=yes does not confirm a live process.\n'
	fi
}

cmd_start() {
	local name=$1
	check_name "$name"
	check_service "$name"
	mkdir -p -- "$RUN_HOME" || fail "cannot create $RUN_HOME"
	link_service "$(service_dir "$name")" "$RUN_HOME/$name"
	printf 'start requested: %s\n' "$name"
}

cmd_stop() {
	local name=$1
	check_name "$name"
	remove_service_link "$RUN_HOME/$name" "stop"
}

cmd_enable() {
	local name=$1
	check_name "$name"
	check_service "$name"
	mkdir -p -- "$ENABLED_HOME" || fail "cannot create $ENABLED_HOME"
	link_service "$(service_dir "$name")" "$ENABLED_HOME/$name"
	printf 'enabled for boot: %s (not started by enable)\n' "$name"
}

cmd_disable() {
	local name=$1
	check_name "$name"
	remove_service_link "$ENABLED_HOME/$name" "disable"
}

cmd_list() {
	local path name enabled requested

	printf '%-24s %-10s %s\n' 'SERVICE' 'ENABLED' 'RUNTIME-REQUEST'
	for path in "$SERVICE_HOME"/*; do
		[ -d "$path" ] || continue
		name=${path##*/}
		valid_name "$name" || continue
		[ -x "$path/run" ] || continue

		enabled=no
		requested=no
		[ -L "$ENABLED_HOME/$name" ] && enabled=yes
		[ -L "$RUN_HOME/$name" ] && requested=yes
		printf '%-24s %-10s %s\n' "$name" "$enabled" "$requested"
	done
}

command=${1-}
case "$command" in
	start|stop|enable|disable|status)
		[ $# -eq 2 ] || { usage >&2; exit 2; }
		;;
	list)
		[ $# -eq 1 ] || { usage >&2; exit 2; }
		;;
	-h|--help|help|'')
		usage
		exit 0
		;;
	*)
		usage >&2
		exit 2
		;;
esac

case "$command" in
	start)
		require_root
		cmd_start "$2"
		;;
	stop)
		require_root
		cmd_stop "$2"
		;;
	enable)
		require_root
		cmd_enable "$2"
		;;
	disable)
		require_root
		cmd_disable "$2"
		;;
	status)
		cmd_status "$2"
		;;
	list)
		cmd_list
		;;
esac
