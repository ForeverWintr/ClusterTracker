CC=g++
CFLAGS= -L../../lib/ -lhamedjpeg -I../../include
TARGET=intensity

$(TARGET): $(TARGET).cc
	$(CC) -o $(TARGET) $(TARGET).cc $(CFLAGS)


clean:
	rm -f *.o *~
