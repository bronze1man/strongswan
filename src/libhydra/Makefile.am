ipseclib_LTLIBRARIES = libhydra.la

libhydra_la_SOURCES = \
hydra.c hydra.h \
kernel/kernel_interface.c kernel/kernel_interface.h \
kernel/kernel_ipsec.c kernel/kernel_ipsec.h \
kernel/kernel_net.c kernel/kernel_net.h \
kernel/kernel_listener.h

libhydra_la_LIBADD = \
  $(top_builddir)/src/libstrongswan/libstrongswan.la

if USE_WINDOWS
  libhydra_la_LIBADD += -lws2_32
endif

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-DIPSEC_DIR=\"${ipsecdir}\" \
	-DPLUGINDIR=\"${plugindir}\"

AM_LDFLAGS = \
  -no-undefined

EXTRA_DIST = Android.mk

# build optional plugins
########################

if MONOLITHIC
SUBDIRS =
else
SUBDIRS = .
endif

if USE_KERNEL_PFKEY
  SUBDIRS += plugins/kernel_pfkey
if MONOLITHIC
  libhydra_la_LIBADD += plugins/kernel_pfkey/libstrongswan-kernel-pfkey.la
endif
endif

if USE_KERNEL_PFROUTE
  SUBDIRS += plugins/kernel_pfroute
if MONOLITHIC
  libhydra_la_LIBADD += plugins/kernel_pfroute/libstrongswan-kernel-pfroute.la
endif
endif

if USE_KERNEL_NETLINK
  SUBDIRS += plugins/kernel_netlink
if MONOLITHIC
  libhydra_la_LIBADD += plugins/kernel_netlink/libstrongswan-kernel-netlink.la
endif
endif

if MONOLITHIC
  SUBDIRS += .
endif
SUBDIRS += tests
