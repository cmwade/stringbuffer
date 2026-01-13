#include <stdio.h>

#include "munit.h"
#include "string_buffer.h"

#define MUNIT(NAME,TEST,SETUP,TEARDOWN,OPTIONS,PARAMETERS) { "/"#NAME,TEST,SETUP,TEARDOWN,OPTIONS,PARAMETERS }

#define MUNIT_TESTS_END { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
#define MUNIT_SUITES_END { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }

// Setup/teardown

static void* setup_sb(const MunitParameter params[], void* data) {
  StringBuffer* sb = malloc(sizeof(StringBuffer));
  sb_init(sb,1,1);
  return (void*)sb;
}

static void teardown_sb(void* data) {
  sb_destroy((StringBuffer*)data);
  free(data);
}

// Callback for printing
void putchar_cb(char c) {
  putchar(c);
}

// Tests

// inserting

// basic typing
MunitResult insert_basic(const MunitParameter params[], void* data) {
  StringBuffer* sb = (StringBuffer*)data;
  sb_open_edit(sb);
  sb_insert_string(sb,"abc");
  munit_assert_size(sb->ilen,==,3);
  munit_assert_size(sb->isize,>,3);
  sb_close_edit(sb);

  munit_assert_string_equal(sb->buffer, "abc");
  munit_assert_size(sb->blen,==,3);
  munit_assert_size(sb->bsize,>,3);
  return MUNIT_OK;
}

// editing around existing text
MunitResult insert_multiple(const MunitParameter params[], void* data) {
  StringBuffer* sb = (StringBuffer*)data;
  sb_open_edit(sb);
  sb_insert_string(sb,"00");
  sb_close_edit(sb);

  sb_cur_set(sb, 0);

  for (int i=0; i<5; i++) {
    sb_open_edit(sb);
    sb_insert_string(sb,"99");
    munit_assert_size(sb->ilen,==,2);
    munit_assert_size(sb->isize,>,2);
    sb_close_edit(sb);
    munit_assert_size(sb->blen,==,2+2*(i+1));
    munit_assert_size(sb->bsize,>,2+2*(i+1));
    sb_cur_right(sb);
    sb_cur_right(sb);
    sb_cur_right(sb);
  }

  munit_assert_string_equal(sb->buffer, "990990999999");
  return MUNIT_OK;
}

// backspace

// deleting existing text
MunitResult bk_delete_existing(const MunitParameter params[], void* data) {
  StringBuffer* sb = (StringBuffer*)data;
  sb_open_edit(sb);
  sb_insert_string(sb,"one two seven three");
  sb_close_edit(sb);
  
  sb_cur_set(sb, 13);
  sb_open_edit(sb);
  for (int i=0; i<6; i++) {
    sb_backspace(sb);
  }
  sb_close_edit(sb);

  munit_assert_string_equal(sb->buffer, "one two three");
  return MUNIT_OK;
}

// replacing existing text
MunitResult bk_replace_existing(const MunitParameter params[], void* data) {
  StringBuffer* sb = (StringBuffer*)data;
  sb_open_edit(sb);
  sb_insert_string(sb, "one two seven four");
  sb_close_edit(sb);
  
  sb_cur_set(sb, 13);
  sb_open_edit(sb);
  for (int i=0; i<5; i++) {
    sb_backspace(sb);
  }
  sb_insert_string(sb,"three");
  sb_close_edit(sb);

  munit_assert_string_equal(sb->buffer, "one two three four");
  return MUNIT_OK;
}

// replacing insert
MunitResult bk_replace_insert(const MunitParameter params[], void* data) {
  StringBuffer* sb = (StringBuffer*)data;
  sb_open_edit(sb);
  sb_insert_string(sb,"bat");
  munit_assert_size(sb->ilen,==,3);
  munit_assert_string_equal(sb->insert, "bat");
  sb_backspace(sb);
  sb_backspace(sb);
  sb_backspace(sb);
  munit_assert_size(sb->ilen,==,0);
  munit_assert_string_equal(sb->insert, "");
  sb_insert_string(sb,"crow");
  munit_assert_size(sb->ilen,==,4);
  munit_assert_string_equal(sb->insert, "crow");
  sb_close_edit(sb);

  munit_assert_string_equal(sb->buffer, "crow");
  return MUNIT_OK;
}

