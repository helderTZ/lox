all: fibonacci/Fibonacci.jar fibonacci.out

fibonacci/Fibonacci.jar: fibonacci/Fibonacci.class
	jar -cf fibonacci/Fibonacci.jar fibonacci/Fibonacci.class

fibonacci/Fibonacci.class: fibonacci/Fibonacci.java
	javac fibonacci/Fibonacci.java

fibonacci.out: fibonacci.c
	gcc -O fibonacci.c -o fibonacci.out

.PHONY: clean

clean:
	rm fibonacci.out fibonacci/Fibonacci.class fibonacci/Fibonacci.jar