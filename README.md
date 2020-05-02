# SoalShiftSISOP20_modul4_F01

### [Source code : ](ssfs.c)

## Soal no.1
Enkripsi versi 1:
- Jika sebuah direktori dibuat dengan awalan “encv1_”, maka direktori tersebut akan menjadi direktori terenkripsi menggunakan metode enkripsi v1.
- Jika sebuah direktori di-rename dengan awalan “encv1_”, maka direktori tersebut akan menjadi direktori terenkripsi menggunakan metode enkripsi v1.
- Apabila sebuah direktori terenkripsi di-rename menjadi tidak terenkripsi, maka isi adirektori tersebut akan terdekrip.
- Setiap pembuatan direktori terenkripsi baru (mkdir ataupun rename) akan tercatat ke sebuah database/log berupa file.
- Semua file yang berada dalam direktori ter enkripsi menggunakan caesar cipher dengan key.

````
"9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO"
````

Misal kan ada file bernama “kelincilucu.jpg” dalam directory FOTO_PENTING, dan key yang dipakai adalah 10
`"encv1_rahasia/FOTO_PENTING/kelincilucu.jpg"` => `"encv1_rahasia/ULlL@u]AlZA(/g7D.|_.Da_a.jpg"`
**Note** :
Dalam penamaan file `'/'` diabaikan, dan ekstensi tidak perlu di encrypt.
Metode enkripsi pada suatu direktori juga berlaku kedalam direktori lainnya yang ada didalamnya.

### PENJELASAN
- Pada soal no 1 kita diminta untuk mengubah keseluruhan path dan nama file menjadi nama samaran dengan menggunakan metode enkripsi caesar cipher. Sehingga kita perlu membuat 2 fungsi, yaitu `encryptPath` untuk melakukan enkripsi dan `decryptPath` untuk mendekripnya.

- Dekripsi perlu dilakukan untuk mengetahui *`path`* sebenarnya dari file yang ada dalam folder fuse

```c
char keyword[100] = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO9(ku@AW1[L";

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

```

### SCREENSHOT HASIL
- Program dijalankan

![jalan](/screenshot/1.png)

- isi folder sebenarnya dan folder fuse

![isi folder fuse](/screenshot/2.png)

- isi folder dengan nama berawalan `"encv1_"`

![isi folder enc](/screenshot/3.png)

![isi folder enc](/screenshot/4.png)

![isi folder enc](/screenshot/5.png)

- isi folder setelah diubah nama menjadi tidak berawalan`"encv1_"`
![isi folder enc](/screenshot/6.png)

![isi folder enc](/screenshot/7.png)

![isi folder enc](/screenshot/8.png)

![isi folder enc](/screenshot/9.png)

## Soal no. 4
- Pada soal no. 4 kita diminta untuk membuat log system yang mencatat command apa saja yang dijalankan pada folder fuse.
- semua command yang dijalankan dicatat pada file `/home/[user]/fs.log`.
- log dibagi menjadi 2 level, **INFO** dan **WARNING**.
- Untuk log level **WARNING**, merupakan pencatatan log untuk syscall `rmdir` dan `unlink`. Sisanya **INFO**
- Format logging:

```
[LEVEL]::[yy][mm][dd]-[HH]:[MM]:[SS]::[CMD]::[DESC ...]
```

### PENJELASAN
- Pada soal ini kita cukup membuat fungsi penulisan ke file log dengan memanfaatkan waktu dipanggilnya command pada folder fuse.
- fungsi ini dipanggil setiap command dijalankan di folder fuse

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

### SCREENSHOT HASIL
- Program dijalankan

![jalan](/screenshot/1.png)

- command dipanggil di terminal lainnya

![isi folder fuse](/screenshot/10.png)

- isi file `fs.log`

![isi folder enc](/screenshot/11.png)

![isi folder enc](/screenshot/12.png)

![isi folder enc](/screenshot/13.png)

![isi folder enc](/screenshot/14.png)

![isi folder enc](/screenshot/15.png)
