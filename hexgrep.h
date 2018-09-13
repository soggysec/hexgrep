#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define AS_STR(arg) #arg
#define AS_STRING(arg) AS_STR(arg)

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif

#define USE_SHORTCUTS 0

#define SHM_DIR "/dev/shm"

#define DEFAULT_PADDING 6
#define MATCH_BRIGHT 0
#define MATCH_COLOR 32
#define MATCH_BG 48
#define OFFSET_BRIGHT 0
#define OFFSET_COLOR 34
#define OFFSET_BG 48

#define STREAM_MATCH_LIMIT ((size_t)0x1000)

#define BUFFER_SIZE ((size_t)0x10000)

typedef enum {
  NA = 0,
  EQ = 1,
  GT = 2,
  GE = 3,
  LT = 4,
  LE = 5,
  NE = 6
} equality_t;

struct options {
  char *file;
  char *range_string;
  int big_endian;
  int debug;
  int padding;
  int use_color;
  size_t stream_match_limit;
  char *recursive;
};

struct options OPTIONS = {.big_endian         = 0,
                          .debug              = 0,
                          .file               = NULL,
                          .padding            = DEFAULT_PADDING,
                          .range_string       = NULL,
                          .stream_match_limit = STREAM_MATCH_LIMIT,
                          .use_color          = 0,
                          .recursive          = NULL};

typedef struct {
  off_t start;
  size_t length;
} range_t;

typedef void (*match_func_t)(unsigned char *data, size_t length,
                             equality_t skip_eq, size_t skip_len);

typedef struct token {
  equality_t skip_eq;
  size_t skip_start;
  size_t skip_count;
  equality_t match_eq;
  unsigned char *match_seq;
  size_t match_len;
  size_t min_len;
  size_t match_offset;
} token_t;

typedef struct match_list {
  off_t offset;
  size_t count;
  struct match_list *next;
} match_list_t;

void print_help(char *name);

const char *equality_to_str(equality_t eq);

size_t parse_bufsize(char *string);

void free_tokens(token_t *tokens);

void print_tokens(token_t *tokens);

int parse_bytes(token_t *token, token_t *previous_token, size_t value);

int parse_token(token_t *token, char *string, token_t *previous_token);

int parse_range(size_t size, range_t *range, char *string);

/*
inline int
match_bytes(
    unsigned char *data,
    token_t *token,
    off_t *data_offset,
    size_t length,
    match_list_t **match_list_ptr);
*/

int match_tokens(token_t *tokens, int t, unsigned char *data,
                 size_t local_offset, size_t max_length, off_t global_offset,
                 char *filename);

void print_match(token_t *tokens, unsigned char *data, size_t global_offset,
                 char *filename);

int search_file(char *file, token_t *tokens, char *range_string);

int search_stream(FILE *fd, token_t *tokens, char *range_string,
                  size_t bufsize);
