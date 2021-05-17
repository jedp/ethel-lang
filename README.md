## Ethel

A basic programming language.

Very much in progress and not useful yet.

### Features

#### Expressions all the way down

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

#### Dynamic typing

```
> val s = 101
101
> type(s)
Int
> s is float
false
> s as float
101.000000
> type(s as float)
Float
> type("moo".length())
Int
> "moo" + 2
Type error
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

#### Dictionaries

```
> val d = dict { 'x': 42, "incr": fn(x) { x + 1 }, 3.14: "pi, roughly" }
{ 3.140000: "pi, roughly"
  "foo": 19
  'x': 42 }
> d['x']
42
> d["incr"](9)
10
> 3.14 in d
true
> d.keys()
{ 3.140000, 'x', "incr" }
```

#### Functions

Functions have lexical scope.

```
> val f = fn(a, b) { a * b }
f
> f(2, 5)
10
> val fib = fn(n) {
    if n <= 0 then return 0  ;; Early returns
    if n == 1 then return 1
    fib(n - 2) + fib(n - 1)  ;; Last expr is still normal result value
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

Quicksort, with a little scoping nit.

```
> val quicksort = fn(a, lo, hi) {
    if lo < hi then {
      var p = partition(a, lo, hi)
      quicksort(a, lo, p-1)
      quicksort(a, p+1, hi)
      }
    }
Function
> val partition = fn(a, lo, hi) {
    val pivot = a[hi]
    var i = lo
    for j in lo..hi {
      if a[j] < pivot then {
        swap(a, i, j)
        i = i + 1
        }
      }
    swap(a, i, hi)
    i
    }
Function
> val swap = fn(a, i, j) {
    val t = a[i]
    a[i] = a[j]
    a[j] = t
    }
Function
> val s = "I like potatoes!"
I like potatoes!
> quicksort(s, 0, s.length() - 1)
<Nil>
> s
  !Iaeeikloopstt
```

#### Loops

```
> var x = 10
10
> val y = while x {
    x = x - 1
    }
0
> y ;; loop is still an expr, returning last value
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

#### Bitwise operators

```
> val a = arr(4)
Byte Array
> a[0] = 255
0xff
> 2345 & a[0]
41
> 1 << 4
16
> 5 & 7 == 5 and 1 << 4 + 2 == 18 ;; Un-astonishing order of operations.
true
> 0x7ff0 ^ 0b11001101101          ;; Mix and match hex and bin representations.
31133
> hex(0x7ff0 ^ 0b11001101101)
0x799d
```

#### Dumps of internal representation

```
> dump("This is the quickening!") ;; Strings get a canonical hex dump.
00000000  54 68 69 73 20 69 73 20  74 68 65 20 71 75 69 63  |This.is.the.quic|
00000010  6b 65 6e 69 6e 67 21                              |kening!|
> dump(arr(40))                   ;; Same for bytearrays.
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000020  00 00 00 00 00 00 00 00                           |........|
> dump('x')                       ;; Bytes get int, hex, and bin value.
120  0x78  01111000
> dump(42)                        ;; Ints get hex and 32-bit bin value.
0x0000002a  00000000 00000000 00000000 00101010
> dump(-6.8)                      ;; 32-bit float repr, for the brave.
1  10000001  10110011001100110011010
> dump(0x799d)                    ;; You can specify ints by hex value.
0x0000799d  00000000 00000000 01111001 10011101
> dump(0b10101100)                ;; Also by binary value.
0x000000ac  00000000 00000000 00000000 10101100
```

