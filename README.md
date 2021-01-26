# rs
Send Manual Router Solicitation

This is useful for manually sending a router solicitation. Very few people will likely want to do this, but I created this because my ISP has some quirky network behaviour in my area. Using the usual IPv6 router configuration results in a working router, but IPv6 packets stop being routed after ~90 minutes. By manually sending an RS every so often the route is kept alive.

## Build

Just run `make`.

## Usage

Run something like `./rs em0`, where em0 is replaced with your own WAN interface name. Must be run as root or with the capability to create raw sockets.

## OS Support

Only tested so far on macOS and OpenBSD.
