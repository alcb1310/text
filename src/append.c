#include "append.h"

/***
 * Appends a character to the buffer
 */
void abAppend(struct abuf *ab, char *s, int len) {
  char *new = realloc(ab->b, ab->len + len);

  if (new == NULL) {
    // Memory allocation failed
    return;
  }

  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}

/***
 * Free the buffer memmory
 */
void abFree(struct abuf *ab) { free(ab->b); }
