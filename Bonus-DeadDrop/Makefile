include ../cpu.mk

TARGETS=receiver sender 
UTILS=util.o 

all: $(TARGETS)

$(UTILS): %.o: %.c %.h
	$(CC) $(CFLAGS) -c -g $< 

%.o: %.c util.h
	$(CC) $(CFLAGS)  -c -g $< 

$(TARGETS): %:%.o util.o
	$(CC) $(CFLAGS) $^ -o $@

run_sender: sender 
	taskset -c $(SENDER_CPU) ./sender

run_receiver: receiver
	taskset -c $(RECEIVER_CPU) ./receiver

.PHONY:	clean

clean:
	$(RM) *.o $(HELPERS) $(TARGETS) 
