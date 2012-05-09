#include <pocketsphinx.h>

int
main(int argc, char *argv[])
{
  ps_decoder_t *ps;
  cmd_ln_t *config;
  FILE *fh;
  char const *hyp, *uttid;
  int16 buf[512];
  int rv;
  int32 score;


if (argc!=3)
  {
   printf("Usage: %s <config> <file.wav>\n",argv[0]);
   return (-1);
  }


  config = cmd_ln_parse_file_r(NULL, ps_args(), argv[1], FALSE);
    
  ps = ps_init(config);

/* Open audio file and start feeding it into the decoder */
  fh = fopen(argv[2], "rb");
  rv = ps_start_utt(ps, "goforward");
  while (!feof(fh))
    {
      size_t nsamp;
      nsamp = fread(buf, 2, 512, fh);
      rv = ps_process_raw(ps, buf, nsamp, FALSE, FALSE);
    }
  rv = ps_end_utt(ps);

/* Get the result and print it */
  hyp = ps_get_hyp(ps, &score, &uttid);
  if (hyp == NULL)
    return 1;
  printf("Recognized: %s with prob %d\n", hyp, ps_get_prob(ps, NULL));

/* Free the stuff */
  fclose(fh);
  ps_free(ps);
  return 0;
}


