#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <pwd.h>
#include <proc/readproc.h>
#include <proc/sysinfo.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

typedef struct user_entry_t user_entry_t;

struct user_entry_t {
  uid_t uid;
  time_t max_time;
  user_entry_t* next;
};

static user_entry_t* db = NULL;

uid_t get_uid(char* name){
  struct passwd* pwd = getpwnam(name);
  if (pwd){
    return pwd->pw_uid;
  } else {
    return atoi(name);
  }
}

user_entry_t* get_user_entry(uid_t* uid){
  user_entry_t* i = db;
  while (i){
    if (i->uid == uid){
      return i;
    }
    i = i->next;
  }
  return NULL;
}

jiff get_current_jiffies(){
  double uptime_secs, idle_secs;
  uptime(&uptime_secs, &idle_secs);
  return uptime_secs * Hertz;
}

void do_kills(){
  jiff cur_time = get_current_jiffies();
  struct proc_t** proc = readproctab(PROC_FILLSTAT | PROC_FILLSTATUS);
  

  while (*proc){
    pid_t pid = (*proc)->tgid;
    uid_t uid = (*proc)->ruid;
    jiff time = (*proc)->start_time;
    user_entry_t* ent = get_user_entry(uid);

    if (ent){
      if (ent->max_time < cur_time - time){
        kill(pid, SIGKILL);
      }
    }

    proc++;
  }
}

void put_user_entry(char* user, char* time){
  user_entry_t* new = malloc(sizeof(user_entry_t));
  new->uid = get_uid(user);
  new->max_time = atoi(time) * Hertz;
  new->next = db;
  db = new;
}

void print_usage(){
  fprintf(stderr, "usage: killlong user max_time [user max_time ...]\n");
  exit(1);
}

int main(int argc, char**argv){
  if (argc == 1){
    print_usage();
  }
  argc--;
  argv++;
  while (argc){
    char* user;
    char* time;

    user = *argv;
    argv++;
    argc--;
    if (argc == 0){
      print_usage();
    }

    time = *argv;
    argv++;
    argc--;
    put_user_entry(user, time);
  }

  do_kills();
  exit(0);
}
