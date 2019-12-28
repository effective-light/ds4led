FLAGS = -Wall
INCLUDES = -I/usr/include/hidapi

ds4led : ds4.o
	gcc ${FLAGS} -o $@ $^ -lhidapi-hidraw

%.o: %.c
	gcc ${FLAGS} ${INCLUDES} -c $<

clean:
	rm -f *.o ds4led