// deleting past insert
MunitResult bk_delete_past(const MunitParameter params[], void* data) {
  StringBuffer* sb = (StringBuffer*)data;
  sb_open_edit(sb);
  sb_insert_string(sb,"124");
  sb_close_edit(sb);

  sb_cur_set(sb, 3);

  sb_open_edit(sb);
  sb_insert_string(sb,"56");
  sb_backspace(sb);
  sb_backspace(sb);
  sb_backspace(sb);
  sb_close_edit(sb);

  munit_assert_string_equal(sb->buffer, "12");
  return MUNIT_OK;
}

// deleting past insert then typing again
MunitResult bk_delete_past_replace(const MunitParameter params[], void* data) {
  StringBuffer* sb = (StringBuffer*)data;
  sb_open_edit(sb);
  sb_insert_string(sb,"124");
  sb_close_edit(sb);

  sb_cur_set(sb,3);

  sb_open_edit(sb);
  sb_insert_string(sb,"56");
  sb_backspace(sb);
  sb_backspace(sb);
  sb_backspace(sb);
  sb_insert_string(sb,"34");
  sb_close_edit(sb);

  munit_assert_string_equal(sb->buffer, "1234");
  return MUNIT_OK;
}

// del

// deleting by itself
MunitResult del_only(const MunitParameter params[], void* data) {
  StringBuffer* sb = (StringBuffer*)data;
  sb_open_edit(sb);
  sb_insert_string(sb,"1243");
  sb_close_edit(sb);

  sb_cur_set(sb, 2);

  sb_open_edit(sb);
  sb_del(sb);
  sb_close_edit(sb);

  munit_assert_string_equal(sb->buffer, "123");
  return MUNIT_OK;
}

// deleting and backspacing
MunitResult del_bk(const MunitParameter params[], void* data) {
  StringBuffer* sb = (StringBuffer*)data;
  sb_open_edit(sb);
  sb_insert_string(sb,"one two twelve nine three");
  sb_close_edit(sb);

  sb_cur_set(sb,14);

  sb_open_edit(sb);
  for (int i=0; i<6; i++) {
    sb_backspace(sb);
    sb_del(sb);
  }
  sb_close_edit(sb);

  munit_assert_string_equal(sb->buffer, "one two three");
  return MUNIT_OK;
}

// deleting and typing
MunitResult del_ins(const MunitParameter params[], void* data) {
  StringBuffer* sb = (StringBuffer*)data;
  sb_open_edit(sb);
  sb_insert_string(sb,"aaaaa");
  sb_close_edit(sb);

  sb_cur_set(sb,1);

  sb_open_edit(sb);
  for (int i=0; i<4; i++) {
    sb_insert(sb, 'b');
    sb_del(sb);
  }
  sb_close_edit(sb);

  munit_assert_string_equal(sb->buffer, "abbbb");
  return MUNIT_OK;
}

// deleting, backspacing, and typing
MunitResult del_bk_type(const MunitParameter params[], void* data) {
  StringBuffer* sb = (StringBuffer*)data;
  sb_open_edit(sb);
  sb_insert_string(sb,"one two twelve eight four");
  sb_close_edit(sb);

  sb_cur_set(sb,14);

  sb_open_edit(sb);
  for (int i=0; i<6; i++) {
    sb_backspace(sb);
    sb_del(sb);
  }
  sb_insert_string(sb, "three");
  sb_close_edit(sb);

  munit_assert_string_equal(sb->buffer, "one two three four");
  return MUNIT_OK;
}

