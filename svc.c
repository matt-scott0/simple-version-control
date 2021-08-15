#include "svc.h"

void *svc_init(void) {
    // TODO: Implement
    //The good old initialisation boogaloo. These structs will be pretty important.

    struct root* helper = malloc(sizeof(struct root));

    helper->numBranches = 1; // initialise to 1 branch, as a master is always made.

    struct branch* master = malloc(sizeof(struct branch));
    master->branchName = strdup("master");
    master->numCommits = 0; //
    master->numTracked = 0;
    master->trackedFiles = NULL;

    helper->branchesArr = master;
    helper->currentBranch = master;


    return (void*)helper;

}

void cleanup(void *helper) {
    // TODO: Implement

    struct root* root = helper;

    for(int i = 0; i < root->numBranches; i++) {  // Just loops through every branch and
                                                 // frees everything associated with them, which is pretty
                                                // much just everything.
      if(root->branchesArr[i].numCommits > 0) {
        for(int j = 0; j < root->branchesArr[i].numCommits; j++) {
          for(int m = 0; m < root->branchesArr[i].commitsArr[j].numFiles; m++) {
            free(root->branchesArr[i].commitsArr[j].filesArr[m].fileName);
            free(root->branchesArr[i].commitsArr[j].filesArr[m].fileContents);
          }
          for(int m = 0; m < root->branchesArr[i].commitsArr[j].changeCount; m++) {
            free(root->branchesArr[i].commitsArr[j].changeArray[m].fileName);
          }
          free(root->branchesArr[i].commitsArr[j].changeArray);
          free(root->branchesArr[i].commitsArr[j].message);
          free(root->branchesArr[i].commitsArr[j].filesArr);
          free(root->branchesArr[i].commitsArr[j].commitID);
          free(root->branchesArr[i].commitsArr[j].inBranch);
        }
        free(root->branchesArr[i].commitsArr);
      }
      for(int j = 0; j < root->branchesArr[i].numTracked; j++) {
        free(root->branchesArr[i].trackedFiles[j].fileName);
        free(root->branchesArr[i].trackedFiles[j].fileContents);
      }
      free(root->branchesArr[i].trackedFiles);
      free(root->branchesArr[i].branchName);
    }

    free(root->branchesArr);
    free(root);

    return;
}

int hash_file(void *helper, char *file_path) {
    // TODO: Implement

    if(file_path == NULL) { // if file_path is null, return -1 (as per specs)
      return -1;
    }

    int hash = 0;

    int c;
    FILE *file = fopen(file_path, "r");
    if (file) {
        for(int i = 0; i < strlen(file_path); i++) {
          hash = (hash + file_path[i]) % 1000;
        }
        while ((c = getc(file)) != EOF)
              hash = (hash + c) % 2000000000;
        fclose(file);
    } else { //If there is no file at the given path, return -2 (as per specs)
      return -2;
    }

    return hash;

}

/* USYD CODE CITATION ACKNOWLEDGEMENT
  * I declare the following lines of code have been copied form the website titled
  * qsort: sorting array of strings, integers and structs and it is not my own work.
  * Original URL: http://www.anyexample.com/programming/c/qsort__sorting_array_of_strings__integers_and_structs.xml
  * last accessed: 27/04/2020
*/
int cstring_cmp(const void *a, const void *b) {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcasecmp(*ia, *ib);
}

