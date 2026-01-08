/*
 * string_buffer.h
 * A tiny C library for efficiently manageing user-editable strings
 * Author: Charlotte Wade
 */

#pragma once

#include <stdbool.h>

typedef struct StringBuffer {
  char* buffer;  // buffer outside of current edit
  size_t blen;   // length of string currently in buffer
  size_t bsize;   // size of main buffer (discounting current edit)
  size_t cursor; // position of cursor from [0,size]
  char*  insert; // characters to insert in current edit
  size_t ilen;   // the length of inserts in current edit, discounting backspace/del
  size_t isize;  // size of insert buffer
  size_t backspace; // number of backspaces in current edit
  size_t del;       // number of deletes in current edit
  bool inedit;   // whether an edit is taking place
} StringBuffer;


void sb_init(StringBuffer* sb, size_t initialSize, size_t initialISize);
void sb_destroy(StringBuffer* sb);
void sb_cur_set(StringBuffer* sb, size_t index);
void sb_cur_right(StringBuffer* sb);
void sb_cur_left(StringBuffer* sb);
void sb_open_edit(StringBuffer* sb);
void sb_close_edit(StringBuffer* sb);
void sb_insert(StringBuffer* sb, char c);
void sb_insert_string(StringBuffer* sb, char* str);
void sb_backspace(StringBuffer* sb);
void sb_del(StringBuffer* sb);
void sb_foreach(StringBuffer* sb, void (*callback)(char c));

#define SB_RIGHT 'r'
#define SB_LEFT 'l'
#define SB_CUR 'c'
#define SB_INS 'i'
#define SB_DEL 'd'
#define SB_BKSP 'b'
void sb_interact(StringBuffer* sb, char command, int arg);

