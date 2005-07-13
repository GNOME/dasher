
inc = -I. -I../../DasherCore
lib = -l

CFLAGS = 
CPPFLAGS = $(inc) -D USER_LOG_TOOL
LDFLAGS = $(lib)
HOBJS = FileLogger.o MemoryLeak.o SimpleTimer.o TimeSpan.o UserLocation.o UserLog.o UserLogParam.o UserLogTrial.o XMLUtil.o

UserLog:	main.o $(HOBJS)
	g++ $(CFLAGS) $(inc) -c main.cpp
	g++ main.o $(HOBJS) $(hobj) $(LDFLAGS)
	mv a.out UserLog

FileLogger.o: FileLogger.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/FileLogger.cpp -o FileLogger.o

MemoryLeak.o: MemoryLeak.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/MemoryLeak.cpp -o MemoryLeak.o
	
SimpleTimer.o: SimpleTimer.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/SimpleTimer.cpp -o SimpleTimer.o
	
TimeSpan.o: TimeSpan.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/TimeSpan.cpp -o TimeSpan.o
	
UserLocation.o: UserLocation.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/UserLocation.cpp -o UserLocation.o
	
UserLog.o: UserLog.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/UserLog.cpp -o UserLog.o
	
UserLogParam.o: UserLogParam.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/UserLogParam.cpp -o UserLogParam.o
	
UserLogTrial.o: UserLogTrial.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/UserLogTrial.cpp -o UserLogTrial.o
	
XMLUtil.o: XMLUtil.cpp
	g++ $(CFLAGS) $(inc) -c ../../DasherCore/XMLUtil.cpp -o XMLUtil.o
	