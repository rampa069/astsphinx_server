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


if (argc==1)
  {
   printf("Usage: %s <file.wav>\n",argv[0]);
   return (-1);
  }

/* Initializing of the configuration */
  config = cmd_ln_init(NULL, ps_args(), TRUE,
                       "-samprate", "8000",
                       "-jsgf", "/opt/etc/astsphinx/grammar/digits-es-4.gram",
                       "-dict", "/opt/etc/astsphinx/dict.es",
                       "-hmm", "/opt/sphinx/es_ES/",
                       NULL);
  ps = ps_init(config);

/* Open audio file and start feeding it into the decoder */
  fh = fopen(argv[1], "rb");
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


