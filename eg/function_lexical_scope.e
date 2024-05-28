// Test lexical scoping.

val x = 42

val f = fn() { x }

val g = fn(x) { f() }

print("Result should be 42")
g(17)

