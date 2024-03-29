
/**
 * When built without test
 */
#include "bitmap.h"
#include "cfg.h"
#include "nusstool.h"
#include "nususb.h"
#include <string.h>
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
  ADDR,

  NUS_TITLE,
  NUS_BOOT_ADDR,
  NUS_CFG_FLAGS,
  NUS_CLOCK_RATE,
  NUS_LU_VER,
  NUS_CATEGORY,
  NUS_UNIQUE,
  NUS_DESTINATION,
  NUS_VERSION,
  NUS_BOOT,
  NUS_LOAD,
  NUS_DUMP,
  NUS_RAM_WR,
  NUS_RAM_RD,
  WR_ARR,
  WR_TXTARR,
  WR_ARRAY_TYPE,

  BMP_1BPP
};

static struct argp_option options[] = {
    {"output", 'o', "FILE", 0,
     "Output to FILE instead of standard output (- for no output)"},
    {"input", 'i', "FILE", 0,
     "Input from FILE instead of standard input (- for no input)"},
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
    {"verbose", 'v', NULL, 0, "Enable output"},
    {"bl", 'B', "OFFSET", 0, "Set buffer lenght"},
    {"addr", 'A', "OFFSET", 0, "Set address for usb operations"},
    {"warray", WR_ARR, "NAME", 0, "Output as const u8 array with name"},
    {"wtxtarray", WR_TXTARR, "NAME", 0,
     "Output as const u8 array but assumes the content of input is a correctly "
     "formatted text input (a, b, c...)"},
    {"warrtype", WR_ARRAY_TYPE, "TYPE", 0, "The data type for the array"},

    {"nustitle", NUS_TITLE, "TITLE", 0, ""},
    {"nusboot", NUS_BOOT_ADDR, "ADDRESS", 0, ""},
    {"nuscfg", NUS_CFG_FLAGS, "CFG", 0, ""},
    {"nusclk", NUS_CLOCK_RATE, "CLOCK_RATE", 0, ""},
    {"nuslu", NUS_LU_VER, "VERSION", 0, ""},
    {"nuscat", NUS_CATEGORY, "CATEGORY", 0, ""},
    {"nusdest", NUS_DESTINATION, "DESTINATION", 0, ""},
    {"nusuniq", NUS_UNIQUE, "UNIQUE", 0, ""},
    {"nusver", NUS_VERSION, "VERSION", 0, ""},
    {"nuswriteusb", NUS_LOAD, NULL, 0, "Load via usb"},
    {"nusbootusb", NUS_BOOT, NULL, 0, "Boot via usb"},
    {"nusdumpusb", NUS_DUMP, NULL, 0,
     "Dump data over usb. Data is read into the buffer until it is filled"},
    {"nuswrusb", NUS_RAM_WR, NULL, 0, "Write buffer to ram"},
    {"nusrdusb", NUS_RAM_RD, NULL, 0, "Read buffer from ram"},

    {"bmp1", BMP_1BPP, NULL, 0,
     "Interpret input as bmp and convert to 1bpp array"},
    {0}};

enum OperationKind {
  NONE,
  PAD_TO,
  PAD_BY,
  SET,
  INJECT_FILE,
  INJECT,
  NUSBOOT,
  NUSLOAD,
  NUSDUMP,
  NUSRAMRD,
  NUSRAMWR,
  BMP_1BPP_OP
};

struct Inject {
  usize at;
  char *data;
};

struct InjectFile {
  usize at;
  char *path;
};

struct PadTo {
  usize to;
};

struct PadBy {
  usize by;
};

struct Set {
  usize at;
  usize len;
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

  char *array_name;
  char *text_array_name;
  char *array_type;

  usize buffer_len;
  u32 addr;

