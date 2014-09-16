#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>

#include "util.h"

static void
do_test_parse_ntp_range (const gchar *field, const gchar *expected_start_str,
    const gchar *expected_stop_str, const gchar *expected_total_str,
    gint64 expected_start, gint64 expected_stop, gint64 expected_total)
{
  gchar *start_str = NULL, *stop_str = NULL, *total_str = NULL;
  guint64 start, stop, total;

  g_assert (dlna_src_parse_npt_range (NULL, field,
      &start_str, &stop_str, &total_str, &start, &stop, &total));

  g_assert_cmpstr (start_str, ==, expected_start_str);
  g_assert_cmpstr (stop_str, ==, expected_stop_str);
  g_assert_cmpstr (total_str, ==, expected_total_str);
  g_assert_cmpuint (start, ==, expected_start);
  g_assert_cmpuint (stop, ==, expected_stop);
  g_assert_cmpuint (total, ==, expected_total);

  g_free (start_str);
  g_free (stop_str);
  g_free (total_str);
}

static void
test_parse_ntp_range (void)
{
  do_test_parse_ntp_range ("TimeSeekRange.dlna.org : npt=335.1-336.1/40445.4 bytes=1539686400-1540210688/304857907200",
      "335.1", "336.1", "40445.4", 335099985920, 336099999744, 40445400842240);
  do_test_parse_ntp_range ("availableSeekRange.dlna.org: 0 npt=0:00:00.000-0:00:48.716 bytes=0-5219255 cleartextbytes=0-5219255",
      "0:00:00.000", "0:00:48.716", NULL, 0, 48716001280, 0);
  do_test_parse_ntp_range ("npt=10.0-/* bytes=24409920-198755327/198755328",
      "10.0", NULL, "*", 10000000000, GST_CLOCK_TIME_NONE, 0);
}

static void
do_test_parse_byte_range (const gchar *field, const gchar *header,
    guint64 expected_start, guint64 expected_stop, guint64 expected_total)
{
  guint64 start, stop, total;

  g_assert (dlna_src_parse_byte_range (NULL, field, header, &start, &stop,
        &total));

  g_assert_cmpuint (start, ==, expected_start);
  g_assert_cmpuint (stop, ==, expected_stop);
  g_assert_cmpuint (total, ==, expected_total);
}

static void
test_parse_byte_range (void)
{
  do_test_parse_byte_range ("TimeSeekRange.dlna.org : npt=335.1-336.1/40445.4 bytes=1539686400-1540210688/304857907200",
      "BYTES", 1539686400, 1540210688, 304857907200);
  do_test_parse_byte_range ("Content-Range: bytes 0-1859295/1859295",
      "BYTES", 0, 1859295, 1859295);
  do_test_parse_byte_range ("Content-Range.dtcp.com: bytes=0-9931928/9931929",
      "BYTES", 0, 9931928, 9931929);
  do_test_parse_byte_range ("availableSeekRange.dlna.org: 0 npt=0:00:00.000-0:00:48.716 bytes=0-219255 cleartextbytes=0-5219255",
      "CLEARTEXTBYTES", 0, 5219255, 0);
}

int
main (int argc,
    char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/util/parse-ntp-range", test_parse_ntp_range);
  g_test_add_func ("/util/parse-byte-range", test_parse_byte_range);

  return g_test_run ();
}
