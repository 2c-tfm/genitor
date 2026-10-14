#!/bin/sh

SERVICE_HOME="${GENITOR_SERVICE_HOME:-/etc/genitor/sv}"
RUN_HOME="${GENITOR_RUN_HOME:-/run/genitor/service}"
ENABLED_HOME="${GENITOR_ENABLED_HOME:-/etc/genitor/enabled}"

usage() {
	cat <<EOF
USAGE : 
	sv start <NAME>			start a service
	sv stop <NAME>			stop a service
	sv enable <NAME>		enable a service
	sv disable <NAME>		disable a service
	sv list <NAME>			list availbale services
	sv status <NAME>		up down ?
EOF
}

fail() {
    printf 'sv: %s\n' "$*" >&2
    exit 1
}


require_root() {
	if (( $(id --user) != 0 )); then
		echo "run it with sudo"
		exit
	fi
}

check_service() {
	local name=$1 dir
	dir=$(service_dir "$name")

	[[ -d "$dir" ]] || fail "service definition not found: $dir"
	[[ -x "$dir/run" ]] || fail "run script missing or not executable: $dir/run"
}

link_service() {
	local name=$1 dir=$2 link=$3

	if [[ -e "$link" && ! -L "$link" ]]; then
		echo "refusing to replace non-symlink path: $link"
		exit
	fi

	ln -sfn -- "$dir" "$link" || fail "cannot create symlink: $link"
}

remove_service_link() {
    local link=$1 label=$2

    if [[ -L "$link" ]]; then
        rm -f -- "$link" || fail "cannot remove $link"
        printf '%s: %s request removed\n' "$label" "${link##*/}"
    elif [[ -e "$link" ]]; then
        fail "refusing to remove non-symlink path: $link"
    else
        printf '%s: %s was not requested\n' "$label" "${link##*/}"
    fi
}

cmd_status() {
    local name=$1 enabled=no requested=no
    check_name "$name"

    if [[ -L "$ENABLED_HOME/$name" ]]; then
        enabled=yes
    fi
    if [[ -L "$RUN_HOME/$name" ]]; then
        requested=yes
    fi

    printf '%s: enabled=%s runtime-request=%s\n' "$name" "$enabled" "$requested"
    if [[ "$requested" == yes ]]; then
        printf '  Note: runtime-request=yes does not confirm a live process.\n'
    fi
}

cmd_start() {
    local name=$1
    check_name "$name"
    check_service "$name"
    mkdir -p -- "$RUN_HOME" || fail "cannot create $RUN_HOME"
    link_service "$name" "$(service_dir "$name")" "$RUN_HOME/$name"
    printf 'start requested: %s\n' "$name"
}

cmd_stop() {
    local name=$1
    check_name "$name"
    remove_service_link "$RUN_HOME/$name" "stop"
    printf 'STOP STOP naughty service\n'
}

cmd_enable() {
    local name=$1
    check_name "$name"
    check_service "$name"
    mkdir -p -- "$ENABLED_HOME" || fail "cannot create $ENABLED_HOME"
    link_service "$name" "$(service_dir "$name")" "$ENABLED_HOME/$name"
    printf 'enabled for boot: %s (not started by enable)\n' "$name"
}

cmd_disable() {
    local name=$1
    check_name "$name"
    remove_service_link "$ENABLED_HOME/$name" "disable"
}


cmd_list() {
    local path name
    shopt -s nullglob

    printf '%-24s %-10s %s\n' 'SERVICE' 'ENABLED' 'RUNTIME-REQUEST'
    for path in "$SERVICE_HOME"/*; do
        [[ -d "$path" ]] || continue
        name=${path##*/}
        valid_name "$name" || continue
        [[ -x "$path/run" ]] || continue

        local enabled=no requested=no
        [[ -L "$ENABLED_HOME/$name" ]] && enabled=yes
        [[ -L "$RUN_HOME/$name" ]] && requested=yes
        printf '%-24s %-10s %s\n' "$name" "$enabled" "$requested"
    done
}


require_root

command=${1-}
case "$command" in
    start|stop|enable|disable)
        [[ $# -eq 2 ]] || { usage >&2; exit 2; }
        ;;
    restore|list)
        [[ $# -eq 1 ]] || { usage >&2; exit 2; }
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
        cmd_start "$2"
        ;;
    stop)
        cmd_stop "$2"
        ;;
    enable)
        cmd_enable "$2"
        ;;
    disable)
        cmd_disable "$2"
        ;;
    list)
        cmd_list
        ;;
esac
