############################################################################
# apps/f1000/f1000_client/Makefile
#
#   Copyright (C) 2008, 2010-2013 Gregory Nutt. All rights reserved.
#   Author: Gregory Nutt <gnutt@nuttx.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name NuttX nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
############################################################################

-include $(TOPDIR)/Make.defs

CFLAGS += -I $(TOPDIR)/libs/lib_iqa_api/include -I $(TOPDIR)/libs/lib_iqa_api/include/hxx
CFLAGS += -I $(TOPDIR)/libs/lib_iqa_evm/include 

# F1000 client

CONFIG_F1000_CLIENT_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_F1000_CLIENT_STACKSIZE ?= 4096

APPNAME = f1000_client

PRIORITY  = $(CONFIG_F1000_CLIENT_PRIORITY)
STACKSIZE = $(CONFIG_F1000_CLIENT_STACKSIZE)

# f1000 client code

ASRCS =
CSRCS = dac_mem.c f1000_command_decoder.c iqa_f1000_chip.c
MAINSRC = f1000_client.c

CONFIG_F1000_CLIENT_PROGNAME ?= f1000_client$(EXEEXT)
PROGNAME = $(CONFIG_F1000_CLIENT_PROGNAME)

MODULE = CONFIG_F1000_CLIENT

-include $(APPDIR)/Application.mk

CLIENT_CFLAGS = -Wall -g -DSTANDALONE
CLIENT_CSRCS  = src/main.c src/client.c src/dac_mem.c src/f1000_regs.c src/eval_expr.c
CLIENT_CSRCS += src/ifproc.c
CLIENT_INCL   = src/client.h src/f1000_regs.h src/eval_expr.h src/ifproc.h
CLIENT_LIBS   = -lreadline -lm

client: $(CLIENT_CSRCS) $(CLIENT_INCL)
	gcc $(CLIENT_CFLAGS) -o client $(CLIENT_CSRCS) $(CLIENT_LIBS)


#vim: noet
