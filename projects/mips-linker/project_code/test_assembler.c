#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <CUnit/Basic.h>

#include "src/utils.h"
#include "src/tables.h"
#include "src/translate_utils.h"
#include "src/translate.h"

const char* TMP_FILE = "test_output.txt";
const int BUF_SIZE = 1024;

/****************************************
 *  Helper functions 
 ****************************************/

int do_nothing() {
    return 0;
}

int init_log_file() {
    set_log_file(TMP_FILE);
    return 0;
}

int check_lines_equal(char **arr, int num) {
    char buf[BUF_SIZE];

    FILE *f = fopen(TMP_FILE, "r");
    if (!f) {
        CU_FAIL("Could not open temporary file");
        return 0;
    }
    for (int i = 0; i < num; i++) {
        if (!fgets(buf, BUF_SIZE, f)) {
            CU_FAIL("Reached end of file");
            return 0;
        }
        CU_ASSERT(!strncmp(buf, arr[i], strlen(arr[i])));
    }
    fclose(f);
    return 0;
}

/****************************************
 *  Test cases for translate_utils.c 
 ****************************************/

void test_translate_reg() {
    CU_ASSERT_EQUAL(translate_reg("$0"), 0);
    CU_ASSERT_EQUAL(translate_reg("$at"), 1);
    CU_ASSERT_EQUAL(translate_reg("$v0"), 2);
    CU_ASSERT_EQUAL(translate_reg("$a0"), 4);
    CU_ASSERT_EQUAL(translate_reg("$a1"), 5);
    CU_ASSERT_EQUAL(translate_reg("$a2"), 6);
    CU_ASSERT_EQUAL(translate_reg("$a3"), 7);
    CU_ASSERT_EQUAL(translate_reg("$t0"), 8);
    CU_ASSERT_EQUAL(translate_reg("$t1"), 9);
    CU_ASSERT_EQUAL(translate_reg("$t2"), 10);
    CU_ASSERT_EQUAL(translate_reg("$t3"), 11);
    CU_ASSERT_EQUAL(translate_reg("$s0"), 16);
    CU_ASSERT_EQUAL(translate_reg("$s1"), 17);
    CU_ASSERT_EQUAL(translate_reg("$3"), -1);
    CU_ASSERT_EQUAL(translate_reg("asdf"), -1);
    CU_ASSERT_EQUAL(translate_reg("hey there"), -1);
}

void test_translate_num() {
    long int output;

    CU_ASSERT_EQUAL(translate_num(&output, "35", -1000, 1000), 0);
    CU_ASSERT_EQUAL(output, 35);
    CU_ASSERT_EQUAL(translate_num(&output, "145634236", 0, 9000000000), 0);
    CU_ASSERT_EQUAL(output, 145634236);
    CU_ASSERT_EQUAL(translate_num(&output, "0xC0FFEE", -9000000000, 9000000000), 0);
    CU_ASSERT_EQUAL(output, 12648430);
    CU_ASSERT_EQUAL(translate_num(&output, "72", -16, 72), 0);
    CU_ASSERT_EQUAL(output, 72);
    CU_ASSERT_EQUAL(translate_num(&output, "72", -16, 71), -1);
    CU_ASSERT_EQUAL(translate_num(&output, "72", 72, 150), 0);
    CU_ASSERT_EQUAL(output, 72);
    CU_ASSERT_EQUAL(translate_num(&output, "72", 73, 150), -1);
    CU_ASSERT_EQUAL(translate_num(&output, "35x", -100, 100), -1);
}

/****************************************
 *  Test cases for tables.c 
 ****************************************/

void test_table_1() {
    int retval;

    SymbolTable* tbl = create_table(SYMTBL_UNIQUE_NAME);
    CU_ASSERT_PTR_NOT_NULL(tbl);

    retval = add_to_table(tbl, "abc", 8);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "efg", 12);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "q45", 16);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "q45", 24); 
    CU_ASSERT_EQUAL(retval, -1); 
    retval = add_to_table(tbl, "bob", 14); 
    CU_ASSERT_EQUAL(retval, -1); 

    retval = get_addr_for_symbol(tbl, "abc");
    CU_ASSERT_EQUAL(retval, 8); 
    retval = get_addr_for_symbol(tbl, "q45");
    CU_ASSERT_EQUAL(retval, 16); 
    retval = get_addr_for_symbol(tbl, "ef");
    CU_ASSERT_EQUAL(retval, -1);
    
    free_table(tbl);

    char* arr[] = { "Error: name 'q45' already exists in table.",
                    "Error: address is not a multiple of 4." };
    check_lines_equal(arr, 2);

    SymbolTable* tbl2 = create_table(SYMTBL_NON_UNIQUE);
    CU_ASSERT_PTR_NOT_NULL(tbl2);

    retval = add_to_table(tbl2, "q45", 16);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl2, "q45", 24); 
    CU_ASSERT_EQUAL(retval, 0);

    free_table(tbl2);
}