int getCommitID(void *helper, char *message) {
  struct root* root = helper;
  int id = 0;


  for(int i = 0; i < strlen(message); i++) {
    id = (id + message[i]) % 1000;
  }

  // This array will be where we store every file that has CHANGED
  // And then we can sort it alphabetically.

  struct changeFileNode* changeFileArr = malloc(sizeof(struct changeFileNode));
  int changeCount = 0;

  if(root->currentBranch->numCommits == 0) { // everythings gonna be addition
    for(int i = 0; i < root->currentBranch->numTracked; i++) {
      changeFileArr = realloc(changeFileArr, sizeof(struct changeFileNode) * (i+1));
      struct changeFileNode tempNode;
      tempNode.fileName = strdup(root->currentBranch->trackedFiles[i].fileName);
      tempNode.changeType = 0;
      changeFileArr[i] = tempNode;
      changeCount++;
    }
  } else { // There are previous commits, what a joke

    struct commit* previousCommit = &root->currentBranch->commitsArr[root->currentBranch->numCommits - 2];

    for(int i = 0; i < root->currentBranch->numTracked; i++) { // Check any files that have been added
      int addFlag = 0;
      for(int j = 0; j < previousCommit->numFiles; j++) {
        if(strcmp(root->currentBranch->trackedFiles[i].fileName,
              previousCommit->filesArr[j].fileName) == 0) {

          addFlag = 1;

        }
      }

      if(addFlag == 0) {
        changeCount++;

        changeFileArr = realloc(changeFileArr,
              sizeof(struct changeFileNode) * changeCount);

        changeFileArr[changeCount - 1].fileName =
              strdup(root->currentBranch->trackedFiles[i].fileName);

        changeFileArr[changeCount - 1].changeType = 0;
      }
    }

    for(int i = 0; i < previousCommit->numFiles; i++) { // Checks any files have been deleted
      int delFlag = 0;
      for(int j = 0; j < root->currentBranch->numTracked; j++) {
        if(strcmp(previousCommit->filesArr[i].fileName,
              root->currentBranch->trackedFiles[j].fileName) == 0) {

          delFlag = 1;

        }
      }

      if(delFlag == 0) {

        changeCount++;

        changeFileArr = realloc(changeFileArr, sizeof(struct changeFileNode) * changeCount);

        changeFileArr[changeCount - 1].fileName =
              strdup(previousCommit->filesArr[i].fileName);

        changeFileArr[changeCount - 1].changeType = 1;

      }
    }


    for(int i = 0; i < root->currentBranch->numTracked; i++) {
      int modFlag = 0;
      for(int j = 0; j < previousCommit->numFiles; j++) {

        if(strcmp(root->currentBranch->trackedFiles[i].fileName,
              previousCommit->filesArr[j].fileName) == 0) {

                if(root->currentBranch->trackedFiles[i].fileHash !=
                    previousCommit->filesArr[j].fileHash) {
                        modFlag = 1;
                }

        }

      }

      if(modFlag == 1) {
        changeCount++;

        changeFileArr = realloc(changeFileArr, sizeof(struct changeFileNode) * changeCount);

        changeFileArr[changeCount - 1].fileName =
              strdup(root->currentBranch->trackedFiles[i].fileName);

        changeFileArr[changeCount - 1].changeType = 2;
      }
    }
  }

  qsort(changeFileArr, changeCount, sizeof(struct changeFileNode), cstring_cmp);

  for(int i = 0; i < changeCount; i++) {
    if(changeFileArr[i].changeType == 0) {
      id = id + 376591;
    } else if(changeFileArr[i].changeType == 1) {
      id = id + 85973;
    } else if(changeFileArr[i].changeType == 2) {
      id = id + 9573681;
    }
    for(int m = 0; m < strlen(changeFileArr[i].fileName); m++) {
      id = (id * ((changeFileArr[i].fileName[m]) % 37)) % 15485863 + 1;
    }
  }


  if(root->currentBranch->numCommits == 0) {

    root->currentBranch->commitsArr[0].changeArray =
              malloc(sizeof(struct changeFileNode) * changeCount);

    for(int i = 0; i < changeCount; i++) {

      root->currentBranch->commitsArr[0].changeArray[i].fileName =
              strdup(changeFileArr[i].fileName);

      root->currentBranch->commitsArr[0].changeArray[i].changeType = changeFileArr[i].changeType;
    }

    root->currentBranch->commitsArr[0].changeCount = changeCount;

  } else {

    root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].changeArray =
                                      malloc(sizeof(struct changeFileNode) * changeCount);

    for(int i = 0; i < changeCount; i++) {

      root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].changeArray[i].fileName =
                                                                    strdup(changeFileArr[i].fileName);

      root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].changeArray[i].changeType =
                                                                    changeFileArr[i].changeType;

    }

    root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].changeCount = changeCount;
  }

  for(int i = 0; i < changeCount; i++ ){
     free(changeFileArr[i].fileName);
   }

   free(changeFileArr);


  return id;

} // I think this fundamentally works, don't worry about rewriting.

