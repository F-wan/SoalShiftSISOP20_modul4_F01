# SoalShiftSISOP20_modul4_F01

### [Source code : ](ssfs.c)

## Soal no. 4
```c
void logWrite(int l, char *cmd, char *desc)
{
  char line[1024];
  memset(line, '\0', strlen(line));
  
  char level[10];
  if(l) strcpy(level, "WARNING");
  else strcpy(level, "INFO");

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  sprintf(line, "[%s] :: %02d%02d%02d-%02d:%02d:%02d::%s::%s\n", level, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, cmd, desc);
  
  FILE *file;
  file = fopen("/home/kaori02/fs.log", "a+");
  fputs(line, file);
  puts(line);
  fclose(file);
}
```