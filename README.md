## Ethel

A basic programming language.

Very much in progress and not useful yet.

### Example

```
make
./repl
> for i in 1..10 do if (i mod 7 == 0) then print("Hello, Ethel!")
Hello, Ethel!
(Nil)
> mut guess = input("Guess my age!") as int
Guess my age! 5
5  (Int)
> age = 42
42  (Int)
> while (guess != age) do begin
    if (guess < age) then print("Too low") else print("Too high")
    guess = input("Guess again") as int
    end
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
> x = if age == 5 then y = 4
Integer 4
Ok
> x
Integer 4
Ok
> y
Integer 4
Ok
>
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