void localDelCheck(void *helper) {
  struct root* root = helper;
  for(int i = 0; i < root->currentBranch->numTracked; i++) {
    if(fopen(root->currentBranch->trackedFiles[i].fileName, "r") == NULL) {
      svc_rm(helper, root->currentBranch->trackedFiles[i].fileName);
    }
  }
  return;
}

void rehash(void *helper) {
  struct root* root = helper;
  for(int i = 0; i < root->currentBranch->numTracked; i++) {
    root->currentBranch->trackedFiles[i].fileHash = hash_file(helper,
                      root->currentBranch->trackedFiles[i].fileName);
  }
  return;
}

int uncommitedChanges(void *helper) {

  struct root* root = helper;

  if(root->currentBranch->numCommits == 0) {
    if(root->currentBranch->numTracked > 0) {
      return 1;
    } else {
      return 0;
    }
  }

  for(int i = 0; i < root->currentBranch->numTracked; i++) {
    if(fopen(root->currentBranch->trackedFiles[i].fileName, "r") == NULL) { // If somethings been locally deleted
      return 1;
    }
  }

  if(root->currentBranch->numTracked != root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].numFiles) { // If theres a different amount of files;
    return 1;
  }

  qsort(root->currentBranch->trackedFiles, root->currentBranch->numTracked, sizeof(struct fileNode), cstring_cmp);
  qsort(root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].filesArr, root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].numFiles, sizeof(struct fileNode), cstring_cmp);

  for(int i = 0; i < root->currentBranch->numTracked; i++) {
    if(strcmp(root->currentBranch->trackedFiles[i].fileName, root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].filesArr[i].fileName)) {
      return -1;
    }
    if(root->currentBranch->trackedFiles[i].fileHash != root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].filesArr[i].fileHash) {
      return -1;
    }
  }
  return 0;

}

char *svc_commit(void *helper, char *message) {
    // TODO: Implement
    struct root* root = helper;

    localDelCheck(helper); // Check if anythings been deleted locally
    rehash(helper); // Rehash the files, so that if theres been changes can actually find it out.

    if(message == NULL) {
      return NULL;
    }

    int tempID;
    char commitIDHex[7];

    if(root->currentBranch->numCommits == 0) { // then we, good sirs, have our very first commit.
      if(root->currentBranch->numTracked == 0) { // Theres no files, it's our first commit, nothing has happened
        return NULL;
      }

      root->currentBranch->commitsArr = malloc(sizeof(struct commit));
      root->currentBranch->commitsArr[0].filesArr = malloc(sizeof(struct fileNode) * root->currentBranch->numTracked);
      root->currentBranch->commitsArr[0].numFiles = root->currentBranch->numTracked;
      root->currentBranch->commitsArr[0].message = strdup(message);
      root->currentBranch->commitsArr[0].inBranch = strdup(root->currentBranch->branchName);


      for(int i = 0; i < root->currentBranch->numTracked; i++) { // Something I learnt very early on is deep copy EVERYTHING.
        root->currentBranch->commitsArr[0].filesArr[i].fileName =
                                                    strdup(root->currentBranch->trackedFiles[i].fileName);
        root->currentBranch->commitsArr[0].filesArr[i].fileContents =
                                                    strdup(root->currentBranch->trackedFiles[i].fileContents);
        root->currentBranch->commitsArr[0].filesArr[i].fileHash =
                                                    root->currentBranch->trackedFiles[i].fileHash;
        root->currentBranch->commitsArr[0].filesArr[i].fileContentSize =
                                                    root->currentBranch->trackedFiles[i].fileContentSize;

      }

      tempID = getCommitID(helper, message);
      sprintf(commitIDHex, "%06x", tempID);


      root->currentBranch->numCommits++;

      root->currentBranch->commitsArr[0].commitID = strdup(commitIDHex);

      return(root->branchesArr[0].commitsArr[0].commitID);

    } else { // THERE WERE OTHER COMMITS, WHAT RUCKUS.

      if(uncommitedChanges(helper) == 0) {
        return NULL;
      } //Works, no touchy touchy.

      root->currentBranch->numCommits++;
      root->currentBranch->commitsArr = realloc(root->currentBranch->commitsArr,
                                        sizeof(struct commit) * root->currentBranch->numCommits);
      root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].filesArr =
                                          malloc(sizeof(struct fileNode) * root->currentBranch->numTracked);

      root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].numFiles = root->currentBranch->numTracked;

      root->currentBranch->commitsArr[root->currentBranch->numCommits - 2].HEAD = 0;

      root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].HEAD = 1;

      root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].message = strdup(message);

      root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].inBranch =
                                          strdup(root->currentBranch->branchName);


      for(int i = 0; i < root->currentBranch->numTracked; i++) {

        root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].filesArr[i].fileName =
                                                                      strdup(root->currentBranch->trackedFiles[i].fileName);

        root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].filesArr[i].fileContents =
                                                                      strdup(root->currentBranch->trackedFiles[i].fileContents);

        root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].filesArr[i].fileHash =
                                                                      root->currentBranch->trackedFiles[i].fileHash;

        root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].filesArr[i].fileContentSize =
                                                                      root->currentBranch->trackedFiles[i].fileContentSize;

      }

      tempID = getCommitID(helper, message);

      sprintf(commitIDHex, "%06x", tempID);

      root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].commitID =
                                                                  strdup(commitIDHex);

      return(root->currentBranch->commitsArr[root->currentBranch->numCommits - 1].commitID);

    }

    return NULL;
}

