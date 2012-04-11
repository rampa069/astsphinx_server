#include "prefork.h"
#include "astsphinx.h"
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//#include <cmd_ln.h>
#include <pocketsphinx.h>

int ARGC;
char **ARGV;
int rv;



char grammdir[255]="/opt/etc/astsphinx/grammar/";
char tempFile[255];



static const char *strtolower( const char *str )
 {
  char *s;
 
  for ( s = str; *s; ++s )
  *s = tolower( *s );
 
  return str;
 }


char *reqtype_to_string(enum e_reqtype e)
{
  switch(e)
  {
    case REQTYPE_GRAMMAR:
      return "GRAMMAR";
    case REQTYPE_START:
      return "START";
    case REQTYPE_DATA:
      return "DATA";
    case REQTYPE_FINISH:
      return "FINISH";
    default:
      return "UNKNOWN";
  }
}



void testserver(int sock)
{

  char **fn = &ARGV[3];

  fsg_model_t *m = NULL;

  
  cmd_ln_t *cmdln = cmd_ln_parse_file_r(NULL, ps_args(), ARGV[2], FALSE);
  if(cmdln == NULL)
  {
    printf("Unable to parse config file %s\n", ARGV[2]);
    return;
  }

  ps_decoder_t *p = ps_init(cmdln);

  if(p == NULL)
  {
    printf("Unable to allocate decoder.\n");
    return;
  }
  fsg_set_t *fsgset = ps_get_fsgset(p);
 
  printf("Initializing grammars.\n");

  int x;

#if defined(GRAMDIR)
  strcpy(grammdir,GRAMDIR);
#endif 


  for(x=3; x<ARGC; x++,fn++)
  {
    strcpy(tempFile,grammdir);
    strcat(tempFile,*fn);
    strcat(tempFile,".gram");
    
    printf("Loading grammar '%s'\n", *fn);

   

    jsgf_t *jsgf_grammar = jsgf_parse_file(tempFile, NULL);

    if (jsgf_grammar) {
                     jsgf_rule_iter_t * itor;
                     jsgf_rule_t *rule = NULL;
                          
                     for (itor = jsgf_rule_iter(jsgf_grammar); itor; itor = jsgf_rule_iter_next(itor)) {
                         rule = jsgf_rule_iter_rule(itor);
                                                                              
                         if (jsgf_rule_public(rule))
                         break;
                      }
                
                 m = jsgf_build_fsg(jsgf_grammar, rule, ps_get_logmath(p), 6.5);
                 if (m) {
                                  fsgset = ps_get_fsgset(p);
                                  //fsgset->pnode_active = m;
                                  fsg_set_add(fsgset, basename(*fn), m); 
                                  fsg_set_select(fsgset, *fn) ;
                                  ps_update_fsgset(p);
                                  printf("Finalizada conversion y carga de gramatica %s\n",*fn);
                                          
                        }
                }
                  
    
    if(m == NULL)
    {
      printf("Unable to set grammar.\n");
	}
    else
    {
      printf("Adding %s to fsgset.\n", *fn);
      if(fsgset == NULL) 
      {
        printf ("Failed to fetch fsgset.\n");
        
      }
      else
      {
//        fsg_set_add(fsgset, *fn, m);
//        fsg_set_select(fsgset, *fn) ;
//        ps_update_fsgset(p);
        
               
      }
    }
  }

  

  // SOCKET ACCEPT HERE
  printf("Waiting on connection.\n");
  int client = accept(sock, NULL, NULL);
  printf("Accepted connection.\n");

  //rv = ps_start_utt(ps, "goforward");

  
  printf("Start decoding\n");
  
  int bcount, dlen;
  char buf[ASTSPHINX_BUFSIZE];
  enum e_reqtype rtype;
  const char *uttid = NULL;
  
  while(1)
  {
    bcount = read(client, &dlen, sizeof(dlen));
    if(bcount != sizeof(dlen))
    {
      printf("Error, read %d bytes, expecting %d (%s)\n", bcount, sizeof(dlen), strerror(errno));
      break;
    }

    bcount = read(client, &rtype, sizeof(rtype));
    if(bcount != sizeof(rtype))
    {
      printf("Error, read %d bytes, expecting %d (%s)\n", bcount, sizeof(rtype), strerror(errno));
      break;
    }
 
     printf("Got dlen: %d for request of type %s\n", dlen, reqtype_to_string(rtype));

    if(dlen > ASTSPHINX_BUFSIZE)
    {
      printf("Error, got dlen of %d but buffer is only %d\n", dlen, ASTSPHINX_BUFSIZE);
      break;
    }

    if(dlen)
    {
      bcount = read(client, buf, dlen);
      if(bcount != dlen)
      {
        printf("Error, read %d bytes, expecting %d (%s)\n", bcount, dlen, strerror(errno));
        break;
      }
    }

    const char * hyp = NULL;
    int32 score = 0;
    int buflen = 0;
    switch(rtype)
    {
      case REQTYPE_START:
        break;

      case REQTYPE_GRAMMAR:
        if(uttid != NULL) // have utterance so stop it first.
        {
          printf("Stopping running decode to switch grammars.\n");
          ps_end_utt(p);
          uttid = NULL;
        }

        printf("Request to switch grammar to '%s'\n", buf);

        fsg_set_select(fsgset, buf);
        printf("Ejecutado fsg_set_select\n");        
                                        
        //ps_update_fsgset(p);
        break;

      case REQTYPE_DATA:
        if(dlen)
        {
          if(uttid == NULL)  // No utt, so start one first.
          {
            printf("No utt, so start one first.\n");
            if(ps_start_utt(p, "goforward") != 0)
            {
              printf("Error starting decoding.\n");
            }
          }

          printf("Ejecutado ps_start_utt.\n");            

          if(ps_process_raw(p, (int16 *)buf, dlen / 2, 0, 0) < 0)
          {
            printf("Error decoding raw data\n");
          }
          hyp = ps_get_hyp(p, &score, &uttid);
          int32 modscore = abs(score) / 30000;
          if(modscore > 1000)
            modscore = 1000;
          if(hyp != NULL && strlen(hyp) != 0)
          {
            printf("Got hyp: %05d %010d '%s'\n", modscore, score, hyp);
            *(int32 *)buf=modscore;
            strncpy(buf+sizeof(int32), strtolower(hyp), ASTSPHINX_BUFSIZE-sizeof(int32));
           // int32_t conf = ps_get_prob(p, NULL);
            //printf("Got confidence score of: %d\n", conf);
            //*(int32 *)buf=conf;
            buflen = sizeof(int32) + strlen(hyp);
          }
          break;
        }
      case REQTYPE_FINISH:
        printf("Finalizing and getting end hypothesis.\n");
        if(uttid == NULL)
        {
          printf("Error - cannot finalize when no processing occuring.\n");
        }

        if(ps_process_raw(p, NULL, 0, 0, 0) < 0)
        {
          printf("Error decoding raw data\n");
        }

        if(ps_end_utt(p) < 0)
        {
          printf("Error ending processing.\n");
        }
        hyp = ps_get_hyp(p, &score, &uttid);
        int32 modscore = abs(score) / 30000;
        if(modscore > 1000)
          modscore = 1000;
        if(hyp != NULL && strlen(hyp) != 0)
        {
          printf("Final hyp: %05d %010d '%s'\n", modscore, score, hyp);
          *(int32 *)buf=modscore;
          strncpy(buf+sizeof(int32), strtolower(hyp), ASTSPHINX_BUFSIZE-sizeof(int32));
          
          int32 conf = ps_get_prob(p, &uttid);
          printf("Got confidence score of: %d\n", conf);
          *(int32 *)buf=conf;
          buflen = sizeof(int32) + strlen(hyp);
        }
        else
        {
          printf("No hypothesis made.\n");
          buf[0] = '\0';
        }
        uttid = NULL;
        break;
    }

    bcount = write(client, &buflen, sizeof(int));
    if(bcount != sizeof(int))
    { 
      printf("Error, wrote %d bytes, expecting %d (%s)\n", bcount, sizeof(int), strerror(errno));
      break;
    }

    bcount = write(client, buf, buflen);
    if(bcount != buflen)
    { 
      printf("Error, wrote %d bytes, expecting %d (%s)\n", bcount, buflen, strerror(errno));
      break;
    }

  }

  //close(logf);

  if(p != NULL)
  {
    printf("Freeing p, new ref count is %d\n", ps_free(p));
  }
  close(client);
  return;
}

int main(int argc, char *argv[])
{
  ARGC = argc;
  ARGV = argv;
  if(ARGC < 3)
  {
    printf("Usage: %s LISTENPORT SPHINXCONFIG GRAMMARFILE GRAMMARFILE ...\n", argv[0]);
    return 0;
  }

  int bindport=0;
  sscanf(argv[1], "%d", &bindport);
  printf("Listening on port: %d\n", bindport);

  prefork_listen(bindport, &testserver);
  return 0;
}
