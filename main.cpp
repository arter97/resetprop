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

/* From Magisk@jni/magiskhide/hide_utils.c */
static const char *snet_prop_key[] = {
	"ro.boot.vbmeta.device_state",
	"ro.boot.verifiedbootstate",
	"ro.boot.flash.locked",
	"ro.boot.selinux",
	"ro.boot.veritymode",
	"ro.boot.warranty_bit",
	"ro.warranty_bit",
	"ro.debuggable",
	"ro.secure",
	"ro.build.type",
	"ro.odm.build.type",
	"ro.system.build.type",
	"ro.vendor.build.type",
	"ro.build.keys",
	"ro.build.tags",
	"ro.odm.build.tags",
	"ro.system.build.tags",
	"ro.vendor.build.tags",
	"ro.build.selinux",
	NULL
};

static const char *snet_prop_value[] = {
	"locked",
	"green",
	"1",
	"enforcing",
	"enforcing",
	"0",
	"0",
	"0",
	"1",
	"user",
	"user",
	"user",
	"user",
	"release-keys",
	"release-keys",
	"release-keys",
	"release-keys",
	"release-keys",
	"0",
	NULL
};

static const char *deriv_prop_key[] = {
	"ro.build.display.id",
	"ro.build.description",
	"ro.build.flavor",
	NULL
};

static const char *fingerprint_key[] = {
	"ro.bootimage.build.fingerprint",
	"ro.build.fingerprint",
	"ro.odm.build.fingerprint",
	"ro.system.build.fingerprint",
	"ro.vendor.build.fingerprint",
	NULL
};

int main(int argc, char** argv) {
	int i;
	char *ptr;
	char str[PROP_VALUE_MAX];

	if (__system_properties_init()) {
		fprintf(stderr, "Failed to initialize system properties\n");
		return 1;
	}

	// Hide all sensitive props
	for (i = 0; snet_prop_key[i]; ++i) {
		setprop(snet_prop_key[i], snet_prop_value[i], false);
	}

	// Hide all derivative props
	for (i = 0; deriv_prop_key[i]; ++i) {
		if (__system_property_get(deriv_prop_key[i], str) == 0)
			continue;

		printf("Replacing \"%s\" with ", str);
		ptr = strstr(str, "test-keys");
		if (ptr) {
			*ptr = '\0';
			strcat(str, "release-keys");
		}

		ptr = strstr(str, "userdebug");
		if (ptr) {
			memmove(ptr, ptr + 5, strlen(ptr) - 4);
			memcpy(ptr, "user", 4);
		}
		printf("\"%s\".\n", str);

		setprop(deriv_prop_key[i], str, false);
	}

	// Use Pixel 2's fingerprint
	for (i = 0; fingerprint_key[i]; ++i) {
		setprop(fingerprint_key[i], "google/walleye/walleye:8.1.0/OPM1.171019.011/4448085:user/release-keys", false);
	}
}
