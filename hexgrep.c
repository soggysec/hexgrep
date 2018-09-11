#include "hexgrep.h"

/////////////////////////////////// Generic ////////////////////////////////////

size_t
parse_bufsize(
    char *string)
{
  size_t size;
  char *tail;
  size = strtoul(string, &tail, 0);
  // lazy switch abuse
  switch (tail[0])
  {
  case 'Y':
    size <<= 10;
  case 'Z':
    size <<= 10;
  case 'E':
    size <<= 10;
  case 'P':
    size <<= 10;
  case 'T':
    size <<= 10;
  case 'G':
    size <<= 10;
  case 'M':
    size <<= 10;
  case 'k':
    size <<= 10;
  default:
    break;
  }
  return size;
}

inline int
le_memcmp(
    const void *a,
    const void *b,
    size_t size)
{
  int i;
  unsigned char *aa, *bb;
  aa = (unsigned char *)a;
  bb = (unsigned char *)b;

  for (i = size - 1; i >= 0; i--)
  {
    if (aa[i] > bb[i])
    {
      return 1;
    }
    else if (aa[i] < bb[i])
    {
      return -1;
    }
  }
  return 0;
}

inline int
be_memcmp(
    const void *a,
    const void *b,
    size_t size)
{
  int i;
  unsigned char *aa, *bb;
  aa = (unsigned char *)a;
  bb = (unsigned char *)b;

  for (i = 0; i < size; i++)
  {
    if (aa[i] > bb[i])
    {
      return 1;
    }
    else if (aa[i] < bb[i])
    {
      return -1;
    }
  }
  return 0;
}

size_t
align_to_page(
    size_t len,
    int up)
{
  size_t pagesize, tmpsize;

  pagesize = getpagesize();
  tmpsize = pagesize;
  while (tmpsize < len)
  {
    tmpsize += pagesize;
  }
  return up ? tmpsize : tmpsize - pagesize;
}

///////////////////////////////////// Help /////////////////////////////////////

void print_help(
    char *name)
{
  printf(
      "Usage\n"
      "  %s [options] [search tokens]\n"
      "\n"
      "Options\n"
      "  -b\n"
      "      Convert values greater than >0xff to a big-endian sequence of bytes. The\n"
      "      default is little-endian.\n"
      "\n"
      "  -c\n"
      "      Color output.\n"
      "\n"
      "  -d\n"
      "      Print debugging information. Pass multiple times to display more information.\n"
      "\n"
      "  -f <path>\n"
      "      Search a file. The file will be mmap'd and match sequences may span the full\n"
      "      file.\n"
      "\n"
      "  -R <path>\n"
      "      Search all files in a given directory.\n"
      "\n"

      "  -h\n"
      "      Display this help message and exit.\n"
      "\n"
      "  -l <length>\n"
      "    The maximum length of a match sequence when reading a stream. The allocated\n"
      "    buffer will be three times. The size is given in bytes as an integer\n"
      "    optionally followed by k, M, G, T, P, E, Z, or Y. Yes, it really will try to\n"
      "    allocate terabytes or more if you tell it to. Default: " AS_STRING(STREAM_MATCH_LENGTH) " bytes.\n"
                                                                                                   "\n"
                                                                                                   "  -p <int>\n"
                                                                                                   "      Pad offsets with zeros up to this width. Default: " AS_STRING(DEFAULT_PADDING) ".\n"
                                                                                                                                                                                         "\n"
                                                                                                                                                                                         "  -r <range specifier>\n"
                                                                                                                                                                                         "      Limit the search range. See \"Range Specifier\" in the manual.\n"
                                                                                                                                                                                         "\n"
                                                                                                                                                                                         "See the man page for details.\n",
      name);
}

//////////////////////////////// Miscellaneous /////////////////////////////////

const char *
eq_to_str(
    equality_t eq)
{
  switch (eq)
  {
  case EQ:
    return "=";
    break;
  case GE:
    return ">=";
    break;
  case GT:
    return ">";
    break;
  case LE:
    return "<=";
    break;
  case LT:
    return "<";
    break;
  case NE:
    return "<>";
    break;
  case NA:
    return "NA";
    break;
  default:
    return "?";
    break;
  }
}

