
/**
 * When built without test
 */
#include "buffer.h"
#include "types.h"
#include "nusheader.h"
#include <string.h>
#include "macros.h"
#ifndef TEST

/// only use main if binary
#if TYPE == bin

#include <stdio.h>
#include <stdlib.h>
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
  BY,
  TO,
  VAL,
  LEN,

  NUS_TITLE,
  NUS_BOOT_ADDR,
  NUS_CFG_FLAGS,
  NUS_CLOCK_RATE,
  NUS_LU_VER,
  NUS_CATEGORY,
  NUS_UNIQUE,
  NUS_DESTINATION,
  NUS_VERSION,
};

static struct argp_option options[] = {
    {"output", 'o', "FILE", 0, "Output to FILE instead of standard output"},
    {"input", 'i', "FILE", 0, "Input from FILE instead of standard input"},
    {"nusph", PNUSH, NULL, 0, "Print the nus-header of the input file"},
    {"nusaddh", ADDNUSH, NULL, 0, "Add space for a nus header to the buffer"},
    {"nusseth", SETNUSH, NULL, 0, "Calculate the nus crc"},
    {"dry", DRY, NULL, 0, "Dry run - no output will be generated"},
    {"op", OP, "OPERATION", 0,
     "The operation type. PAD_TO = 1, PAD_BY = 2, SET = 3, INJECT_FILE = 4, "
     "INJECT = 5"},
    {"at", AT, "OFFSET", 0,
     "Where data should be inserted. For INJECT INJECT_FILE and SET."},
    {"path", PATH, "FILE", 0, "Path to be injected by INJECT_FILE"},
    {"data", DATA, "BYTES", 0, "Data to be used by INJECT"},
    {"to", TO, "OFFSET", 0, "PAD_TO offset"},
    {"by", BY, "OFFSET", 0, "PAD_BY offset"},
    {"val", VAL, "BYTE", 0, "SET value"},
    {"len", LEN, "OFFSET", 0, "SET length"},

    {"nustitle", NUS_TITLE, "TITLE", 0, ""},
    {"nusboot", NUS_BOOT_ADDR, "ADDRESS", 0, ""},
    {"nuscfg", NUS_CFG_FLAGS, "CFG", 0, ""},
    {"nusclk", NUS_CLOCK_RATE, "CLOCK_RATE", 0, ""},
    {"nuslu", NUS_LU_VER, "VERSION", 0, ""},
    {"nuscat", NUS_CATEGORY, "CATEGORY", 0, ""},
    {"nusdest", NUS_DESTINATION, "DESTINATION", 0, ""},
    {"nusuniq", NUS_UNIQUE, "UNIQUE", 0, ""},
    {"nusver", NUS_VERSION, "VERSION", 0, ""},
    {0}};

enum OperationKind { NONE, PAD_TO, PAD_BY, SET, INJECT_FILE, INJECT };

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

struct Set {
  size at;
  size len;
  u8 val;
};

union Operation {
  struct Inject inject;
  struct InjectFile inject_file;
  struct PadTo pad_to;
  struct PadBy pad_by;
  struct Set set;
};

struct Arguments {
  char *output_file;
  char *input_file;

  bool pnush;
  bool addnush;
  bool setnush;
  bool dry;

  char *nus_title;
  char *nus_boot_addr;
  char *nus_cfg_flags;
  char *nus_clock_rate;
  char *nus_lu_ver;

  char nus_category;
  char *nus_unique;
  char nus_destination;
  char nus_version;

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
    break;
  case NUS_TITLE:
    arguments->nus_title = arg;
    break;
  case NUS_BOOT_ADDR:
    arguments->nus_boot_addr = arg;
    break;
  case NUS_CATEGORY:
    arguments->nus_category = arg[0];
    break;
  case NUS_CLOCK_RATE:
    arguments->nus_clock_rate = arg;
    break;
  case NUS_DESTINATION:
    arguments->nus_destination = arg[0];
    break;
  case NUS_LU_VER:
    arguments->nus_lu_ver = arg;
    break;
  case NUS_UNIQUE:
    arguments->nus_unique = arg;
    break;
  case NUS_VERSION:
    arguments->nus_version = arg[0];
    break;
  case NUS_CFG_FLAGS:
    arguments->nus_cfg_flags = arg;
    break;

