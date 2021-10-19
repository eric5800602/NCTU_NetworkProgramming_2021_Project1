CC:=g++
exe:=npshell
obj:=npshell.o

all:$(obj)
	$(CC) -o $(exe) $(obj)
%.o:%.c
	$(CC) -c $^ -o $@

.PHONY:clean
clean:
	rm -rf $(obj) $(exe)
