/* ".LST" file output for gplink
   Copyright (C) 2004, 2005
   Craig Franklin

This file is part of gputils.

gputils is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

gputils is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with gputils; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "stdhdr.h"

#include "libgputils.h"
#include "gplink.h"
#include "cod.h"

#ifdef STDC_HEADERS
#include <stdarg.h>
#endif

static gp_boolean list_enabled;
static gp_section_type *line_section;

static void
open_src(const char *name, gp_symbol_type *symbol)
{
  char file_name[PATH_MAX + 1];
  struct list_context *new = malloc(sizeof(*new));
  int i;

  assert(name != NULL);

  new->f = fopen(name, "rt");
  if (new->f == NULL) {
    /* Try searching include pathes */
    for (i = 0; i < state.numpaths; i++) {
      snprintf(file_name, sizeof(file_name), "%s" COPY_CHAR "%s", state.paths[i], name);
      new->f = fopen(file_name, "rb");
      if (new->f != NULL) {
        name = file_name;
        break;
      }
    }
    if (new->f == NULL) {
      /* The path may belong to a build procedure other than this */
      const char *p = strrchr(name, PATH_CHAR);

      if (p != NULL) {
        for (i = 0; i < state.numpaths; i++) {
          snprintf(file_name, sizeof(file_name), "%s%s", state.paths[i], p);
          new->f = fopen(file_name, "rb");
          if (new->f != NULL) {
            name = file_name;
            break;
          }
        }
      }
    }
  }

  if (new->f != NULL) {
    new->name = strdup(name);
    new->missing_source = false;
  } else {
    new->missing_source = true;

    if (getenv("GPUTILS_WARN_MISSING_SRC")) {
      gp_warning("Cannot find source file \"%s\".", name);
    }
  }

  new->symbol = symbol;
  new->line_number = 1;
  new->prev = state.lst.src;

  state.lst.src = new;
}

static void
close_src(void)
{
  struct list_context *old;

  assert(state.lst.src != NULL);

  old = state.lst.src;
  state.lst.src = state.lst.src->prev;
  free(old);
}

static void
lst_line(const char *format, ...)
{
  if (state.lst.f != NULL) {
    va_list args;
    va_start(args, format);
    vfprintf(state.lst.f, format, args);
    va_end(args);
    fputc('\n', state.lst.f);
  }
}

gp_linenum_type *
find_line_number(gp_symbol_type *symbol, int line_number)
{
  gp_section_type *section;
  gp_linenum_type *line = NULL;

  section = state.object->sections;

  /* FIXME: This too slow. */
  while (section != NULL) {
    line = section->line_numbers;
    while (line != NULL) {
      if ((line->symbol == symbol) && (line->line_number == line_number)) {
        if (section != line_section) {
          /* switching sections, so update was_org with the new
             address */
          state.lst.was_org = line->address;
          line_section = section;
        }
        return line;
      }
      line = line->next;
    }
    section = section->next;
  }

  return NULL;
}

static char *
expand(const char *buf)
{
  int is, id;
  static char dest[520], c;

  for (is = 0, id = 0; (c = buf[is]) != '\0' && id < (sizeof(dest) - 2); ++is) {
    if (c == '\t') {
      unsigned int n = 8 - (id % 8);

      while (n-- && id < (sizeof(dest) - 2)) {
        dest[id++] = ' ';
      }
    }
    else {
      dest[id++] = c;
    }
  }
  dest[id] = '\0';
  return dest;
}