equality_t
str_to_eq(
    char *str,
    char **tail)
{
  equality_t eq;
  switch (str[0])
  {
  case '>':
    str++;
    switch (str[0])
    {
    case '=':
      str++;
      eq = GE;
      break;
      //         case '<':
      //           str ++;
      //           eq = NE;
      //           break;
    default:
      eq = GT;
      break;
    }
    break;
  case '<':
    str++;
    switch (str[0])
    {
    case '=':
      str++;
      eq = LE;
      break;
    case '>':
      str++;
      eq = NE;
      break;
    default:
      eq = LT;
      break;
    }
    break;
  case '=':
    eq = EQ;
    str++;
  default:
    eq = NA;
    break;
  }
  if (tail != NULL)
  {
    (*tail) = str;
  }
  return eq;
}

// I don't expect anyone to actually use this, but I prefer completeness.
size_t
str_to_number(
    char *str)
{
  size_t n, l, i;
  l = strlen(str);
  n = 0;

  if (OPTIONS.big_endian)
  {
    for (i = 0; i < l; i++)
    {
      n <<= 8;
      n += str[i];
    }
  }
  else
  {
    for (i = l - 1; i >= 0; i--)
    {
      n <<= 8;
      n += str[i];
    }
  }
  return n;
}

//////////////////////////////////// Tokens ////////////////////////////////////

void free_tokens(
    token_t *tokens)
{
  int i;
  for (i = 0;; i++)
  {
    if (tokens[i].match_seq == NULL)
    {
      free(tokens);
      return;
    }
    else
    {
      free(tokens[i].match_seq);
    }
  }
}

// Returns non-zero if something was printed.
int print_skip_tokens(
    token_t *token,
    int print_parens)
{
  switch (token->skip_eq)
  {
  case NA:
    return 0;
    break;
  case EQ:
    if (token->skip_start > 0)
    {
      if (print_parens)
      {
        printf("(#=%zu)", token->skip_start);
      }
      else
      {
        printf("#=%zu", token->skip_start);
      }
    }
    else
    {
      return 0;
    }
    break;
  case LE:
    if (print_parens)
    {
      printf("(");
    }
    if (token->skip_start > 0)
    {
      printf("#=%zu ", token->skip_start);
    }
    printf("#<=%zu", token->skip_count);
    if (print_parens)
    {
      printf(")");
    }
    break;
  case GE:
    if (print_parens)
    {
      printf("(#>=%zu)", token->skip_start);
    }
    else
    {
      printf("#>=%zu", token->skip_start);
    }
    break;
  case NE:
    if (print_parens)
    {
      printf("(");
    }
    if (token->skip_start > 0)
    {
      printf("#=%zu ", token->skip_start);
    }
    printf("#<>%zu", token->skip_count);
    if (print_parens)
    {
      printf(")");
    }
    break;
  default:
    fprintf(
        stderr,
        "encountered unexpected equality in parsed tokens: (%s)",
        eq_to_str(token->skip_eq));
    return 0;
    break;
  }
  return 1;
}

void print_tokens(
    token_t *tokens)
{
  int i, j;
  for (i = 0;; i++)
  {
    if (tokens[i].match_seq == NULL)
    {
      print_skip_tokens(tokens + i, 1);
      return;
    }
    else if (i > 0)
    {
      printf(" ");
    }
    printf("(");
    if (print_skip_tokens(tokens + i, 0))
    {
      printf(" ");
    }
    printf("%s0x", eq_to_str(tokens[i].match_eq));
    if (OPTIONS.big_endian)
    {
      for (j = 0; j < tokens[i].match_len; j++)
      {
        printf("%02x", tokens[i].match_seq[j]);
      }
    }
    else
    {
      for (j = tokens[i].match_len - 1; j >= 0; j--)
      {
        printf("%02x", tokens[i].match_seq[j]);
      }
    }
    printf(")");
  }
}

