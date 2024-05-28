// Test the for loop.

val s = "abc"
var n = 0

// Sum the values of the bytes in s, except 'b'.
for c in s do
  if c == 'b' then continue
  n = n + c

// Result.
print("Expect asc 'a' + 'c' = 196")
n
