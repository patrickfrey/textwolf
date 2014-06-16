
CCINCLUDES= -Iinclude/
CCFLAGS= -g -Wall -O4 
CC= gcc
LINK= g++ -lc
LINKFLAGS=
LIBS=
OBJS=\
	examples/TextScanner.o\
	examples/XMLPathSelect.o\
	examples/XMLScanner.o\
	examples/XMLScanner_chunkwise.o

PRGS=\
	tests/readStdinIterator.o\
	tests/test_TextReader.o\
	tests/test_XMLPathSelect.o\
	tests/test_XMLScanner.o

%.o : %.cpp
	$(CC) -c -o $@ $(CCFLAGS) $(CCINCLUDES) $<

%: %.o $(OBJS)
	$(LINK) -o $@ $(LINKFLAGS) $(OBJS) $< $(LIBS)


all: $(PRGS) $(OBJS)

clean:
	-@rm -f $(OBJS) $(PRGS) $(PRGS)


