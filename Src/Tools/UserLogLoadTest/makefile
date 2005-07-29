
inc = -I. -I../../DasherCore
lib = -lexpat -ldashercore -ldasheralphabet -l dasherlm -lcanna -L../../DasherCore -L../../DasherCore/Alphabet -L../../DasherCore/LanguageModelling

CFLAGS = 
CPPFLAGS = $(inc) 
LDFLAGS = $(lib)

UserLog: main.o $(HOBJS)
	g++ $(CFLAGS) $(inc) -c main.cpp
	g++ main.o $(HOBJS) $(hobj) $(LDFLAGS)
	mv a.out UserLogLoadTest