void print_token_metadata(
    token_t *tokens)
{
  int t, i;
  t = 0;
  do
  {
    if (t > 0)
    {
      printf("\n");
    }
    printf("token:          %d\n", t);
    //     printf("token address:  %p\n", tokens + t);
    printf("skip equality:  %s\n", eq_to_str(tokens[t].skip_eq));
    printf("skip start:     %zu\n", tokens[t].skip_start);
    printf("skip count:     %zu\n", tokens[t].skip_count);
    printf("match equality: %s\n", eq_to_str(tokens[t].match_eq));
    printf("match sequence:");
    for (i = 0; i < tokens[t].match_len; i++)
    {
      printf(" %02x", tokens[t].match_seq[i]);
    }
    printf("\n");
    printf("match offset:   %zu\n", tokens[t].match_offset);
    printf("minimum length: %zu\n", tokens[t].min_len);
    t++;
  } while (tokens[t].match_seq != NULL);
}

unsigned char *
prepare_token_bytes(
    token_t *token,
    token_t *previous_token,
    size_t n)
{
  unsigned char *ptr;
  size_t tmp;

  if (
      token->skip_eq == NA &&
      (token->match_eq == EQ || token->match_eq == NE) &&
      previous_token != NULL &&
      token->match_eq == previous_token->match_eq)
  {
    tmp = previous_token->match_len + n;
    ptr = realloc(previous_token->match_seq, tmp);
    if (ptr == NULL)
    {
      perror("failed to realloc memory for byte sequence");
      return NULL;
    }
    previous_token->match_seq = ptr;
    ptr += previous_token->match_len;
    previous_token->match_len += n;
  }
  else
  {
    ptr = malloc(n);
    if (ptr == NULL)
    {
      perror("failed to malloc memory for byte sequence");
      return NULL;
    }
    token->match_seq = ptr;
    token->match_len = n;
  }
  return ptr;
}

int parse_bytes(
    token_t *token,
    token_t *previous_token,
    size_t value)
{
  unsigned int i, n = 1;
  size_t tmp;
  unsigned char b, *ptr;

  tmp = value;
  while (tmp /= 0x100)
  {
    n++;
  }
  ptr = prepare_token_bytes(token, previous_token, n);
  if (ptr == NULL)
  {
    return 1;
  }
  for (i = 0; i < n;)
  {
    b = value % 0x100;
    value /= 0x100;
    if (OPTIONS.big_endian)
    {
      ptr[n - (++i)] = b;
    }
    else
    {
      ptr[i++] = b;
    }
  }
  return 0;
}

int parse_token(
    token_t *token,
    char *string,
    token_t *previous_token)
#define BAD_TOKEN                                                \
  fprintf(stderr, "error: invalid search token (%s)\n", string); \
  return 1
{
  int skip;
  char *head, *ptr;
  size_t n;
  equality_t eq;

  skip = 0;
  ptr = NULL;

  head = string;

  // shortcuts
  if (head[0] == '?' && head[1] == '\0')
  {
    head = "#=1";
  }
  else if (head[0] == '*' && head[1] == '\0')
  {
    head = "#>=0";
  }
  else if (head[0] == '+' && head[1] == '\0')
  {
    head = "#>=1";
  }
  switch (head[0])
  {
  case '\0':
    BAD_TOKEN;
    break;
  case '#':
    skip = 1;
    head++;
    break;
  default:
    skip = 0;
    ;
    break;
  }
  eq = str_to_eq(head, &ptr);
  if (ptr == NULL || ptr[0] == '\0')
  {
    BAD_TOKEN;
  }
  else if (eq == NA)
  {
    eq = EQ;
  }
  head = ptr;

  if (skip)
  {
    if (head[0] == '/')
    {
      head++;
      n = str_to_number(head);
    }
    else
    {
      n = strtoul(head, &ptr, 0);
      if (head == ptr)
      {
        BAD_TOKEN;
      }
    }
    switch (eq)
    {
    case LT:
      n--;
      eq = LE;
      break;
    case GT:
      n++;
      eq = GE;
      break;
    case NE:
      if (n == 0)
      {
        n = 1;
        eq = GE;
      }
      break;
    default:
      break;
    }
    switch (eq)
    {
    case EQ:
      token->skip_start += n;
      if (token->skip_eq == NA)
      {
        token->skip_eq = EQ;
      }
      break;
    case LE:
      switch (token->skip_eq)
      {
      case GE:
        break;
      case NE:
        token->skip_eq = GE;
      default:
        token->skip_eq = LE;
        token->skip_count += n;
        break;
      }
      break;
    case GE:
      token->skip_eq = GE;
      token->skip_start += n;
      break;
    case NE:
      switch (token->skip_eq)
      {
      case GE:
        break;
      case NE:
        if (n == 1 && token->skip_count == 1)
        {
          break;
        }
        else
        {
          token->skip_eq = GE;
        }
      case LE:
        if (token->skip_count != 0)
        {
          token->skip_eq = GE;
          break;
        }
      default:
        token->skip_eq = NE;
        token->skip_count += n;
        break;
      }
      break;
    default:
      break;
    }
    return 0;
  }
  else
  {
    /*
    if (token->skip_eq == NA)
    {
      token->skip_eq = EQ;
    }
    */
    token->match_eq = eq;
    if (head[0] == '/')
    {
      head++;
      n = strlen(head);
      if (n == 0)
      {
        BAD_TOKEN;
      }
      ptr = (char *)prepare_token_bytes(token, previous_token, n);
      if (ptr == NULL)
      {
        perror("failed to allocate memory for byte sequence (string)");
        return 1;
      }
      else
      {
        memcpy(ptr, head, n);
        return 0;
      }
    }
    else
    {
      n = strtoul(head, &ptr, 0);
      if (head == ptr)
      {
        BAD_TOKEN;
      }
      return parse_bytes(token, previous_token, n);
    }
  }
}

