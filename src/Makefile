# app

CC = gcc
CFLAGS = -Wall -g -lpthread
TARGET = tiny_web
OBJS = utils.o rio.o net.o threadpool.o $(TARGET).o

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(CFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@

%.o : %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -rf *.o $(TARGET)
