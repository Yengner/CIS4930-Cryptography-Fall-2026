# Homework 1

Due: September 11, 2026 at 11:59 PM.

This folder currently contains Parts 2 and 3.

## Part 2 - HMAC

Compile:

```bash
gcc hmac.c -lssl -lcrypto -o hmac
```

Run:

```bash
./hmac Message1.txt SharedKey1.txt
```

Expected generated files:

- `Key.txt`
- `ProcessedKey.txt`
- `FinalHash.txt`

## Part 3 - Challenge-Response Protocol

Compile:

```bash
gcc alice.c -lssl -lcrypto -o alice
gcc bob.c -lssl -lcrypto -o bob
```

Run in this order:

```bash
./alice Message1.txt SharedKey1.txt A_ctr.txt A_nonce.txt
./bob Ciphertext.txt Signature.txt SharedKey1.txt B_ctr.txt B_nonce.txt
./alice Message1.txt SharedKey1.txt A_ctr.txt A_nonce.txt
```

Expected generated files:

- `Key.txt`
- `Ciphertext.txt`
- `Signature.txt`
- `Response.txt`
- `Acknowledgment.txt`
- updated `A_ctr.txt`, `A_nonce.txt`, `B_ctr.txt`, `B_nonce.txt`

`RequiredFunctionsHW1.c` is included in this folder as the instructor-provided reference/template.
