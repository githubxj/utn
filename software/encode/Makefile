#################################################################
CC = arm-linux-gcc
#CC = gcc
LD = ld
AR = ar

STRIP = arm-linux-strip
#STRIP = ls
RM = rm -f 
RM_CMD = $(RM) *.BAK *.bak *.o , * *~ *.a

LIBRARIES = $(SYS_LIBRARIES)


#CFLAGS = -Wall  -g  -mbig-endian
LDFLAGS = -L/usr/local/crosssqlite/crossppc/lib
CFLAGS = -Wall -g -I/usr/local/crosssqlite/crossppc/include

#DO NOT DELETE
SRC = .
OBJS = .

TARGETS = encodedaemon

all:$(TARGETS)
	$(STRIP)	 $^


$(OBJS)/main.o:$(SRC)/main.c
	$(CC) $(CFLAGS) -c $(SRC)/main.c -o $@ 
$(OBJS)/processmsg.o:$(SRC)/processmsg.c
	$(CC) $(CFLAGS) -c $(SRC)/processmsg.c -o $@
$(OBJS)/serial_comm.o:$(SRC)/../common/serial_comm.c
	$(CC) $(CFLAGS) -c $(SRC)/../common/serial_comm.c -o $@

$(OBJS)/logger.o: $(SRC)/../common/logger.c 
	$(CC) $(CFLAGS) -c $(SRC)/../common/logger.c -o $@
$(OBJS)/iniFile.o: $(SRC)/../common/iniFile.c 
	$(CC) $(CFLAGS) -c $(SRC)/../common/iniFile.c -o $@	

encodedaemon:main.o processmsg.o serial_comm.o logger.o iniFile.o
	$(CC) $(CFLAGS) -o $@ $^ -ldl -lpthread


clean:
	rm -f *.o $(TARGETS)











