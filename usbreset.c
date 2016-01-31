#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <linux/usbdevice_fs.h>

int main(int argc, char **argv) {
	int fd, err=0;
	if (argc != 2) {
		printf("usbreset - send a port reset to an USB device\n"
		       "this should be equivalent to physically unplugging and replugging a device\n\n"
		       "usage: usbreset /dev/bus/usb/busid/deviceid\n"
		       "e.g.   usbreset /dev/bus/usb/002/001\n"
		       "(see lsusb output for bus id and device id)\n");
		return 1;
	}
	if((fd = open(argv[1], O_WRONLY)) == -1) {;
		perror("open");
		return 1;
	}

	printf("resetting USB device %s\n", argv[1]);

	if(ioctl(fd, USBDEVFS_RESET, 0) == -1) {
		perror("ioctl");
		err = 1;
	} else printf("reset successful\n");

	close(fd);
	return err;
}

