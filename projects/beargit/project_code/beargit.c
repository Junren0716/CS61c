#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - You are given the following helper functions:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the homework spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */
int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);

  FILE* fbranches = fopen(".beargit/.branches", "w");
  fprintf(fbranches, "%s\n", "master");
  fclose(fbranches);
   
  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");
  write_string_to_file(".beargit/.current_branch", "master");

  return 0;
}



/* beargit add <filename>
 * 
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR: File <filename> already added
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s already added\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit status
 *
 * See "Step 1" in the homework 1 spec.
 *
 */

int beargit_status() {
  /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.index", "r");
  char file_name[FILENAME_SIZE];
  fprintf(stdout, "%s\n\n", "Tracked files:");
  int counter = 0;
  while (fgets(file_name, sizeof(file_name), findex)) {
    strtok(file_name, "\n");
    fprintf(stdout, "%s", "  ");
    fprintf(stdout, "%s\n", file_name);
    counter++;
  }
  fprintf(stdout, "\n%d %s\n", counter, "files total");
  fclose(findex);
  return 0;
}

/* beargit rm <filename>
 * 
 * See "Step 2" in the homework 1 spec.
 *
 */

int beargit_rm(const char* filename) {
  /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");
  char file_name[FILENAME_SIZE];
  int found = 0;
  while (fgets(file_name, sizeof(file_name), findex)) {
    strtok(file_name, "\n");
    if (strcmp(file_name, filename) == 0) {
      found = 1;
    } else {
      fprintf(fnewindex, "%s\n", file_name);
    }
  }
  fclose(findex);
  fclose(fnewindex);
  fs_mv(".beargit/.newindex", ".beargit/.index");
  if (found == 0) {
    fprintf(stderr, "ERROR: File %s not tracked\n", filename);
    return 1;
  }
  return 0;
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the homework 1 spec.
 *
 */

const char* go_bears = "GO BEARS!";

int is_commit_msg_ok(const char* msg) {
  /* COMPLETE THE REST */
  int flipper = 0;
  char bears_count[] = "GO BEARS!";
  int i;
  for (i = 0; *msg != '\0'; msg++) {
    if (*msg == bears_count[i]) {
      flipper = 1;
      if (bears_count[i + 1] == '\0') {
        return 1;
      }
      i++;
    } else if (flipper == 1) {
      flipper = 0;
      i = 0;
    }
  }
  return 0;
}

void next_commit_id_part1(char* commit_id) {
  /* COMPLETE THE REST */
  char* new_id = commit_id;
  while (*new_id != '\0') {
    if (*new_id == '0') {
      *new_id = '6';
      new_id++;
      continue;
    } else if (*new_id == '6') {
      *new_id = '1';
      break;
    } else if (*new_id == '1') {
      *new_id = 'c';
      break;
    } else if (*new_id == 'c') {
      *new_id = '6';
      new_id++;
    } else {
      *new_id = '6';
      new_id++;
    }
  }
}

int beargit_commit(const char* msg) {
  char *check_head = malloc(BRANCHNAME_SIZE + 1);
  read_string_from_file(".beargit/.current_branch", check_head, BRANCHNAME_SIZE);
  if (*check_head == '\0') {
    fprintf(stderr, "%s\n", "ERROR: Need to be on HEAD of a branch to commit");
    return 1;
  }
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR: Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id); 

  /* COMPLETE THE REST */
  char *filename = malloc(strlen(".beargit/") + strlen(commit_id) + 1);
  sprintf(filename, "%s/%s", ".beargit", commit_id);
  fs_mkdir(filename);

  char *index = malloc(strlen(filename) + strlen("/.index") + 1);
  sprintf(index, "%s%s", filename, "/.index");
  fs_cp(".beargit/.index", index);

  char *prev = malloc(strlen(filename) + strlen("/.prev") + 1);
  sprintf(prev, "%s%s", filename, "/.prev");
  fs_cp(".beargit/.prev", prev);

  FILE* filelist = fopen(".beargit/.index", "r");
  char file[FILENAME_SIZE];
  while (fgets(file, sizeof(file), filelist)) {
    strtok(file, "\n");
    char *new_file = malloc(strlen(filename) + strlen(file) + 1);
    sprintf(new_file, "%s/%s", filename, file);
    fs_cp(file, new_file);
  }
  char* commit_message = malloc(strlen(filename) + strlen("/.msg") + 1);
  sprintf(commit_message, "%s%s", filename, "/.msg");
  write_string_to_file(commit_message, msg);
  write_string_to_file(".beargit/.prev", commit_id);
  fclose(filelist);
  return 0;
}

