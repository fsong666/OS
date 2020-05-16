CL=clang
FLAG=-std=c11 -pedantic -Wall -Wextra
all: helloworld shellutilstest
helloworld: hello.o
	$(CL) $(FLAG) -o $@ $<

shellutilstest: test.o shellutils.o
	$(CL) $(FLAG)  -o $@ $^ 
%.o:%.c 
	$(CL) $(FLAG) -c -g -O0 $<
.PHONY: clean all
clean: 
	rm -f *.o helloworld shellutilstest
