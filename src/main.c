
/**
 * When built without test
 */
#include "buffer.h"
#include "types.h"
#include "nusheader.h"
#include <string.h>
#ifndef TEST

/// only use main if binary
#if TYPE == bin

#include <stdio.h>
#include <argp.h>

const char *argp_program_version = "nusstool 0.1";
const char *argp_program_bug_address = "<lukas@krickl.dev>";

static char doc[] = "nusstool";

static char args_doc[] = "";

// anything past the ascii range can still be used as a key
enum ArgpKeys {
  PNUSH = 0x80,
  ADDNUSH,
  SETNUSH,
  DRY,
  OP,
  AT,
  PATH,
  DATA,
  TO,
  VAL,
  LEN
};

static struct argp_option options[] = {
    {"output", 'o', "FILE", 0, "Output to FILE instead of standard output"},
    {"input", 'i', "FILE", 0, "Input from FILE instead of standard input"},
    {"pnush", PNUSH, NULL, 0, "Print the nus-header of the input file"},
    {"addnush", ADDNUSH, NULL, 0, "Add space for a nus header to the buffer"},
    {"setnush", SETNUSH, NULL, 0, "Calculate the nus crc"},
    {"dry", DRY, NULL, 0, "Dry run - no output will be generated"},
    {"op", OP, "OPERATION", 0,
     "The operation type. PAD_TO = 1, PAD_BY = 2, INSERT = 3, INJECT_FILE = 4, "
     "INJECT = 5"},
    {"at", AT, "OFFSET", 0,
     "Where data should be inserted. For INJECT INJECT_FILE and INSERT."},
    {"path", PATH, "FILE", 0, "Path to be injected by INJECT_FILE"},
    {"data", DATA, "BYTES", 0, "Data to be used by INJECT"},
    {"to", TO, "OFFSET", 0, "PAD_TO offset"},
    {"val", VAL, "BYTE", 0, "INSERT value"},
    {"len", LEN, "OFFSET", 0, "INSERT length"},
    {0}};

enum OperationKind { NONE, PAD_TO, PAD_BY, INSERT, INJECT_FILE, INJECT };

struct Inject {
  size at;
  char *data;
};

struct InjectFile {
  size at;
  char *path;
};

struct PadTo {
  size to;
};

struct PadBy {
  size by;
};

struct Insert {
  size at;
  size len;
  u8 val;
};

union Operation {
  struct Inject inject;
  struct InjectFile inject_file;
  struct PadTo pad_to;
  struct PadBy pad_by;
  struct Insert insert;
};

struct Arguments {
  char *output_file;
  char *input_file;

  bool pnush;
  bool addnush;
  bool setnush;
  bool dry;
  bool nus_crc;

  // mutually exclusive options
  enum OperationKind op_kind;
  union Operation op;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct Arguments *arguments = state->input;

  switch (key) {
  case 'o':
    arguments->output_file = arg;
    break;
  case 'i':
    arguments->input_file = arg;
    break;
  case PNUSH:
    arguments->pnush = TRUE;
    break;
  case ADDNUSH:
    arguments->addnush = TRUE;
    break;
  case SETNUSH:
    arguments->setnush = TRUE;
  case DRY:
    arguments->dry = TRUE;
    break;
  case OP:
    arguments->op_kind = atoi(arg);
    break;
  case AT:
    break;
  case ARGP_KEY_ARG:
    if (state->arg_num >= 0) {
      /* Too many arguments. */
      argp_usage(state); // NOLINT
    }

    // arguments->args[state->arg_num] = arg;

    break;
  case ARGP_KEY_END:
    if (state->arg_num < 0) {
      /* Not enough arguments. */
      argp_usage(state); // NOLINT
    }
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv) {
  struct Arguments arguments;
  memset(&arguments, 0, sizeof(struct Arguments));

  /* Default values. */
  arguments.output_file = NULL;
  arguments.input_file = NULL;

  FILE *in = stdin;
  FILE *out = stdout;

  argp_parse(&argp, argc, argv, 0, 0, &arguments); // NOLINT

  if (arguments.output_file) {
    out = fopen(arguments.output_file, "we");
  }
  if (arguments.input_file) {
    in = fopen(arguments.input_file, "re");
  }

  // all actions are applied to the buffer which is read here
  Buffer buffer;
  buffer_init(&buffer);
  buffer_read(&buffer, in);

  if (arguments.addnush) {
    nus_add_header(&buffer);
  }

  if (arguments.setnush) {
    NusHeader header;
    nus_from_bytes(&header, buffer.data, buffer.len);
    nus_set_header(&buffer, &header);
  }

  if (arguments.pnush) {
    NusHeader header;
    nus_from_bytes(&header, buffer.data, buffer.len);
    nus_fprint(stdout, &header);
  }
  if (!arguments.dry) {
    buffer_write(&buffer, out);
  }

  buffer_free(&buffer);

  if (arguments.output_file) {
    fclose(out);
  }
  if (arguments.input_file) {
    fclose(in);
  }

  return 0;
}

#endif
#endif

/**
 * When built with test
 */
#ifdef TEST

#include "macros.h"
#include "nusheader.h"

int main(int argc, char **argv) {
  const struct CMUnitTest tests[] = {cmocka_unit_test(test_crc_fail),
                                     cmocka_unit_test(test_crc)};
  return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif
