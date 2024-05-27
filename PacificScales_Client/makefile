CXX = g++
CXXFLAGS = -std=c++0x -Wall -pedantic-errors -g

SRCS =  main.cpp parse.cpp reporting_service.cpp
OBJS = ${SRCS:.cpp=.o}
HEADERS = reporting_service.h parse.h

MAIN = psclient

all: ${MAIN}
	@echo   Compiled all

${MAIN}: ${OBJS}
	${CXX} ${CXXFLAGS} ${OBJS} -o ${MAIN}

.cpp.o:
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	${RM} ${PROGS} ${OBJS} *.o *~.
