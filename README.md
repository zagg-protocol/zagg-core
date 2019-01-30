# What is ZAGG Protocol?
> WIP. This is being updated as you read

ZAGG Protocol’s implementation of enterprise-grade privacy on public blockchain is designed to mainstream enterprise-oriented payments and settlement use cases in blockchain.

ZAGG Protocol is building a novel privacy-preserving Blockchain. Innovations in ZAGG Protocol bring privacy to public blockchain through a hybrid model of tracking assets and values on the blockchain with both Accounts and UTXOs.This solution adapts the privacy and robustness of the UTXO model’s construct to build a flexible and programmable account-based blockchain.

Implementing privacy on account-based blockchains without leaking data and securing against double spend is a challenge with existing technology. Innovations in ZAGG Protocol bring privacy to public blockchain through a hybrid model of tracking assets and values on the blockchain with both Accounts and UTXOs. This solution adapts the privacy and robustness of the UTXO model’s construct to build a flexible and programmable account-based blockchain. The protocol also introduces the concept of Integrated subchains that will allow for nodes to privately transact one-to-one or in a coalition group within public chain without side chains/off-chains while allowing for mathematically provable assertions about effects of the private transactions without leaking information about the private transactions or coalitions.

ZAGG Protocol is based on [stellar](https://github.com/stellar) and [Bitcoin](https://github.com/bitcoin/bitcoin) code, it intends to add enterprise-grade privacy through Zero Knowledge Proofs (ZKPs).

Stellar-core is a replicated state machine that maintains a local copy of a cryptographic ledger and processes transactions against it, in consensus with a set of peers.
It implements the [Stellar Consensus Protocol](https://github.com/stellar/stellar-core/blob/master/src/scp/readme.md), a _federated_ consensus protocol.
It is written in C++14 and runs on Linux, OSX and Windows.
Learn more by reading the [overview document](https://github.com/stellar/stellar-core/blob/master/docs/readme.md).

Technical details of ZAGG Protocol are available in the [protocol document](https://www.zaggprotocol.com).


### Quick References (WIP)

[Introduction](https://github.com/zagg-protocol/docs/blob/master/Concepts/Introduction.md)

 [concepts](https://github.com/zagg-protocol/docs/tree/master/Concepts) 
 
[Protocol Overview](https://github.com/zagg-protocol/docs/blob/master/Concepts/protocol-overview.md)

[Business Case & Adoption](https://github.com/zagg-protocol/docs/blob/master/Concepts/business-case.md)
Learn more by reading the [docs](https://github.com/zagg-protocol/docs).



# Development Approach

ZAGG Protocol is starting the development from the stable build of Stellar. In the first phase, ZAGG intends to add UTXOs model to existing accounts based stellar. In the next phase, ZAGG Protocol team to build Zero-Knowledge Proofs along with other primitives to achieve transaction privacy.

A more detailed development plan is as follows:

### Protocol V0.1
> Integration of bitcoin code base into the stellar fork to achieve dual accounts balance and UTXO balances.

**Assumptions:**
* Dual Ledgers with Dual accounts 
* Dual consensus 

**Description:**
After this step, User will be able to create a transaction via CLI. Use the data as input to the UTXO route. The communication is not in XDR and it's in either plain text or hex format. Stellar code invokes a call to Bitcoin code base and registers the transaction.  This step is the first step to create one merged running instance of both the protocols as foundation for next steps.


### Protocol V0.2
> Both Account balance transactions and UTXO based transaction will go through Stellar consensus protocol (SCP) 

In this step the transaction set is applied on the ledger after the consensus. This will add a new transaction to Stellar and the system will support both accounts state changes and UTXOs on the global ledger(s). There will be only one (Stellar) Consensus mechanism on the network.
 
**Assumptions:**
* Dual Ledgers with Dual accounts
* Single consensus
* Multiple Nodes

### Protocol V0.3
> Add Account To UTXO and UTXO to Account conversions

This step will allow the user to move assets (multiple types) from her account into UTXOs (multiple types) and back and execute transfer of those assets/UTXOs to other users on the network over SCP. This step will also ensure no double spending and assets/resource consistency across both the types for any given user and on the network.

### Protocol V0.4
> Replace Bitcoin with ZCash

The above three steps of changes will then be applied to ZCash Sapling code base and it will be merged with Stellar code base. Subsequent code changes will be on the Zero Knowledge Proofs, Commitments and Nullifiers functionality so that the system will be able to support shielded ZKP transactions on Stellar consensus.

### Protocol V0.5
> Integrating the ledgers

The accounts and shielded UTXOs transactions will be merged so that all the transactions will be recorded and retrieved from one global ledger.

### Protocol V0.6
> Adding Private networks

Zagg supports creation of private subnetworks for forming coalitions where the transactions are visible to all the parties but not to the public. All the transactions are verified by the public nodes but cannot be seen by the public nodes.

### Protocol V0.7
> Adding Smart contracts

Zagg will increase the capability of smart contracts to enable different business models to operate on the network.


# Current Status V 0.0.1


## Simple Summary
 `CommandHandler::utxoHandler`  route in stellar which calls `sendrawtransactionzagg` function in bitcoin static library to enable UTXO transactions in zagg-core on a single node. 

## Abstract
A UTXO transaction from sender UTXOs to receiver UTXOs has to be transmitted in hex format to the `CommandHandler::utxoHandler` route in Stellar. The UTXOs that are being spent have to be chosen and the transaction has to be signed before it is sent to the route in stellar

## Motivation
Zagg protocol enables storage of value and assets in both account and UTXO primitives to benefit from the advantages of both the primitives. Having UTXOs as primitives allows us to establish privacy on top of Stellar Consensus Protocol.


## Specification
A signed UTXO transaction in the hex format sent to a stellar route should be commited to the UTXO part of the blockchain.

The data structures needed to enable UTXO transaction are the following: 
* CTransaction: Data sturcture of transaction
* CTxIn: Input UTXO data structure
* CTxOut: Output UTXO data structure
```
class CTransaction
{
public:
    // Default transaction version.
    static const int32_t CURRENT_VERSION=2;

    // Changing the default transaction version requires a two step process: first
    // adapting relay policy by bumping MAX_STANDARD_VERSION, and then later date
    // bumping the default CURRENT_VERSION at which point both CURRENT_VERSION and
    // MAX_STANDARD_VERSION will be equal.
    static const int32_t MAX_STANDARD_VERSION=2;

    // The local variables are made const to prevent unintended modification
    // without updating the cached hash value. However, CTransaction is not
    // actually immutable; deserialization and assignment are implemented,
    // and bypass the constness. This is safe, as they update the entire
    // structure, including the hash.
    const std::vector<CTxIn> vin;
    const std::vector<CTxOut> vout;
    const int32_t nVersion;
    const uint32_t nLockTime;

private:
    /** Memory only. */
    const uint256 hash;
    const uint256 m_witness_hash;

    uint256 ComputeHash() const;
    uint256 ComputeWitnessHash() const;

public:
    /** Construct a CTransaction that qualifies as IsNull() */
    CTransaction();

    /** Convert a CMutableTransaction into a CTransaction. */
    CTransaction(const CMutableTransaction &tx);
    CTransaction(CMutableTransaction &&tx);

    template <typename Stream>
    inline void Serialize(Stream& s) const {
        SerializeTransaction(*this, s);
    }

    /** This deserializing constructor is provided instead of an Unserialize method.
     *  Unserialize is not possible, since it would require overwriting const fields. */
    template <typename Stream>
    CTransaction(deserialize_type, Stream& s) : CTransaction(CMutableTransaction(deserialize, s)) {}

    bool IsNull() const {
        return vin.empty() && vout.empty();
    }

    const uint256& GetHash() const { return hash; }
    const uint256& GetWitnessHash() const { return m_witness_hash; };

    // Return sum of txouts.
    CAmount GetValueOut() const;
    // GetValueIn() is a method on CCoinsViewCache, because
    // inputs must be known to compute value in.

    /**
     * Get the total transaction size in bytes, including witness data.
     * "Total Size" defined in BIP141 and BIP144.
     * @return Total transaction size in bytes
     */
    unsigned int GetTotalSize() const;

    bool IsCoinBase() const
    {
        return (vin.size() == 1 && vin[0].prevout.IsNull());
    }

    friend bool operator==(const CTransaction& a, const CTransaction& b)
    {
        return a.hash == b.hash;
    }

    friend bool operator!=(const CTransaction& a, const CTransaction& b)
    {
        return a.hash != b.hash;
    }

    std::string ToString() const;

    bool HasWitness() const
    {
        for (size_t i = 0; i < vin.size(); i++) {
            if (!vin[i].scriptWitness.IsNull()) {
                return true;
            }
        }
        return false;
    }
};

```
```
/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
class CTxIn
{
public:
    COutPoint prevout;
    CScript scriptSig;
    uint32_t nSequence;
    CScriptWitness scriptWitness; //!< Only serialized through CTransaction

    /* Setting nSequence to this value for every input in a transaction
     * disables nLockTime. */
    static const uint32_t SEQUENCE_FINAL = 0xffffffff;

    /* Below flags apply in the context of BIP 68*/
    /* If this flag set, CTxIn::nSequence is NOT interpreted as a
     * relative lock-time. */
    static const uint32_t SEQUENCE_LOCKTIME_DISABLE_FLAG = (1U << 31);

    /* If CTxIn::nSequence encodes a relative lock-time and this flag
     * is set, the relative lock-time has units of 512 seconds,
     * otherwise it specifies blocks with a granularity of 1. */
    static const uint32_t SEQUENCE_LOCKTIME_TYPE_FLAG = (1 << 22);

    /* If CTxIn::nSequence encodes a relative lock-time, this mask is
     * applied to extract that lock-time from the sequence field. */
    static const uint32_t SEQUENCE_LOCKTIME_MASK = 0x0000ffff;

    /* In order to use the same number of bits to encode roughly the
     * same wall-clock duration, and because blocks are naturally
     * limited to occur every 600s on average, the minimum granularity
     * for time-based relative lock-time is fixed at 512 seconds.
     * Converting from CTxIn::nSequence to seconds is performed by
     * multiplying by 512 = 2^9, or equivalently shifting up by
     * 9 bits. */
    static const int SEQUENCE_LOCKTIME_GRANULARITY = 9;

    CTxIn()
    {
        nSequence = SEQUENCE_FINAL;
    }

    explicit CTxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);
    CTxIn(uint256 hashPrevTx, uint32_t nOut, CScript scriptSigIn=CScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(prevout);
        READWRITE(scriptSig);
        READWRITE(nSequence);
    }

    friend bool operator==(const CTxIn& a, const CTxIn& b)
    {
        return (a.prevout   == b.prevout &&
                a.scriptSig == b.scriptSig &&
                a.nSequence == b.nSequence);
    }

    friend bool operator!=(const CTxIn& a, const CTxIn& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

```

```
/** An output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
class CTxOut
{
public:
    CAmount nValue;
    CScript scriptPubKey;

    CTxOut()
    {
        SetNull();
    }

    CTxOut(const CAmount& nValueIn, CScript scriptPubKeyIn);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(nValue);
        READWRITE(scriptPubKey);
    }

    void SetNull()
    {
        nValue = -1;
        scriptPubKey.clear();
    }

    bool IsNull() const
    {
        return (nValue == -1);
    }

    friend bool operator==(const CTxOut& a, const CTxOut& b)
    {
        return (a.nValue       == b.nValue &&
                a.scriptPubKey == b.scriptPubKey);
    }

    friend bool operator!=(const CTxOut& a, const CTxOut& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};
```


## Rationale
Creating  accounts and UTXOs primitives in zagg for value transfer is essential to build privacy, private network and smart contract features in the future. This is the basis for the upcoming features.



## Backwards Compatibility
This is fully backwards compatible

## Test Cases
Some test cases that must be considered include:

Input UTXO should be equal to Output UTXOs. The existing test cases in the UTXO code already handle this.

## Implementation
The implementation needed the following changes:
##### Build Changes
#### Route addition
#### Addition of new transaction function
#### Exposing bitcoin functionality to stellar
#### Integrating all these changes in stellar code



# To be updated

The installations instructions and running instructions are not yet updated. The following are the instructions to run a standalone stellar node.
# Installation

See [Installation](./INSTALL.md)

# Stellar Running tests

run tests with:
  `src/stellar-core --test`

run one test with:
  `src/stellar-core --test  testName`

run one test category with:
  `src/stellar-core --test '[categoryName]'`

Categories (or tags) can be combined: AND-ed (by juxtaposition) or OR-ed (by comma-listing).

Tests tagged as [.] or [hide] are not part of the default test test.

supported test options can be seen with
  `src/stellar-core --test --help`

display tests timing information:
  `src/stellar-core --test -d yes '[categoryName]'`

xml test output (includes nested section information):
  `src/stellar-core --test -r xml '[categoryName]'`

# Running tests against postgreSQL

There are two options.  The easiest is to have the test suite just
create a temporary postgreSQL database cluster in /tmp and delete it
after the test.  That will happen by default if you run `make check`.

You can also use an existing database cluster so long as it has
databases named `test0`, `test1`, ..., `test9`, and `test`.  To set
this up, make sure your `PGHOST` and `PGUSER` environment variables
are appropriately set, then run the following from bash:

    for i in $(seq 0 9) ''; do
        psql -c "create database test$i;"
    done

You will need to set the `TEMP_POSTGRES` environment variable to 0
in order to use an existing database cluster.

# Running tests in parallel

The `make check` command also supports parallelization. This functionality is
enabled with the following environment variables:
* `TEST_SPEC`: Used to run just a subset of the tests (default: "~[.]")
* `NUM_PARTITIONS`: Partitions the test suite (after applying `TEST_SPEC`) into
`$NUM_PARTITIONS` disjoint sets (default: 1)
* `RUN_PARTITIONS`: Run only a subset of the partitions, indexed from 0
(default: "$(seq 0 $((NUM_PARTITIONS-1)))")
* `TEMP_POSTGRES`: Automatically generates temporary database clusters instead
of using an existing cluster (default: 1)

For example,
`env TEST_SPEC="[history]" NUM_PARTITIONS=4 RUN_PARTITIONS="0 1 3" make check`
will partition the history tests into 4 parts then run parts 0, 1, and 3.

# Running stress tests
We adopt the convention of tagging a stress-test for subsystem foo as [foo-stress][stress][hide].

Then, running
* `stellar-core --test [stress]` will run all the stress tests,
* `stellar-core --test [foo-stress]` will run the stress tests for subsystem foo alone, and
* neither `stellar-core --test` nor `stellar-core --test [foo]` will run stress tests.
