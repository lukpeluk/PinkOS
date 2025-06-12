 /*
 *   interrupts.h
 *
 *  Created on: Apr 18, 2010
 *      Author: anizzomc
 */

#ifndef INTERRUPS_H_
#define INTERRUPS_H_

#include <interrupts/idtLoader.h>

void _irq00Handler(void);
void _irq01Handler(void);
void _irq02Handler(void);
void _irq03Handler(void);
void _irq04Handler(void);
void _irq05Handler(void);
void _irq08Handler(void);
void _irq0CHandler(void);

void _irq80Handler(void);

void _exception0Handler(void);
void _exception6Handler(void);
void _exception1Handler(void);
void _exception2Handler(void);
void _exception3Handler(void);
void _exception4Handler(void);
void _exception5Handler(void);
void _exception7Handler(void);
void _exception8Handler(void);
void _exception9Handler(void);
void _exception10Handler(void);
void _exception11Handler(void);
void _exception12Handler(void);
void _exception13Handler(void);
void _exception14Handler(void);
void _exception15Handler(void);
void _exception16Handler(void);
void _exception17Handler(void);
void _exception18Handler(void);
void _exception19Handler(void);
void _exception20Handler(void);
void _exception21Handler(void);
void _exception22Handler(void);
void _exception23Handler(void);
void _exception24Handler(void);
void _exception25Handler(void);
void _exception26Handler(void);
void _exception27Handler(void);
void _exception28Handler(void);
void _exception29Handler(void);
void _exception30Handler(void);
void _exception31Handler(void);

void _cli(void);

void _sti(void);

void _hlt(void);

void picMasterMask(uint8_t mask);

void picSlaveMask(uint8_t mask);

//Termina la ejecución de la cpu.
void haltcpu(void);

#endif /* INTERRUPS_H_ */
