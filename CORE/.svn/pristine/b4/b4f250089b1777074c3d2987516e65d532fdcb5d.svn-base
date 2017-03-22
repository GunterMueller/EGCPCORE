# Makefile
# 	for preparing softwares, under $(COREPATH)/builds,

-include ../Make.config


######################################################################
#	GMP Version
######################################################################
# GMP 5.1 http://gmplib.org/gmp5.1.html
# GMP 5.0 http://gmplib.org/gmp5.0.html
# GMP 5.0.3 or later version is compatible with Mac OS X 10.8

# CHEE: The following NOT be hardcoded here!  Moved into ../Make.config...
#	GMP=gmp-5.0.1
#	MPFR=mpfr-3.0.0  # Chee: obsolete (Aug2011)
#	MPFR=mpfr-3.0.1  # old
#	MPFR=mpfr-3.1.2 
#	SIP=sip-4.3.2

PLATFORM=macx-g++
PREFIX=$(shell dirname `pwd`)
GMP_PREFIX=${PREFIX}/gmp
MPFR_PREFIX=${PREFIX}/mpfr


# GMP's configure program will try to build 64-bits library on Sun platform, 
# 	so we have to force it to build 32-bits library.
OS=$(shell uname)
ifeq ($(OS), SunOS)
  GMP_ABI="ABI=32"
endif

all:

gmp: config-gmp build-gmp install-gmp

mpfr: config-mpfr build-mpfr install-mpfr

# config, build and install gmp
config-gmp: ${GMP}
	cd ${GMP}; ./configure CC=gcc ${GMP_ABI} --prefix=${GMP_PREFIX}

build-gmp:
	cd ${GMP}; ${MAKE}

install-gmp:
	cd ${GMP}; ${MAKE} install

# config, build and install mpfr
config-mpfr: ${MPFR}
	cd ${MPFR}; ./configure CC=gcc --prefix=${MPFR_PREFIX} \
	    --with-gmp-include=${GMP_PREFIX}/include \
	    --with-gmp-lib=${GMP_PREFIX}/lib

build-mpfr:
	cd ${MPFR}; ${MAKE}

install-mpfr:
	cd ${MPFR}; ${MAKE} install

# config, build and install sip
config-sip: ${SIP}
	cd ${SIP}; python configure.py -x -p ${PLATFORM} \
	    -b ${PREFIX}/bin -d ${PREFIX}/lib \
	    -e ${PREFIX}/inc -v ${PREFIX}/sip

build-sip:
	cd ${SIP}; ${MAKE}

install-sip:
	cd ${SIP}; ${MAKE} install


${GMP}: ${GMP}.tar.gz
	# OLD: gunzip -cd ${GMP}.tar.gz | tar xvf -
	tar xvf ${GMP}.tar.gz

${GMP}.tar.gz: 
	wget ftp://ftp.gnu.org/gnu/gmp/${GMP}.tar.gz

${MPFR}: ${MPFR}.tar.xz
	# OLD: gunzip -cd ${MPFR}.tar.gz | tar xvf -
	tar xvf ${MPFR}.tar.xz

${MPFR}.tar.xz:
	wget http://www.mpfr.org/mpfr-current/${MPFR}.tar.xz

${SIP}: ${SIP}.tar.gz
	# OLD: gunzip -cd ${SIP}.tar.gz | tar xvf -
	tar xvf ${SIP}.tar.gz
	
${SIP}.tar.gz: 
	wget http://www.riverbankcomputing.com/Downloads/sip4/${SIP}.tar.gz

