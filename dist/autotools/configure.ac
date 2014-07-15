#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(RepastHPC, 2.0.0, repast-interest@sourceforge.net)
AM_INIT_AUTOMAKE
LT_INIT
AC_CONFIG_MACRO_DIR([m4])


AC_CONFIG_SRCDIR([src/repast_hpc/AgentId.cpp])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC([mpicc mpixlc])
AC_PROG_CXX([mpic++ mpixlcxx])


#BOOST_REQUIRE([1.39])

# Checks for libraries.
AC_CHECK_LIB([curl], [curl_version])
AC_CHECK_LIB([mpi], [MPI_Send])
AC_CHECK_LIB([netcdf], [nc_open])

BOOST_SERIALIZATION()
BOOST_MPI()
BOOST_FILESYSTEM()
BOOST_SYSTEM()

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_TYPE_UINT32_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_HEADER_STDC
AC_CHECK_FUNCS([floor rint sqrt])

AC_CONFIG_FILES([Makefile
                 src/repast_hpc/Makefile
                 src/relogo/Makefile
                 src/zombie/Makefile
                 src/rumor/Makefile])
AC_OUTPUT
