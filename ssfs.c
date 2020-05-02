#define FUSE_USE_VERSION 28
#define HAVE_SETXATTR

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

char *dirpath = "/home/kaori02/Documents";
char keyword[] = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";
int key = 10;

void strip_ext(char *fname)
{
  char *end = fname + strlen(fname);

  while (end > fname && *end != '.') end--;

  if (end > fname) *end = '\0';
}

void encrypt(char *path)
{
  for(int i = 0; i < strlen(path); i++)
  {
    int j = 0;
    while(1)
    {
      if(path[i] == keyword[j])
      {
        int temp = j + key;
        if(temp > strlen(keyword) - 1)
        {
          int sisaKeAkhir = strlen(keyword) - 1 - j;
          temp = key - sisaKeAkhir - 1;
        }
        path[i] = keyword[temp];
        break;
      }
      j++;
    }
  }
}

void decrypt(char *path)
{
  for(int i = 0; i < strlen(path); i++)
  {
    int j = strlen(keyword);
    while(1)
    {
      if(path[i] == keyword[j])
      {
        int temp = j - key;
        if(temp < 0)
        {
          temp = strlen(keyword) - (key - j);
        }
        path[i] = keyword[temp];
        break;
      }
      j--;
    }
  }
}

void encryptPath(char *path)
{
	char str[1000];
  strcpy(str, path);
  
  char *extension = strrchr(str, '.');

  if(!extension || extension == str) return;
  
  char tipe[100];
  strcpy(tipe, extension);

  strip_ext(str);
  printf("ext funct = %s\n", str);
  
  char* token = strtok(str, "/");
  sprintf(path, "%s", token);
  
  token = strtok(NULL, "/"); 
  
  while (token != NULL) 
  { 
    encrypt(token);
    strcat(path, "/");
    strcat(path, token);
    token = strtok(NULL, "/"); 
  }
  strcat(path, tipe);
}

void decryptPath(char *path)
{
	char str[1000];
  strcpy(str, path);

  char *extension = strrchr(str, '.');

  if(!extension || extension == str) return;
  
  char tipe[100];
  strcpy(tipe, extension);

  strip_ext(str);
  printf("\next funct = %s\n", str);
  
  char* token = strtok(str, "/");
  sprintf(path, "%s", token);
  
  token = strtok(NULL, "/"); 
  while (token != NULL) 
  { 
    decrypt(token);
    strcat(path, "/");
    strcat(path, token);
    token = strtok(NULL, "/"); 
  }
  strcat(path, tipe);
}

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

static int xmp_getattr(const char *path, struct stat *stbuf)
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

  res = lstat(fpath, stbuf);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_access(const char *path, int mask)
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
  res = access(fpath, mask);

  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
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

  res = readlink(fpath, buf, size - 1);
  if (res == -1)
    return -errno;

  buf[res] = '\0';
  return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];

  if (!strcmp(path, "/"))
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else
  {
    //enkrip nama folder
    sprintf(fpath, "%s%s", dirpath, path);
  }

  DIR *dp;
  struct dirent *de;

  (void)offset;
  (void)fi;

  logWrite(0, "LS", fpath);
  dp = opendir(fpath);
  if (dp == NULL)
    return -errno;

  while ((de = readdir(dp)) != NULL)
  {
    struct stat st;
    memset(&st, 0, sizeof(st));
    st.st_ino = de->d_ino;
    st.st_mode = de->d_type << 12;
    //d_name dekrip
    if (filler(buf, de->d_name, &st, 0))
      break;
  }

  closedir(dp);
  return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
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
  int res;

  /* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
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

static int xmp_mkdir(const char *path, mode_t mode)
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

  int res;
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

  if (!strcmp(path, "/"))
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else
  {
    sprintf(fpath, "%s%s", dirpath, path);
  }
  logWrite(1, "RM", fpath);
  res = unlink(fpath);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_rmdir(const char *path)
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
  int res;
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
  logWrite(0, "RENAME", desc);
  res = rename(fpath, topath);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_link(const char *from, const char *to)
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
  res = link(fpath, topath);
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

  if (!strcmp(path, "/"))
  {
    path = dirpath;
    sprintf(fpath, "%s", path);
  }
  else
  {
    sprintf(fpath, "%s%s", dirpath, path);
  }
  res = truncate(fpath, size);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
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
  struct timeval tv[2];

  tv[0].tv_sec = ts[0].tv_sec;
  tv[0].tv_usec = ts[0].tv_nsec / 1000;
  tv[1].tv_sec = ts[1].tv_sec;
  tv[1].tv_usec = ts[1].tv_nsec / 1000;

  res = utimes(fpath, tv);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
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
  logWrite(0, "CAT", fpath);
  res = open(fpath, fi->flags);
  if (res == -1)
    return -errno;

  close(res);
  return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
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
  int fd;
  int res;

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

static int xmp_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
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
  int fd;
  int res;

  (void)fi;
  fd = open(fpath, O_WRONLY);
  if (fd == -1)
    return -errno;

  res = pwrite(fd, buf, size, offset);
  if (res == -1)
    res = -errno;

  close(fd);
  return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
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

  res = statvfs(fpath, stbuf);
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

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
  /* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
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
  (void)fpath;
  (void)fi;
  return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
                     struct fuse_file_info *fi)
{
  /* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
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
  (void)fpath;
  (void)isdatasync;
  (void)fi;
  return 0;
}

#ifdef HAVE_SETXATTR
static int xmp_setxattr(const char *path, const char *name, const char *value,
                        size_t size, int flags)
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

  int res = lsetxattr(fpath, name, value, size, flags);
  if (res == -1)
    return -errno;
  return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
                        size_t size)
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
  int res = lgetxattr(fpath, name, value, size);
  if (res == -1)
    return -errno;
  return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
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
  int res = llistxattr(fpath, list, size);
  if (res == -1)
    return -errno;
  return res;
}

static int xmp_removexattr(const char *path, const char *name)
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
  int res = lremovexattr(fpath, name);
  if (res == -1)
    return -errno;
  return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .access = xmp_access,
    .readlink = xmp_readlink,
    .readdir = xmp_readdir,
    .mknod = xmp_mknod,
    .mkdir = xmp_mkdir,
    .symlink = xmp_symlink,
    .unlink = xmp_unlink,
    .rmdir = xmp_rmdir,
    .rename = xmp_rename,
    .link = xmp_link,
    .chmod = xmp_chmod,
    .chown = xmp_chown,
    .truncate = xmp_truncate,
    .utimens = xmp_utimens,
    .open = xmp_open,
    .read = xmp_read,
    .write = xmp_write,
    .statfs = xmp_statfs,
    .create = xmp_create,
    .release = xmp_release,
    .fsync = xmp_fsync,
#ifdef HAVE_SETXATTR
    .setxattr = xmp_setxattr,
    .getxattr = xmp_getxattr,
    .listxattr = xmp_listxattr,
    .removexattr = xmp_removexattr,
#endif
};

int main(int argc, char *argv[])
{
  umask(0);
  return fuse_main(argc, argv, &xmp_oper, NULL);
}