int main(int argc, char* const* argv) {
  MunitTest inserttests[] = {
    MUNIT(basic,insert_basic,setup_sb,teardown_sb,MUNIT_TEST_OPTION_NONE,NULL),
    MUNIT(multiple,insert_multiple,setup_sb,teardown_sb,MUNIT_TEST_OPTION_NONE,NULL),
    MUNIT_TESTS_END
  };

  MunitTest backspacetests[] = {
    MUNIT(deleteexisting,bk_delete_existing,setup_sb,teardown_sb,MUNIT_TEST_OPTION_NONE,NULL),
    MUNIT(replaceexisting,bk_replace_existing,setup_sb,teardown_sb,MUNIT_TEST_OPTION_NONE,NULL),
    MUNIT(replaceinsert,bk_replace_insert,setup_sb,teardown_sb,MUNIT_TEST_OPTION_NONE,NULL),
    MUNIT(deletepast,bk_delete_past,setup_sb,teardown_sb,MUNIT_TEST_OPTION_NONE,NULL),
    MUNIT(deletepastreplace,bk_delete_past_replace,setup_sb,teardown_sb,MUNIT_TEST_OPTION_NONE,NULL),
    MUNIT_TESTS_END
  };

  MunitTest deltests[] = {
    MUNIT(delbasic,del_only,setup_sb,teardown_sb,MUNIT_TEST_OPTION_NONE,NULL),
    MUNIT(delbackspace,del_bk,setup_sb,teardown_sb,MUNIT_TEST_OPTION_NONE,NULL),
    MUNIT(deltype,del_ins,setup_sb,teardown_sb,MUNIT_TEST_OPTION_NONE,NULL),
    MUNIT(delbktype,del_bk_type,setup_sb,teardown_sb,MUNIT_TEST_OPTION_NONE,NULL),
    MUNIT_TESTS_END
  };

  MunitSuite insert = {
    "/insert", // name
    NULL, // tests (fill in later, must be constant if declared here)
    NULL, // suites
    1, // iterations
    MUNIT_SUITE_OPTION_NONE // options
  };
  insert.tests = inserttests;

  MunitSuite backspace = {
    "/backspace",
    NULL,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
  };
  backspace.tests = backspacetests;

  MunitSuite del = {
    "/del",
    NULL,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
  };
  del.tests = deltests;

  MunitSuite suites[] = {
    insert,
    backspace,
    del,
    MUNIT_SUITES_END
  };

  MunitSuite all = {
    "/stringbuffer",
    NULL,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
  };
  all.suites = suites;

  if (argc>1 && strcmp(argv[1],"--interact") == 0) {
    StringBuffer sb;
    sb_init(&sb,1,1);

    printf("Interactive test:\n");
    char command = '\0';
    int arg = '\0';

    while (1) {
      printf("------------------------------\n");
      sb_foreach(&sb,putchar_cb);
      putchar('\n');
      printf("------------------------------\n");
      printf("Length: %lu\n",sb_len(&sb));
      if (sb.inedit && sb.ilen > 0) printf("Insert: %s\n",sb.insert);
      printf("Buffer: \n%s\n",sb.buffer);
      for (int i=0; i<=sb.cursor; i++) {
        putchar(i==sb.cursor ? '^' : ' ');
      }
      putchar('\n');
      printf("[rIGHT:lEFT:cURSOR:iNS:dEL:bKSP:qUIT][arg]\n");
      command = getchar();
      if (command == 'q') {
        break;
      }
      if (command == SB_INS) {
        arg = getchar();
      } else {
        int result = scanf("%d",&arg);
        if (result == EOF || result == 0) {
          printf("Invalid arg.\n");
          while (getchar() != '\n');
          continue;
        }
      }
      while (getchar() != '\n');

      sb_interact(&sb,command,arg);
    }

    sb_destroy(&sb);

    return 0;
  } else {
    return munit_suite_main(&all, NULL, argc, argv);
  }
}
