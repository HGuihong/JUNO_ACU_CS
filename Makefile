#*************************************************************************
# Copyright (c) 2002 The University of Chicago, as Operator of Argonne
# National Laboratory.
# Copyright (c) 2002 The Regents of the University of California, as
# Operator of Los Alamos National Laboratory.
# This file is distributed subject to a Software License Agreement found
# in the file LICENSE that is included with this distribution. 
#*************************************************************************
#
# Makefile,v 1.4 2007/05/22 14:34:07 jba Exp
#
# Modify, by Guihong 2021/08/16
TOP = ../../..
include $(TOP)/configure/CONFIG

PROD_LIBS = ca Com  
ca_DIR=$(EPICS_BASE_LIB)
Com_DIR=$(EPICS_BASE_LIB)


PROD_HOST = marchive

marchive_SRCS += marchive.c

USR_CFLAGS += -I/usr/local/mysql/include

USR_LDFLAGS += -L/usr/local/mysql/lib

marchive_LIBS += $(EPICS_BASE_HOST_LIBS)

marchive_SYS_LIBS += mysqlclient pthread z m dl ssl crypto



include $(TOP)/configure/RULES

