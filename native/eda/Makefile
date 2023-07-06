CFILES += main.c
CFILES += scf_eda.pb-c.c
CFILES += scf_eda_pb.c

CFLAGS += -g
CFLAGS += -I./
CFLAGS += -I../../util

LDFLAGS += -lprotobuf-c

all:
	gcc $(CFLAGS) $(CFILES) $(LDFLAGS)

clean:
	rm *.o
