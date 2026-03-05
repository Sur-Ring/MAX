CC := /opt/rocm/llvm/bin/clang++
CFLAGS := -g -Wall -DHAVE_UTIMENSAT -I. -Iinclude -I../common -I/opt/rocm/include $(shell pkg-config --cflags libnl-3.0)
LIBS=$(shell pkg-config --libs libnl-3.0)  -L/opt/rocm/lib -lhsa-runtime64

OBJS := $(patsubst %.cc, ./obj/%.o, $(wildcard *.cc))

TARGET := max_deamon

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o build/$@ $^ $(LIBS)

./obj/%.o: %.cc
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/* build/*

load: $(TARGET)
	./build/$(TARGET) | tee log.log
