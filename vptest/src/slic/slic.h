/*
 * slic.h
 *
 *  Created on: Jun 23, 2013
 *      Author: rob
 */

#ifndef SLIC_H_
#define SLIC_H_

#define NUMBER_OF_CHAN 1
#define NUMBER_OF_PROSLIC (NUMBER_OF_CHAN)

typedef struct chanStatus chanState; //forward declaration

typedef void (*procState) (chanState *pState, ProslicInt eInput);


/*
** structure to hold state information
*/
struct chanStatus {
	proslicChanType *ProObj;
} ;

#endif /* SLIC_H_ */
