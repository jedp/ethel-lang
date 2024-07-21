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

## CONST_INT

Pack LSB first. Use only as many bytes as necessary.

Example:

Const int 42:
```

```