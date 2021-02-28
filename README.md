## Ethel

A basic programming language.

Very much in progress and not useful yet.

### Example

```
make
./repl
> for i in 1 to 10 do if (i mod 7 == 0) then print("Hello, Ethel!")
Hello, Ethel!
Nil
Ok
> age = input("How old are you?") as int
How old are you? 5
Integer 5
Ok
> if age > 0 and age < 10 then print("You are small")
You are small
Ok
>
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
- Immutable variables, except for loop indices
- Type casting within reason
- Input and output
 
Yet to come:
- Memory management and GC
- Functions and stack frames
- Graphics (stm32f4 LCD driver)
- Compiler
