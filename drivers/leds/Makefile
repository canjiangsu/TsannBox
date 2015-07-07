CROSS = arm-linux-

all: myleds_test

myleds_test: myleds_test.c
	$(CROSS)gcc -o $@ myleds_test.c -static

clean:
	@rm -rf myleds_test *.o
