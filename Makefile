INCS=-I/opt/local/include
LIBS=-L/usr/local/lib -lGLEW -lglfw  -framework GLUT -framework OpenGL -framework Cocoa

all: app

app: main.o common.o
	g++ $(LIBS) main.o common.o -o app
	rm -f *.o

main.o: main.cpp
	g++ -c $(INCS) main.cpp -o main.o

common.o: common.cpp
	g++ -c $(INCS) common.cpp -o common.o

.PHONY: clean

clean:
	rm -vf app
