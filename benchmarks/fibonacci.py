import timeit
# from memoization import cached

# @cached
def fib(n):
	if (n < 2):
		return n
	return fib(n-2) + fib(n-1)

start = timeit.default_timer()
print(fib(35))
print("Took: ", timeit.default_timer() - start)
