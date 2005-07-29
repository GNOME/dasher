
inc = -I. -I../../DasherCore
lib = -lexpat

CFLAGS = 
CPPFLAGS = $(inc) 
LDFLAGS = $(lib)

HOBJS = FileLogger.o MemoryLeak.o SimpleTimer.o TimeSpan.o UserLocation.o UserLog.o UserLogParam.o UserLogTrial.o XMLUtil.o Alphabet.o AlphabetMap.o Trace.o AlphIO.o

UserLog: main.o $(HOBJS)
	g++ $(CFLAGS) $(inc) -c main.cpp
	g++ main.o $(HOBJS) $(hobj) $(LDFLAGS)
	mv a.out UserLogLoadTest

FileLogger.o: ../../DasherCore/FileLogger.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/FileLogger.cpp -o FileLogger.o

MemoryLeak.o: ../../DasherCore/MemoryLeak.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/MemoryLeak.cpp -o MemoryLeak.o

SimpleTimer.o: ../../DasherCore/SimpleTimer.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/SimpleTimer.cpp -o SimpleTimer.o

TimeSpan.o: ../../DasherCore/TimeSpan.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/TimeSpan.cpp -o TimeSpan.o

UserLocation.o: ../../DasherCore/UserLocation.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/UserLocation.cpp -o UserLocation.o

UserLog.o: ../../DasherCore/UserLog.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/UserLog.cpp -o UserLog.o

UserLogParam.o: ../../DasherCore/UserLogParam.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/UserLogParam.cpp -o UserLogParam.o

UserLogTrial.o: ../../DasherCore/UserLogTrial.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/UserLogTrial.cpp -o UserLogTrial.o

XMLUtil.o: ../../DasherCore/XMLUtil.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/XMLUtil.cpp -o XMLUtil.o

AlphabetMap.o: ../../DasherCore/Alphabet/AlphabetMap.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/Alphabet/AlphabetMap.cpp -o AlphabetMap.o

Alphabet.o: ../../DasherCore/Alphabet/Alphabet.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/Alphabet/Alphabet.cpp -o Alphabet.o

Trace.o: ../../Common/Trace.cpp
	g++ $(CFLAGS) $(inc) -c ../../Common/Trace.cpp -o Trace.o

AlphIO.o: ../../DasherCore/Alphabet/AlphIO.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/Alphabet/AlphIO.cpp -o AlphIO.o
