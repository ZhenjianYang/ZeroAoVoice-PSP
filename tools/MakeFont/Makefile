TARGET = MakeFont.out
OBJS = MakeFont.o

LIBS = -lfreetype -lstdc++
INCS = -I/usr/include/freetype2

ifeq ($(DEBUG),1)
CFLAGS = -O0 -Wall -g -ggdb3 $(INCS)
CXXFLAGS =  -O0 -Wall -std=c++17 -g -ggdb3 $(INCS)
LDFLAGS =
else
CFLAGS = -O3 -Wall $(INCS)
CXXFLAGS =  -O3 -Wall -std=c++17 $(INCS)
LDFLAGS = -s
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

rebuild: clean all

clean:
	rm -rf $(TARGET) $(OBJS)

