#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

char *dirpath = "/home/kaori02/Documents";
char keyword[100] = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO9(ku@AW1[L";

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

void encryptPath(char *src) 
{
  int enc_len = strlen(src);

  for (int i = strlen(src); i >= 0; i--) 
  {
    if(src[i] == '/') break;

    if(src[i] == '.')
    {
      enc_len = i - 1;
      break;
    }
  }

  int enc_begin = 0;
  for (int i = 1; i < enc_len; i++)
  {
    if (src[i] == '/') enc_begin = i;
  }

  for (int i = enc_begin; i <= enc_len; i++) 
  {
    if(src[i] == '/') continue;

    int keyword_index = 0;
    while(1)
    {
      if(src[i] == keyword[keyword_index])
      {
        src[i] = keyword[keyword_index + 10];
        break;
      }
      keyword_index++;
    }
  }
  printf("enc : %s\n", src);
}

void decryptPath(char *src) 
{
  int dec_len = strlen(src); 
  int dec_begin = 0;
    
  for (int i = 1; i < dec_len; i++)
  {
    if(src[i] == '/' || src[i + 1] == '\0') 
    {
      dec_begin = i + 1;
      break;
    }
  }

  for (int i = strlen(src); i >= 0; i--)
  {
    if (src[i] == '.') 
    {
      dec_len = i - 1;
      break;
    }
  }

  for (int i = dec_begin; i <= dec_len; i++) 
  {
    if(src[i] == '/') continue;

    int keyword_index = strlen(keyword) - 1;
    while(1)
    {
      if(src[i] == keyword[keyword_index])
      {
        src[i] = keyword[keyword_index - 10];
        break;
      }
      keyword_index--;
    }
  }
  printf("dec : %s\n", src);
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
  int res;
  char temp[1000];

  strcpy(temp, path);

  if(strncmp(path, "/encv1_", 6) == 0)
    decryptPath(temp);

  char fpath[1000];
  sprintf(fpath, "%s%s", dirpath, temp);

  res = lstat(fpath, stbuf);

  if(res == -1)
    return -errno;

  return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];
  char tmp[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else
  {
    strcpy(tmp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptPath(tmp);

    sprintf(fpath, "%s%s", dirpath, tmp);
  }

  int res = 0;

  DIR *dp;
  struct dirent *de;

  (void)offset;
  (void)fi;

  logWrite(0, "LS", fpath);
  dp = opendir(fpath);

  if(dp == NULL)
    return -errno;

  while ((de = readdir(dp)) != NULL)
  {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
      continue;

    struct stat st;
    memset(&st, 0, sizeof(st));
    st.st_ino = de->d_ino;
    st.st_mode = de->d_type << 12;

    char temp[1000];
    strcpy(temp, de->d_name);

    if (strncmp(path, "/encv1_", 6) == 0)
      encryptPath(temp);

    res = (filler(buf, temp, &st, 0));

    if (res != 0)
      break;
  }

  closedir(dp);

  return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptPath(temp);

    sprintf(fpath, "%s%s", dirpath, temp);
  }

  int res = 0;
  int fd = 0;

  (void)fi;

  fd = open(fpath, O_RDONLY);

  if (fd == -1)
    return -errno;

  res = pread(fd, buf, size, offset);

  if (res == -1) 
    res = -errno;

  close(fd);

  return res;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
  int res;

  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptPath(temp);

    sprintf(fpath, "%s%s", dirpath, temp);
  }

  if (S_ISREG(mode)) 
  {
    res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
    if (res >= 0)
      res = close(res);
  } 
  else if (S_ISFIFO(mode))
    res = mkfifo(fpath, mode);
  else
    res = mknod(fpath, mode, rdev);

  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
  char fpath[1000];

  if (!strcmp(path, "/"))
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else
  {
    sprintf(fpath, "%s%s", dirpath, path);
  }

  (void)fi;

  int res;
  logWrite(0, "TOUCH", fpath);
  res = creat(fpath, mode);
  if (res == -1)
    return -errno;

  close(res);

  return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
  int res;

  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptPath(temp);

    sprintf(fpath, "%s%s", dirpath, temp);
  }

  logWrite(0, "MKDIR", fpath);
  res = mkdir(fpath, mode);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_unlink(const char *path)
{
  int res;

  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptPath(temp);

    sprintf(fpath, "%s%s", dirpath, temp);
  }

  logWrite(1, "RM", fpath);
  res = unlink(fpath);
  if (res == -1)
    return -errno;
  
  return 0;
}