void *get_commit(void *helper, char *commit_id) {
    // TODO: Implement

    if(commit_id == NULL) {
      return NULL;
    }

    struct root* root = helper;

    struct commit* returnCommit;

    for(int i = 0; i < root->numBranches; i++) {
      for(int j = 0; j < root->branchesArr[i].numCommits; j++) {
        if(strcmp(root->branchesArr[i].commitsArr[j].commitID, commit_id) == 0) {
          returnCommit = &root->branchesArr[i].commitsArr[j];
          return returnCommit;
        }
      }
    }
    return NULL;
}

char **get_prev_commits(void *helper, void *commit, int *n_prev) {
    // TODO: Implement

    struct root* root = helper;

    if(n_prev == NULL) {
      return NULL;
    }

    *n_prev = 0;

    if(commit == NULL) {
      return NULL;
    }

    struct commit* tempCommit = commit;

    char** returnArr = malloc(sizeof(char*));

    for(int i = 0; i < root->numBranches; i++) {
      for(int j = 0; j < root->branchesArr[i].numCommits; j++) {
        if(&root->branchesArr[i].commitsArr[j] == tempCommit) {
          if(j == 0) {
            free(returnArr);
            return NULL;
          } else {
            *n_prev = 1;
            returnArr[0] = root->branchesArr[i].commitsArr[j - 1].commitID;
            return returnArr;
          }
        }
      }
    }

    return NULL;
}

void print_commit(void *helper, char *commit_id) {
    // TODO: Implement

    struct root* root = helper;

    if(commit_id == NULL) {
      printf("Invalid commit id\n");
      return;
    };

    int commitFound = 0;
    struct commit* currentCommit;

    for(int i = 0; i < root->numBranches; i++) {
      for(int j = 0; j < root->branchesArr[i].numCommits; j++) {
        if(strcmp(root->branchesArr[i].commitsArr[j].commitID, commit_id) == 0) {
          commitFound = 1;
          currentCommit = &root->branchesArr[i].commitsArr[j];
        }
      }
    }

    if(commitFound == 0) {
      printf("Invalid commit id\n");
      return;
    }


    printf("%s [%s]: %s\n", currentCommit->commitID,
                            currentCommit->inBranch,
                            currentCommit->message);

    for(int i = 0; i < currentCommit->changeCount; i++) {
      if(currentCommit->changeArray[i].changeType == 0) {
        printf("    + %s\n", currentCommit->changeArray[i].fileName);
      }
    }

    for(int i = 0; i < currentCommit->changeCount; i++) {
      if(currentCommit->changeArray[i].changeType == 1) {
        printf("    - %s\n", currentCommit->changeArray[i].fileName);
      }
    }

    for(int i = 0; i < currentCommit->changeCount; i++) {
      if(currentCommit->changeArray[i].changeType == 2) {
        printf("    / %s\n", currentCommit->changeArray[i].fileName);
      }
    }

    printf("\n");
    printf("    Tracked files (%d):\n", currentCommit->numFiles);
    for(int i = 0; i < currentCommit->numFiles; i++) {
      printf("    [%10d] %s\n", currentCommit->filesArr[i].fileHash,
                                currentCommit->filesArr[i].fileName);
    }

    return;
}

