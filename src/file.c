#include "file.h"
#include "input.h"
#include "row.h"
#include "syntax.h"
#include "terminal.h"

/***
 * converts rows to a string
 *
 * @param *buflen The length of the buffer
 *
 * @return the file as a string
 */
char *editorRowsToString(int *buflen) {
  int totlen = 0;
  for (int j = 0; j < E.numrows; j++) {
    totlen += E.row[j].size + 1;
  }
  *buflen = totlen;

  char *buf = malloc(totlen);
  char *p = buf;

  for (int j = 0; j < E.numrows; j++) {
    memcpy(p, E.row[j].chars, E.row[j].size);
    p += E.row[j].size;
    *p = '\n';
    p++;
  }

  return buf;
}

/***
 * Reads a file into a buffer
 *
 * @param filename The name of the file to open
 */
void editorOpen(char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    die("editorOpen: fopen");
  }

  E.filename = strdup(filename);
  editorSelectSyntaxHighlight();

  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;

  while ((linelen = getline(&line, &linecap, fp)) != -1) {
    while (linelen > 0 &&
           (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
      linelen--;
    }

    editorInsertRow(E.numrows, line, linelen);
  }

  free(line);
  fclose(fp);

  E.dirty = 0;
}

/***
 * Writes the current file to disk
 */
void editorSave() {
  if (E.filename == NULL) {
    E.filename = editorPrompt("Save as: %s", NULL);
    if (E.filename == NULL) {
      editorSetStatusMessage("Save aborted");
      return;
    }

    editorSelectSyntaxHighlight();
  }

  int len;
  char *buf = editorRowsToString(&len);

  // O_RDWR = open for reading and writing
  // O_CREAT = create the file if it doesn't exist
  // 0644 = rw-r--r--
  int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
  if (fd != 1) {
    if (ftruncate(fd, len) != -1) {
      if (write(fd, buf, len) != -1) {
        close(fd);
        free(buf);
        editorSetStatusMessage("%d bytes written to '%s'", len, E.filename);
        E.dirty = 0;
        return;
      }
    }

    close(fd);
  }
  free(buf);
  editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
}
