/* This file is part of msolve.
 *
 * msolve is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * msolve is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with msolve.  If not, see <https://www.gnu.org/licenses/>
 *
 * Authors:
 * Mickael Gastineau */

#if (defined __MINGW64__)

extern void* __mingw_aligned_offset_malloc(size_t size, size_t alignment, size_t offset);
/* emulation of the POSIX function posix_memalign */
int posix_memalign(void** memptr, size_t alignment, size_t size)
{
  *memptr = __mingw_aligned_offset_malloc(size, alignment, 0);
  if (*memptr == NULL)
    return -1;
  else
    return 0;
}

/* emulation of the POSIX function getdelim using fgetc */
ssize_t getdelim(char** restrict linep, size_t* restrict linecapp, int delimiter, FILE* restrict stream)
{
  char* buf;
  int c;
  ssize_t ipos;
  size_t upos;
  size_t chunk_grow = 64;

  if (linep == NULL || linecapp == NULL)
  {
    errno = EINVAL;
    return -1;
  }
  if (stream == NULL)
  {
    errno = EBADF;
    return -1;
  }

  if (feof(stream))
  {
    return -1;
  }

  if (*linep == NULL || *linecapp < 2)
  {
    char* buf = (char*)realloc(*linep, chunk_grow);
    if (buf == NULL)
    {
      return -1;
    }
    *linep = buf;
    *linecapp = chunk_grow;
  }


  ipos = 0;
  upos = 0;
  while ((c = fgetc(stream)) != EOF)
  {
    if (upos + 2 == *linecapp)
    {
      ssize_t bufsize = *linecapp;
      if (chunk_grow < 2048) chunk_grow *= 2;
      bufsize += chunk_grow;
      if (bufsize >= SSIZE_MAX)
      {
        errno = EOVERFLOW;
        return -1;
      }
      buf = realloc(*linep, bufsize);
      if (buf == NULL)
      {
        return -1;
      }
      *linep = buf;
      *linecapp = bufsize;
    }

    (*linep)[ipos++] = (char)c;
    upos++;
    if (c == delimiter)
    {
      (*linep)[ipos++] = '\0';
      upos++;
      return ipos;
    }
  }
  if (c != EOF && ferror(stream))
  {
    return -1;
  }

  if (c == EOF && ipos == 0)
  {
    return -1;
  }

  (*linep)[ipos++] = '\0';
  return ipos;
}

/* emulation of the POSIX function getline using getdelim */
ssize_t getline(char** linep, size_t* linecapp, FILE* stream)
{
  int delimiter = '\n';
  return getdelim(linep, linecapp, delimiter, stream);
}
#endif