CXX=g++
RM=rm -f
CPPFLAGS=-g -Wall
SRCS=shell.cpp parse.cpp
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
PROG=simpleShell

all: $(PROG)

$(PROG): $(OBJS)
	$(CXX) -o $(PROG).exe $(OBJS)

%.o: %.c
	$(CXX) -c $(CPPFLAGS) $< -o $@

clean:
	$(RM) $(OBJS) $(PROG).exe

run: $(PROG).exe
	./$(PROG).exe

