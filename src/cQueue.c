/*!\file cQueue.c
** \author SMFSW
** \version 1.0
** \date 2017/07/14
** \copyright BSD 3-Clause License (c) 2017, SMFSW
** \brief Queue handling library (designed in c on STM32)
** \details Queue handling library (designed in c on STM32)
**/


#include <string.h>
#include <stdlib.h>

#include "cQueue.h"


#define INC_IDX(ctr, end, start)	if (ctr < (end-1))	{ ctr++; }		\
									else				{ ctr = start; }	//!< Increments buffer index \b cnt rolling back to \b start when limit \b end is reached

#define DEC_IDX(ctr, end, start)	if (ctr > (start))	{ ctr--; }		\
									else				{ ctr = end-1; }	//!< Decrements buffer index \b cnt rolling back to \b end when limit \b start is reached


void q_init(Queue_t * q, uint16_t size_rec, uint16_t nb_recs, QueueType type, bool overwrite)
{
	q->rec_nb = nb_recs;
	q->rec_sz = size_rec;
	q->impl = type;
	q->ovw = overwrite;
	
	if (q->init == 0x5AA5)	{ q_kill(q); }	// Free existing data (if any)
	q->queue = (uint8_t *) malloc(nb_recs * size_rec);

	q->init = 0x5AA5;

	q_clean(q);
}

void q_kill(Queue_t * q)
{
	free(q->queue);
}


void q_clean(Queue_t * q)
{
	q->in = 0;
	q->out = 0;
	q->cnt = 0;
}


bool q_push(Queue_t * q, void * record)
{
	if ((!q->ovw) && q_isFull(q))	{ return false; }
	
	uint8_t * pStart = q->queue + (q->rec_sz * q->in);
	memcpy(pStart, record, q->rec_sz);
	
	INC_IDX(q->in, q->rec_nb, 0);
	
	if (!q_isFull(q))	{ q->cnt++; }	// Increase records count
	else if (q->ovw)					// Queue is full and ovwite is allowed
	{
		if (q->impl == FIFO)			{ INC_IDX(q->out, q->rec_nb, 0); }	// as oldest record is overwriten, increment out
		//else if (q->impl == LIFO)	{}										// Nothing to do in this case
	}
	
	return true;
}

bool q_pop(Queue_t * q, void * record)
{
	uint8_t * pStart;
	
	if (q_isEmpty(q))	{ return false; }	// No more records
	
	if (q->impl == FIFO)
	{
		pStart = q->queue + (q->rec_sz * q->out);
		INC_IDX(q->out, q->rec_nb, 0);
	}
	else if (q->impl == LIFO)
	{
		DEC_IDX(q->in, q->rec_nb, 0);
		pStart = q->queue + (q->rec_sz * q->in);
	}
	else	{ return false; }
	
	memcpy(record, pStart, q->rec_sz);
	q->cnt--;	// Decrease records count
	return true;
}

bool q_peek(Queue_t * q, void * record)
{
	uint8_t * pStart;
	
	if (q_isEmpty(q))	{ return false; }	// No more records
	
	if (q->impl == FIFO)
	{
		pStart = q->queue + (q->rec_sz * q->out);
		// No change on out var as it's just a peek
	}
	else if (q->impl == LIFO)
	{
		uint16_t rec = q->in;	// Temporary var for peek (no change on var with DEC_IDX)
		DEC_IDX(rec, q->rec_nb, 0);
		pStart = q->queue + (q->rec_sz * rec);
	}
	else	{ return false; }
	
	memcpy(record, pStart, q->rec_sz);
	return true;
}

bool q_drop(Queue_t * q)
{
	if (q_isEmpty(q))			{ return false; }	// No more records
	
	if (q->impl == FIFO)		{ INC_IDX(q->out, q->rec_nb, 0); }
	else if (q->impl == LIFO)	{ DEC_IDX(q->in, q->rec_nb, 0); }
	else						{ return false; }
	
	q->cnt--;	// Decrease records count
	return true;
}

