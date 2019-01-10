#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_

#include "system_property_api.cpp"
#include <fcntl.h>
#include <string.h>

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
	char slot_suffix_prop[PROP_VALUE_MAX + 1];
	char spl_prop[PROP_VALUE_MAX + 1];
	char android_prop[PROP_VALUE_MAX + 1];

	if (__system_properties_init()) {
		fprintf(stderr, "Failed to initialize system properties\n");
		return 1;
	}

	if (!__system_property_get("ro.boot.slot_suffix", slot_suffix_prop)) {
		// Assume A-only device
		boot_img_path[strlen(boot_img_path) - 2] = '\0';
	} else {
		boot_img_path[strlen(boot_img_path) - 1] = slot_suffix_prop[1];
	}

	printf("Got boot.img path: %s\n", boot_img_path);

	// Get Android version and security patch level
	// Code by phhusson
	int fd = open(boot_img_path, O_RDONLY);
	uint32_t val;

	lseek(fd, 11 * 4, SEEK_SET);
	read(fd, &val, sizeof(val));
	close(fd); // No longer used

	int android = val >> 11;
	int a = android >> 14;
	int b = (android >> 7) & 0x7f;
	int c = android & 0x7f;

	int spl = val & 0x7ff;
	int y = 2000 + (spl >> 4);
	int m = spl & 0xf;

	sprintf(android_prop, "%d.%d.%d", a, b, c);
	sprintf(spl_prop, "%04d-%02d-%02d", y, m, 1);

	printf("Android version: %s\n", android_prop);
	printf("Security patch level: %s\n", spl_prop);

	setprop("ro.build.version.release", android_prop, false);
	setprop("ro.build.version.security_patch", spl_prop, false);

	printf("Restarting keymaster-3-0\n");
	setprop("ctl.restart", "keymaster-3-0", true);
}
