# Please READ INSTALL.md  
  
`./autogen.sh` - to generate all local make.in files and configs  
  
if you are facing issues with the build w.r.t postgres module rerun the config disabling the postgres module  
`./configure --disable-postgres`   
  
use `make -j [N]` to compile the source code. `[N]` is number of cores of your cpu; example `make -j 4` if you have 4 core processor  

  
### Create databases first, can use the same command to reset the databases  

#### Commands for Node1
`./src/stellar-core --conf config/node01/stellar-core.cfg --newdb`  
`./src/stellar-core --conf config/node01/stellar-core.cfg --newhist local`    
`./src/stellar-core --conf config/node01/stellar-core.cfg --forcescp`   
`./src/stellar-core --conf config/node01/stellar-core.cfg`  
  
#### Commands for Node2
`./src/stellar-core --conf config/node02/stellar-core.cfg --newdb`  
`./src/stellar-core --conf config/node02/stellar-core.cfg --newhist local`     
`./src/stellar-core --conf config/node02/stellar-core.cfg --forcescp`   
`./src/stellar-core --conf config/node02/stellar-core.cfg`   
  
### Create an account:  
generate a seed pair first  
1. `./src/stellar-core --genseed`
Example output: Secret seed: SBKYQFOSM2NHEWAUDKNF65P73OS3RUOSS6YNXCVL6PWZUASGINSB2KGM
				Public: GBVYMYUH4BA3ADUU5M6OVANT2VVYSJPDNVMFNIPTSGLECNZAD2QR3Q3C

### Running bitcoin deamon with stellar

We have added an option (without any parameter) called `--bitcoind`. So now stellar will run with bitcoin deamon only if we specify `--bitcoind` option with stellar command.

```
./src/stellar-core --conf config/node02/stellar-core.cfg --bitcoind
```

We can also configure data directory in stellar's configuration file using `BITCOIN_DATADIR` macro. In case this macro is not present in the configuration file and `--bitcoind` option is provided then the deamon will run with bitcoin's default data directory. Please do the following configuration in `stellar-config.cfg` file to configure bitcoin's data directory.

```
## Bitcoin realted configs
# To run bitcoin in regtest mode. default is true
BITCOIN_REGTEST=true
# Specify the datadirectory for bitcoin. default is /home/$user/.bitcoin
BITCOIN_DATADIR="path_of_bitcoin_data_directory"
```


