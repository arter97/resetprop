#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_

#include "system_property_api.cpp"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

static int kmsg_fd;

#undef printf
#define printf(fmt, ...)                      \
    do {                                      \
        dprintf(kmsg_fd, fmt, ##__VA_ARGS__); \
    } while (0);

#undef fprintf
#define fprintf(file, fmt, ...)               \
    do {                                      \
        dprintf(kmsg_fd, fmt, ##__VA_ARGS__); \
    } while (0);

static int setprop(const char *name, const char *value, const bool trigger) {
	int ret;

	prop_info *pi = (prop_info*) __system_property_find(name);
	if (pi != nullptr) {
		if (trigger) {
			if (strncmp(name, "ro.", 3) == 0) __system_property_del(name);
			ret = __system_property_set(name, value);
		} else {
			ret = __system_property_update(pi, value, strlen(value));
		}
	} else {
		printf("resetprop: New prop [%s]\n", name);
		if (trigger) {
			ret = __system_property_set(name, value);
		} else {
			ret = __system_property_add(name, strlen(name), value, strlen(value));
		}
	}

	printf("resetprop: setprop [%s]: [%s] by %s\n", name, value,
		trigger ? "property_service" : "modifing prop data structure");

	if (ret)
		printf("resetprop: setprop error\n");

	return ret;
}

int main() {
	char boot_img_path[] = "/dev/block/bootdevice/by-name/boot__";
	char fastboot_prop[PROP_VALUE_MAX + 1];
	char slot_suffix_prop[PROP_VALUE_MAX + 1];
	char spl_prop[PROP_VALUE_MAX + 1];
	char android_prop[PROP_VALUE_MAX + 1];

	int fd;
	uint32_t val;

	int android, a, b, c;
	int spl, y, m;

	/*
	 * Remove this binary
	 *
	 * fd is preserved so it won't mess up rest of the operation
	 * but be invisible to the user
	 */
	char self[PATH_MAX];
	memset(self, 0, sizeof(self));
	readlink("/proc/self/exe", self, PATH_MAX);
	remove(self);

	// Set stdout and stderr to the kernel log
	kmsg_fd = open("/dev/kmsg", O_WRONLY);

	while (__system_properties_init()) {
		fprintf(stderr, "resetprop: failed to initialize system properties\n");
		usleep(100 * 1000); // Sleep for 100ms
	}

	if (__system_property_get("ro.boot.mode", fastboot_prop)) {
		if (!strcmp(fastboot_prop, "fastboot")) {
			printf("resetprop: booting from fastboot, use 9.0.0 and 2025-12\n");
			a = 9; b = 0; c = 0;
			y = 2025; m = 12;

			goto end;
		}
	}

	if (!__system_property_get("ro.boot.slot_suffix", slot_suffix_prop)) {
		// Assume A-only device
		boot_img_path[strlen(boot_img_path) - 2] = '\0';
	} else {
		boot_img_path[strlen(boot_img_path) - 1] = slot_suffix_prop[1];
	}

	printf("resetprop: got boot.img path: %s\n", boot_img_path);

	// Get Android version and security patch level
	// Code by phhusson
	while ((fd = open(boot_img_path, O_RDONLY)) < 0) {
		fprintf(stderr, "resetprop: waiting for boot image to appear\n");
		usleep(100 * 1000); // Sleep for 100ms
	}

	lseek(fd, 11 * 4, SEEK_SET);
	read(fd, &val, sizeof(val));
	close(fd); // No longer used

	android = val >> 11;
	a = android >> 14;
	b = (android >> 7) & 0x7f;
	c = android & 0x7f;

	spl = val & 0x7ff;
	y = 2000 + (spl >> 4);
	m = spl & 0xf;

end:
	sprintf(android_prop, "%d.%d.%d", a, b, c);
	sprintf(spl_prop, "%04d-%02d-%02d", y, m, 1);

	printf("resetprop: Android version: %s\n", android_prop);
	printf("resetprop: security patch level: %s\n", spl_prop);

	setprop("ro.build.version.release", android_prop, false);
	setprop("ro.build.version.security_patch", spl_prop, false);
}
