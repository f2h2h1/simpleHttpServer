cc = gcc
project = http
count = 
mainsrc = $(project)$(count).c
executable = $(project)$(count).exe
dynamiclib = -lwsock32 -lws2_32
target = target/
src = src/

all: $(executable)

$(executable): $(src)$(mainsrc) linklist.o parser.o
	$(cc) $(src)$(mainsrc) $(target)linklist.o $(target)parser.o -o $(target)$(executable) $(dynamiclib)

linklist.o: $(src)linklist.c
	$(cc) -c $(src)linklist.c -o $(target)linklist.o

parser.o: $(src)parser.c
	$(cc) -c $(src)parser.c -o $(target)parser.o

clean:
	del $(target)*.o $(executable)

clean-exe:
	del $(target)$(executable)

exe:
	$(target)$(executable)