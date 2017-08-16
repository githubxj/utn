######################################################################

#CC = arm-linux-g++ 
CC = ppc-linux-g++ 
#CC = g++ 
LD = ld
AR = ar

#STRIP = arm-linux-strip
STRIP = ppc-linux-strip
#STRIP = ls
RM = rm -f
RM_CMD = $(RM) *.BAK *.bak *.o ,* *~ *.a

LIBRARIES = $(SYS_LIBRARIES)


#CFLAGS= -Wall -g  -mbig-endian
CFLAGS= -Wall -g -I../../common
#CFLAGS= -Wall -g -DCOVOND -I../../common
# DO NOT DELETE
SRC = .
OBJS = .

BASE_OBJECTS= \
	$(OBJS)/CgiUtils.o \
	$(OBJS)/Lang.o \
	$(OBJS)/FormEntry.o \
	$(OBJS)/FormFile.o \
	$(OBJS)/CGISession.o \
	$(OBJS)/CgiRequest.o 
	
TARGETS = main.cgi

 all: $(TARGETS)
	$(STRIP) $^

host:
	cp -f $^ /var/www/cgi-bin/
	cd /var/www/cgi-bin/
	ln -s main.cgi login.cgi
	ln -s main.cgi frameview.cgi 
	ln -s main.cgi sidemenu.cgi 
	ln -s main.cgi chgpwd.cgi 
	ln -s main.cgi show_card.cgi
	ln -s main.cgi show_onu.cgi 
	ln -s main.cgi topoview.cgi
	ln -s main.cgi chgcard.cgi
	ln -s main.cgi perfview.cgi
	ln -s main.cgi chgonu.cgi
	ln -s main.cgi getalarm.cgi
	ln -s main.cgi clearalarm.cgi
	ln -s main.cgi netconfig.cgi
	ln -s main.cgi reboot.cgi
	ln -s main.cgi snmpconfig.cgi
	ln -s main.cgi sysdate.cgi
	ln -s main.cgi sfpinfo.cgi
	ln -s main.cgi nttperfview.cgi

#	@echo "............"

$(OBJS)/CgiRequest.o: $(SRC)/CgiRequest.cpp $(SRC)/CgiRequest.h
	$(CC) $(CFLAGS) -c $(SRC)/CgiRequest.cpp -o $@
$(OBJS)/CgiUtils.o: $(SRC)/CgiUtils.cpp $(SRC)/CgiUtils.h
	$(CC) $(CFLAGS) -c $(SRC)/CgiUtils.cpp -o $@
$(OBJS)/Lang.o: $(SRC)/Lang.cpp
	$(CC) $(CFLAGS) -c $(SRC)/Lang.cpp -o $@
$(OBJS)/FormEntry.o: $(SRC)/FormEntry.cpp $(SRC)/FormEntry.h
	$(CC) $(CFLAGS) -c $(SRC)/FormEntry.cpp -o $@
$(OBJS)/FormFile.o: $(SRC)/FormFile.cpp $(SRC)/FormFile.h
	$(CC) $(CFLAGS) -c $(SRC)/FormFile.cpp -o $@
$(OBJS)/CGISession.o: $(SRC)/CGISession.cpp $(SRC)/CGISession.h
	$(CC) $(CFLAGS) -c $(SRC)/CGISession.cpp -o $@
$(OBJS)/CppSQLite3.o: $(SRC)/CppSQLite3.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CppSQLite3.cpp -o $@
$(OBJS)/iniFile.o: $(SRC)/../../common/iniFile.c
	$(CC) $(CFLAGS) -c $(SRC)/../../common/iniFile.c -o $@	
$(OBJS)/logger.o: $(SRC)/../../common/logger.c
	$(CC) $(CFLAGS) -c $(SRC)/../../common/logger.c -o $@	

$(OBJS)/Comm.o: $(SRC)/Comm.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/Comm.cpp -o $@

