main:
	cc main.c -o snakie -lncurses -pthread -W -O2 

clean:
	rm -f snakie
