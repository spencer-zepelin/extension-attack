******
NOTE: SHA1 has been proven to be insecure and should not be used where a cryptographically secure hash function is needed.
******

Applied Cryptography
MPCS 56530

SHA1 Hash and Length Extension Attack
Project 2

Spencer Zepelin

October 27, 2019

------------------


Building and Running the Program
---
A makefile has been provided to compile the program in an 
environment with the gcc compiler. The executable "sha1" can be 
compiled by simply entering the command "make".

There are three operation modes in which the program can be run:

1. sum - Reads data from STDIN and generates the SHA1 hash in 
hexadecimalto STDOUT. While aesthetically unpleasing, no 
convenience linebreak is added to the end of the hash to promote 
ease of use if chaining with other utilities.

Command:
<DATA TO STDOUT> | ./sha1 sum

2. extend - Reads the extension data from STDIN and calculates
the new hash were the extension data added to the prefix hash.

Command:
<EXTENSION DATA TO STDOUT> | ./sha1 extend <hash of prefix> <byte-length of prefix>

3. Generate - Writes a new stream to STDOUT composed of
the prefix read from STDIN, the padding added to it, and a chosen
suffix. For use in validating correcness of length extension hashes.

Command:
<PREFIX DATA TO STDOUT> | ./sha1 generate <suffix>


Testing
---
The program passed all tests included in the prompt on departmental
Linux machines. Additionally, the successful function was
demonstrated on the following cases:

  - Zero length inputs
  - 64-byte divisible inputs
  - Multi block inputs


Program Design 
---
This project was implemented with surprisingly few lines of code.
State "words" and bit length are the core variables, and they
are initialized in accordance with the mode in which the program is
run. Messages up 64-bytes in size are read sequentially and 
processed in accordance with the SHA1 algorithm to update the state.
After the final block is read, the message is appropriately padded and
the final block or blocks are processed.

Beyond the branched initialization, all three modes run over the same
lines of code. When the "generate" mode is run, data is pushed to
STDOUT rather than processed. Though this makes certain segments of
the code slightly less readable, it keeps the overall structure very
linear.

The code has been liberally commented and error handling added to 
ensure the program exits gracefully for non-conforming inputs.

I elected not to include a max size check for the input since it is,
theoretically, capable of handling well-over an exabyte of data with 
this algorithm.





