# Stringbuffer
## A tiny C library for efficiently managing user-editable strings, by Charlotte Wade

### What's in the box?

Stringbuffer provides a new type `struct StringBuffer`, also referenceable by simply `StringBuffer`, and a suite of functions prefixed with `sb_` to manipulate it. This struct and its functions implement a dynamically-resizing string, editable via common text-editor-like abstractions such as "insert at cursor," "backspace," and "delete." Under the hood, `StringBuffer` keeps track of the cumulative state of edits being made to the string and avoids resolving them into the internal character array until absolutely necessary (or until instructed), thereby minimizing redundant memory operations. This optimization is easily abstracted away from the user; the current state of the string can still be accessed as if it were being edited in real-time.

### Try it before you use it!

Stringbuffer comes with a test suite (built with `munit`) and an interactive demo. To build it, run the provided `Makefile`, configuring it for your system if neccessary. The test suite should be built at `tests/build/stringbuffertests`. To access the interactive mode, run `stringbuffertests --interact`.

### I love it! How can I get it?

Prototypes are provided in `include/string_buffer.h` and implemented in `src/string_buffer.c`. Grab the repository with `git clone --recursive`, copy these into your own project and integrate them with the build system of your choice.

### I hate it! What do I do?

Get ahold of me at `cmwade731@gmail.com`, or if you're feeling particularly driven, submit a PR!

## Documentation

### The `StringBuffer` struct

The provided `sb_` functions should cover enough use cases that you shouldn't need to directly access the fields of the struct. However, if you wish to add functionality to the struct, or to otherwise do something generally more complicated than I accounted for, the fields of the struct are documented here.

* `char* buffer`
    * The main string buffer. *May not reflect the state of the full string accurately if an edit is in-progress, since the provided functions attempt to delay memory operations.*
* `size_t blen`
    * The length of the string stored in the main string buffer.
* `size_t bsize`
    * The size of the memory block allocated to store the main buffer. The provided functions handle growing this for you automatically.
* `size_t cursor`
    * The position of the cursor within the main buffer. Note that this is *not* the same as where a cursor might be in a traditional text editor; rather, it is the position in the main buffer where the current edit started. Inserting text, backspacing, and deleting will not move this cursor, and moving the cursor in the middle of the edit will cause the displayed string to change as if the cursor had been in the new location when the user previously started making edits.
* `char* insert`
    * The buffer used to store characters which have been inserted during the current edit.
* `size_t ilen`
    * The length of the string stored in the insert buffer.
* `size_t isize`
    * The size of the memory block allocated to store the insert buffer. The provided functions handle growing this for you automatically.
* `size_t backspace`
    * The number of characters backspaced from the main buffer in the current edit. If there are characters in the insert buffer, requesting a backspace will remove the last character from the insert buffer rather than incrementing this field.
* `size_t del`
    * The number of characters deleted (like with the `del` key on your keyboard) from the main buffer in the current edit.
* `bool inedit`
    * Query this to check whether an edit is currently in progress.

### The `sb_` functions

Stringbuffer provides a number of functions, prefixed with `sb_`, for manipulating `StringBuffer` instances. They are documented here.

#### Memory Management

##### `void sb_init(StringBuffer* sb, size_t initialSize, size_t initialISize);`

Initializes a `StringBuffer` with the given initial buffer size and initial insert buffer size. Both the sizes must be at least one, and this function must be called before any edits are made. You are responsible for providing a pointer to the `StringBuffer` using the stack or the allocator of your choice.

##### `void sb_destroy(StringBuffer* sb);`

Frees all heap-dwelling assets owned by the `StringBuffer`. Will not free the pointer to the `StringBuffer` if it is allocated on the heap; you must do that yourself.

#### Cursor management

##### `void sb_cur_set(StringBuffer* sb, size_t index);`

Places the cursor at the location specified. Valid locations are from 0 to one index past the end of the string; bounds checking is done automatically.

##### `void sb_cur_right(StringBuffer* sb);`

Moves the cursor one index to the right. Will not move into an invalid location.

##### `void sb_cur_left(StringBuffer* sb);`

Moves the cursor one index to the left. Will not move into an invalid location.

#### Editing

##### `void sb_open_edit(StringBuffer* sb);`

Configures the `StringBuffer` to begin accepting edits. Must be called before any inserts, backspaces, or deletes.

##### `void sb_close_edit(StringBuffer* sb);`

Resolves the current edit into the main buffer. In general, any insert/delete operations should be done between an `sb_open_edit` and an `sb_close_edit`, except in the case of `sb_interact` which handles that for you.

##### `void sb_insert(StringBuffer* sb, char c);`

Inserts the given character.

##### `void sb_insert_string(StringBuffer* sb, char* str);`

Inserts the given string one character at a time.

##### `void sb_backspace(StringBuffer* sb);`

Simulates pressing the `backspace` key on a keyboard. If no characters have been inserted into the insert buffer, will effectively remove one character to the left of the cursor. Otherwise, will remove the most recently inserted character from the insert buffer.

##### `void sb_del(StringBuffer* sb);`

Simulates pressing the `del` key on a keyboard, removing a character at the cursor location.

##### `void sb_interact(StringBuffer* sb, char command, int arg);`

By far the easiest way to interact with the `StringBuffer`, useful for exposing it to an end user. Accepts a command representing a specific string operation and an argument, then performs the given edit. Valid commands are as follows:

* `'r'` (Macro'd as `SB_RIGHT`) and `'l'` (Macro'd as `SB_LEFT`)
    * Moves the cursor by `[arg]` spaces
* `'c'` (Macro'd as `SB_CUR`)
    * Moves the cursor to index `[arg]`
* `'i'` (Macro'd as `SB_INS`)
    * Inserts the character `[arg]` at the cursor
* `'d'` (Macro'd as `SB_DEL`)
    * Simulates pressing the `delete` key on a keyboard `[arg]` times
* `'b'` (Macro'd as `SB_BKSP`)
    * Simulates pressing the `backspace` key on a keyboard `[arg]` times

It is not neccessary to use `sb_open_edit` or `sb_close_edit` while using this function; it handles opening/closing edits automatically and minimizes how often edits must be resolved.

#### Displaying

##### `size_t sb_len(StringBuffer* sb);`

Gets the length of the current string, taking into account the current edit but not resolving it. Useful for bounds-checking `sb_peek`.

##### `char* sb_peek(StringBuffer* sb, size_t index);`

Returns a pointer to the character in the string at position `index`, taking into account the current edit but not resolving it. **Note** that the characters in front or behind of this pointer are not guaranteed; the actual next character in the string may be anywhere in the main or insert buffers depending on the state of the current edit. **Also note** that `sb_peek` will return `NULL` if the index provided is out-of-bounds, though it guarantees a valid pointer if `index>=0 && index<sb_len(sb)`.

##### `void sb_foreach(StringBuffer* sb, void (*callback)(char c));`

Runs the provided callback for each character in the string in order, taking into account the current edit but not resolving it. Useful for displaying or otherwise accessing the contents of the string without needing to care whether an edit is in progress.


