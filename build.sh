#!/usr/bin/env sh
set -e

for arg in "$@"; do declare $arg='1'; done

mkdir -p build

# Wayland
WL_GENERATED=src/linux/wayland/generated
XDG_SHELL_XML=/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
mkdir -p "$WL_GENERATED"
if [ ! -f "$WL_GENERATED/xdg-shell-protocol.c" ] || [ "$XDG_SHELL_XML" -nt "$WL_GENERATED/xdg-shell-protocol.c" ]; then
    wayland-scanner client-header "$XDG_SHELL_XML" "$WL_GENERATED/xdg-shell-client-protocol.h"
    wayland-scanner private-code "$XDG_SHELL_XML" "$WL_GENERATED/xdg-shell-protocol.c"
fi

clang -std=c23 -D_GNU_SOURCE -O0 -g -Wall -Wextra -Wpedantic -Isrc \
      $(pkg-config --cflags wayland-client) \
      src/pendulum/pendulum_main.c -o build/pendulum \
      $(pkg-config --libs wayland-client) -lm

if [[ "$run" == "1" ]]; then
    ./build/pendulum
fi