/* beargit log
 *
 * See "Step 4" in the homework 1 spec.
 * 
 */

int beargit_log(int limit) {
  /* COMPLETE THE REST */
  int x = INT_MAX;
  if (limit != INT_MAX) {
    x = limit;
  }
  char *prev_commit_id = malloc(COMMIT_ID_SIZE + 1);
  read_string_from_file(".beargit/.prev", prev_commit_id, COMMIT_ID_SIZE);
  if (strcmp("0000000000000000000000000000000000000000", prev_commit_id) == 0) {
      fprintf(stderr, "ERROR: There are no commits!\n");
      return 1;
  }
  while (x > 0) {
    if (strcmp("0000000000000000000000000000000000000000", prev_commit_id) == 0) {
      break;
    }
    fprintf(stdout, "\n%s %s\n", "commit", prev_commit_id);
    char *prev_message = malloc(MSG_SIZE + 1);
    char *prev_dir = malloc(strlen(".beargit/") + strlen(prev_commit_id) + strlen("/.msg") + 1);
    sprintf(prev_dir, "%s/%s/%s", ".beargit", prev_commit_id, ".msg");
    read_string_from_file(prev_dir, prev_message, MSG_SIZE);
    fprintf(stdout, "    %s\n", prev_message);
    char *prev_directory = malloc(strlen(".beargit/") + strlen(prev_commit_id) + strlen("/.prev") + 1);
    sprintf(prev_directory, "%s/%s/%s", ".beargit", prev_commit_id, ".prev");
    read_string_from_file(prev_directory, prev_commit_id, COMMIT_ID_SIZE);
    x--;
  }
  fprintf(stdout, "\n");
  return 0;
}


const char* digits = "61c";

void next_commit_id(char* commit_id) {
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);

  // The first COMMIT_ID_BRANCH_BYTES=10 characters of the commit ID will
  // be used to encode the current branch number. This is necessary to avoid
  // duplicate IDs in different branches, as they can have the same pre-
  // decessor (so next_commit_id has to depend on something else).
  int n = get_branch_number(current_branch);
  for (int i = 0; i < COMMIT_ID_BRANCH_BYTES; i++) {
    commit_id[i] = digits[n%3];
    n /= 3;
  }

  // Use next_commit_id to fill in the rest of the commit ID.
  next_commit_id_part1(commit_id + COMMIT_ID_BRANCH_BYTES);
}


// This helper function returns the branch number for a specific branch, or
// returns -1 if the branch does not exist.
int get_branch_number(const char* branch_name) {
  FILE* fbranches = fopen(".beargit/.branches", "r");

  int branch_index = -1;
  int counter = 0;
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), fbranches)) {
    strtok(line, "\n");
    if (strcmp(line, branch_name) == 0) {
      branch_index = counter;
    }
    counter++;
  }

  fclose(fbranches);

  return branch_index;
}

/* beargit branch
 *
 * See "Step 5" in the homework 1 spec.
 *
 */

int beargit_branch() {
  /* COMPLETE THE REST */
  FILE* branches = fopen(".beargit/.branches", "r");
  char branch_name[BRANCHNAME_SIZE];
  char *current_branch = malloc(BRANCHNAME_SIZE + 1);
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);
  while (fgets(branch_name, sizeof(branch_name), branches)) {
    strtok(branch_name, "\n");
    if (strcmp(current_branch, branch_name) == 0) {
      fprintf(stdout, "* ");
    } else {
      fprintf(stdout, "  ");
    }
    fprintf(stdout, "%s\n", branch_name);
  }
  fclose(branches);
  return 0;
}

/* beargit checkout
 *
 * See "Step 6" in the homework 1 spec.
 *
 */

