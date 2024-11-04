# Common definitions.  ROOT should be defined before loading this file.
# This file should not need to be modified.   Local modifications should
# go in config/config.mk.

# If there is a file named config.mk in directory config, load it.
# Otherwise, load config_default.mk...
CONFIGS = $(ROOT)/config
ifneq ("$(wildcard $(CONFIGS)/config.mk)","")
CONFIG = $(CONFIGS)/config.mk
else
CONFIG = $(CONFIGS)/config_default.mk
endif
include $(CONFIG)

# Important subdirectories
SRC = $(ROOT)/src
INC = $(ROOT)/include
DRIVER_DIR = $(SRC)/driver
SET_DIR = $(SRC)/hashset
LIST_DIR = $(SRC)/list
QUEUE_DIR = $(SRC)/queue
PQUEUE_DIR = $(SRC)/pqueue
SCHEDULE_DIR = $(SRC)/schedule

# Important files
AR_INC = $(INC)/AtomicReference.cvh
AR_SRC = $(SRC)/util/AtomicReference.cvl
AMR_INC = $(INC)/AtomicMarkableReference.cvh
AMR_SRC = $(SRC)/util/AtomicMarkableReference.cvl
AB_INC = $(INC)/AtomicBoolean.cvh
AB_SRC = $(SRC)/util/AtomicBoolean.cvl
AI_INC = $(INC)/AtomicInteger.h
AI_SRC = $(SRC)/util/AtomicInteger.cvl
COND_INC = $(INC)/Condition.h
COND_SRC = $(SRC)/util/Condition.cvl
COND2_INC = $(INC)/Condition_dl.h
COND2_SRC = $(SRC)/util/Condition_dl.cvl
TID_INC = $(INC)/tid.h
TID_SRC = $(SRC)/util/tid.cvl
HASH_INC = $(INC)/hash.cvh
HASH_SRC = $(SRC)/util/hash.cvl
LOCK_INC = $(INC)/Lock.h
LOCK_SRC = $(SRC)/util/ReentrantLock.cvl
FAIRLOCK_SRC = $(SRC)/util/FairReentrantLock.cvl
ARRAYLIST_INC = $(INC)/ArrayList.h
ARRAYLIST_SRC = $(SRC)/util/ArrayList.cvl
DRIVER_INC = $(INC)/driver.h $(INC)/perm.h $(INC)/schedule.h $(INC)/types.h \
  $(INC)/tid.h
DRIVER_B_SRC = $(DRIVER_DIR)/driver_base_b.cvl $(DRIVER_DIR)/perm.c \
  $(DRIVER_DIR)/schedule.cvl $(SRC)/util/tid.cvl
DRIVER_NB_SRC = $(DRIVER_DIR)/driver_base_nb.cvl $(DRIVER_DIR)/perm.c \
  $(DRIVER_DIR)/schedule.cvl $(SRC)/util/tid.cvl
DRIVER_SET_B = $(DRIVER_DIR)/driver_set_b.cvl
DRIVER_SET_NB = $(DRIVER_DIR)/driver_set_nb.cvl
DRIVER_QUEUE_B = $(DRIVER_DIR)/driver_queue_b.cvl
DRIVER_QUEUE_NB = $(DRIVER_DIR)/driver_queue_nb.cvl
DRIVER_PQUEUE_B = $(DRIVER_DIR)/driver_pqueue_b.cvl
DRIVER_PQUEUE_NB = $(DRIVER_DIR)/driver_pqueue_nb.cvl

# Verification commands
VERIFY = $(CIVL) verify -userIncludePath=$(INC)
JROOT = $(ROOT)/java
JSRC=$(JROOT)/src/ampver/av
MOD_PATH = $(JROOT)/bin:$(CIVL_ROOT)/mods/dev.civl.mc/bin:$(CIVL_ROOT)/mods/dev.civl.abc/bin:$(CIVL_ROOT)/mods/dev.civl.sarl/bin:$(CIVL_ROOT)/mods/dev.civl.gmc/bin:$(VSL_DEPS)/mods/antlr3:$(VSL_DEPS)/mods/antlr4
MAIN_CLASS = $(JROOT)/bin/ampver/av/AMPVer.class
AMPVER = $(JAVA) -p $(MOD_PATH) -m ampver/av.AMPVer -root=$(ROOT)

myall: all

clean::
	rm -rf CIVLREP AVREP_* *~ *.tmp a.out *.exec *.o

.PHONY: myall clean

all: $(MAIN_CLASS)

SOURCES = $(JROOT)/src/ampver/module-info.java \
    $(JSRC)/AMPVer.java \
    $(JSRC)/Step.java \
    $(JSRC)/Schedule.java \
    $(JSRC)/SetScheduleIterator.java \
    $(JSRC)/QueueScheduleIterator.java \
    $(JSRC)/PQScheduleIterator.java \
    $(JSRC)/AVUtil.java

$(MAIN_CLASS): $(SOURCES)
	$(JAVAC) -d $(JROOT)/bin/ampver \
          -p $(CIVL_ROOT)/mods/dev.civl.mc/bin $(SOURCES)