void calculate_min_lengths(
    token_t *tokens)
{
  int i;
  size_t total;

  for (i = 0; tokens[i].match_seq != NULL; i++)
  {
  }
  i--;
  total = 0;
  for (; i >= 0; i--)
  {
    total += tokens[i].skip_start + tokens[i].match_len;
    tokens[i].min_len = total;
  }
}

/////////////////////////////////// Matching ///////////////////////////////////

#define MATCH_LOOP_BODY                                      \
  limit = length - (i + tokens[t].match_len);                \
  if (tokens[t + 1].min_len > limit)                         \
  {                                                          \
    return 0;                                                \
  }                                                          \
  tokens[t].match_offset = i;                                \
  if (tokens[t + 1].match_seq == NULL)                       \
  {                                                          \
    if (length >= tokens[t + 1].skip_start)                  \
    {                                                        \
      print_match(tokens, data, global_offset, filename);    \
    }                                                        \
  }                                                          \
  else if (match_tokens(                                     \
               tokens,                                       \
               t + 1,                                        \
               data,                                         \
               tokens[t].match_offset + tokens[t].match_len, \
               local_offset_limit,                           \
               global_offset, filename))                     \
  {                                                          \
    return 1;                                                \
  }

#define MATCH(COND)                                                           \
  if (big_endian)                                                             \
  {                                                                           \
    for (i = start; i <= end; i++)                                            \
    {                                                                         \
      if (be_memcmp(data + i, tokens[t].match_seq, tokens[t].match_len) COND) \
      {                                                                       \
        MATCH_LOOP_BODY                                                       \
      }                                                                       \
    }                                                                         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    for (i = start; i <= end; i++)                                            \
    {                                                                         \
      if (le_memcmp(data + i, tokens[t].match_seq, tokens[t].match_len) COND) \
      {                                                                       \
        MATCH_LOOP_BODY                                                       \
      }                                                                       \
    }                                                                         \
  }

#define MATCH_SWITCH                                            \
  switch (tokens[t].match_eq)                                   \
  {                                                             \
  case EQ:                                                      \
    MATCH(== 0)                                                 \
    big_endian = 1;                                             \
    break;                                                      \
  case NE:                                                      \
    MATCH(!= 0)                                                 \
    big_endian = 1;                                             \
    break;                                                      \
  case GT:                                                      \
    MATCH(> 0)                                                  \
    break;                                                      \
  case GE:                                                      \
    MATCH(>= 0)                                                 \
    break;                                                      \
  case LT:                                                      \
    MATCH(< 0)                                                  \
    break;                                                      \
  case LE:                                                      \
    MATCH(<= 0)                                                 \
    break;                                                      \
  default:                                                      \
    fprintf(                                                    \
        stderr,                                                 \
        "error: unhandled equality type in match token (%s)\n", \
        eq_to_str(tokens[t].match_eq));                         \
    return 1;                                                   \
  }