$(OBJS)/CgiMain.o: $(SRC)/CgiMain.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CgiMain.cpp -o $@
$(OBJS)/CLogin.o: $(SRC)/CLogin.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CLogin.cpp -o $@
$(OBJS)/CFrameView.o: $(SRC)/CFrameView.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CFrameView.cpp -o $@
$(OBJS)/CTopoView.o: $(SRC)/CTopoView.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CTopoView.cpp -o $@
$(OBJS)/CSidemenu.o: $(SRC)/CSidemenu.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CSidemenu.cpp -o $@
$(OBJS)/CPasswordChg.o: $(SRC)/CPasswordChg.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CPasswordChg.cpp -o $@
$(OBJS)/CGetCard.o: $(SRC)/CGetCard.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CGetCard.cpp -o $@
$(OBJS)/CGetOnu.o: $(SRC)/CGetOnu.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CGetOnu.cpp -o $@
$(OBJS)/CCardChg.o: $(SRC)/CCardChg.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CCardChg.cpp -o $@
$(OBJS)/CPerf.o: $(SRC)/CPerf.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CPerf.cpp -o $@
$(OBJS)/COnuChg.o: $(SRC)/COnuChg.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/COnuChg.cpp -o $@
$(OBJS)/CAlarm.o: $(SRC)/CAlarm.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CAlarm.cpp -o $@
$(OBJS)/CClearAlarm.o: $(SRC)/CClearAlarm.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CClearAlarm.cpp -o $@
$(OBJS)/CNetconfig.o: $(SRC)/CNetconfig.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CNetconfig.cpp -o $@
$(OBJS)/CReboot.o: $(SRC)/CReboot.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CReboot.cpp -o $@
$(OBJS)/CSnmpconfig.o: $(SRC)/CSnmpconfig.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CSnmpconfig.cpp -o $@
$(OBJS)/CSysdate.o :$(SRC)/CSysdate.cpp
	$(CC) $(CFLAGS) -c $(SRC)/CSysdate.cpp -o $@

$(OBJS)/CGetSfp.o :$(SRC)/CGetSfp.cpp
	$(CC) $(CFLAGS) -c $(SRC)/CGetSfp.cpp -o $@
$(OBJS)/CRefreshCheck.o :$(SRC)/CRefreshCheck.cpp
	$(CC) $(CFLAGS) -c $(SRC)/CRefreshCheck.cpp -o $@
$(OBJS)/CUpgrade.o :$(SRC)/CUpgrade.cpp
	$(CC) $(CFLAGS) -c $(SRC)/CUpgrade.cpp -o $@
$(OBJS)/CCount.o :$(SRC)/CCount.cpp
	$(CC) $(CFLAGS) -c $(SRC)/CCount.cpp -o $@	
$(OBJS)/CVlan.o :$(SRC)/CVlan.cpp
	$(CC) $(CFLAGS) -c $(SRC)/CVlan.cpp -o $@	
$(OBJS)/CMulticast.o :$(SRC)/CMulticast.cpp
	$(CC) $(CFLAGS) -c $(SRC)/CMulticast.cpp -o $@
$(OBJS)/CONUIDmgmt.o :$(SRC)/CONUIDmgmt.cpp
	$(CC) $(CFLAGS) -c $(SRC)/CONUIDmgmt.cpp -o $@
$(OBJS)/CL3Config.o :$(SRC)/CL3Config.cpp
	$(CC) $(CFLAGS) -c $(SRC)/CL3Config.cpp -o $@
$(OBJS)/CRoute.o :$(SRC)/CRoute.cpp
	$(CC) $(CFLAGS) -c $(SRC)/CRoute.cpp -o $@
$(OBJS)/CGetGe8.o: $(SRC)/CGetGe8.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CGetGe8.cpp -o $@
$(OBJS)/CFsaCard.o: $(SRC)/CFsaCard.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CFsaCard.cpp -o $@
$(OBJS)/CNttPerfView.o: $(SRC)/CNttPerfView.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CNttPerfView.cpp -o $@
$(OBJS)/CEpnView.o: $(SRC)/CEpnView.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CEpnView.cpp -o $@
$(OBJS)/CGetEPOnu.o: $(SRC)/CGetEPOnu.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CGetEPOnu.cpp -o $@
$(OBJS)/CEPOnuChg.o: $(SRC)/CEPOnuChg.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CEPOnuChg.cpp -o $@
$(OBJS)/CHDCard.o: $(SRC)/CHDCard.cpp 
	$(CC) $(CFLAGS) -c $(SRC)/CHDCard.cpp -o $@


main.cgi:	$(BASE_OBJECTS) CgiMain.o CLogin.o iniFile.o logger.o CTopoView.o CFrameView.o CSidemenu.o CPasswordChg.o  CGetCard.o CPerf.o \
			CCardChg.o  CGetOnu.o COnuChg.o CAlarm.o CClearAlarm.o CNetconfig.o CReboot.o CSnmpconfig.o CVlan.o CMulticast.o \
			CSysdate.o CGetSfp.o Comm.o CppSQLite3.o CRefreshCheck.o CCount.o CONUIDmgmt.o CL3Config.o CRoute.o CGetGe8.o CFsaCard.o \
			CNttPerfView.o CEpnView.o CGetEPOnu.o CEPOnuChg.o CHDCard.o CUpgrade.o
	$(CC)  $(CFLAGS) -o $@ $^ -lsqlite3  -ldl -lpthread

clean:
	rm -f *.o  $(TARGETS)


