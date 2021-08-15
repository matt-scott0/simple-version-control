#ifndef svc_h
#define svc_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct resolution {
    // NOTE: DO NOT MODIFY THIS STRUCT
    char *file_name;
    char *resolved_file;
} resolution;

struct root {
  struct branch* branchesArr;
  struct branch* currentBranch;
  int numBranches;
};

struct branch {
  struct commit* commitsArr;
  int numCommits;
  struct fileNode* trackedFiles;
  int numTracked;
  char* branchName;
};

struct commit {
    char* commitID;
    char* message;
    char* inBranch;
    struct fileNode* filesArr;
    struct changeFileNode* changeArray;
    int changeCount;
    int numFiles;
    int HEAD; // if 1, it is the head. If 0, naaaah.
};

struct fileNode {
  char* fileName; // max filename is 260 chars long (exc null term)
  char* fileContents;
  int fileContentSize;
  int fileHash;
};

struct changeFileNode {
  char* fileName;
  int changeType; // 0 = add, 1 == delete, 2 == modification
};


int getCommitID(void *helper, char *message);

int cstring_cmp(const void *a, const void *b);

void localDelCheck(void *helper);

void rehash(void *helper);

int uncommitedChanges(void *helper);

void *svc_init(void);

void cleanup(void *helper);

int hash_file(void *helper, char *file_path);

char *svc_commit(void *helper, char *message);

void *get_commit(void *helper, char *commit_id);

char **get_prev_commits(void *helper, void *commit, int *n_prev);

void print_commit(void *helper, char *commit_id);

int svc_branch(void *helper, char *branch_name);

int svc_checkout(void *helper, char *branch_name);

char **list_branches(void *helper, int *n_branches);

int svc_add(void *helper, char *file_name);

int svc_rm(void *helper, char *file_name);

int svc_reset(void *helper, char *commit_id);

char *svc_merge(void *helper, char *branch_name, resolution *resolutions, int n_resolutions);

#endif