int checkout_commit(const char* commit_id) {
  /* COMPLETE THE REST */
  if (strcmp("0000000000000000000000000000000000000000", commit_id) != 0) {
    FILE* index = fopen(".beargit/.index", "r");
    char *filenames = malloc(FILENAME_SIZE + 1);
    while (fgets(filenames, sizeof(filenames), index)) {
      strtok(filenames, "\n");
      fs_rm(filenames);
    }
    fclose(index);
    char *checked_index = malloc(strlen(".beargit/") + strlen(commit_id) + strlen("/.index") + 1);
    sprintf(checked_index, "%s/%s/%s", ".beargit", commit_id, ".index");
    fs_cp(checked_index, ".beargit/.index");
    FILE* ch_index = fopen(checked_index, "r");
    char ch_index_names[FILENAME_SIZE];
    while (fgets(ch_index_names, sizeof(ch_index_names), ch_index)) {
      strtok(ch_index_names, "\n");
      char *file = malloc(strlen(".beargit/") + strlen(commit_id) + strlen(ch_index_names) + 1);
      sprintf(file, "%s/%s/%s", ".beargit", commit_id, ch_index_names);
      fs_cp(file, ch_index_names);
    }
    fclose(ch_index);
  } else {
    write_string_to_file(".beargit/.index", "\0");
  }
  write_string_to_file(".beargit/.prev", commit_id);
  return 0;
}

int is_it_a_commit_id(const char* commit_id) {
  /* COMPLETE THE REST */
  if (strlen(commit_id) != COMMIT_ID_BYTES) {
    return 0;
  }
  for (; commit_id == "\0"; commit_id++) {
    if (commit_id != "6" || commit_id != "1" || commit_id != "c") {
      return 0;
    }
  }
  return 1;
}

int beargit_checkout(const char* arg, int new_branch) {
  // Get the current branch
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);

  // If not detached, update the current branch by storing the current HEAD into that branch's file...
  // Even if we cancel later, this is still ok.
  if (strlen(current_branch)) {
    char current_branch_file[BRANCHNAME_SIZE + 100];
    sprintf(current_branch_file, ".beargit/.branch_%s", current_branch);
    fs_cp(".beargit/.prev", current_branch_file);
  }

  // Check whether the argument is a commit ID. If yes, we just stay in detached mode
  // without actually having to change into any other branch.
  if (is_it_a_commit_id(arg)) {
    char commit_dir[FILENAME_SIZE + 100] = ".beargit/";
    strcat(commit_dir, arg);
    if (!fs_check_dir_exists(commit_dir)) {
      fprintf(stderr, "ERROR: Commit %s does not exist\n", arg);
      return 1;
    }

    // Set the current branch to none (i.e., detached).
    write_string_to_file(".beargit/.current_branch", "");

    return checkout_commit(arg);
  }

  // Just a better name, since we now know the argument is a branch name.
  const char* branch_name = arg;

  // Read branches file (giving us the HEAD commit id for that branch).
  int branch_exists = (get_branch_number(branch_name) >= 0);

  // Check for errors.
  if ((branch_exists && new_branch)) {
    fprintf(stderr, "ERROR: A branch named %s already exists\n", branch_name);
    return 1;
  } else if (!branch_exists && !new_branch) {
    fprintf(stderr, "ERROR: No branch %s exists\n", branch_name);
    return 1;
  }

  // File for the branch we are changing into.
  char branch_file[FILENAME_SIZE] = ".beargit/.branch_"; 
  strcat(branch_file, branch_name);

  // Update the branch file if new branch is created (now it can't go wrong anymore)
  if (new_branch) {
    FILE* fbranches = fopen(".beargit/.branches", "a");
    fprintf(fbranches, "%s\n", branch_name);
    fclose(fbranches);
    fs_cp(".beargit/.prev", branch_file); 
  }

  write_string_to_file(".beargit/.current_branch", branch_name);

  // Read the head commit ID of this branch.
  char branch_head_commit_id[COMMIT_ID_SIZE];
  read_string_from_file(branch_file, branch_head_commit_id, COMMIT_ID_SIZE);

  // Check out the actual commit.
  return checkout_commit(branch_head_commit_id);
}
