#VAR=mem.o reg.o Sim1553B_chan.o simbc.o simbus.o simmt.o simrt.o definition.o log.o wordParser.o stdafx.o transException.o
VAR=mem.o reg.o Sim1553B_chan.o simbc.o simbus.o simmt.o simrt.o definition.o log.o wordParser.o stdafx.o transException.o
CFLAGS += -g -D _LOG_LEVEL_INFO_  -fPIC
Sim1553B_chan:$(VAR)
	g++ -g -o Sim1553B_chan $(VAR)
libsim61580.so:$(VAR)
	g++ -shared -g -o libsim61580.so $(VAR)
Sim1553B_chan.o:simmt.h simmt.h simbc.h Sim1553B_chan.cpp
	g++ -c $(CFLAGS) -o Sim1553B_chan.o  Sim1553B_chan.cpp
simbc.o:reg.h mem.h simbus.h transException.h simbc.cpp stdafx.o
	g++ -c $(CFLAGS) -o simbc.o simbc.cpp
simrt.o:reg.h mem.h simbus.h transException.h simrt.cpp stdafx.o
	g++ -c $(CFLAGS) -o simrt.o simrt.cpp
simmt.o:reg.h mem.h transException.h simbus.h simmt.cpp stdafx.o
	g++ -c $(CFLAGS) -o simmt.o simmt.cpp
reg.o:definition.h reg.cpp stdafx.o
	g++ -c $(CFLAGS) -o reg.o reg.cpp
mem.o:definition.h mem.cpp stdafx.o
	g++ -c $(CFLAGS) -o mem.o mem.cpp
wordParser.o:wordParser.h simbus.o stdafx.o
	g++ -c $(CFLAGS) -o wordParser.o wordParser.cpp
simbus.o:definition.h simbus.cpp stdafx.o
	g++ -c $(CFLAGS) -o simbus.o simbus.cpp
transException.o:definition.h transException.cpp stdafx.o
	g++ -c $(CFLAGS) -o transException.o transException.cpp
definition.o:definition.h log.o stdafx.o
	g++ -c $(CFLAGS) -o definition.o definition.cpp
log.o:log.h stdafx.o
	g++ -c $(CFLAGS) -o log.o log.cpp
stdafx.o:stdafx.h targetver.h
	g++ -c $(CFLAGS) -o stdafx.o stdafx.cpp

.PHONY:clean
clean:
	rm -- *.so *.o 