static void
write_src(int last_line)
{
  #define LINESIZ 520
  char linebuf[LINESIZ];
  char dasmbuf[LINESIZ];
  char *pc;
  gp_linenum_type *line = NULL;
  gp_boolean first_time;
  int org = 0;

  /* if the source file wasn't found, can't write it to the list file */
  if (state.lst.src->missing_source) {
    return;
  }

  while (1) {
    /* when last_line is 0 print all lines, else print to last_line */
    if ((last_line) && (state.lst.src->line_number > last_line)) {
      break;
    }

    if (fgets(linebuf, LINESIZ, state.lst.src->f) == NULL) {
      break;
    }

    state.lst.was_org = org;

    if (list_enabled) {
      /* Eat the trailing newline. */
      pc = strrchr(linebuf, '\n');

      if (pc != NULL) {
        *pc = '\0';
      }

      first_time = true;

      line = find_line_number(state.lst.src->symbol, state.lst.src->line_number);
      while (line != NULL) {
        unsigned int len;

        /* print all instructions generated by this line of the source */

        if (line->line_number != state.lst.src->line_number) {
          break;
        }

        if (first_time == false) {
          /* only print the source line the first time */
          linebuf[0] = '\0';
        }

        state.cod.emitting = 1;
        org = line->address;
        len = b_memory_get_unlisted_size(line_section->data, org);

        if (len == 0) {
          lst_line("%42s %s", "", linebuf);
          cod_lst_line(COD_NORMAL_LST_LINE);
        }
        else {
          if ((org & 1) || (len < 2)) {
            /* even address or less then two byts to disassemble: disassemble one byte */
            if (len != 0) {
              unsigned char byte;

              b_memory_assert_get(line_section->data, org, &byte, NULL, NULL);
              gp_disassemble_byte(line_section->data, org, state.class, dasmbuf, sizeof(dasmbuf));
              lst_line("%06lx   %02x       %-24s %s",
                       gp_processor_byte_to_org(state.class, org),
                       (unsigned int)byte,
                       expand(dasmbuf),
                       linebuf);
              b_memory_set_listed(line_section->data, org, 1);
              state.lst.was_org = org;
              cod_lst_line(COD_NORMAL_LST_LINE);
              ++org;
            }
          }
          else {
            unsigned short word;
            int num_bytes;

            state.class->i_memory_get(line_section->data, org, &word, NULL, NULL);
            num_bytes = gp_disassemble_size(line_section->data, org, state.class, 0x80,
                                            state.processor->prog_mem_size, GPDIS_SHOW_ALL_BRANCH,
                                            dasmbuf, sizeof(dasmbuf), len);
            lst_line("%06lx   %04x     %-24s %s",
                     gp_processor_byte_to_org(state.class, org), word,
                     expand(dasmbuf), linebuf);
            b_memory_set_listed(line_section->data, org, num_bytes);
            state.lst.was_org = org;
            cod_lst_line(COD_NORMAL_LST_LINE);
            org += 2;

            if (num_bytes > 2) {
              state.lst.was_org = org;
              state.class->i_memory_get(line_section->data, org, &word, NULL, NULL);
              lst_line("%06lx   %04x", gp_processor_byte_to_org(state.class, org), word);
              cod_lst_line(COD_NORMAL_LST_LINE);
              org += 2;

              if (line->next != NULL) {
                /* skip the line number for the other half of this instruction */
                line = line->next;
              }
            }
          }
        }
        first_time = false;
        line = line->next;
      }

      if (first_time) {
        lst_line("%42s %s", "", linebuf);
        state.cod.emitting = 0;
        cod_lst_line(COD_NORMAL_LST_LINE);
      }
    }

    state.lst.src->line_number++;
  }
}

/*
 * lst_init - initialize the lst file
 */

static void
lst_init(void)
{
  if (state.lstfile != OUT_NAMED) {
    snprintf(state.lstfilename, sizeof(state.lstfilename),
             "%s.lst", state.basefilename);
  }

  if ((gp_num_errors > 0) || (state.lstfile == OUT_SUPPRESS)) {
    state.lst.f = NULL;
    state.lst.enabled = false;
    unlink(state.lstfilename);
  } else {
    state.lst.f = fopen(state.lstfilename, "wt");

    if (state.lst.f == NULL) {
      perror(state.lstfilename);
      exit(1);
    }
    state.lst.enabled = true;
  }

  if (!state.lst.enabled) {
    return;
  }

  state.lst.was_org = 0;
  state.cod.emitting = 0;

  lst_line("%s", GPLINK_VERSION_STRING);
  lst_line("%s", GPUTILS_COPYRIGHT_STRING);
  lst_line("Listing File Generated: %s", state.startdate);
  lst_line(" ");
  lst_line(" ");
  lst_line("Address  Value    Disassembly              Source");
  lst_line("-------  -----    -----------              ------");
}

void
write_lst(void)
{
  gp_symbol_type *symbol = state.object->symbols;
  gp_aux_type *aux;
  gp_boolean first_time = true;

  lst_init();

  if (!state.lst.enabled) {
    return;
  }

  list_enabled = true;
  state.lst.src = NULL;

  /* scan through the file symbols */
  while (symbol != NULL) {
    if (symbol->class == C_FILE) {
      /* open a new file */
      aux = symbol->aux_list;
      assert(aux != NULL);

      if (aux->_aux_symbol._aux_file.line_number) {
        /* it is an include file, so print the current file
           until the line number is reached */
        assert(state.lst.src != NULL);
        write_src(aux->_aux_symbol._aux_file.line_number);
      } else {
        /* it is not an include, so enable listing */
        list_enabled = true;
      }

      open_src(aux->_aux_symbol._aux_file.filename, symbol);

      if (first_time) {
        /* write the line numbers for the lst file header */
        cod_lst_line(COD_FIRST_LST_LINE);
        cod_lst_line(COD_NORMAL_LST_LINE);
        cod_lst_line(COD_NORMAL_LST_LINE);
        cod_lst_line(COD_NORMAL_LST_LINE);
        cod_lst_line(COD_NORMAL_LST_LINE);
        cod_lst_line(COD_NORMAL_LST_LINE);
        cod_lst_line(COD_NORMAL_LST_LINE);
        first_time = false;
      }
    } else if (symbol->class == C_EOF) {
      /* print the rest of the current file then, close it */
      write_src(0);
      close_src();
    } else if (symbol->class == C_LIST) {
      if (strcasecmp(symbol->name, ".list") == 0) {
        write_src(symbol->value);
        list_enabled = true;
      } else if (strcasecmp(symbol->name, ".nolist") == 0) {
        write_src(symbol->value);
        list_enabled = false;
      } else {
        assert(0);
      }
    }
    symbol = symbol->next;
  }

  fclose(state.lst.f);
}
