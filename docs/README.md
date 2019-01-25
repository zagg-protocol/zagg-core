Zagg Protocol is a privacy enabled accounts based public blockchain network which uses Stellar Consensus Protocol. 

`zagg-core` is the software used to run a node in Zagg Protocol . It is a C++ implementation of an accounts based privacy enabled blockchain. Zagg protocol has a hybrid model which comprises of both accounts and UTXOs. Both accounts and UTXOs are fundamental primitives in Zagg Protocol. 

For a brief snapshot of the roadmap you can take a [look at this](https://github.com/zagg-protocol/docs/blob/master/Roadmap.md). (more details coming soon!)

The following is a description of the first step in the development of `zagg-core`.

`zagg-core` enables a utxo based transaction and a account based transaction in a single node. There is no consensus involved at this stage because there is no network. The transactions are transmitted to the node using a Command Line Interface (CLI). 

The account based transaction are enabled using `stellar-core`. UTXO based transactions are enabled by compiling bitcoin software into a static library. The databases remain separate. Accounts are maintained in ledgers of stellar-core. UTXOs are stored in levelDB.

