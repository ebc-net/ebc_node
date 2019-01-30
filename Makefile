SRCDIR = src
SRC=$(wildcard $(SRCDIR)/*.cpp)
OBJ=$(patsubst $(SRCDIR)/%.cpp, $(SRCDIR)/%.o ,$(SRC))

CC = g++


LIBDIR += lib/linux
LIBS += -ludt4  -lprotobuf-lite -lpthread -lebcCryptoLib
INCLUDE += include

CXXFLAGS += -O0 -Wall -g -D_REENTRANT -pipe -std=c++11
CXXFLAGS += -L$(LIBDIR) -I$(INCLUDE)

TARGET = ebc

all:$(TARGET)
	rm -rf $(OBJ)


$(TARGET): $(OBJ)
	$(CC) $(CXXFLAGS) -o $@  $^  $(LIBS)


$(SRCDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CXXFLAGS) -o $@ -c $<

udt:
	cd test
	$(CC) $(CXXFLAGS) -o udt udt.cpp $(LIBS)

.PHONY:clean



clean:
	rm -rf $(OBJ) $(TARGET)
