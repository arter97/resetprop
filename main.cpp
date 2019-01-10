#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_

#include "system_property_api.cpp"
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

int main(int argc, char** argv) {
	if (__system_properties_init()) {
		fprintf(stderr, "Failed to initialize system properties\n");
		return 1;
	}

	setprop(argv[1], argv[2], false);
}
