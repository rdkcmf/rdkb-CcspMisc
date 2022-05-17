bin_PROGRAMS = EventSubscriber

AM_CPPFLAGS = -I${PKG_CONFIG_SYSROOT_DIR} \
              -I=$(includedir)/rbus
AM_CFLAGS = -D_ANSC_LINUX -D_ANSC_USER -D_ANSC_LITTLE_ENDIAN_
EventSubscriber_SOURCES = event_subscriber.c

EventSubscriber_LDFLAGS = -lrbus

