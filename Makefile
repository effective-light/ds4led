FLAGS = -Wextra

ds4led : ds4.o
	gcc ${FLAGS} -o $@ $^ -ludev

%.o: %.c
	gcc ${FLAGS} ${INCLUDES} -c $<

clean:
	rm -f *.o ds4led
