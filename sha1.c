#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*** Nonlinear Functions ***/
// Rounds 0-19 func
uint32_t nl1(uint32_t x, uint32_t y, uint32_t z){
	return (x & y) | ((~x) & z);
}
// Rounds 20-39 and 60-79 func
uint32_t nl2and4(uint32_t x, uint32_t y, uint32_t z){
	return (x ^ y ^ z);
}
// Rounds 40-59
uint32_t nl3(uint32_t x, uint32_t y, uint32_t z){
	return (x & y) | (x & z) | (y & z);
}
/**************************/

// Circular Shift Helper
uint32_t cshift_left(uint32_t x, unsigned short shift){
	return ( x << shift ) | ( x >> (32 - shift));
}

// Implements SHA1 algorithm over a single message block
void process_block(uint32_t *state, uint8_t *message){
	// Expanded words array
	uint32_t W[80];
	// Message buffer
	uint8_t M[64];
	// Read from message into message buffer
	memcpy(M, message, 64);
	// For first 16 4-byte words, read directly from message buffer
	for (int t = 0; t < 16; t++){
		W[t] = M[t * 4] << 24;
        W[t] |= M[t * 4 + 1] << 16;
        W[t] |= M[t * 4 + 2] << 8;
        W[t] |= M[t * 4 + 3];
	}

	// For all 64 other 4-byte words, follow sha1 expansion function
	for (int t = 16; t < 80; t++){
		W[t] = W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16];
		W[t] = cshift_left(W[t], 1);
	}

	// Copy state into temp variables
	uint32_t a = state[0];
	uint32_t b = state[1];
	uint32_t c = state[2];
	uint32_t d = state[3];
	uint32_t e = state[4];


	// COMPRESSION PHASE
	for (int round = 0; round < 80; round++){
		// Temp variable
		uint32_t x;
		// First step nonlinear transforms and constants depend on which round
		if (round < 20){
			x = cshift_left(a, 5) + nl1(b, c, d) + e + W[round] + 0x5A827999;
		} else if (round < 40){
			x = cshift_left(a, 5) + nl2and4(b, c, d) + e + W[round] + 0x6ED9EBA1;
		} else if (round < 60){
			x = cshift_left(a, 5) + nl3(b, c, d) + e + W[round] + 0x8F1BBCDC;
		} else {
			x = cshift_left(a, 5) + nl2and4(b, c, d) + e + W[round] + 0xCA62C1D6;
		}
		e = d;
		d = c;
		c = cshift_left(b, 30);
		b = a;
		a = x;
	}

	// Update state
	state[0] = state[0] + a;
	state[1] = state[1] + b;
	state[2] = state[2] + c;
	state[3] = state[3] + d;
	state[4] = state[4] + e;
	// Block processing complete
}