int svc_branch(void *helper, char *branch_name) {
    // TODO: Implement

    struct root* root = helper;

    const char test[66] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
                        'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
                        'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                        's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2',
                        '3', '4', '5', '6', '7', '8', '9', '-', '/', '_'}; // Talk about an inneficient way to do this,
                                                                          // Jeez.

    if(branch_name == NULL) {
      return -1;
    }

    for(int i = 0; i < strlen(branch_name); i++) { // Just goes through every
                                                  // character in the name,
                                                  // and compares it with
                                                  //the valid chars in the test array.
      if(strchr(test, branch_name[i])) {
        continue;
      } else {
//        printf("Homeboy this ain't valid\n");
        return -1;
      }
    }

    if(uncommitedChanges(helper) == 1) {
      return -3;
    }

    for(int i = 0; i < root->numBranches; i++) {
      if(strcmp(root->branchesArr[i].branchName, branch_name) == 0) { // this has probably been the single most typed line tbh
        return -2;
      }
    }


    struct branch* curBranchCopy = malloc(sizeof(struct branch));

    memcpy(curBranchCopy, root->currentBranch, sizeof(struct branch));

    //Sadly, we need to deep copy every portion of the previous branch into this new branch.
    //This results in this whopper for loop.

    root->numBranches++;
    root->branchesArr = realloc(root->branchesArr, sizeof(struct branch) * root->numBranches);

    int nI = root->numBranches - 1; // nI stands for newIndex, makes this code a LOT more readable.

    root->branchesArr[nI].branchName = strdup(branch_name);
    root->branchesArr[nI].numTracked = curBranchCopy->numTracked;
    root->branchesArr[nI].numCommits = curBranchCopy->numCommits;
    root->branchesArr[nI].trackedFiles = malloc(sizeof(struct fileNode) * curBranchCopy->numTracked);
    root->branchesArr[nI].commitsArr = malloc(sizeof(struct commit) * curBranchCopy->numCommits);

    for(int i = 0; i < curBranchCopy->numTracked; i++) {

      root->branchesArr[nI].trackedFiles[i].fileName =
              strdup(curBranchCopy->trackedFiles[i].fileName);

      root->branchesArr[nI].trackedFiles[i].fileContents =
              strdup(curBranchCopy->trackedFiles[i].fileContents);

      root->branchesArr[nI].trackedFiles[i].fileHash =
              curBranchCopy->trackedFiles[i].fileHash;

      root->branchesArr[nI].trackedFiles[i].fileContentSize =
              curBranchCopy->trackedFiles[i].fileContentSize;

    }

    for(int i = 0; i < curBranchCopy->numCommits; i++) {

      root->branchesArr[nI].commitsArr[i].commitID =
              strdup(curBranchCopy->commitsArr[i].commitID);

      root->branchesArr[nI].commitsArr[i].message =
              strdup(curBranchCopy->commitsArr[i].message);

      root->branchesArr[nI].commitsArr[i].inBranch =
              strdup(curBranchCopy->commitsArr[i].inBranch);

      root->branchesArr[nI].commitsArr[i].changeCount =
              curBranchCopy->commitsArr[i].changeCount;

      root->branchesArr[nI].commitsArr[i].numFiles =
              curBranchCopy->commitsArr[i].numFiles;

      root->branchesArr[nI].commitsArr[i].HEAD = 0;

      root->branchesArr[nI].commitsArr[i].filesArr =
              malloc(sizeof(struct fileNode) * curBranchCopy->commitsArr[i].numFiles);


      for(int j = 0; j < curBranchCopy->commitsArr[i].numFiles; j++) {

        root->branchesArr[nI].commitsArr[i].filesArr[j].fileName =
              strdup(curBranchCopy->commitsArr[i].filesArr[j].fileName);

        root->branchesArr[nI].commitsArr[i].filesArr[j].fileContents =
              strdup(curBranchCopy->commitsArr[i].filesArr[j].fileContents);

        root->branchesArr[nI].commitsArr[i].filesArr[j].fileContentSize =
              curBranchCopy->commitsArr[i].filesArr[j].fileContentSize;

        root->branchesArr[nI].commitsArr[i].filesArr[j].fileHash =
              curBranchCopy->commitsArr[i].filesArr[j].fileHash;


      }
      root->branchesArr[nI].commitsArr[i].changeArray =
        malloc(sizeof(struct changeFileNode) * curBranchCopy->commitsArr[i].changeCount);

      for(int j = 0; j < curBranchCopy->commitsArr[i].changeCount; j++) {

        root->branchesArr[nI].commitsArr[i].changeArray[j].fileName =
              strdup(curBranchCopy->commitsArr[i].changeArray[j].fileName);

        root->branchesArr[nI].commitsArr[i].changeArray[j].changeType =
              curBranchCopy->commitsArr[i].changeArray[j].changeType;

      }

    }

    for(int i = 0; i < root->numBranches; i++) { // As reallocing the branches array makes it
                                                // bigger than the size of memory, it actually
                                                //frees what the root->currentBranch is pointing too.
                                                //This makes it still point to that branch, Just
                                                // at the new memory location.
      if(strcmp(root->branchesArr[i].branchName, curBranchCopy->branchName) == 0) {
        root->currentBranch = &root->branchesArr[i];
      }
    }

    free(curBranchCopy);

    return 0;
}