inline int
match_tokens(
    token_t *tokens,
    int t,
    unsigned char *data,
    size_t local_offset,
    size_t local_offset_limit,
    off_t global_offset,
    char *filename)
{
  int big_endian;
  size_t i, start, end, limit, length;
  length = local_offset_limit - local_offset;

  big_endian = OPTIONS.big_endian;

  start = local_offset + tokens[t].skip_start;
  limit = local_offset_limit - tokens[t].match_len;

  switch (tokens[t].skip_eq)
  {
  case NE:
    end = tokens[t].skip_count - 1;
    if (limit < end)
    {
      end = limit;
      MATCH_SWITCH
      return 0;
    }
    MATCH_SWITCH
    start = tokens[t].skip_count + 1;
    end = limit;
    MATCH_SWITCH;
    return 0;
    break;
  case GE:
    end = limit;
    break;
  case LE:
    end = start + tokens[t].skip_count;
    if (limit < end)
    {
      end = limit;
    }
    break;
  default:
    end = (t > 0 ? start : limit);
    break;
  }
  MATCH_SWITCH
  return 0;
}

void print_match(
    token_t *tokens,
    unsigned char *data,
    size_t global_offset,
    char *filename)
{
  size_t i, t, start;

  if (OPTIONS.debug > 1)
  {
    print_token_metadata(tokens);
  }

  if (OPTIONS.use_color)
  {
    printf("%c[%d;%d;%dm", 0x1B, OFFSET_BRIGHT, OFFSET_COLOR, OFFSET_BG);
  }

  t = 0;
  start = tokens[t].match_offset - tokens[t].skip_start;
  if (filename)
  {
    printf("%s:%0*zx", filename, OPTIONS.padding, global_offset + start);
  }
  else
  {
    printf("%0*zx", OPTIONS.padding, global_offset + start);
  }

  if (OPTIONS.use_color)
  {
    printf("%c[0m", 0x1B);
  }

  i = start;
  while (1)
  {
    for (; i < tokens[t].match_offset; i++)
    {
      printf(" %02x", data[i]);
    }
    if (tokens[t].match_seq == NULL)
    {
      break;
    }
    if (OPTIONS.use_color)
    {
      printf("%c[%d;%d;%dm", 0x1B, MATCH_BRIGHT, MATCH_COLOR, MATCH_BG);
    }
    for (; i < tokens[t].match_offset + tokens[t].match_len; i++)
    {
      printf(" %02x", data[i]);
    }
    if (OPTIONS.use_color)
    {
      printf("%c[0m", 0x1B);
    }
    t++;
  }
  printf("\n");
}

///////////////////////////////// File Search //////////////////////////////////

int search_file(
    char *file,
    token_t *tokens,
    char *range_string)
{
  int fd, rc;
  unsigned char *data;
  size_t tmp_offset, global_offset;
  range_t range;
  struct stat st;

  fd = open(file, O_RDONLY);
  if (fd == -1 || fstat(fd, &st) == -1 || !S_ISREG(st.st_mode))
  {
    fprintf(stderr, "failed to open %s for reading", file);
    return 1;
  }
  if (range_string != NULL)
  {
    if (parse_range(st.st_size, &range, range_string))
    {
      return 1;
    }
  }
  else
  {
    range.start = 0;
    range.length = st.st_size;
  }

  // mmap offset must be a multiple of the page length
  global_offset = align_to_page(range.start, 0);
  tmp_offset = range.start - global_offset;
  data = mmap(NULL, range.length + tmp_offset, PROT_READ, MAP_PRIVATE, fd, global_offset);
  range.start = tmp_offset;

  if (data == MAP_FAILED)
  {
    perror("failed to mmap file");
    close(fd);
    return 1;
  }
  if (close(fd) == -1)
  {
    perror("failed to close file");
    munmap(data, range.length);
    return 1;
  }

  rc = match_tokens(tokens, 0, data, range.start, range.start + range.length, global_offset, file);

  if (munmap(data, range.length))
  {
    perror("failed to munmap file");
    return 1;
  }
  return rc;
}

