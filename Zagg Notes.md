Please READ INSTALL.md

./autogen.sh - to generate all local make.in files and configs

if you are facing issues with the build w.r.t postgres module rerun the config disabling the postgres module
./configure --disable-postgres

use make -j [N] to compile the source code. [N] is number of cores of your cpu; example make -j 4 if you have 4 core processor 


Create databases first, can use the same command to reset the databases
./src/stellar-core --conf config/node01/stellar-core.cfg --newdb  
./src/stellar-core --conf config/node02/stellar-core.cfg --newdb 



./src/stellar-core --conf config/node01/stellar-core.cfg --newhist vs 
./src/stellar-core --conf config/node02/stellar-core.cfg --newhist vs 



./src/stellar-core --conf config/node01/stellar-core.cfg --forcescp 
./src/stellar-core --conf config/node02/stellar-core.cfg --forcescp 


./src/stellar-core --conf config/node01/stellar-core.cfg 
./src/stellar-core --conf config/node02/stellar-core.cfg 


starting the nodes
./src/stellar-core -c config/node01/stellar-core.cfg 
./src/stellar-core -c config/node02/stellar-core.cfg 

Create an account: 
generate a seed pair first
1. ./src/stellar-core --genseed
Example output: Secret seed: SBKYQFOSM2NHEWAUDKNF65P73OS3RUOSS6YNXCVL6PWZUASGINSB2KGM
				Public: GBVYMYUH4BA3ADUU5M6OVANT2VVYSJPDNVMFNIPTSGLECNZAD2QR3Q3C

