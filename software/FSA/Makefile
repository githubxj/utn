######################################################################

CC = arm-linux-gcc
#CC = gcc


STRIP = arm-linux-strip
#STRIP = ls
RM = rm -f
RM_CMD = $(RM) *.BAK *.bak *.o ,* *~ *.a

LIBRARIES = $(SYS_LIBRARIES)


#CFLAGS= -Wall -g  -mbig-endian
CFLAGS= -Wall -g -I/usr/local/crosssqlite/include -I../emsagent
#CFLAGS= -Wall -g -I/usr/local/crosssqlite/include -I../emsagent -DDAT_CARD
# DO NOT DELETE
SRC = .
OBJS = .

TARGETS = fsadaemon watchdog
#fpga_fifo comtest

all: $(TARGETS)
	$(STRIP) $^

#	@echo "............"

$(OBJS)/main.o: $(SRC)/main.c 
	$(CC) $(CFLAGS) -c $(SRC)/main.c -o $@
$(OBJS)/db_interface.o: $(SRC)/db_interface.c 
	$(CC) $(CFLAGS) -c $(SRC)/db_interface.c -o $@
$(OBJS)/data_pipe.o: $(SRC)/data_pipe.c 
	$(CC) $(CFLAGS) -c $(SRC)/data_pipe.c -o $@
$(OBJS)/message_proc.o: $(SRC)/message_proc.c 
	$(CC) $(CFLAGS) -c $(SRC)/message_proc.c -o $@
$(OBJS)/fsalib.o: $(SRC)/fsalib.c 
	$(CC) $(CFLAGS) -c $(SRC)/fsalib.c -o $@
$(OBJS)/proc_alarm.o: $(SRC)/proc_alarm.c 
	$(CC) $(CFLAGS) -c $(SRC)/proc_alarm.c -o $@
$(OBJS)/syscall_wrapper.o: $(SRC)/syscall_wrapper.c 
	$(CC) $(CFLAGS) -c $(SRC)/syscall_wrapper.c -o $@
$(OBJS)/serial_comm.o: $(SRC)/../common/serial_comm.c 
	$(CC) $(CFLAGS) -c $(SRC)/../common/serial_comm.c -o $@
$(OBJS)/logger.o: $(SRC)/../common/logger.c 
	$(CC) $(CFLAGS) -c $(SRC)/../common/logger.c -o $@
$(OBJS)/iniFile.o: $(SRC)/../common/iniFile.c 
	$(CC) $(CFLAGS) -c $(SRC)/../common/iniFile.c -o $@
$(OBJS)/refresh_check.o: $(SRC)/../emsagent/refresh_check.c 
	$(CC) $(CFLAGS) -c $(SRC)/../emsagent/refresh_check.c -o $@
$(OBJS)/fsa_alarm.o: $(SRC)/fsa_alarm.c 
	$(CC) $(CFLAGS) -c $(SRC)/fsa_alarm.c -o $@

$(OBJS)/fpga_filo.o: $(SRC)/fpga_fifo.c 
	$(CC) $(CFLAGS) -c $(SRC)/fpga_fifo.c -o $@
	
$(OBJS)/485test.o: $(SRC)/485test.c 
	$(CC) $(CFLAGS) -c $(SRC)/485test.c -o $@
$(OBJS)/comtest.o: $(SRC)/comtest.c 
	$(CC) $(CFLAGS) -c $(SRC)/comtest.c -o $@

$(OBJS)/watchdog.o: $(SRC)/watchdog.c 
	$(CC) $(CFLAGS) -c $(SRC)/watchdog.c -o $@

fsadaemon:	main.o message_proc.o serial_comm.o fsalib.o logger.o iniFile.o db_interface.o data_pipe.o proc_alarm.o syscall_wrapper.o refresh_check.o fsa_alarm.o
	$(CC)  $(CFLAGS)  -o $@ $^ -lsqlite3 -ldl  -lpthread 
	$(STRIP) $@
watchdog: watchdog.o
	$(CC) $(CFLAGS)  -o $@ $^
	$(STRIP) $@
	
fpga_fifo:	fpga_fifo.o fsalib.o logger.o iniFile.o
	$(CC)  $(CFLAGS) -lpthread -o $@ $^


comtest:	comtest.o 
	$(CC)  $(CFLAGS) -o $@ $^
	$(STRIP) $@
	
clean:
	rm -f *.o  $(TARGETS)