int main(int argc, char ** args){
	// State array
	uint32_t state[5];
	// Length counter
	uint64_t bits_read;

	/*** SUM AND GENERATE BRANCH ***/
	if ((! strcmp(args[1], "sum")) || (! strcmp(args[1], "generate")) ){
		// Error handling
		if ( (! strcmp(args[1], "sum")) && (argc != 2) ) {
			printf("\nError: Improper number of arguments. \n'sum' mode reads the hash input data from STDIN and should be run as follows:\n\n    <executable> sum\n\n");
        	return EXIT_FAILURE;
		} else if ( (! strcmp(args[1], "generate")) && (argc != 3) ) {
			printf("\nError: Improper number of arguments. \n'generate' mode reads the prefix data from STDIN and should be run as follows:\n\n    <executable> generate <extension>\n\n");
        	return EXIT_FAILURE;
		}

		// Initialization words hardcoded per spec
		state[0] = 0x67452301;
		state[1] = 0xEFCDAB89;
		state[2] = 0x98BADCFE;
		state[3] = 0x10325476;
		state[4] = 0xC3D2E1F0;
		
		// Initial bits read is 0
		bits_read = 0;
	} 

	/*** EXTEND BRANCH ***/
	else if (! strcmp(args[1], "extend")){
		// Error handling
		if (argc != 4){
			printf("\nError: Improper number of arguments. \n'extend' mode reads the extension from STDIN and should be run as follows:\n\n    <executable> extend <prefix hash> <prefix byte length>\n\n");
        	return EXIT_FAILURE;
		}
		// Initialize state based on computed hash given as argument
		for (int i = 0; i < 5; i++){
			// Buffer to split hash into words
			char in_string[9];
			// Slice hash input into words and copy to buffer
			strlcpy(&in_string[0], &(args[2][i * 8]), 9);
			// Convert hex string into 32-byte int
			state[i] = (uint32_t) strtol(in_string, NULL, 16);
		}

		// Determine bits read in prefix based on argument
		// Need long because can have > 2^32 blocks
		long prefix_size = atoi(args[3]);
		long num_blocks;
		// No added block or evenly divisible
		if (prefix_size % 64 < 56) {
			num_blocks = (prefix_size / 64) + 1;
		} else { // added block
			num_blocks = (prefix_size / 64) + 2;
		}
		// Convert from number of blocks in prefix to number of bits
		bits_read = num_blocks * 512;
	} 
	// Invalid argument
	else {
		printf("\nError: Improper command '%s' given.\n\nPlease choose from one of the following:\n  sum\n  extend\n  generate\n\n", args[1]);
        return EXIT_FAILURE;
	}

	// Buffer for message chunk: 64 bytes => 512 bits
	uint8_t inbuff[64];
	// Read in up to 64 bytes
	int bytes_read = fread(inbuff, 1, 64, stdin);


	// If full block
	while (bytes_read == 64){
		// Increment bits read by block size
		bits_read += 512;
		// If "generate" mode, push full block input to STDOUT
		if (! strcmp(args[1], "generate")){
			printf("%.*s", bytes_read, inbuff);
		}
		// sum or extend mode
		else{
			// Update the state
			process_block(state, inbuff);
		}
		// Read in the next block
		bytes_read = fread(inbuff, 1, 64, stdin);
	}
	// If "generate" mode, push final input to STDOUT
	if (! strcmp(args[1], "generate")){
		printf("%.*s", bytes_read, inbuff);
	}
	// Final block read
	// Update bits read based on final read
	bits_read += bytes_read * 8;
	
	// Create pad
	// append "1" bit
	// If "generate" mode, push 1 bit to STDOUT
	if (! strcmp(args[1], "generate")){
		putchar(0x80);
	// sum or extend branch
	} else {
		inbuff[bytes_read] = 0x80;
	}
	// Case 1: Need to add another block after this one (no room for size message)
	if (bytes_read > 55){
		// Fill remainder of buff with 0s
		for (int i = bytes_read + 1; i < 64; i++){
			// If generate mode, add zero bytes to STDOUT
			if (! strcmp(args[1], "generate")){
				putchar(0x00);
			// sum or extend branch
			} else {
				inbuff[i] = 0;
			}
		}
		// Process filled block
		process_block(state, inbuff);
		// Create one more block for size
		// Begin with 56 zero bytes
		for (int i = 0; i < 56; i++){
			// If generate mode, add zero bytes to STDOUT
			if (! strcmp(args[1], "generate")){
				putchar(0x00);
			// sum or extend branch
			} else {
				inbuff[i] = 0;
			}
		}
	// Case 2: Room in final block for length message
	// Note: This path will be taken when message evenly divisble by 64 bytes
	// Strictly speaking, we are also adding an extra block in that case
	} else {
		// Fill buffer up to byte 56 with zero bytes
		for (int i = bytes_read + 1; i < 56; i++){
			// If generate mode, add zero bytes to STDOUT
			if (! strcmp(args[1], "generate")){
				putchar(0x00);
			// sum or extend branch
			} else {
				inbuff[i] = 0;
			}
		}
	}
	// Add length of message in bits to final 8-bytes of the last block
	inbuff[56] = bits_read >> 56;
	inbuff[57] = bits_read >> 48;
	inbuff[58] = bits_read >> 40;
	inbuff[59] = bits_read >> 32;
	inbuff[60] = bits_read >> 24;
	inbuff[61] = bits_read >> 16;
	inbuff[62] = bits_read >> 8;
	inbuff[63] = bits_read;

	// If generate mode, add length bytes to STDOUT
	if (! strcmp(args[1], "generate")){
		for (int i = 56; i < 64; i++){
			putchar(inbuff[i]);
		}
		// Also add suffix to STDOUT
		printf("%s", args[2]);
	// If in sum or extend mode, process final block and print result
	} else {
		process_block(state, inbuff);
		// Print full hash to STDOUT
		// Note: there is no convenience line break at the end to aid output functionality
		printf("%08x%08x%08x%08x%08x", state[0], state[1], state[2], state[3], state[4]);
	}
	return EXIT_SUCCESS;
}




