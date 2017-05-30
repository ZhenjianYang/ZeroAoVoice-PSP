TARGET = AddScnsToIso.out
OBJS = AddScnsToIso.o

LIBS = -lstdc++ -lstdc++fs

CFLAGS = -O3 -Wall
CXXFLAGS =  -O3 -Wall -std=c++17
LDFLAGS = -s

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

rebuild: clean all

clean:
	rm -rf $(TARGET) $(OBJS)

