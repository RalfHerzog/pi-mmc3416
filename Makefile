CC=gcc
CFLAGS= -O3 -Wall -g
LIBS= -lm -lzmq
AR=ar

ALLBIN=getmmc3416 zmq_publisher

all: ${ALLBIN}

clean:
	rm -f *.o ${ALLBIN}

getmmc3416: i2c_mmc3416.o getmmc3416.o
	$(CC) i2c_mmc3416.o getmmc3416.o -o getmmc3416 ${LIBS}

zmq_publisher: i2c_mmc3416.o zmq_publisher.o
	$(CC) i2c_mmc3416.o zmq_publisher.o -o zmq_publisher ${LIBS}