int svc_checkout(void *helper, char *branch_name) {
    // TODO: Implement

    struct root* root = helper;

    if(branch_name == NULL) {
      return -1;
    }

    int foundBranch = 0;
    for(int i = 0; i < root->numBranches; i++) {
      if(strcmp(root->branchesArr[i].branchName, branch_name) == 0) {
        foundBranch = 1;
      }
    }

    if(foundBranch == 0) {
      return -1;
    }


    if(uncommitedChanges(helper) == 1) {
      return -2;
    }


    for(int i = 0; i < root->numBranches; i++) {
      if(strcmp(root->branchesArr[i].branchName, branch_name) == 0) {
        root->currentBranch = &root->branchesArr[i];
      }
    }

    return 0;
}

char **list_branches(void *helper, int *n_branches) {
    // TODO: Implement

    struct root* root = helper;

    if(n_branches == NULL) {
      return NULL;
    }

    char** returnArr = malloc(sizeof(char*) * root->numBranches);

    for(int i = 0; i < root->numBranches; i++) {
      returnArr[i] = root->branchesArr[i].branchName;
    }

    for(int i = 0; i < root->numBranches; i++) {
      printf("%s\n", root->branchesArr[i].branchName);
    }

    *n_branches = root->numBranches;
    return returnArr;
}

int svc_add(void *helper, char *file_name) {
    // TODO: Implement


    struct root* root = helper;

    if(file_name == NULL) {
      return -1; //Return -1 if file name is null, as per specs
    }

    if(!fopen(file_name, "r")) {
      return -3;
    }
    for(int i = 0; i < root->currentBranch->numTracked; i++) {
      if(root->currentBranch->trackedFiles[i].fileHash == hash_file(helper, file_name)) {
        return -2;
      }
    }


    if(root->currentBranch->numTracked > 0) {

      root->currentBranch->trackedFiles = realloc(root->currentBranch->trackedFiles,
                    sizeof(struct fileNode) * (root->currentBranch->numTracked + 1));

    } else {

      root->currentBranch->trackedFiles = malloc(sizeof(struct fileNode));

    }

    root->currentBranch->numTracked++;

    struct fileNode newFile;
    newFile.fileName = file_name;
    newFile.fileHash = hash_file(helper, file_name);

    /* I really, really didn't like the idea of making physical directories.
    For some reason it just didn't really click with me, so instead I decided
    to map all the contents of files directly into memory. I understnad it's not
    the most efficient or best way, but just in terms of making progress with this
    assignment, I decided to try and do it my way.*/

    struct stat sb;

    int filenumber = fileno(fopen(file_name, "rb"));

    fstat(filenumber, &sb);

    char *file_in_memory = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, filenumber, 0); // mapping the file to memory.

    newFile.fileContentSize = sb.st_size;

    int nI = root->currentBranch->numTracked - 1;

    root->currentBranch->trackedFiles[nI].fileName = malloc(strlen(file_name) + 1);


    strcpy(root->currentBranch->trackedFiles[nI].fileName, newFile.fileName);
    root->currentBranch->trackedFiles[nI].fileContentSize = newFile.fileContentSize;
    root->currentBranch->trackedFiles[nI].fileContents = strdup(file_in_memory);
    root->currentBranch->trackedFiles[nI].fileHash = newFile.fileHash;


    return root->currentBranch->trackedFiles[nI].fileHash;

}

