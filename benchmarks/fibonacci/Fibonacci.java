package fibonacci;

public class Fibonacci {
  public static int fib(int n) {
    if (n<2) {
      return n;
    }
    return fib(n-2) + fib(n-1);
  }

  public static void main(String args[]) {
    double start = System.currentTimeMillis() / 1000.0;
    System.out.println(fib(35));
    double end = System.currentTimeMillis() / 1000.0;
    System.out.println(end-start);
  }
}