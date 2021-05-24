# Crafting Interpreters

Implementing a Lox interpreter in Java (jlox) and C (clox) closely following the book http://craftinginterpreters.com/ and then porting to C++ (cpplox) , Rust (rox)  and Haskell (haskellox).

## Build

To build:
- __jlox__ : run `ant compile jar` inside `jlox/` folder and the executable jar file will be in `jlox/build/jar/Lox.jar`.
- __clox__ : run `make` inside `clox/` folder and the executable file will be in `clox/clox`.
- __cpplox__: run inside cpplox/ folder: `mkdir build && cmake .. && make -j`
- __rox__: run `cargo build` inside rox/ folder. (TBI)
- __haskellox__: (TBI)
