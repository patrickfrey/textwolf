#
# Make file for GNU to build the textwolf test programs
#
CFLAGS=-g -O4 -fstrict-aliasing -pedantic -Wall -Wunused -Wno-import -Wformat -Wformat-y2k -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wswitch-enum -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wmissing-noreturn -Wno-multichar -Wparentheses -Wredundant-decls -Wdisabled-optimization -Wno-long-long -Werror -Wfatal-errors
PROGS=feedPhoneBookDataXML test_XMLPathSelect createPhoneBookDataXML test_TextReader test_XMLScanner test_Echo

all: $(PROGS)
clean:
	rm -f *~ *.o $(PROGS) textwolf/iterator/*~

%: %.o
	g++ -lc -o $@ $<

%.o : %.cpp textwolf.hpp
	g++ -c -o $@  $(CFLAGS) $<

