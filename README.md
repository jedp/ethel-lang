## Ethel

A basic programming language.

Very much in progress and not useful yet.

### Example

```
make
./repl
> for i in 1..10 if (i mod 7 == 0) then print("Hello, Ethel!")
Hello, Ethel!
(Nil)
> var guess = input("Guess my age!") as int
Guess my age! 5
5  (Int)
> val age = 42
42  (Int)
> while guess != age {
    if guess < age then print("Too low") else print("Too high")
    guess = input("Guess again") as int
    }
Too low
Guess again 66
Too high
Guess again 15
Too low
Guess again 42
Ok
```

Expressions all the way down.

```
> val age = 42
42
> val x = if age > 1 then val y = "Ethel".length()
5
> x
5
> y
5
```

Lists

```
> val l = list of Int { 4, 5, 6, 7, 8, 9 }
{ 4, 5, 6, 7, 8, 9 }
> l.tail().slice(2, 4)
{ 7, 8 }
> l.tail().slice(2, 4).tail().head()
8
> l.removeFirst() * l.head()
20
> l
{ 5, 6, 7, 8, 9 }
```

Byte Arrays and Strings

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
```

### Details

- Lexer
- Recursive-descent parser with precedence climbing
- AST
- REPL

- Everything is an expression
- Environment scopes for variables
- Mutable or immutable variables
- Type casting within reason
- Input and output
 
Yet to come:
- So many things

