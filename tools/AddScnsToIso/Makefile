TARGET = AddScnsToIso.out
OBJS = AddScnsToIso.o

LIBS = -lstdc++ -lstdc++fs

ifeq ($(DEBUG),1)
CFLAGS = -O0 -Wall -g -ggdb3
CXXFLAGS =  -O0 -Wall -std=c++17 -g -ggdb3
LDFLAGS =
else
CFLAGS = -O3 -Wall
CXXFLAGS =  -O3 -Wall -std=c++17
LDFLAGS = -s
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

rebuild: clean all

clean:
	rm -rf $(TARGET) $(OBJS)

