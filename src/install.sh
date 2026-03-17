#!/usr/bin/env bash

RED='\033[31m'
GREEN='\033[32m'
BLUE='\033[34m'
RESET='\033[0m'

iamroot() {
    [ "$EUID" -eq 0 ]
}

osrelease() {
    local id=""
    while IFS= read -r line; do
        line=$(echo "$line" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
        if [[ -z "$line" || "$line" =~ ^# ]]; then
            continue
        fi
        if [[ "$line" == *"="* ]]; then
            k="${line%%=*}"
            v="${line#*=}"
            if [[ "$v" =~ ^\".*\"$ ]]; then
                v="${v#\"}"
                v="${v%\"}"
            fi
            if [[ "$k" == "ID" ]]; then
                id="$v"
                break
            fi
        fi
    done < /etc/os-release
    echo "$id"
}

main() {
    local distro_id
    distro_id=$(osrelease)

    if [[ -z "$distro_id" ]]; then
        echo -e "${RED}Cannot detect the distribution ID. Contact the repository owner or script author Cero0xA672@outlook.com ask for help.${RESET}" >&2
        return 1
    fi

    echo -e "${BLUE}Your device's distribution ID: $distro_id${RESET}"

    local Install=""
    case "$distro_id" in
        ubuntu|debian|linuxmint|pop|kali)
            Install="apt install -y gcc flex bison llvm clang-18 libclang-18-dev git"
            ;;
        arch|manjaro|endeavouros)
            Install="pacman -S --noconfirm gcc flex bison llvm clang git"
            ;;
        fedora)
            Install="dnf install -y gcc flex bison llvm clang git"
            ;;
        rhel|centos|rocky|almalinux)
            Install="dnf install -y gcc flex bison llvm clang git"
            ;;
        opensuse|opensuse-leap|opensuse-tumbleweed|suse)
            Install="zypper install -y gcc flex bison llvm clang git"
            ;;
        alpine)
            Install="apk add gcc flex bison llvm clang git"
            ;;
        gentoo)
            Install="emerge -v gcc flex bison llvm clang git"
            ;;
        void)
            Install="xbps-install -y gcc flex bison llvm clang git"
            ;;
        *)
            echo -e "${RED}Unsupported distribution: $distro_id${RESET}" >&2
            return 1
            ;;
    esac

    local cmd
    if iamroot; then
        cmd="$Install"
    else
        cmd="sudo $Install"
    fi

    echo -e "${GREEN}Executing: $cmd${RESET}"
    eval "$cmd"
    local ret=$?
    if [ $ret -ne 0 ]; then
        echo -e "${RED}Installation failed with exit code $ret${RESET}" >&2
        return $ret
    fi

    echo -e "${GREEN}Installation successful!${RESET}"
    return 0
}

main "$@"