  case DRY:
    arguments->dry = TRUE;
    break;

  case OP:
    arguments->op_kind = atoi(arg);
    break;
  case AT:
    if (arguments->op_kind == INJECT) {
      arguments->op.inject.at = atoi(arg);
    } else if (arguments->op_kind == INJECT_FILE) {
      arguments->op.inject_file.at = atoi(arg);
    } else if (arguments->op_kind == SET) {
      arguments->op.set.at = atoi(arg);
    } else {
      return ARGP_ERR_UNKNOWN;
    }
    break;
  case DATA:
    if (arguments->op_kind == INJECT) {
      arguments->op.inject.data = arg;
    } else {
      return ARGP_ERR_UNKNOWN;
    }
    break;
  case LEN:
    if (arguments->op_kind == SET) {
      arguments->op.set.len = atoi(arg);
    } else {
      return ARGP_ERR_UNKNOWN;
    }
    break;
  case PATH:
    if (arguments->op_kind == INJECT_FILE) {
      arguments->op.inject_file.path = arg;
    } else {
      return ARGP_ERR_UNKNOWN;
    }
    break;
  case BY:
    if (arguments->op_kind == PAD_BY) {
      arguments->op.pad_by.by = atoi(arg);
    } else {
      return ARGP_ERR_UNKNOWN;
    }
    break;
  case TO:
    if (arguments->op_kind == PAD_TO) {
      arguments->op.pad_to.to = atoi(arg);
    } else {
      return ARGP_ERR_UNKNOWN;
    }
    break;
  case VAL:
    if (arguments->op_kind == SET) {
      arguments->op.set.val = atoi(arg);
    } else {
      return ARGP_ERR_UNKNOWN;
    }
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

  switch (arguments.op_kind) {
  case NONE:
    break;
  case PAD_TO:
    buffer_pad_to(&buffer, arguments.op.pad_to.to, 0);
    break;
  case PAD_BY:
    buffer_pad_by(&buffer, arguments.op.pad_by.by, 0);
    break;
  case SET:
    buffer_set(&buffer, arguments.op.set.at, arguments.op.set.val,
               arguments.op.set.len);
    break;
  case INJECT_FILE: {
    FILE *f = fopen(arguments.op.inject_file.path, "re");
    if (f) {
      buffer_inject_file(&buffer, arguments.op.inject_file.at, f);
    } else {
      fprintf(stderr, "Unable to open %s\n", arguments.op.inject_file.path);
    }
    break;
  }
  case INJECT:
    buffer_inject(&buffer, arguments.op.inject.at,
                  (u8 *)arguments.op.inject.data,
                  strlen(arguments.op.inject.data));
    break;
  }

  if (arguments.addnush) {
    nus_add_header(&buffer);
  }

  if (arguments.setnush) {
    NusHeader header;
    nus_from_bytes(&header, buffer.data, buffer.len);

    // modify the header if needed
    if (arguments.nus_cfg_flags) {
      header.cfg_flags = atoi(arguments.nus_cfg_flags);
    }
    if (arguments.nus_boot_addr) {
      header.boot_addr = atoi(arguments.nus_boot_addr);
    }
    if (arguments.nus_clock_rate) {
      header.clck_rate = atoi(arguments.nus_clock_rate);
    }
    if (arguments.nus_lu_ver) {
      header.lu_ver = atoi(arguments.nus_lu_ver);
    }
    if (arguments.nus_category) {
      header.category = arguments.nus_category;
    }
    if (arguments.nus_unique) {
      header.unique[0] = arguments.nus_unique[0];
      if (arguments.nus_unique[0] != '\0') {
        header.unique[1] = arguments.nus_unique[1];
      }
    }
    if (arguments.nus_version) {
      header.version = arguments.nus_version;
    }
    if (arguments.nus_destination) {
      header.destination = arguments.nus_destination;
    }
    if (arguments.nus_title) {
      memset(header.title, 0, NUS_TITLE_LEN);
      size len = MIN(NUS_TITLE_LEN, strlen(arguments.nus_title));
      strncpy(header.title, arguments.nus_title, len);
    }

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
