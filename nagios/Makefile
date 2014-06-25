CC	= gcc
CFLAGS	= -W -Wall -Wextra -O2
PLUGINS	= check_apcupsd check_mountpoints check_openvpn check_ram

all: $(PLUGINS)

install: $(PLUGINS)
	install $< $(DESTDIR)/usr/lib/nagios/plugins/$<

$(PLUGINS):
	$(CC) -o $@ $(CFLAGS) $@.c

clean:
	rm -rf $(PLUGINS)
