Please READ INSTALL.md

./autogen.sh - to generate all local make.in files and configs

if you are facing issues with the build w.r.t postgres module rerun the config disabling the postgres module
./configure --disable-postgres

use make -j [N] to compile the source code. [N] is number of cores of your cpu; example make -j 4 if you have 4 core processor 


Create databases first, can use the same command to reset the databases
./src/stellar-core --conf config/node01/stellar-core.cfg --newdb
./src/stellar-core --conf config/node02/stellar-core.cfg --newdb

starting the nodes
./src/stellar-core -c config/node01/stellar-core.cfg
./src/stellar-core -c config/node02/stellar-core.cfg
