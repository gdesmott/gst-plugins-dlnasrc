/* Copyright (C) 2013 Cable Television Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL CABLE TELEVISION LABS INC. OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

/**
 * Convert supplied string which represents normal play time (npt) into
 * nanoseconds.  The format of NPT is as follows:
 *
 * npt time  = npt sec | npt hhmmss
 *
 * npt sec   = 1*DIGIT [ "." 1*3DIGIT ]
 * npthhmmss = npthh":"nptmm":"nptss["."1*3DIGIT]
 * npthh     = 1*DIGIT     ; any positive number
 * nptmm     = 1*2DIGIT    ; 0-59
 * nptss     = 1*2DIGIT    ; 0-59
 *
 * @param	dlna_src			this element, needed for logging
 * @param	string				normal play time string to convert
 * @param	media_time_nanos	npt string value converted into nanoseconds
 *
 * @return	true if no problems encountered, false otherwise
 */
static gboolean
dlna_src_npt_to_nanos (GstDlnaSrc * dlna_src, gchar * string,
    guint64 * media_time_nanos_out)
{
  gboolean ret = FALSE;
  guint64 media_time_nanos;

  guint hours = 0;
  guint mins = 0;
  float secs = 0.;

  if (sscanf (string, "%u:%u:%f", &hours, &mins, &secs) == 3) {
    /* Long form */
    media_time_nanos =
        ((hours * 60 * 60 * 1000) + (mins * 60 * 1000) +
        (secs * 1000)) * 1000000L;
    ret = TRUE;

    GST_LOG_OBJECT (dlna_src,
        "Convert npt str %s hr=%d:mn=%d:s=%f into nanosecs: %"
        G_GUINT64_FORMAT, string, hours, mins, secs, media_time_nanos);
  } else if (sscanf (string, "%f", &secs) == 1) {
    /* Short form */
    media_time_nanos = (secs * 1000) * 1000000L;
    ret = TRUE;
    GST_LOG_OBJECT (dlna_src,
        "Convert npt str %s secs=%f into nanosecs: %"
        G_GUINT64_FORMAT, string, secs, media_time_nanos);
  } else {
    GST_ERROR_OBJECT (dlna_src,
        "Problems converting npt str into nanosecs: %s", string);
  }
  if (ret && media_time_nanos_out)
    *media_time_nanos_out = media_time_nanos;

  return ret;
}

static gboolean
parse_range_part (GstDlnaSrc * dlna_src, gchar ** cursor, char *format,
    char **result_str, guint64 * result, gboolean is_total)
{
  gchar tmp[32] = { 0 };
  if (sscanf (*cursor, format, tmp) == -1)
    return FALSE;

  *cursor += strlen (tmp) + 1;

  if (result_str) {
    if (*result_str)
      g_free (*result_str);
    *result_str = g_strdup (tmp);
  }

  if (is_total && strcmp (tmp, "*") == 0) {
    if (result)
      *result = 0;
    return TRUE;
  } else
    return dlna_src_npt_to_nanos (dlna_src, tmp, result);
}

/**
 * Parse the npt (normal play time) range which may be contained in the following headers:
 *
 * TimeSeekRange.dlna.org : npt=335.1-336.1/40445.4 bytes=1539686400-1540210688/304857907200
 *
 * availableSeekRange.dlna.org: 0 npt=0:00:00.000-0:00:48.716 bytes=0-5219255 cleartextbytes=0-5219255
 *
 * @param   dlna_src    this element instance
 * @param   field_str   string containing HEAD response field header and value
 * @param   start_str   starting time in string form read from header response field
 * @param   stop_str    end time in string form read from header response field
 * @param   total_str   total time in string form read from header response field
 * @param   start       starting time in nanoseconds converted from string representation
 * @param   stop        end time in nanoseconds converted from string representation
 * @param   total       total time in nanoseconds converted from string representation
 *
 * @return  returns TRUE
 */
gboolean
dlna_src_parse_npt_range (GstDlnaSrc * dlna_src, const gchar * field_str,
    gchar ** start_str, gchar ** stop_str, gchar ** total_str,
    guint64 * start, guint64 * stop, guint64 * total)
{
  gchar *field, *cursor;

  /* Convert everything to upper case */
  field = g_ascii_strup (field_str, -1);

  /* Extract NPT portion of header value */
  cursor = strstr (field, "NPT");
  if (!cursor)
    goto fail;

  cursor = strstr (cursor, "=");
  if (!cursor)
    goto fail;

  cursor++;                     /* '=' */

  /* Read start value and '-' */
  if (!parse_range_part (dlna_src, &cursor, "%31[^-]-%*s", start_str, start,
          FALSE))
    goto fail;

  /* Read stop value, if any */
  if (g_ascii_isdigit (cursor[0])) {
    if (!parse_range_part (dlna_src, &cursor, "%31[^/ ]%*s", stop_str, stop,
            FALSE))
      goto fail;
  }

  /* Do we have the total length? */
  if (cursor[0] == '/') {
    cursor++;                   /* '/' */
    if (!parse_range_part (dlna_src, &cursor, "%31s %*s", total_str, total,
            TRUE))
      goto fail;
  }

  g_free (field);
  return TRUE;

fail:
  GST_WARNING_OBJECT (dlna_src,
      "Problems parsing npt from HEAD response field header value: %s",
      field_str);
  g_free (field);
  return FALSE;
}
