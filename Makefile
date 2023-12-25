CFLAGS := -Wall $(shell pkgconf --cflags --libs libevdev)

splitdev: splitdev.c
