#ifndef STDPINK_H
#define STDPINK_H

#include <syscallCodes.h>
#include <environmentApiEndpoints.h>
#include <stdint.h>

void strcpy(unsigned char * dest, unsigned char * src);

/*
 * Prints a string to the console
 * @param string the string to print
 * @return void
*/ 
void print(unsigned char * string);

/*
 * Prints a string with the given format to the console 
 * @param format the format of the string
 * @param ... the arguments to replace in the format
 * @return void
*/
void printf(unsigned char * format, ...);

/*
 * Prints a single character to the console
 * @param c the character to print
 * @return void
*/
void putChar(unsigned char c);

/*
 * Reads a character from the console
 * @return the character read
*/
unsigned char getChar();

/*
 * Reads from the console strings
 * and numbers given a certain format
 * @param format the format of the string
 * @param ... pointers to the variables to store the values
 * @return void
*/
void scanf(unsigned char * format, ...);


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

#endif