  bool pnush;
  bool addnush;
  bool setnush;
  bool dry;
  bool noinput;

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
  case 'v':
    nuss_verbose = 1;
    break;
  case 'B':
    arguments->buffer_len = atoi(arg);
    break;
  case 'A':
    arguments->addr = atoi(arg);
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
  case NUS_BOOT:
    arguments->op_kind = NUSBOOT;
    break;
  case NUS_LOAD:
    arguments->op_kind = NUSLOAD;
    break;
  case NUS_DUMP:
    arguments->op_kind = NUSDUMP;
    break;
  case NUS_RAM_WR:
    arguments->op_kind = NUSRAMWR;
    break;
  case NUS_RAM_RD:
    arguments->op_kind = NUSRAMRD;
    break;
  case WR_ARR:
    arguments->array_name = arg;
    break;
  case WR_TXTARR:
    arguments->text_array_name = arg;
    break;
  case WR_ARRAY_TYPE:
    arguments->array_type = arg;
    break;
  case BMP_1BPP:
    arguments->op_kind = BMP_1BPP_OP;
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv) {
  int exit_code = 0;

  struct Arguments arguments;
  memset(&arguments, 0, sizeof(struct Arguments));

  /* Default values. */
  arguments.output_file = NULL;
  arguments.input_file = NULL;
  arguments.array_type = "const unsigned char";

  FILE *in = stdin;
  FILE *out = stdout;

  argp_parse(&argp, argc, argv, 0, 0, &arguments); // NOLINT

  if (arguments.input_file && strncmp(arguments.input_file, "-", 1) == 0) {
    arguments.noinput = TRUE;
  }
  if (arguments.output_file && strncmp(arguments.output_file, "-", 1) == 0) {
    arguments.dry = TRUE;
  }

  if (arguments.output_file && !arguments.dry) {
    out = fopen(arguments.output_file, "we");
  }
  if (arguments.input_file && !arguments.noinput) {
    in = fopen(arguments.input_file, "re");
  }

  if (out == NULL || in == NULL) {
    fprintf(stderr, "Unable to open file\n");
    return -1;
  }

  // all actions are applied to the buffer which is read here
  Buffer buffer;
  buffer_init(&buffer);
  if (!arguments.noinput) {
    buffer_read(&buffer, in);
  }

  if (buffer.len < arguments.buffer_len) {
    buffer_pad_to(&buffer, arguments.buffer_len, 0);
  }

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
  case NUSBOOT:
    if ((exit_code = nus_usb_boot(&buffer)) && nuss_verbose) {
      fprintf(stderr, "boot failed\n");
    }
    break;
  case NUSLOAD:
    if ((exit_code = nus_usb_load(&buffer, arguments.addr)) && nuss_verbose) {
      fprintf(stderr, "load failed\n");
    }
    break;
  case NUSDUMP:
    if ((exit_code = nus_usb_dump(&buffer, arguments.addr)) && nuss_verbose) {
      fprintf(stderr, "dump failed\n");
    }
    break;
  case NUSRAMWR:
    if ((exit_code = nus_usb_ram_wr(&buffer, arguments.addr)) && nuss_verbose) {
      fprintf(stderr, "write failed\n");
    }
    break;
  case NUSRAMRD:
    if ((exit_code = nus_usb_ram_rd(&buffer, arguments.addr)) && nuss_verbose) {
      fprintf(stderr, "read failed\n");
    }
    break;
  case BMP_1BPP_OP:
    if ((exit_code = bitmap_to_1bpp(&buffer)) && nuss_verbose) {
      fprintf(stderr, "bmp conversion failed\n");
    }
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
      usize len = MIN(NUS_TITLE_LEN, strlen(arguments.nus_title));
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
    if (arguments.array_name) {
      buffer_write_array(&buffer, out, arguments.array_name,
                         arguments.array_type);
    } else if (arguments.text_array_name) {
      buffer_write_text_array(&buffer, out, arguments.text_array_name,
                              arguments.array_type);
    } else {
      buffer_write(&buffer, out);
    }
  }

  buffer_free(&buffer);

  if (arguments.output_file) {
    fclose(out);
  }
  if (arguments.input_file) {
    fclose(in);
  }

  return exit_code;
}

#endif
#endif

/**
 * When built with test
 */
#ifdef TEST

#include "macros.h"
#include "nusheader.h"
#include "buffer.h"

int main(int argc, char **argv) {
  const struct CMUnitTest tests[] = {cmocka_unit_test(test_crc_fail),
                                     cmocka_unit_test(test_crc),
                                     cmocka_unit_test(test_bmp1_converter)};
  return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif
