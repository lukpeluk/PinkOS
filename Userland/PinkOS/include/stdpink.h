#ifndef STDPINK_H
#define STDPINK_H

#include <syscallCodes.h>
#include <environmentApiEndpoints.h>
#include <stdint.h>

void strcpy(char * dest, char * src);

/*
 * Prints a string to the console
 * @param string the string to print
 * @return void
*/ 
void print(char * string);

/*
 * Prints a string with the given format to the console 
 * @param format the format of the string
 * @param ... the arguments to replace in the format
 * @return void
*/
void printf(char * format, ...);

/*
 * Prints a single character to the console
 * @param c the character to print
 * @return void
*/
void putChar(char c);

/*
 * Reads a character from the console
 * @return the character read
*/
char getChar();

/*
 * Clears the stdin buffer so that future reads are not yelding previously inserted chars
*/
void clearStdinBuffer();

/*
 * Reads from the console strings
 * and numbers given a certain format
 * @param format the format of the string
 * @param ... pointers to the variables to store the values
 * @return void
*/
void scanf(char * format, ...);


void seedRandom(uint64_t seed);

/*
 * Gives a random number between min and max
 * @param min the minimum value
 * @param max the maximum value
 * @return the random number
*/
uint32_t randInt(uint32_t min, uint32_t max);

void enableBackgroundAudio();
void disableBackgroundAudio();

void clear();

void sleep(uint64_t millis);


uint64_t getMillisElapsed();

/*
 * Compares two memory blocks
 * @param s1 the first memory block
 * @param s2 the second memory block
 * @param n the number of bytes to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
*/
uint64_t memcmp(const void *s1, const void *s2, uint64_t n);

#endif