void test_table_2() {
    int retval, max = 100;

    SymbolTable* tbl = create_table(SYMTBL_UNIQUE_NAME);
    CU_ASSERT_PTR_NOT_NULL(tbl);

    char buf[10];
    for (int i = 0; i < max; i++) {
        sprintf(buf, "%d", i);
        retval = add_to_table(tbl, buf, 4 * i);
        CU_ASSERT_EQUAL(retval, 0);
    }

    for (int i = 0; i < max; i++) {
        sprintf(buf, "%d", i);
        retval = get_addr_for_symbol(tbl, buf);
        CU_ASSERT_EQUAL(retval, 4 * i);
    }

    free_table(tbl);
}


void test_addu() {
    FILE* x1 = fopen("gold.txt", "w");
    char* args0[3];
    args0[0] = "$s0";
    args0[1] = "$s1";
    args0[2] = "$x0";
    int r_val = write_rtype(0x21, x1, args0, 3);
    CU_ASSERT_EQUAL(r_val, -1);
    fclose(x1);

    FILE* x2 = fopen("gold.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s1";
    args1[2] = "$s2";
    int r_val1 = write_rtype(0x21, x2, args1, 3);
    CU_ASSERT_EQUAL(r_val1, 0);
    fclose(x2);
}

void test_or() {
    FILE* x1 = fopen("silver.txt", "w");
    char* args0[3];
    args0[0] = "$s0";
    args0[1] = "$s1";
    args0[2] = "$s2";
    int r_val = write_rtype(0x25, x1, args0, 4);
    CU_ASSERT_EQUAL(r_val, -1);
    fclose(x1);

    FILE* x2 = fopen("silver.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s1";
    args1[2] = "$s2";
    int r_val1 = write_rtype(0x25, x2, args1, 3);
    CU_ASSERT_EQUAL(r_val1, 0);
    fclose(x2);
}

void test_slt() {
    FILE* x1 = fopen("gold.txt", "w");
    char* args0[3];
    args0[0] = "$s1";
    args0[1] = "$s2";
    args0[2] = "$s3";
    int r_val = write_rtype(0x2a, x1, args0, 2);
    CU_ASSERT_EQUAL(r_val, -1);
    fclose(x1);

    FILE* x2 = fopen("gold.txt", "w");
    char* args1[3];
    args1[0] = "$s1";
    args1[1] = "$s2";
    args1[2] = "$s3";
    int r_val1 = write_rtype(0x2a, x2, args1, 4);
    CU_ASSERT_EQUAL(r_val1, -1);
    fclose(x2);

}

void test_sltu() {
    uint8_t funct = 0x2b;
    FILE* x1 = fopen("silver.txt", "w");
    char* args0[3];
    args0[0] = "$s1";
    args0[1] = "$s2";
    args0[2] = "$s3";
    int r_val = write_rtype(0x2b, x1, args0, 2);
    CU_ASSERT_EQUAL(r_val, -1);
    fclose(x1);

    FILE* x2 = fopen("silver.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s1";
    args1[2] = "$s2";
    int r_val1 = write_rtype(0x2b, x2, args1, 4);
    CU_ASSERT_EQUAL(r_val1, -1);
    fclose(x2);
}

void test_sll() {
    FILE* x1 = fopen("ruby.txt", "w");
    char* args0[3];
    args0[0] = "$x0";
    args0[1] = "$t1";
    args0[2] = "0x2";
    int r_val = write_shift(0x00, x1, args0, 3);
    CU_ASSERT_EQUAL(r_val, -1);
    fclose(x1);

    FILE* x2 = fopen("ruby.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s1";
    args1[2] = "0x15";
    int r_val1 = write_shift(0x00, x2, args1, 3);
    CU_ASSERT_EQUAL(r_val1, 0);
    fclose(x2);
}


void test_jr() {
    FILE* x1 = fopen("gold.txt", "w");
    char* args0[1];
    args0[0] = "$x0";
    int r_val = write_jr(0x08, x1, args0, 1);
    CU_ASSERT_EQUAL(r_val, -1);
    fclose(x1);

    FILE* x2 = fopen("gold.txt", "w");
    char* args1[1];
    args1[0] = "$ra";
    int r_val1 = write_jr(0x08, x2, args1, 1);
    CU_ASSERT_EQUAL(r_val1, 0);
    fclose(x2);
}

void test_addiu() {
    FILE* x1 = fopen("rails.txt", "w");
    char* args0[3];
    args0[0] = "$s0";
    args0[1] = "$s1";
    args0[2] = "3";
    int r_val = write_addiu(0x9, x1, args0, 4);
    CU_ASSERT_EQUAL(r_val, -1);
    fclose(x1);

    FILE* x2 = fopen("rails.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s1";
    args1[2] = "5";
    int r_val1 = write_addiu(0x9, x2, args1, 3);
    CU_ASSERT_EQUAL(r_val1, 0);
    fclose(x2);
}

void test_ori() {
    FILE* x1 = fopen("blue.txt", "w");
    char* args0[3];
    args0[0] = "$t0";
    args0[1] = "$b1";
    args0[2] = "3";
    int r_val = write_ori(0xd, x1, args0, 3);
    CU_ASSERT_EQUAL(r_val, -1);
    fclose(x1);

    FILE* x2 = fopen("blue.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s1";
    args1[2] = "3";
    int r_val1 = write_ori(0xd, x2, args1, 4);
    CU_ASSERT_EQUAL(r_val1, -1);
    fclose(x2);
}

void test_lui() {
    FILE* x1 = fopen("gold.txt", "w");
    char* args0[2];
    args0[0] = "$x0";
    args0[1] = "3";
    int r_val = write_lui(0xf, x1, args0, 2);
    CU_ASSERT_EQUAL(r_val, -1);
    fclose(x1);

    FILE* x2 = fopen("gold.txt", "w");
    char* args1[2];
    args1[0] = "$s0";
    args1[1] = "3";
    int r_val1 = write_lui(0xf, x2, args1, 1);
    CU_ASSERT_EQUAL(r_val1, -1);
    fclose(x2);
}

void test_lb() {
    FILE* x1 = fopen("blue.txt", "w");
    char* args0[3];
    args0[0] = "$s0";
    args0[1] = "$s0";
    args0[2] = "3";
    int r_val = write_mem(0x20, x1, args0, 1);
    CU_ASSERT_EQUAL(r_val, -1);
    fclose(x1);

    FILE* x2 = fopen("blue.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s1";
    args1[2] = "3";
    int r_val1 = write_mem(0x20, x2, args1, 7);
    CU_ASSERT_EQUAL(r_val1, -1);
    fclose(x2);
 }

  void test_lw() {
    FILE* x1 = fopen("red.txt", "w");
    char* args0[3];
    args0[0] = "$s0";
    args0[1] = "$s0";
    args0[2] = "4";
    int r_val = write_mem(0x23, x1, args0, 1);
    CU_ASSERT_EQUAL(r_val, -1);
    fclose(x1);

    FILE* x2 = fopen("red.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s1";
    args1[2] = "4";
    int r_val1 = write_mem(0x23, x2, args1, 7);
    CU_ASSERT_EQUAL(r_val1, -1);
    fclose(x2);
 }


  void test_sb() {
    FILE* x1 = fopen("ruby.txt", "w");
    char* args0[3];
    args0[0] = "$x0";
    args0[1] = "$s0";
    args0[2] = "4";
    int r_val = write_mem(0x28, x1, args0, 3);
    CU_ASSERT_EQUAL(r_val, -1);
    fclose(x1);

    FILE* x2 = fopen("ruby.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s0";
    args1[2] = "4";
    int r_val1 = write_mem(0x28, x2, args1, 1);
    CU_ASSERT_EQUAL(r_val1, -1);
    fclose(x2);
 }


 void test_sw() {
    FILE* x1 = fopen("gold.txt", "w");
    char* args0[3];
    args0[0] = "$x0";
    args0[1] = "$s0";
    args0[2] = "4";
    int r_val = write_mem(0x2b, x1, args0, 3);
    CU_ASSERT_EQUAL(r_val, -1);
    fclose(x1);

    FILE* x2 = fopen("blue.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s0";
    args1[2] = "4";
    int r_val1 = write_mem(0x2b, x2, args1, 1);
    CU_ASSERT_EQUAL(r_val1, -1);
    fclose(x2);

    FILE* x3 = fopen("blue.txt", "w");
    char* args2[3];
    args2[0] = "$s0";
    args2[1] = "4";
    args2[2] = "$s1";
    int r_val2 = write_mem(0x2b, x3, args2, 3);
    CU_ASSERT_EQUAL(r_val2, 0);
    fclose(x3);
 }

  void test_li() {
    FILE* x1 = fopen("diamond.txt", "w");
    char* name = "li";
    char* args0[2];
    args0[0] = "$s0";
    args0[1] = "5";
    unsigned r_val =  write_pass_one(x1, name, args0, 1);
    CU_ASSERT_EQUAL(r_val, 0);
    fclose(x1);

    FILE* x2 = fopen("diamond.txt", "w");
    char* args1[2];
    args1[0] = "$s0";
    args1[1] = "5";
    unsigned r_val1 =  write_pass_one(x2, name, args1, 5);
    CU_ASSERT_EQUAL(r_val1, 0);
    fclose(x2);
 }

   void test_blt() {
    FILE* x1 = fopen("pearl.txt", "w");
    char* name = "blt";
    char* args0[3];
    args0[0] = "$t0";
    args0[1] = "$t1";
    args0[2] = "523";
    unsigned r_val =  write_pass_one(x1, name, args0, 1);
    CU_ASSERT_EQUAL(r_val, 0);
    fclose(x1);

    FILE* x2 = fopen("pearl.txt", "w");
    char* args1[3];
    args1[0] = "$rs";
    args1[1] = "$rt";
    args1[2] = "Label";
    unsigned r_val1 =  write_pass_one(x2, name, args1, 3);
    CU_ASSERT_EQUAL(r_val1, 2);
    fclose(x2);
 }


/****************************************
 *  Add your test cases here
 ****************************************/

int main(int argc, char** argv) {
    CU_pSuite pSuite1 = NULL, pSuite2 = NULL;
    CU_pSuite pSuite3 = NULL, pSuite4 = NULL;



    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    /* Suite 1 */
    pSuite1 = CU_add_suite("Testing translate_utils.c", NULL, NULL);
    if (!pSuite1) {
        goto exit;
    }
    if (!CU_add_test(pSuite1, "test_translate_reg", test_translate_reg)) {
        goto exit;
    }
    if (!CU_add_test(pSuite1, "test_translate_num", test_translate_num)) {
        goto exit;
    }

    /* Suite 2 */
    pSuite2 = CU_add_suite("Testing tables.c", init_log_file, NULL);
    if (!pSuite2) {
        goto exit;
    }
    if (!CU_add_test(pSuite2, "test_table_1", test_table_1)) {
        goto exit;
    }
    if (!CU_add_test(pSuite2, "test_table_2", test_table_2)) {
        goto exit;
    }

   /* Suite 3 */
    pSuite3 = CU_add_suite("Testing translate.c: PART N1", NULL, NULL);
    if (!pSuite3) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "addu instruction", test_addu)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "or instruction", test_or)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "slt instruction", test_slt)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "sltu instruction", test_sltu)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "sll instruction", test_sll)) {
        goto exit;
    }
     if (!CU_add_test(pSuite3, "jr instruction", test_jr)) {
        goto exit;
    }
     if (!CU_add_test(pSuite3, "addiu instruction", test_addiu)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "ori instruction", test_ori)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "lui instruction", test_lui)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "lb instruction", test_lb)) {
        goto exit;
    }
     if (!CU_add_test(pSuite3, "lw instruction", test_lw)) {
        goto exit;
    }
     if (!CU_add_test(pSuite3, "sb instruction", test_sb)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "sw instruction", test_sw)) {
        goto exit;
    }

    /* Suite 4 */
    pSuite4 = CU_add_suite("Testing translate.c: PART N2", NULL, NULL);
    if (!pSuite4) {
        goto exit;
    }
    if (!CU_add_test(pSuite4, "li instruction", test_li)) {
        goto exit;
    }
    if (!CU_add_test(pSuite4, "blt instruction", test_blt)) {
        goto exit;
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

exit:
    CU_cleanup_registry();
    return CU_get_error();;
}