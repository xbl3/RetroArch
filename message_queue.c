/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2015 - Daniel De Matteis
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <boolean.h>
#include "message_queue.h"
#include <compat/posix_string.h>

struct queue_elem
{
   unsigned duration;
   unsigned prio;
   char *msg;
};

struct msg_queue
{
   struct queue_elem **elems;
   size_t ptr;
   size_t size;
   char *tmp_msg;
};

/**
 * msg_queue_new:
 * @size              : maximum size of message
 *
 * Creates a message queue with maximum size different messages.
 *
 * Returns: NULL if allocation error, pointer to a message queue
 * if successful. Has to be freed manually.
 **/
msg_queue_t *msg_queue_new(size_t size)
{
   msg_queue_t *queue = (msg_queue_t*)calloc(1, sizeof(*queue));
   if (!queue)
      return NULL;

   queue->size = size + 1;
   queue->elems = (struct queue_elem**)
      calloc(queue->size,sizeof(struct queue_elem*)); 

   if (!queue->elems)
   {
      free(queue);
      return NULL;
   }
   queue->ptr = 1;

   return queue;
}

/**
 * msg_queue_free:
 * @queue             : pointer to queue object
 *
 * Frees message queue..
 **/
void msg_queue_free(msg_queue_t *queue)
{
   if (queue)
   {
      msg_queue_clear(queue);
      free(queue->elems);
   }
   free(queue);
}

/**
 * msg_queue_push:
 * @queue             : pointer to queue object
 * @msg               : message to add to the queue
 * @prio              : priority level of the message
 * @duration          : how many times the message can be pulled
 *                      before it vanishes (E.g. show a message for
 *                      3 seconds @ 60fps = 180 duration).
 *
 * Push a new message onto the queue.
 **/
void msg_queue_push(msg_queue_t *queue, const char *msg,
      unsigned prio, unsigned duration)
{
   size_t tmp_ptr = 0;
   struct queue_elem *new_elem = NULL;

   if (!queue || queue->ptr >= queue->size)
      return;

   new_elem = (struct queue_elem*)calloc(1, sizeof(struct queue_elem));
   if (!new_elem)
      return;

   new_elem->prio = prio;
   new_elem->duration = duration;
   new_elem->msg = msg ? strdup(msg) : NULL;

   queue->elems[queue->ptr] = new_elem;
   tmp_ptr = queue->ptr++;

   while (tmp_ptr > 1)
   {
      struct queue_elem *parent = queue->elems[tmp_ptr >> 1];
      struct queue_elem *child  = queue->elems[tmp_ptr];

      if (child->prio <= parent->prio)
         break;

      queue->elems[tmp_ptr >> 1] = child;
      queue->elems[tmp_ptr] = parent;

      tmp_ptr >>= 1;
   }
}

/**
 * msg_queue_clear:
 * @queue             : pointer to queue object
 *
 * Clears out everything in the queue.
 **/
void msg_queue_clear(msg_queue_t *queue)
{
   size_t i;

   if (!queue)
      return;

   for (i = 1; i < queue->ptr; i++)
   {
      if (queue->elems[i])
      {
         free(queue->elems[i]->msg);
         free(queue->elems[i]);
         queue->elems[i] = NULL;
      }
   }
   queue->ptr = 1;
   free(queue->tmp_msg);
   queue->tmp_msg = NULL;
}

/**
 * msg_queue_pull:
 * @queue             : pointer to queue object
 *
 * Pulls highest priority message in queue.
 *
 * Returns: NULL if no message in queue, otherwise a string
 * containing the message.
 **/
const char *msg_queue_pull(msg_queue_t *queue)
{
   struct queue_elem *front  = NULL, *last = NULL,
                     *parent = NULL, *child = NULL;
   size_t tmp_ptr = 1;
    
   (void)parent;
   (void)child;
   (void)tmp_ptr;
    
   /* Nothing in queue. */
   if (!queue || queue->ptr == 1)
      return NULL;

   front = (struct queue_elem*)queue->elems[1];
   front->duration--;
   if (front->duration > 0)
      return front->msg;

   free(queue->tmp_msg);
   queue->tmp_msg = front->msg;
   front->msg = NULL;

   front = (struct queue_elem*)queue->elems[1];
   last  = (struct queue_elem*)queue->elems[--queue->ptr];
   queue->elems[1] = last;
   free(front);

   for (;;)
   {
      size_t switch_index = tmp_ptr;
      bool left = (tmp_ptr * 2 <= queue->ptr)
         && (queue->elems[tmp_ptr] < queue->elems[tmp_ptr * 2]);
      bool right = (tmp_ptr * 2 + 1 <= queue->ptr)
         && (queue->elems[tmp_ptr] < queue->elems[tmp_ptr * 2 + 1]);

      if (!left && !right)
         break;

      if (left && !right)
         switch_index <<= 1;
      else if (right && !left)
         switch_index += switch_index + 1;
      else
      {
         if (queue->elems[tmp_ptr * 2]
               >= queue->elems[tmp_ptr * 2 + 1])
            switch_index <<= 1;
         else
            switch_index += switch_index + 1;
      }

      parent = (struct queue_elem*)queue->elems[tmp_ptr];
      child  = (struct queue_elem*)queue->elems[switch_index];
      queue->elems[tmp_ptr] = child;
      queue->elems[switch_index] = parent;
      tmp_ptr = switch_index;
   }

   return queue->tmp_msg;
}
