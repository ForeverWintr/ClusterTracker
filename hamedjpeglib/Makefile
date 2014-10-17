
all: frame.o huffman.o jpeg.o myio.o quant.o scan.o yuv.o jpeg.h
#	g++ -o register register.o frame.o huffman.o jpeg.o myio.o quant.o scan.o yuv.o

frame.o: frame.cc frame.h myio.h
	g++ -c frame.cc

huffman.o: huffman.cc huffman.h
	g++ -c huffman.cc

jpeg.o: jpeg.cc jpeg.h
	g++ -c jpeg.cc

myio.o: myio.cc myio.h
	g++ -c myio.cc

quant.o: quant.cc quant.h
	g++ -c quant.cc

scan.o: scan.cc scan.h
	g++ -c scan.cc

yuv.o: yuv.cc yuv.h
	g++ -c yuv.cc

clean:
	rm -f *.o *~
