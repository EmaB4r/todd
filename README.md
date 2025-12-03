# todd  
A Todo utility tool

At the moment is only able to recursively scan all directories, starting from the calling point

## Compilation
*Linux Only*
the makefile will compile todd.c and move it into the  
`/usr/local/bin` folder using sudo

## Usage:
In your code write a comment like
```c
// TODO: this thing is non behaving as it should
//
// an idea could be this or that
```

and then in your shell simply call `todd`