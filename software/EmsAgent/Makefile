######################################################################

CC = ppc-linux-g++ 
#CC = gcc
LD = ld
AR = ar

STRIP = ppc-linux-strip
#STRIP = ls
RM = rm -f
RM_CMD = $(RM) *.BAK *.bak *.o ,* *~ *.a

LIBRARIES = $(SYS_LIBRARIES)


#CFLAGS= -Wall -g  -mbig-endian
CFLAGS= -Wall -g 
# DO NOT DELETE
SRC = .
OBJS = .

TARGETS = emsagent  rtc_time
# comtest

all: $(TARGETS)
	$(STRIP) $^

#	@echo "............"

$(OBJS)/main.o: $(SRC)/main.c 
	$(CC) $(CFLAGS) -c $(SRC)/main.c -o $@
$(OBJS)/message_proc.o: $(SRC)/message_proc.c 
	$(CC) $(CFLAGS) -c $(SRC)/message_proc.c -o $@
$(OBJS)/alarm_proc.o: $(SRC)/alarm_proc.c 
	$(CC) $(CFLAGS) -c $(SRC)/alarm_proc.c -o $@
$(OBJS)/card_mgmt.o: $(SRC)/card_mgmt.c 
	$(CC) $(CFLAGS) -c $(SRC)/card_mgmt.c -o $@
$(OBJS)/refresh_check.o: $(SRC)/refresh_check.c 
	$(CC) $(CFLAGS) -c $(SRC)/refresh_check.c -o $@
$(OBJS)/serial_comm.o: $(SRC)/../common/serial_comm.c 
	$(CC) $(CFLAGS) -c $(SRC)/../common/serial_comm.c -o $@
$(OBJS)/sw_ctrl.o: $(SRC)/sw_ctrl.c 
	$(CC) $(CFLAGS) -c $(SRC)/sw_ctrl.c -o $@

$(OBJS)/logger.o: $(SRC)/../common/logger.c 
	$(CC) $(CFLAGS) -c $(SRC)/../common/logger.c -o $@
$(OBJS)/iniFile.o: $(SRC)/../common/iniFile.c 
	$(CC) $(CFLAGS) -c $(SRC)/../common/iniFile.c -o $@

$(OBJS)/485test.o: $(SRC)/485test.c 
	$(CC) $(CFLAGS) -c $(SRC)/485test.c -o $@

$(OBJS)/405_time.o: $(SRC)/405_time.c 
	$(CC) $(CFLAGS) -c $(SRC)/405_time.c -o $@

$(OBJS)/back_mgmt.o: $(SRC)/back_mgmt.c 
	$(CC) $(CFLAGS) -c $(SRC)/back_mgmt.c -o $@

$(OBJS)/proc_svc_switch.o: $(SRC)/proc_svc_switch.c 
	$(CC) $(CFLAGS) -c $(SRC)/proc_svc_switch.c -o $@

emsagent:	main.o message_proc.o serial_comm.o card_mgmt.o alarm_proc.o sw_ctrl.o  logger.o iniFile.o refresh_check.o back_mgmt.o proc_svc_switch.o
	$(CC)  $(CFLAGS) -o $@ $^ -lsqlite3  -ldl -lpthread
	$(STRIP) $@
	
comtest:	485test.o serial_comm.o
	$(CC)  $(CFLAGS) -o $@ $^
	$(STRIP) $@

rtc_time: 405_time.o
	$(CC)  $(CFLAGS) -o $@ $^
	$(STRIP) $@

clean:
	rm -f *.o  $(TARGETS)


