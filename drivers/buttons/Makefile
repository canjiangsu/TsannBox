CROSS = arm-linux-
obj = button_test
SRCS = $(obj).c

all: $(obj)

$(obj): $(SRCS)
	$(CROSS)gcc -o $@ $(SRCS)  -static

clean:
	@rm -rf $(obj) *.o
