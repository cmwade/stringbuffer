#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "string_buffer.h"

// Initialization / cleanup

void sb_init(StringBuffer* sb, size_t initialSize, size_t initialISize) {
  sb->buffer = malloc(sizeof(char)*initialSize);
  sb->buffer[0] = '\0';
  sb->blen = 0;
  sb->bsize = initialSize;
  sb->cursor = 0;
  sb->insert = malloc(sizeof(char)*initialISize);
  sb->ilen = 0;
  sb->isize = initialISize;
  sb->backspace = 0;
  sb->del = 0;
  sb->inedit = false;
}

void sb_destroy(StringBuffer* sb) {
  free(sb->buffer);
  free(sb->insert);
}

// cursor movement

void sb_cur_set(StringBuffer* sb, size_t index) {
  index = index>sb->blen ? sb->blen : index;
  sb->cursor = index;
}

void sb_cur_right(StringBuffer* sb) {
  if (sb->cursor < sb->blen) { (sb->cursor)++; }
}

void sb_cur_left(StringBuffer* sb) {
  if ((sb->cursor) > 0) { (sb->cursor)--; }
}

// editing

void sb_open_edit(StringBuffer* sb) {
  sb->insert[0] = '\0';
  sb->ilen = 0;
  sb->inedit = true;
  sb->backspace = 0;
  sb->del = 0;
}

void sb_close_edit(StringBuffer* sb) {
  size_t newlen = sb->blen + sb->ilen - sb->backspace - sb->del;
  if (sb->bsize < newlen+1) {
    sb->buffer = realloc(sb->buffer, sizeof(char)*newlen*2);
    sb->bsize = newlen*2;
  }
  char* left = sb->buffer+(sb->cursor-sb->backspace); // insert middle section here
  char* right = left+sb->ilen; // from here on is the right section

  // move the right section to where it needs to be
  // have to use memmove because ranges overlap
  memmove(right, sb->buffer+sb->cursor+sb->del, (sb->blen-(sb->cursor+sb->del))+1);

  // fill in the middle part
  memcpy(left, sb->insert, sb->ilen);
  
  // left part is just still there
  // now just set the inedit flag and new length
  sb->blen = newlen;
  sb->inedit = false;
}

void sb_insert(StringBuffer* sb, char c) {
  while (sb->ilen + 2 > sb->isize) {
    sb->insert = realloc(sb->insert, sizeof(char)*sb->isize*2);
    sb->isize *= 2;
  }

  sb->insert[(sb->ilen)++] = c;
  sb->insert[sb->ilen] = '\0';
}

void sb_insert_string(StringBuffer* sb, char* str) {
  size_t i = 0;
  while (str[i] != '\0') {
    sb_insert(sb, str[i++]);
  }
}

void sb_backspace(StringBuffer* sb) {
  // If there's characters in the edit, remove them
  if (sb->ilen > 0) {
    sb->insert[--(sb->ilen)] = '\0';
  // Otherwise, add backspaces to the edit
  } else if (sb->backspace < sb->cursor) {
    sb->backspace++;
  }
}

void sb_del(StringBuffer* sb) {
  if (sb->del < sb->blen - sb->cursor) {
    sb->del++;
  }
}

// access

// Call a callback for each character of sb, in order
// Works even in the middle of an edit
void sb_foreach(StringBuffer* sb, void (*callback)(char c)) {
  if (!(sb->inedit)) {
    for (size_t i=0; i<sb->blen; i++) {
      callback(sb->buffer[i]);
    }
  } else if (sb->buffer[0] == '\0') {
    for (size_t j=0; j<sb->ilen; j++) {
      callback(sb->insert[j]);
    }
  } else {
    for (size_t i=0; i<sb->blen; i++) {
      if (i==sb->cursor-sb->backspace) {
        for (int j=0; j<sb->ilen; j++) {
          callback(sb->insert[j]);
        }
        i = sb->cursor+sb->del-1; // -1 because i will be incremented
      } else {
        callback(sb->buffer[i]);
      }
    }
  }
}

// interacting

void sb_interact(StringBuffer* sb, char command, int arg) {
  switch (command) {
    case SB_RIGHT:
      if (sb->inedit) sb_close_edit(sb);
      sb_cur_set(sb,sb->cursor+arg);
      break;
    case SB_LEFT:
      if (sb->inedit) sb_close_edit(sb);
      if (arg>sb->cursor) arg=sb->cursor;
      sb_cur_set(sb,sb->cursor-arg);
      break;
    case SB_CUR:
      if (sb->inedit) sb_close_edit(sb);
      if (arg<0) arg=0;
      sb_cur_set(sb,arg);
      break;
    case SB_INS:
      if (!(sb->inedit)) sb_open_edit(sb);
      sb_insert(sb,arg);
      break;
    case SB_DEL:
      if (!(sb->inedit)) sb_open_edit(sb);
      for (int i=0; i<arg; i++) sb_del(sb);
      break;
    case SB_BKSP:
      if (!(sb->inedit)) sb_open_edit(sb);
      for (int i=0; i<arg; i++) sb_backspace(sb);
      break;
    default:
      break;
  }
}
