AC_PREREQ([2.69])
AC_INIT([synthe3],[0.1],[Sebastien.Hinderer@ens-lyon.org])

# Programs
AC_PROG_CC
AC_PROG_CXX

# Libraries 
AC_CHECK_LIB([asound], [snd_pcm_open])

# Headers
AC_CHECK_HEADERS([fcntl.h, sys/ioctl.h, sys/time.h, alsa/asoundlib.h])

# Typedefs, structures and compiler characteristics
AC_CHECK_HEADER_STDBOOL

# Library functions
AC_CHECK_FUNCS([gettimeofday, memset, strdup])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_HEADERS([config.h])
AC_OUTPUT
