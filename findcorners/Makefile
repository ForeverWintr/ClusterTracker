CC=g++
CFLAGS= -L../../lib/ -lhamedjpeg -I../../include
TARGET=findcorners

$(TARGET): $(TARGET).cc
	$(CC) -o $(TARGET) $(TARGET).cc $(CFLAGS)


clean:
	rm -f *.o *~
