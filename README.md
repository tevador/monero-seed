## Build
```
git clone https://github.com/tevador/monero-seed.git
cd monero-seed
mkdir build && cd build
cmake ..
make
```

## Features

* embedded wallet birthday to optimize restoring from the seed (only blocks after the wallet birthday have to be scanned for transactions)
* embedded network type (mainnet/stagenet/testnet) to prevent accidental misuse of the seed on a different network
* advanced checksum based on Reed-Solomon linear code, which allows certain types of errors to be detected without false positives and provides limited error correction capability
* built-in way to make seeds incompatible between different coins, e.g. a seed for Aeon cannot be accidentally used to restore a Monero wallet

## Usage

### Create a new seed

```
> ./monero-seed --create [--coin <monero|aeon>] [--net <MAIN|STAGE|TEST>] [--date <yyyy-MM-dd>]
```

Example:
```
> ./monero-seed --create --coin monero --net MAIN --date 2100/03/14
Mnemonic phrase: test park taste security oxygen decorate essence ridge ship fish vehicle dream fluid pattern
- coin: monero
- network: MAIN
- private key: 7b816d8134e29393b0333eed4b6ed6edf97c156ad139055a706a6fb9599dcf8c
- created on or after: 02/Mar/2100
```

### Restore seed
```
./monero-seed --restore "<14-word seed>" [--coin <monero|aeon>]
```

Example:

```
> ./monero-seed --restore "test park taste security oxygen decorate essence ridge ship fish vehicle dream fluid pattern" --coin monero
- coin: monero
- network: MAIN
- private key: 7b816d8134e29393b0333eed4b6ed6edf97c156ad139055a706a6fb9599dcf8c
- created on or after: 02/Mar/2100
```

Attempting to restore the same seed under a different coin will fail:
```
> ./monero-seed --restore "test park taste security oxygen decorate essence ridge ship fish vehicle dream fluid pattern" --coin aeon
ERROR: phrase is invalid (checksum mismatch)
```

Restore has limited error correction capability, namely it can correct a single erasure (illegible word with a known location).
This can be tested by replacing a word with `xxxx`:

```
> ./monero-seed --restore "test park xxxx security oxygen decorate essence ridge ship fish vehicle dream fluid pattern" --coin monero
Warning: corrected erasure: xxxx -> taste
- coin: monero
- network: MAIN
- private key: 7b816d8134e29393b0333eed4b6ed6edf97c156ad139055a706a6fb9599dcf8c
- created on or after: 02/Mar/2100
```

## Implementation details

The mnemonic phrase contains 154 bits of data, which are used as follows:

* 2 bits for the network type
* 3 bits reserved for future use
* 10 bits for approximate wallet birthday
* 128 bits for the private key seed
* 11 bits for checksum

### Wordlist

The mnemonic phrase uses the BIP-39 wordlist, which has 2048 words, allowing 11 bits to be stored in each word. It has some additional useful properties,
for example each word can be uniquly identified by its first 4 characters. The wordlist is available for 9 languages (this repository only uses the English list).

### Network type

The network type is stored in 2 bits as follows:

* `00` = mainnet
* `01` = stagenet
* `10` = testnet
* `11` = invalid

The "Iinvalid" value can be used to support future extensions of the mnemonic seed to more than 14 words. Setting these two bits to `11` will prevent the first 14-words of a longer seed from being a valid 14-word seed (the checksum alone cannot prevent this).

### Reserved bits

There are 3 reserved bits for future use. Since there is no dedicated "version" field, the current implementation requires all reserved bits to be set to `0` for backwards compatibility.

Possible use cases for the reserved bits include:

* a flag to differentiate between normal and "short" address format (with view key equal to the spend key)
* different KDF algorithms for generating the private key
* seed encrypted with a passphrase

### Wallet birthday

The mnemonic phrase doesn't store block height but the approximate date when the wallet was created. This allows the seed to be generated offline without access to the blockchain. Wallet software can easily convert a date to the corresponding block height when restoring a seed.

The wallet creation date has a resolution of 2629746 seconds (1/12 of the average Gregorian year). All dates between June 2020 and September 2105 can be represented.

### Private key seed

The private key is derived from the 128-bit seed using PBKDF2-HMAC-SHA256 with 4096 iterations.The wallet birthday and network type are used as a salt. 128-bit seed provides the same level of security as the elliptic curve used by Monero.

Future extensions may define other KDFs.

### Checksum

The mnemonic phrase can be treated as a polynomial over GF(2048), which allows us to use an efficient Reed-Solomon error correction code with one check word. All single-word errors can be detected and all single-word erasures can be corrected without false positives.

To prevent the seed from being accidentally used with a different cryptocurrency, a coin-specific value is subtracted from the first data-word after the checksum is calculated. Checksum validation will fail unless the wallet software adds the same value back to the first data-word when restoring.
