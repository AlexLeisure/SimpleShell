CXX=g++
RM=rm -f
CPPFLAGS=-g --WALL
SRCS=shell.cpp parse.cpp
OBJS=$(subst %.cpp,%.o,$(SRCS))
PROG=simpleShell

all: $(PROG)

$(PROG): $(OBJS)
	$(CXX) -o $(PROG) $(OBJS)

%.o: %.c
	$(CXX) -c $(CPPFLAGS) $< -o $@

clean:
	$(RM) $(OBJS)