static int xmp_rmdir(const char *path)
{
  int res;

  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptPath(temp);

    sprintf(fpath, "%s%s", dirpath, temp);
  }

  logWrite(1, "RMDIR", fpath);
  res = rmdir(fpath);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
  int res;
  char fpath[1000];

  if (!strcmp(from, "/"))
  {
    from = dirpath;
    sprintf(fpath, "%s", from);
  }
  else
  {
    sprintf(fpath, "%s%s", dirpath, from);
  }

  char topath[1000];

  if (!strcmp(to, "/"))
  {
    to = dirpath;
    sprintf(topath, "%s", to);
  }
  else
  {
    sprintf(topath, "%s%s", dirpath, to);
  }
  char desc[1000];
  sprintf(desc, "%s to %s", fpath, topath);
  logWrite(0, "SYMLINK", desc);
  res = symlink(fpath, topath);
  
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_rename(const char *from, const char *to)
{
  int res;

  char ffrom[1000];
  if (strcmp(from, "/") == 0)
  {
    from = dirpath;
    sprintf(ffrom, "%s", from);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, from);

    if(strncmp(from, "/encv1_", 6) == 0)
      decryptPath(temp);

    sprintf(ffrom, "%s%s", dirpath, temp);
  }

  char fto[1000];
  if (strcmp(to, "/") == 0)
  {
    to = dirpath;
    sprintf(fto, "%s", to);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, to);

    if(strncmp(to, "/encv1_", 6) == 0)
      decryptPath(temp);

    sprintf(fto, "%s%s", dirpath, temp);
  }

  char desc[1000];
  sprintf(desc, "%s to %s", ffrom, fto);
  logWrite(0, "RENAME", desc);
  res = rename(ffrom, fto);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
  int res;
  char fpath[1000];

  if (!strcmp(path, "/"))
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else
  {
    sprintf(fpath, "%s%s", dirpath, path);
  }
  logWrite(0, "CHMOD", fpath);
  res = chmod(fpath, mode);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
  int res;
  char fpath[1000];

  if (!strcmp(path, "/"))
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else
  {
    sprintf(fpath, "%s%s", dirpath, path);
  }
  logWrite(0, "CHOWN", fpath);
  res = lchown(fpath, uid, gid);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
  int res;

  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptPath(temp);

    sprintf(fpath, "%s%s", dirpath, temp);
  }

  res = truncate(fpath, size);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
  int res;

  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptPath(temp);

    sprintf(fpath, "%s%s", dirpath, temp);
  }

  logWrite(0, "CAT", fpath);
  res = open(fpath, fi->flags);
  if (res == -1)
    return -errno;

  close(res);
  return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptPath(temp);

    sprintf(fpath, "%s%s", dirpath, temp);
  }

  int fd;
  int res;

  (void) fi;
  fd = open(fpath, O_WRONLY);
  if (fd == -1)
    return -errno;

  res = pwrite(fd, buf, size, offset);
  if (res == -1)
    res = -errno;

  close(fd);

  return res;
}

static struct fuse_operations xmp_oper = {
  .getattr  = xmp_getattr,
  .readdir  = xmp_readdir,
  .mknod    = xmp_mknod,
  .mkdir    = xmp_mkdir,
  .symlink  = xmp_symlink,
  .unlink   = xmp_unlink,
  .rmdir    = xmp_rmdir,
  .rename   = xmp_rename,
  .chmod    = xmp_chmod,
  .chown    = xmp_chown,
  .truncate = xmp_truncate,
  .open     = xmp_open,
  .read     = xmp_read,
  .write    = xmp_write,
  .create   = xmp_create,
};

int main(int argc, char *argv[])
{
  umask(0);
  return fuse_main(argc, argv, &xmp_oper, NULL);
}