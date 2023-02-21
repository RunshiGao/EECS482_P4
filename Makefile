CC=g++ -g -Wall -std=c++17

# List of source files for your file server
FS_SOURCES=server.cpp filesystem.cpp

# Generate the names of the file server's object files
FS_OBJS=${FS_SOURCES:.cpp=.o}

all: fs app test1 test2 test3 test4 test5 test6 test7 test8 test9 test10 test11 test12 test13 test14

# Compile the file server and tag this compilation
fs: ${FS_OBJS} libfs_server.o
	./autotag.sh
	${CC} -o $@ $^ -pthread -ldl

# Compile a client program
app: app.cpp libfs_client.o
	${CC} -o $@ $^

test%: test%.cpp libfs_client.o
	${CC} -o $@ $^

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${FS_OBJS} fs app test1 test2 test3 test4 test5 test6 test7 test8 test9 test10 test11 test12 test13 test14
