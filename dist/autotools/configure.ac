#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
define([repast_hpc_version],
       regexp(esyscmd(cat version.txt),[\([.0-9]*\)],[\1]))

AC_PREREQ(2.62)
AC_INIT(RepastHPC, [repast_hpc_version()], repast-interest@sourceforge.net)
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC([mpicc mpixlc bgclang])
AC_PROG_CXX([mpic++ mpixlcxx bgclang++])

AC_CONFIG_SRCDIR([src/repast_hpc/AgentId.cpp])
AC_CONFIG_HEADER([config.h])

REPAST_HPC_VERSION=repast_hpc_version()
AC_SUBST(REPAST_HPC_VERSION)
AC_SUBST(REPAST_HPC_VERSION_UNDERSCORE)

# Need ar and ranlib to build static libraries
AC_CHECK_PROG(AR, ar, ar, AC_MSG_ERROR(Cannot find ar))
AC_PROG_RANLIB

if [[ ${prefix} == "NONE" ]]
then
  AC_MSG_ERROR("You must provide --prefix")
fi

# Check OS
USE_MAC="no"
if [[ $( uname ) == "Darwin" ]]
then
        AC_MSG_RESULT([detected Mac.])
        USE_MAC="yes"
fi
AC_SUBST(USE_MAC)

# We prefer to use cp -u for installation
AC_CACHE_CHECK([for cp that supports -u], [ac_cv_path_cp_u],
            [AC_PATH_PROGS_FEATURE_CHECK([CP_U], [cp],
              [[cp_u_out=`cp -u /dev/null cp_u.token 2>&1 > /dev/null`
                test "$?" = 0 \
                && ac_cv_path_cp_u=yes || ac_cv_path_cp_u=no ]],
              [])])
AC_SUBST([CP_U], [$ac_cv_path_cp_u])
[[ -r cp_u.token ]] && rm cp_u.token


# Set shared object suffix (.so on Linux, .dylib on Mac)
libext=so
if [[ $(uname) = "Darwin" ]]
then
   libext=dylib
fi
AR_SUFFIX=a
AC_SUBST(libext)

USE_NETCDF=
USE_NETCDF_LIB_DIR=
AC_ARG_WITH(netcdf,
  AS_HELP_STRING([--with-necdf],
    [root netcdf directory that contains include and lib]),
    [
      USE_NETCDF=${withval}
    ],
    [])

if [[ ! -z "${USE_NETCDF}" ]]
then
  if [[ -d "${USE_NETCDF}/include" ]]; then
    CPPFLAGS="$CPPFLAGS -I${USE_NETCDF}/include"
  fi

  if [[ -d "${USE_NETCDF}/lib" ]]; then
    LDFLAGS="$LDFLAGS -L${USE_NETCDF}/lib"
    USE_NETCDF_LIB_DIR="${USE_NETCDF}/lib"
  fi
fi


USE_NETCDF_INCLUDE=
AC_ARG_WITH(netcdf-include,
  AS_HELP_STRING([--with-netcdf-include],
    [directory containing netcdf headers (netcdf.h).]),
    [
      USE_NETCDF_INCLUDE=${withval}
    ],
    [])

AC_ARG_WITH(netcdf-lib-dir,
  AS_HELP_STRING([--with-netcdf-lib-dir],
    [directory containing netcdf libraries.]),
    [
      USE_NETCDF_LIB_DIR=${withval}
    ],
    [])

if [[ ! -z "${USE_NETCDF_INCLUDE}" ]]
then
  CPPFLAGS="$CPPFLAGS -I${USE_NETCDF_INCLUDE}"
fi

if [[ ! -z "${USE_NETCDF_LIB_DIR}" ]]
then
  LDFLAGS="$LDFLAGS -L${USE_NETCDF_LIB_DIR}"
fi
AC_SUBST(USE_NETCDF_LIB_DIR)

USE_NETCDF_CXX=
USE_NETCDF_CXX_LIB_DIR=
AC_ARG_WITH(netcdf-cxx,
  AS_HELP_STRING([--with-necdf-cxx],
    [root netcdf-cxx directory that contains include and lib]),
    [
      USE_NETCDF_CXX=${withval}
    ],
    [])

if [[ ! -z "${USE_NETCDF_CXX}" ]]
then
  if [[ -d "${USE_NETCDF_CXX}/include" ]]; then
    CPPFLAGS="$CPPFLAGS -I${USE_NETCDF_CXX}/include"
  fi

  if [[ -d "${USE_NETCDF_CXX}/lib" ]]; then
    LDFLAGS="$LDFLAGS -L${USE_NETCDF_CXX}/lib"
    USE_NETCDF_CXX_LIB_DIR="${USE_NETCDF_CXX}/lib"
  fi
