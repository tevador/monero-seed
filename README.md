**This repository was a proof of concept and is now obsolete. The idea was implemented into a library [here](https://github.com/tevador/polyseed).**

## Build
```
git clone https://github.com/tevador/monero-seed.git
cd monero-seed
mkdir build && cd build
cmake ..
make
```

## Features

* 36% shorter than the current 25-word seed used by Monero
* embedded wallet birthday to optimize restoring from the seed (only blocks after the wallet birthday have to be scanned for transactions)
* 5 bits reserved for future updates
* advanced checksum based on Reed-Solomon linear code, which allows certain types of errors to be detected without false positives and provides limited error correction capability
* built-in way to make seeds incompatible between different coins, e.g. a seed for Aeon cannot be accidentally used to restore a Monero wallet

## Usage

### Create a new seed

```
> ./monero-seed --create [--date <yyyy-MM-dd>] [--coin <monero|aeon>]
```

Example:
```
> ./monero-seed --create --date 2100/03/14 --coin monero
Mnemonic phrase: lab park submit photo priority syrup speak couch fire crack canoe use spoil balance false solar
- coin: monero
- private key: 2bb5cfea96a0f981bf5da4fff74f43ae833682298276bce87bca455dfc14af5d
- created on or after: 02/Mar/2100
```

### Restore seed
```
./monero-seed --restore "<16-word seed>" [--coin <monero|aeon>]
```

Example:

```
> ./monero-seed --restore "lab park submit photo priority syrup speak couch fire crack canoe use spoil balance false solar" --coin monero
- coin: monero
- private key: 2bb5cfea96a0f981bf5da4fff74f43ae833682298276bce87bca455dfc14af5d
- created on or after: 02/Mar/2100
```

Attempting to restore the same seed under a different coin will fail:
```
> ./monero-seed --restore "lab park submit photo priority syrup speak couch fire crack canoe use spoil balance false solar" --coin aeon
ERROR: phrase is invalid (checksum mismatch)
```

Restore has limited error correction capability, namely it can correct a single erasure (illegible word with a known location).
This can be tested by replacing a word with `xxxx`:

```
> ./monero-seed --restore "lab park xxxx photo priority syrup speak couch fire crack canoe use spoil balance false solar" --coin monero
Warning: corrected erasure: xxxx -> submit
- coin: monero
- private key: 2bb5cfea96a0f981bf5da4fff74f43ae833682298276bce87bca455dfc14af5d
- created on or after: 02/Mar/2100
```

## Implementation details

The mnemonic phrase contains 176 bits of data, which are used as follows:

* 5 bits reserved for future use
* 10 bits for approximate wallet birthday
* 150 bits for the private key seed
* 11 bits for checksum

### Wordlist

The mnemonic phrase uses the BIP-39 wordlist, which has 2048 words, allowing 11 bits to be stored in each word. It has some additional useful properties,
for example each word can be uniquly identified by its first 4 characters. The wordlist is available for 9 languages (this repository only uses the English list).

### Reserved bits

There are 5 reserved bits for future use. Possible use cases for the reserved bits include:

* different address formats
* different KDF algorithms for generating the private key
* seed encrypted with a passphrase

Backwards compatibility is achieved under these two conditions:

1. Reserved (unused) bits are required to be 0. The software should return an error otherwise.
2. When defining a new feature bit, 0 should be the previous behavior.

### Wallet birthday

The mnemonic phrase stores the approximate date when the wallet was created. This allows the seed to be generated offline without access to the blockchain. Wallet software can easily convert a date to the corresponding block height when restoring a seed.

The wallet birthday has a resolution of 2629746 seconds (1/12 of the average Gregorian year). All dates between June 2020 and September 2105 can be represented.

### Private key derivation

The private spend key is derived from the 150-bit seed using PBKDF2-HMAC-SHA256 with 4096 iterations.The wallet birthday and the 5 reserved/feature bits are used as a salt. Future extensions may define other KDFs.

### Security

Calculating a Curve25519 private key from the public key using the Pollard's rho method takes on average <code>2<sup>125.8</sup></code> operations [[ref](https://safecurves.cr.yp.to/rho.html)]. Bruteforcing a 128-bit seed would take on average <code>2<sup>127</sup></code> iterations. Therefore it may seem that a 128-bit seed provides a similar security level as the elliptic curve itself and we could shorten the mnemonic phrase to just 14 words.

However, the math changes when we consider an attack against many keys at once. Breaking one public key out of a million using the Pollard's rho method would still take on average <code>2<sup>125.8</sup></code> operations. On the other hand, when attacking a million wallets generated in the same month (thus havng the same wallet birthday bitstring), one would expect to recover the first seed after just <code>2<sup>107</sup></code> attempts. While this is still infeasible, it's a noticeable drop in the security margin.

Using a 150-bit private key seed means that for up to ~17 million new active wallets generated each month, a brute force search of the seed space to recover at least one private key seed would be at least as hard as breaking a public key directly. The security margin is probably even higher due to a much larger constant factor for the bruteforce search.

### Checksum

The mnemonic phrase can be treated as a polynomial over GF(2048), which allows us to use an efficient Reed-Solomon error correction code with one check word. All single-word errors can be detected and all single-word erasures can be corrected without false positives.

To prevent the seed from being accidentally used with a different cryptocurrency, a coin-specific value is subtracted from the first data-word after the checksum is calculated. Checksum validation will fail unless the wallet software adds the same value back to the first data-word when restoring.
