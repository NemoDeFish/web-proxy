EXE=htproxy
SRC= main.c sockets.c htproxy.c htproxyutils.c linkedlist.c cache.c stretch.c
HDR= headers/sockets.h headers/htproxy.h headers/htproxyutils.h headers/linkedlist.h headers/cache.h headers/stretch.h

$(EXE): $(SRC) $(HDR)
	cc -Wall -g -o $(EXE) $(SRC)

format:
	clang-format -style=file -i *.c *.h

clean: 
	rm -f $(EXE) *.o