fi

USE_NETCDF_CXX_INCLUDE=
AC_ARG_WITH(netcdf-cxx-include,
  AS_HELP_STRING([--with-netcdf-cxx-include],
    [directory containing netcdf-cxx headers (netcdfcpp.h).]),
    [
      USE_NETCDF_CXX_INCLUDE=${withval}
    ],
    [])

AC_ARG_WITH(netcdf-cxx-lib-dir,
  AS_HELP_STRING([--with-netcdf-cxx-lib-dir],
    [directory containing netcdf-cxx libraries.]),
    [
      USE_NETCDF_CXX_LIB_DIR=${withval}
    ],
    [])

if [[ ! -z "${USE_NETCDF_CXX_INCLUDE}" ]]
then
  CPPFLAGS="$CPPFLAGS -I${USE_NETCDF_CXX_INCLUDE}"
fi

if [[ ! -z "${USE_NETCDF_CXX_LIB_DIR}" ]]
then
  LDFLAGS="$LDFLAGS -L${USE_NETCDF_CXX_LIB_DIR}"
fi
AC_SUBST(USE_NETCDF_CXX_LIB_DIR)

USE_CURL_INCLUDE=
AC_ARG_WITH(curl-include,
  AS_HELP_STRING([--with-curl-include],
    [directory containing curl headers (curl.h).]),
    [
      USE_CURL_INCLUDE=${withval}
    ],
    [])

USE_CURL_LIB_DIR=
AC_ARG_WITH(curl-lib-dir,
  AS_HELP_STRING([--with-curl-lib-dir],
    [directory containing curl libraries.]),
    [
      USE_CURL_LIB_DIR=${withval}
    ],
    [])
AC_SUBST(USE_CURL_LIB_DIR)

if [[ ! -z "${USE_CURL_INCLUDE}" ]]
then
  CPPFLAGS="$CPPFLAGS -I${USE_CURL_INCLUDE}"
fi

if [[ ! -z "${USE_CURL_LIB_DIR}" ]]
then
  LDFLAGS="$LDFLAGS -L${USE_CURL_LIB_DIR}"
fi


USE_BOOST_INCLUDE=
AC_ARG_WITH(boost-include,
  AS_HELP_STRING([--with-boost-include],
    [directory containing boost headers (boost/*.hpp).]),
    [
      USE_BOOST_INCLUDE=${withval}
    ],
    [])

USE_BOOST_LIB_DIR=
AC_ARG_WITH(boost-lib-dir,
  AS_HELP_STRING([--with-boost-lib-dir],
    [directory containing boost libraries.]),
    [
      USE_BOOST_LIB_DIR=${withval}
    ],
    [])
AC_SUBST(USE_BOOST_LIB_DIR)

USE_BOOST_LIB_SUFFIX=
AC_ARG_WITH(boost-lib-suffix,
  AS_HELP_STRING([--with-boost-lib-suffix],
    [suffix in boost library names (e.g -mt in libboost_serialization-mt).]),
    [
      USE_BOOST_LIB_SUFFIX=${withval}
    ],
    [])

if [[ ! -z "${USE_BOOST_INCLUDE}" ]]
then
  CPPFLAGS="$CPPFLAGS -I${USE_BOOST_INCLUDE}"
fi

if [[ ! -z "${USE_BOOST_LIB_DIR}" ]]
then
  LDFLAGS="$LDFLAGS -L${USE_BOOST_LIB_DIR}"
fi

BOOST_SERIALIZATION("${USE_BOOST_LIB_SUFFIX}")
BOOST_MPI("${USE_BOOST_LIB_SUFFIX}")
BOOST_FILESYSTEM("${USE_BOOST_LIB_SUFFIX}")
BOOST_SYSTEM("${USE_BOOST_LIB_SUFFIX}")

AC_SUBST(BOOST_MPI_LIBS)
AC_SUBST(BOOST_SERIALIZATION_LIBS)
AC_SUBST(BOOST_FILESYSTEM_LIBS)
AC_SUBST(BOOST_SYSTEM_LIBS)

#AC_MSG_RESULT("${CPPFLAGS}")
#AC_MSG_RESULT("${LDFLAGS}")
# Checks for libraries.
AC_CHECK_LIB([curl], [curl_version])
AC_CHECK_LIB([netcdf], [nc_open], [], [AC_MSG_ERROR(Cannot find netcdf lib)])
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([netcdfcpp.h], [], [AC_MSG_ERROR(Cannot find netcdfcpp headers)])
AC_LANG_POP([C++])
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

AC_CONFIG_FILES(Makefile
  src/repast_hpc/module.mk)
AC_OUTPUT
