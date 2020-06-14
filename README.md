## Build
```
git clone https://github.com/tevador/monero-seed.git
cd monero-seed
mkdir build && cd build
cmake ..
make
```

## Usage

### Create a new seed

The sole argument is the wallet creation date in `yyyy/MM/dd` format.
```
> ./monero-seed --create 2100/03/14
Mnemonic phrase: pumpkin alter spice lend position sentence surface snow atom lobster exotic robot profit chase
- version: 1
- private key: 9a1a9fe303f84d39277c0e87ccf42aa78f19c28127b2187d574266f29992971f
- created on or after: 02/Mar/2100
```

### Restore seed

```
> ./monero-seed --restore "pumpkin alter spice lend position sentence surface snow atom lobster exotic robot profit chase"
- version: 1
- private key: 9a1a9fe303f84d39277c0e87ccf42aa78f19c28127b2187d574266f29992971f
- created on or after: 02/Mar/2100
```

Restore has limited error correction capability, namely it can correct a single erasure (illegible symbol with a known location).
This can be tested by replacing a word with `xxxx`:

```
> ./monero-seed --restore "pumpkin alter xxxx lend position sentence surface snow atom lobster exotic robot profit chase"
Warning: corrected erasure: xxxx -> spice
- version: 1
- private key: 9a1a9fe303f84d39277c0e87ccf42aa78f19c28127b2187d574266f29992971f
- created on or after: 02/Mar/2100
```

## Implementation details

The mnemonic phrase contains 154 bits of data, which are used as follows:

* 3 bits for version (this allows the format to be updated up to 7 times)
* 2 bits reserved for future use
* 10 bits for approximate wallet creation date
* 128 bits for the private key seed
* 11 bits for error detection/correction

### Wordlist

Uses the wordlist from BIP-39. It has 2048 words, allowing 11 bits to be stored in each word. It has some additional useful properties,
for example each word can be uniquly identified by its first 4 characters.

### Wallet creation date

The mnemonic phrase doesn't store block height, but the time when the wallet was created. This allows the seed to be generated
offline without access to the blockchain. Wallet software can easily convert a date to the corresponding block height when restoring a seed.
The wallet creation date has a resolution of 2629746 seconds (1/12 of the average Gregorian year). All dates between June 2020
and September 2105 can be represented.

### Private key seed

PBKDF2 with 4096 iterations is used to generate the private key from the 128-bit seed included in the mnemonic phrase. The wallet creation date is used as a salt. 128-bit seed provides the same level of security as the elliptic curve used by Monero.

### Reserved bits

There are 2 reserved bits for future use. Possible use cases:

* a flag to differentiate between normal and "short" address format (with view key equal to the spend key)
* different KDF algorithms for generating the private key

### Error detection/correction

The mnemonic phrase can be treated as a polynomial over GF(2048), which allows us to use an efficient Reed-Solomon ECC with one check word. All single-word errors can be detected and all single-word erasures can be corrected.
