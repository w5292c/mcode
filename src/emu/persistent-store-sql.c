/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2020 Alexander Chumakov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "persistent-store.h"

#include "hw-nvm.h"

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <sys/stat.h>

static const char MCodeSeparator[] = "/";
static const char MCodeFilename[] = "store.db";
static const char MCodeDirectory[] = ".mcode";

static char *ensure_directory(void);

void persist_store_load(uint8_t id, void *data, uint8_t length)
{
  char *const filename = ensure_directory();
  sqlite3 *db = NULL;
  sqlite3_stmt *res = NULL;

  int rc = sqlite3_open(filename, &db);
  if (rc != SQLITE_OK) {
    printf("Error: cannot open DB");
    sqlite3_close(db);
    return;
  }

  const char *const sql = "SELECT value FROM store WHERE id = ?";
  rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
  if (rc == SQLITE_OK) {
    sqlite3_bind_int(res, 1, id + 1);
  } else {
      fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
  }

  gsize decodedLength = 0;
  guchar *decoded = NULL;
  int step;
  do {
    step = sqlite3_step(res);
    if (step != SQLITE_ROW) {
      break;
    }

    const gchar *value = (const gchar *)sqlite3_column_text(res, 0);
    decoded = g_base64_decode(value, &decodedLength);
  } while (0);
  sqlite3_finalize(res);
  sqlite3_close(db);

  if (decoded) {
    memcpy(data, decoded, MIN(decodedLength, length));
  } else {
    printf("Error: failed to get value for ID: %d\r\n", id);
  }
}

void persist_store_save(uint8_t id, const void *data, uint8_t length)
{
  char *const filename = ensure_directory();

  sqlite3 *db = NULL;
  sqlite3_stmt *res = NULL;
  int rc = sqlite3_open(filename, &db);
  if (rc != SQLITE_OK) {
    printf("Error: cannot open DB");
    sqlite3_close(db);
    return;
  }

  const char *const sql = "UPDATE store SET value = ? WHERE id = ?";
  rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
  if (rc == SQLITE_OK) {
    gchar *const encodedData = g_base64_encode(data, length);
    sqlite3_bind_text(res, 1, encodedData, -1, g_free);
    sqlite3_bind_int(res, 2, id + 1);
  } else {
      fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
  }

  int step;
  do {
    step = sqlite3_step(res);
    if (step != SQLITE_ROW) {
      printf("Step result: %d\r\n", step);
      break;
    }
  } while (1);

  sqlite3_finalize(res);
  sqlite3_close(db);

  g_free(filename);
}

char *ensure_directory(void)
{
  const char *const home = getenv("HOME");
  char *const dirName = g_strjoin(MCodeSeparator, home, MCodeDirectory, NULL);
  /* ignore result, as the directory may also be created */
  mkdir(dirName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  char *const fileName = g_strjoin(MCodeSeparator, dirName, MCodeFilename, NULL);
  g_free(dirName);
  return fileName;
}
