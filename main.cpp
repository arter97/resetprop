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
	const char spl_prop[] = "2025-12-01";
	const char android_prop[] = "9.0.0";

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

	printf("resetprop: Android version: %s\n", android_prop);
	printf("resetprop: security patch level: %s\n", spl_prop);

	setprop("ro.build.version.release", android_prop, false);
	setprop("ro.build.version.security_patch", spl_prop, false);
}
