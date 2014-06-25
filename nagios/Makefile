CC	= gcc
CFLAGS	= -W -Wall -Wextra -O2
PLUGINS	= check_apcupsd check_mountpoints check_openvpn check_ram
DIRECT	= check_buildmaster_freeze check_git check_mailman check_master.sh check_real_disk check_serveraid check_smart check_snort check_vmware_modules

all: $(PLUGINS)

install: $(PLUGINS) $(DIRECT)
	mkdir -p $(DESTDIR)/usr/lib/nagios/plugins/
	for plugin in "$?"; do \
		install $$plugin $(DESTDIR)/usr/lib/nagios/plugins/ ; \
	done

$(PLUGINS):
	$(CC) -o $@ $(CFLAGS) $@.c

clean:
	rm -rf $(PLUGINS)
