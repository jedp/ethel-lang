# Ethel

A basic programming language.

Very much in progress and not useful yet.

## Features

### Expressions all the way down

```
> val age = 42
42
> val x = if not(age < 1) then val y = "Ethel".length()
5
> x
5
> y
5
```

### Dynamic typing

```
> val s = 101
101
> typeof(s)
Int
> s is float
false
> s as float
101.000000
> typeof(s as float)
Float
> typeof("moo".length())
Int
> "moo" + 2
Type error
```

### Data Types

Primitives:
- Boolean (`True`, `False`)
- Byte (`'c'`, `42`, `0x17`)
- Int (`123`)
- Float (`3.14`)
- Byte Array (`arr(n)`)
- String (`"hello, world"`)

Compound:
- List (`list { 1, 'x', "foo", fn(x) { x + 1 }, ... }`)
- Dictionary (`dict { 'a' => 1, 42 => True, "up" => fn(vec) { list { vec[0], vec[1] + 1 } }  ... }`)
- Function (`fn(x) { x + 1 }`)

Keys of dictionary can be any primitive type.

Values of lists and dictionaries can be any type, including other lists and dictionaries.

#### Byte Arrays and Strings

```
> val a = arr(4)
ByteArray(4)
> a.length()
4
> a[2]
0x00
> a[2] = 'c'
'c'
> a[2]
'c'
> del a
<Nil>
> val a = arr(4) { 'o', 'h', 'a', 'i' }
ByteArray(4)
> print(a)
"ohai"
> a[1] = 'm'
'm'
> print(a)
"omai"
> 'a' in a
true
```

#### Lists

```
> val l = list { 4, 5, 6, 7, 8, 9 }
{ 4, 5, 6, 7, 8, 9 }
> l.tail().slice(2, 4)
{ 7, 8 }
> l.tail().slice(2, 4).tail().head()
8
> l.removeFirst() * l.head()
20
> l
{ 5, 6, 7, 8, 9 }
> 7 in l
true
```

#### Dictionaries

```
> val d = dict { 'x' => 42, "incr" => fn(x) { x + 1 }, 3.14 => "pi, roughly" }
{ 3.140000 => "pi, roughly"
  'x' => 42
  "incr" => <Function> }
> d['x']
42
> d["incr"](9)
10
> 3.14 in d
true
> d.keys()
{ 3.140000, 'x', "incr" }
```

### Functions

Functions are first-class objects.

```
> fn(x) { x + 1 }(5)         // Anonymous (lambda)
6
> val f = fn(a, b) { a * b } // As durable values
f
> f(2, 5)
10
> val fib = fn(n) {
    if n <= 0 then return 0  // Early returns
    if n == 1 then return 1
    fib(n - 2) + fib(n - 1)  // Last expr is still normal result value
    }
Function
> for i in 1..10 { print(i, fib(i)) }
1 1
2 1
3 2
4 3
5 5
6 8
7 13
8 21
9 34
10 55
<Nil>
```

Lexical scope.

```
> val x = 42
42
> val f = fn() { x }
<Function>
> val g = fn(x) { f() }
<Function>
> g(17)
42
```

Closures.

```
> val f = fn(x) {
    var i = x + 100
    fn(y) {
      i = i + y
      i
    }
  }
<Function>
> val x = f(10)
<Function>
> x(1)
111
> x(3)
114
```

### Loops

- For
- Do-While
- While

```
> var x = 10
10
> val y = while x {
    x = x - 1
    }
0
> y // loop is still an expr, returning last value
0
> x
0
> for i in 0..3 { print(i) }
0
1
2
3
<Nil>
```

### Bitwise operators

```
> val a = arr(4)
Byte Array
> a[0] = 255
0xff
> 2345 & a[0]
41
> 1 << 4
16
> 5 & 7 == 5 and 1 << 4 + 2 == 18 // Un-astonishing order of operations.
true
> 0x7ff0 ^ 0b11001101101          // Mix and match hex and bin representations.
31133
> hex(0x7ff0 ^ 0b11001101101)
0x799d
```

### Garbage collection

```
> val x = 42
42
> { val y = 19 }
19
> gc()
GC freed 1632 bytes. Bytes avail: 15999432.
<Nil>
> x
42
```

### Dumps of internal representation

It's always instructive to see how data is actually stored as bits.

```
> dump("This is the quickening!") // Strings get a canonical hex dump.
00000000  54 68 69 73 20 69 73 20  74 68 65 20 71 75 69 63  |This.is.the.quic|
00000010  6b 65 6e 69 6e 67 21                              |kening!|
> dump(arr(40))                   // Same for bytearrays.
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000020  00 00 00 00 00 00 00 00                           |........|
> dump('x')                       // Bytes get int, hex, and bin value.
120  0x78  01111000
> dump(42)                        // Ints get hex and 32-bit bin value.
0x0000002a  00000000 00000000 00000000 00101010
> dump(-6.8)                      // 32-bit float repr, for the brave.
1  10000001  10110011001100110011010
> dump(0x799d)                    // You can specify ints by hex value.
0x0000799d  00000000 00000000 01111001 10011101
> dump(0b10101100)                // Also by binary value.
0x000000ac  00000000 00000000 00000000 10101100
```

