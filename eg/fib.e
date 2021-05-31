{
  ; Return fibonacci number for x.
  val fib = fn(x) {
    if x <= 0 then return 0
    if x == 1 then return 1
    fib(x - 1) + fib(x - 2)
  }

  val n = input("Fibonacci number for:") as int

  fib(n)
}
