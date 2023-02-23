#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<assert.h>
#include<math.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<errno.h>


typedef struct statics{
  int n_dir;
  int n_file;
}Statics;

int
count(char *path, char *key)
{
  int sum = 0, len = strlen(path);
  for(int i=0; i<len; i++){
    if(path[i] == key[0])
      sum++;
  }
  return sum;
}

void
mp0(char *path, char *key, Statics *ans)
{
  char buf[64], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "mp0: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "mp0: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    ans -> n_file += 1;
    printf("%s %d\n", path, count(path, key));
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }

    ans -> n_dir += 1;
    printf("%s %d\n", path, count(path, key));

    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }

      mp0(buf, key, ans);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    printf("input format wrong\n");
    exit(0);
  }

  int test_fd;
  struct stat st;

  if((test_fd = open(argv[1], 0)) < 0){
    fprintf(2, "%s [error opening dir]\n", argv[1]);
    exit(1);
  }

  if(fstat(test_fd, &st) < 0){
    fprintf(2, "mp0: cannot stat %s\n", argv[1]);
    close(test_fd);
    exit(1);
  }

  if(st.type != T_DIR){
    fprintf(2, "%s [error opening dir]\n", argv[1]);
    close(test_fd);
    exit(1);
  }
  close(test_fd);

  int pid;
  int fd[2];
  Statics ans;
  ans = {0, 0};

  pipe(fd);

  pid = fork();
  if(pid == 0){
    close(fd[0]);

    mp0(argv[1], argv[2], &ans); //root_directory, key

    write(fd[1], &ans, sizeof(Statics));

    close(fd[1]);
  }
  else{
    close(fd[1]);

    read(fd[0], &ans, sizeof(Statics));
    printf("\n%d directories, %d files\n", ans.n_dir-1, ans.n_file);

    close(fd[0]);
  }

  

  exit(0);
}