int svc_rm(void *helper, char *file_name) {
    // TODO: Implement
    struct root* root = helper;

    if(file_name == NULL) {
      return -1;
    }

    int inArray = 0;
    for(int i = 0; i < root->currentBranch->numTracked; i++) {
      if(strcmp(root->currentBranch->trackedFiles[i].fileName, file_name) == 0) {
        inArray = 1;
      }
    }
    if(inArray == 0) {
        return -2;
    }

    int deletedFileHash = 0;
    deletedFileHash = hash_file(helper, file_name);

    struct fileNode* tempArray = malloc(sizeof(struct fileNode)
                        * root->currentBranch->numTracked - 1);

    int tempCount = 0;

    for(int i = 0; i < root->currentBranch->numTracked; i++) {
      if(strcmp(root->currentBranch->trackedFiles[i].fileName, file_name)) {

        tempArray[tempCount].fileName =
                strdup(root->currentBranch->trackedFiles[i].fileName);

        tempArray[tempCount].fileContents =
                strdup(root->currentBranch->trackedFiles[i].fileContents);

        tempArray[tempCount].fileHash =
                root->currentBranch->trackedFiles[i].fileHash;

        tempArray[tempCount].fileContentSize =
                root->currentBranch->trackedFiles[i].fileContentSize;

        tempCount++;

      }
    }

    for(int i = 0; i < root->currentBranch->numTracked; i++) {
      free(root->currentBranch->trackedFiles[i].fileName);
      free(root->currentBranch->trackedFiles[i].fileContents);
    }

    root->currentBranch->numTracked--;
    root->currentBranch->trackedFiles = realloc(root->currentBranch->trackedFiles,
                      sizeof(struct fileNode) * root->currentBranch->numTracked);

    for(int i = 0; i < tempCount; i++) { // Copies all the data from the temp array back into tracked.
      root->currentBranch->trackedFiles[i].fileName = strdup(tempArray[i].fileName);
      root->currentBranch->trackedFiles[i].fileContents = strdup(tempArray[i].fileContents);
      root->currentBranch->trackedFiles[i].fileContentSize = tempArray[i].fileContentSize;
      root->currentBranch->trackedFiles[i].fileHash = tempArray[i].fileHash;
    }


    for(int i = 0; i < tempCount; i++) {
      free(tempArray[i].fileName);
      free(tempArray[i].fileContents);
    }

    free(tempArray);

    return deletedFileHash;
}

int svc_reset(void *helper, char *commit_id) {
    // TODO: Implement

    struct root* root = helper;

    int commitFound = 0;

    if(commit_id == NULL) {
      return -1;
    }

    for(int i = 0; i < root->currentBranch->numCommits; i++) {
      if(strcmp(root->currentBranch->commitsArr[i].commitID, commit_id) == 0) {
        commitFound = 1;
        break;
      }
    }

    if(commitFound == 0) {
      return -2;
    }

    //Didnt really get around to finishing this off, just got the error cases out of the way. :(

    return 0;
}

char *svc_merge(void *helper, char *branch_name, struct resolution *resolutions, int n_resolutions) {
    // TODO: Implement

    struct root* root = helper;

    if(branch_name == NULL) {
      printf("Invalid branch name\n");
      return NULL;
    }

    int branchFound = 0;
    for(int i = 0; i < root->numBranches; i++) {
      if(strcmp(root->branchesArr[i].branchName, branch_name) == 0) {
        branchFound = 1;
        break;
      }
    }

    if(branchFound == 0) {
      printf("Branch not found\n");
      return NULL;
    }

    if(strcmp(branch_name, root->currentBranch->branchName) == 0) {
      printf("Cannot merge a branch with itself\n");
      return NULL;
    }

    if(uncommitedChanges(helper) == 1) {
      printf("Changes must be committed\n");
      return NULL;
    }

    return NULL;
}
