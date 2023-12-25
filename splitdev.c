#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <libevdev/libevdev-uinput.h>

void fatal(const char *msg)
{
	perror(msg);
	exit(errno);
}

struct libevdev * open_input_device(const char *iname)
{

	int fd = open(iname, O_RDONLY);
	if (fd == -1)
		fatal("Open input");

	struct libevdev *dev = NULL;
	int rc = libevdev_new_from_fd(fd, &dev);
	if (rc < 0)
		fatal("Input evdev");

	if (libevdev_grab(dev, LIBEVDEV_GRAB))
		fatal("Grab");
	
	return dev;
}
	 
struct libevdev_uinput * create_keyboard()
{
	struct libevdev *dev = libevdev_new();
	libevdev_set_name(dev, "Split Keyboard");
	libevdev_set_id_vendor(dev, 0x1235);
	libevdev_set_id_product(dev, 0x9876);
	libevdev_enable_event_type(dev, EV_KEY);
	for (int i=0; i < 256; ++i)
		libevdev_enable_event_code(dev, EV_KEY, i, NULL);

	struct libevdev_uinput *uidev;
	int err = libevdev_uinput_create_from_device(dev,
			LIBEVDEV_UINPUT_OPEN_MANAGED,
			&uidev);
	if (err != 0)
		fatal("Create keyboard");
	return uidev;
}

struct libevdev_uinput * create_mouse()
{
	struct libevdev *dev = libevdev_new();
	libevdev_set_name(dev, "Split Mouse");
	libevdev_set_id_vendor(dev, 0x1235);
	libevdev_set_id_product(dev, 0x6789);
	libevdev_enable_event_type(dev, EV_REL);
	libevdev_enable_event_code(dev, EV_REL, REL_X, NULL);
	libevdev_enable_event_code(dev, EV_REL, REL_Y, NULL);
	libevdev_enable_event_code(dev, EV_REL, REL_WHEEL, NULL);
	libevdev_enable_event_code(dev, EV_REL, REL_WHEEL_HI_RES, NULL);
	libevdev_enable_event_type(dev, EV_KEY);
	libevdev_enable_event_code(dev, EV_KEY, BTN_LEFT, NULL);
	libevdev_enable_event_code(dev, EV_KEY, BTN_MIDDLE, NULL);
	libevdev_enable_event_code(dev, EV_KEY, BTN_RIGHT, NULL);


	struct libevdev_uinput *uidev;
	int err = libevdev_uinput_create_from_device(dev,
			LIBEVDEV_UINPUT_OPEN_MANAGED,
			&uidev);
	if (err != 0)
		fatal("Create mouse");
	return uidev;
}

int main(int argc, char *argv[])
{
	const char *iname = "/dev/input/by-id/usb-Logitech_USB_Receiver-if02-event-mouse";

	if (argc > 2) {
		fprintf(stderr, "Usage: splitdev [device path]\n");
		exit(-1);
	} else if (argc == 2) {
		iname = argv[1];
	}

	struct libevdev *dev = open_input_device(iname);
	struct libevdev_uinput *mouse = create_mouse();
	struct libevdev_uinput *keyboard = create_keyboard();
	unsigned int flags = LIBEVDEV_READ_FLAG_NORMAL|LIBEVDEV_READ_FLAG_BLOCKING;

	for (;;) {
		struct input_event ev;
		int rc = libevdev_next_event(dev, flags, &ev);
		if (rc != LIBEVDEV_READ_STATUS_SUCCESS) {
			continue;
		}

		if (ev.type == EV_REL) {
			libevdev_uinput_write_event(mouse, ev.type, ev.code, ev.value);
			libevdev_uinput_write_event(mouse, 0,0,0);
		} else if (ev.type == EV_KEY) {
			if (ev.code == BTN_LEFT || ev.code == BTN_RIGHT || ev.code == BTN_MIDDLE) {
				libevdev_uinput_write_event(mouse, ev.type, ev.code, ev.value);
				libevdev_uinput_write_event(mouse, 0,0,0);
			} else {
				libevdev_uinput_write_event(keyboard, ev.type, ev.code, ev.value);
				libevdev_uinput_write_event(keyboard, 0,0,0);
			}
		}
	}

	libevdev_uinput_destroy(mouse);
	libevdev_uinput_destroy(keyboard);
	return 0;
}
