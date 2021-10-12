CC:=g++
exe:=npshell
obj:=npshell.o
cmd:= bin
target:= bin

all:$(obj)
	$(CC) -o $(exe) $(obj)
	$(CC) $(cmd)/noop.cpp -o $(target)/noop
	$(CC) $(cmd)/number.cpp -o $(target)/number
	$(CC) $(cmd)/removetag.cpp -o $(target)/removetag
	$(CC) $(cmd)/removetag0.cpp -o $(target)/removetag0
%.o:%.c
	$(CC) -c $^ -o $@

.PHONY:clean
clean:
	rm -rf $(obj) $(exe) $(target)/noop $(target)/number $(target)/removetag $(target)/removetag0