void recursive_search(const char *name, token_t *tokens, char *range_string)
{
  DIR *dir;
  struct dirent *entry;

  if (!(dir = opendir(name)))
    return;

  char path[1024];
  while ((entry = readdir(dir)) != NULL)
  {
    memset(path, 0, sizeof(path));
    if (entry->d_type == DT_DIR)
    {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;
      snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
      recursive_search(path, tokens, range_string);
    }
    else
    {
      snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
      search_file(path, tokens, range_string);
    }
  }
  closedir(dir);
}

//////////////////////////////// Stream Search /////////////////////////////////

inline size_t
fill_buffer(
    unsigned char *data,
    size_t bufsize)
{
  size_t total, tmpsize;
  total = 0;
  while ((tmpsize = fread(data + total, 1, bufsize - total, stdin)) > 0)
  {
    total += tmpsize;
  }
  return total;
}

int search_stream(
    FILE *fd,
    token_t *tokens,
    char *range_string,
    size_t stream_match_limit)
{
  int rc;
  unsigned char *data;
  off_t global_offset;
  size_t bufsize, tmpsize, used, length, remaining;
  range_t range;

  if (range_string != NULL)
  {
    if (parse_range(0, &range, range_string))
    {
      return 1;
    }
    // Matches are limited by the range.
    if (range.length < stream_match_limit)
    {
      stream_match_limit = range.length;
    }
  }
  else
  {
    range.start = 0;
    range.length = 0;
  }

  if (tokens[0].min_len > stream_match_limit)
  {
    fprintf(stderr, "error: the minimum match length exceeds the maximum match length limit\n");
    return 1;
  }

  bufsize = align_to_page(stream_match_limit, 1);
  data = malloc(bufsize * 2);
  if (data == NULL)
  {
    perror("failed to allocate memory for input");
    return 1;
  }

  // skip to range start
  global_offset = 0;
  while (global_offset < range.start)
  {
    tmpsize = range.start - global_offset;
    if (tmpsize > bufsize << 1)
    {
      tmpsize = fread(data, 1, bufsize << 1, stdin);
    }
    else
    {
      tmpsize = fread(data, 1, tmpsize, stdin);
    }
    if (tmpsize)
    {
      global_offset += tmpsize;
    }
    else
    {
      fprintf(stderr, "warning: end of input before range start\n");
      free(data);
      return 0;
    }
  }

  remaining = range.length;
  used = fill_buffer(data + bufsize, bufsize);
  do
  {
    memcpy(data, data + bufsize, bufsize);
    used += fill_buffer(data + bufsize, bufsize);
    length = MIN(bufsize, used);
    if (range.length)
    {
      if (remaining < length)
      {
        length = remaining;
      }
      remaining -= bufsize;
    }
    rc = match_tokens(tokens, 0, data, 0, length, global_offset, NULL);
    if (rc)
    {
      break;
    }
    used -= length;
    global_offset += bufsize;
  } while (
      (!range.length || remaining > tokens[0].min_len) &&
      used >= bufsize);

  free(data);
  return rc;
}

/////////////////////////////// Argument Parsing ///////////////////////////////

