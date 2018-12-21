## Overview

This is a sample library. We did not want to put this libarry in `lib` , like any other library such as `xdr`,`libsodium` etc, rather we wanted to put in separate folder, called `utxo` which is present in the root directory. 

- Once we extract the Bitcoin code, we can put inside `utxo/lib` folder and have to make some changes in `utxo/Makefile.am` and `utxo/configure.ac`
- When ever we build our `stellar-core`, our `utxo` library should also get built just like any other libarry



## Preparing the library

- We added our library file , add.cpp and add.h in lib folder.
- We configured `utxo/configure.ac` and `utxo/Makefile.am` files.
- **Running make manually :** So now when you run commands like `./autogen.sh` followed by command `./configure`, it will generate `Makefile`
- And then if you run, `make` command, it will create executable `utxo` under the utxo directory.
- Also `libutxo.a` library will get generated inside `lib` folder. This is the library that stellar-core has to use.
- You can now run `utxo` and see if thing are working properly.

## Setting up utxo library with stellar-core

- **Running make automatically:** : So far we understood how do we run the make in utxo libary manually. But ideally it run automatically when you run make for stellar-core, just like any other library.
- To do that, we added `utxo/Makefile` in `AC_CONFIG_FILES` - line 229 in `configure.ac` file of stellar-core.
- We have to tell `stellar-core` to use this library. for that we added the line `utxo_LIBS='$(top_builddir)/utxo/lib/libutxo.a'` in `configure.ac` file of stellar-core.
- Next thing is we have to let `stellar-core` link this library. For that, we have added, `$(utxo_LIBS)` to linker,`stellar_core_LDADD` in `src/Makefile.am` file.
- That is it. 

## Execution

- Clone `utxoBuild` branch
- Now when we run `./autogen.sh` -> `./configure` -> `make -j 8` commands for stellar-core, `utxo/lib/libutxo.a` library file will get generated.

## Use

- First import the library `#include "utxo/lib/add.h"`
- ```
   AddClass utxolibobj;
   utxolibobj.addValues(15,13);
  ```
- This is implemented in `CommandHandler.cpp` file, line no 856.




- [Automake Doc](https://www.gnu.org/software/automake/manual/automake.html#A-Library)