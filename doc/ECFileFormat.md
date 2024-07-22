# Ethel Compiled File Format

```
4               Magic (EJED)
1               Major rev
1               Minor rev
1               Const pool count (CPC)
consts_info     Const pool
1               Locals count
locals_info     Locals
code            Bytecode
```

## CONST_BOOLEAN

No size, just a value.

```
const bool true:

CONST_BOOLEAN, 1
```

## CONST_INT

Pack LSB first. Use only as many bytes as necessary.

Twos-complement values.

Examples:

```
const int 42:

CONST_INT, 1, 42
```

Const int 456:
```
const int 456:

CONST_INT, 2, 200, 1
```

456 = 200 + 256