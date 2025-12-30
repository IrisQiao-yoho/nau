#include<unistd.h> 
#include<signal.h> 
#include<sys/param.h> 
#include<sys/types.h> 
#include<sys/stat.h>
#include<stdio.h>
#include<unistd.h>
#include<time.h>
#include<stdlib.h>
#include<string.h>
void init_daemon(void) 
 { 
 int pid; 
 int i;

if(pid=fork()) 
 exit(0);
else if(pid<0)
 exit(1);

setsid();
if(pid=fork())
 exit(0);
else if(pid<0)
 exit(1);

for(i=0;i< NOFILE;++i)
 {
  close(i); 
 }
 chdir("/");
 umask(0);
 } 

int check()
{
 FILE* stream;
 int i=0;
 char cmd[]="ps aux | grep gre |grep -v grep |grep -v sh|grep -v daemon|awk -F \' \' \'{print $2}\'";
 char line[1024];
 stream=popen(cmd,"r");
 memset(line,0,sizeof(line));
 fgets(line,sizeof(line),stream);
 pclose(stream);
 i=atoi(line);
 return i;
}
int main() 
{ 
 FILE *fp; 
 time_t t; 
 int i=0;
 char cmd[]="/home/nau/gre_admin >/dev/null &";
 init_daemon();
while(1)
{ 
 sleep(5);
 i=check();
 if(i==0)
 {
  system(cmd);

} 
} 
} 