token_t *
parse_args(
    int argc,
    char **argv,
    int *rc)
{
  int i, j;
  token_t *tokens, *previous_token;

  (*rc) = EXIT_FAILURE;
  OPTIONS.debug = 0;

  while ((i = getopt(argc, argv, "bdcf:hl:p:r:R:")) != -1)
  {
    switch (i)
    {
    case 'b':
      OPTIONS.big_endian = 1;
      break;
    case 'c':
      OPTIONS.use_color = 1;
      break;
    case 'd':
      OPTIONS.debug++;
      break;
    case 'f':
      OPTIONS.file = optarg;
      break;
    case 'R':
      OPTIONS.recursive = optarg;
      break;
    case 'l':
      OPTIONS.stream_match_limit = parse_bufsize(optarg);
      break;
    case 'p':
      OPTIONS.padding = strtoul(optarg, NULL, 0);
      break;
    case 'r':
      OPTIONS.range_string = optarg;
      break;
    case 'h':
      print_help(argv[0]);
      (*rc) = EXIT_SUCCESS;
      return NULL;
    case '?':
      switch (optopt)
      {
      case 'f':
      case 'l':
      case 'p':
      case 'r':
        fprintf(stderr, "error: -%c requires an argument\n", optopt);
        break;
      case 'R':
        fprintf(stderr, "error: -%c requires a starting directory\n", optopt);
        break;

      default:
        fprintf(stderr, "error: unrecognized option `-%c'\n", optopt);
      }
      print_help(argv[0]);
      return NULL;
      break;
    default:
      abort();
      break;
    }
  }

  i = argc - optind;
  // +1 for empty terminator
  tokens = calloc(i + 1, sizeof(token_t));
  if (tokens == NULL)
  {
    perror("failed to allocate memory for tokens");
    return NULL;
  }
  previous_token = NULL;
  for (i = 0, j = optind; j < argc; j++)
  {
    tokens[i].match_seq = NULL;
    if (parse_token(&tokens[i], argv[j], previous_token))
    {
      free_tokens(tokens);
      return NULL;
    }
    if (tokens[i].match_seq != NULL)
    {
      previous_token = tokens + (i++);
    }
  }
  tokens[i].match_seq = NULL;
  if (tokens[0].match_seq == NULL)
  {
    free_tokens(tokens);
    fprintf(stderr, "error: no (valid) search tokens\n");
    return NULL;
  }
  calculate_min_lengths(tokens);
  //   if (tokens[0].skip_eq == NA)
  //   {
  //     tokens[0].skip_eq = GE;
  //   }
  return tokens;
}

int parse_range(
    size_t size,
    range_t *range,
    char *string)
{
  char *head, *tail;
  off_t end = 0;
  int c = 0;
  head = string;

  range->start = strtoul(head, &tail, 0);
  if (range->start < 0)
  {
    range->start += size;
  }
  if (head != tail)
  {
    c += 1 << 0;
    head = tail;
  }
  if (head[0] == ':')
  {
    head++;
  }
  else
  {
    fprintf(stderr, "error: invalid range specifier (%s)\n", string);
    return 1;
  }
  range->length = labs(strtoul(head, &tail, 0));
  if (head != tail)
  {
    c += 1 << 1;
    head = tail;
  }
  else
  {
    if (head[0] == ':')
    {
      head++;
    }
    else
    {
      fprintf(stderr, "error: invalid range specifier (%s)\n", string);
      return 1;
    }
    end = strtoul(head, &tail, 0);
    if (end <= 0)
    {
      end += size;
    }
    if (head != tail)
    {
      c += 1 << 2;
    }
  }

  switch (c)
  {
  // start and length already set
  case 2:
  case 3:
  case 7:
    break;
  // start set, length 0
  case 1:
    range->length = size - range->start;
    break;
  // start 0, length 0, end set
  case 4:
  // start set, length 0, end set
  case 5:
    range->length = end - range->start;
  // length, end
  case 6:
    range->start = end - range->length;
  }

  if (range->start < 0)
  {
    fprintf(stderr, "error: invalid range specifier (%s)\n", string);
    return 1;
  }
  if (size && range->start + range->length > size)
  {
    if (range->start > size)
    {
      fprintf(stderr, "error: invalid range specifier (%s)\n", string);
      return 1;
    }
    else
    {
      range->length = size - range->start;
    }
  }
  return 0;
}

///////////////////////////////////// Main /////////////////////////////////////

int main(
    int argc,
    char **argv)
{
  int rc;
  token_t *tokens;

  opterr = 0;

  tokens = parse_args(argc, argv, &rc);
  if (tokens == NULL)
  {
    return rc;
  }

  if (OPTIONS.debug)
  {
    printf("tokens: ");
    print_tokens(tokens);
    printf("\n");
    if (OPTIONS.debug > 1)
    {
      printf("\n");
      print_token_metadata(tokens);
      printf("\n");
    }
  }
  if (OPTIONS.file)
  {
    rc = search_file(OPTIONS.file, tokens, OPTIONS.range_string);
  }
  else if (OPTIONS.recursive)
  {
    recursive_search(OPTIONS.recursive, tokens, OPTIONS.range_string);
  }
  else
  {
    rc = search_stream(stdin, tokens, OPTIONS.range_string, OPTIONS.stream_match_limit);
  }
  free_tokens(tokens);
  return (rc ? EXIT_FAILURE : EXIT_SUCCESS);
}