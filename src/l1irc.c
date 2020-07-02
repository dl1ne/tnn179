#include "tnn.h"
#ifdef L1IRC
#include "conversd.h"

#define IRC_NAMESIZE       63

char ircloginbuf[2048];

static T_INTERFACE *ifpp;               /* Zeiger auf das Aktuelle Interface.*/

void IrcAwayCommand(CONNECTION *cp);                  /* Username aufloesen. */
void IrcChannelCommand(CONNECTION *cp);                 /* CHANNEL-Kommando. */
void IrcHelpCommand(CONNECTION *cp);                        /* Help Kommand. */
void IrcLinksCommand(CONNECTION *cp);                     /* Links-Kommando. */
void IrcListCommand(CONNECTION *cp);                       /* List-Kommando. */
void IrcNickCommand(CONNECTION *cp);                  /* Nickname aufloesen. */
void IrcModeCommand(CONNECTION *cp);                       /* Mode-Kommando. */
void IrcNamesCommand(CONNECTION *cp);                     /* Names-Kommando. */
void IrcPartCommand(CONNECTION *cp);                  /* Channel schliessen. */
void IrcPersonalCommand(CONNECTION *cp);   /* Personal-Text setzen/loeschen. */
void IrcPingCommand(CONNECTION *cp);
void IrcPongCommand(CONNECTION *cp);
void IrcPrivmsgCommand(CONNECTION *cp);                     /* MSG-Kommando. */
void IrcQuitCommand(CONNECTION *cp);                       /* Quit-Kommando. */
void IrcUserHostCommand(CONNECTION *cp);                                  /* */
void IrcSquitCommand(CONNECTION *cp);
void IrcUserCommand(CONNECTION *cp);
void IrcWhoCommand(CONNECTION *cp);                         /* Who-Kommando. */
void IrcWhoisCommand(CONNECTION *cp);                 /* Userdaten ausgeben. */
void IrcTopicCommand(CONNECTION *cp);                 /* Topic setzen.       */

typedef struct cmdtable_irc
{
  char *name;
  void (*fnc)(CONNECTION *);
  char *help;
} CMDTABLE_IRC;

CMDTABLE_IRC cmdtable_irc[] =
{
 {"away",        IrcAwayCommand,      "AWAY"},
 {"channel",     IrcChannelCommand,   "CHAN"},
 {"help",        IrcHelpCommand,      "HELP"},
 {"nick",        IrcNickCommand,      "NICK"},
 {"join",        IrcChannelCommand,   "JOIN"},
 {"links",       IrcLinksCommand,     "LINKS"},
 {"list",        IrcListCommand,      "LIST"},
 {"mode",        IrcModeCommand,      "MODE"},
 {"names",       IrcNamesCommand,     "NAMES"},
 {"part",        IrcPartCommand,      "PART"},
 {"personal",    IrcPersonalCommand,  "PERS"},
 {"ping",        IrcPingCommand,      "PING"},
 {"pong",        IrcPongCommand,      "PONG"},
 {"privmsg",     IrcPrivmsgCommand,   "PRIVMSG"},
 {"quit",        IrcQuitCommand,      "QUIT"},
 {"user",        IrcUserCommand,      "USER"},
 {"userhost",    IrcUserHostCommand,  "USERHOST"},
 {"topic",       IrcTopicCommand,     "TOPIC"},
 {"squit",       IrcSquitCommand,     "SQUIT"},
 {"who",         IrcWhoCommand,       "WHO"},
 {"whois",       IrcWhoisCommand,     "WHOIS"},
 {NULL,          0,                    NULL}
};

/* IRC-Server Einstellung aendern/setzen. */
void ccpirc(void)
{
  MBHEAD         *mbp;
  char            ch;
  int             tmp_fd       = EOF;
  int             newloglevel  = 0;
  int             new_tcp_port = 0;
  int             i;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(ccpirc)";
#endif /* DEBUG_MODUS */

  ifpp = &ifp[IRC_ID];                  /* Dann Zeiger auf das IRC-Interface. */

  if (issyso() && skipsp(&clicnt, &clipoi))           /* Sysop will aendern.  */
  {
    clicnt--;
    ch = toupper(*clipoi++);

    switch (ch)
    {
      case 'P':                             /* Sysop will IRC-Port aéndern */
        if (!skipsp(&clicnt, &clipoi))
        {
          putmsg("Invalid Parameter\r");
          return;
        }

        new_tcp_port = nxtlong(&clicnt, &clipoi);      /* Neuen Port ermitteln */

        if (  (new_tcp_port < 1)
            ||(new_tcp_port > 65535))
        {
          putmsg("TCP-Port not valid, not changed !!!\r");
          return;
        }

        if (ifpp->actively == FALSE)                    /* IRC ist deaktiv.*/
        {
          putmsg("TCP-Port haven not switched on!\r");
          return;
        }
        /* Wenn NEUER Port und ALTER Port gleich sind, nicht neu Initialisieren.*/
        if (ifpp->tcpport == Htons((unsigned short)new_tcp_port))
        {
          putmsg("TCP-Port successfully changed\r");
          return;
        }

                                           /* Neuen TCP-Port Initialisieren. */
        if ((tmp_fd = SetupTCP(ifpp->name, (unsigned short)new_tcp_port)) != EOF)
        {
#ifdef OS_STACK
          int tmp_fd_OS;

          if ((tmp_fd_OS = SetupOS(ifpp, (unsigned short)new_tcp_port)) != EOF)
          {
            /* alten Socket schliessen */
            close(ifpp->OsSock);
            /* Neuen Socket merken */
            ifpp->OsSock = tmp_fd_OS;
          }
          else
            {
              Close(tmp_fd);
              putmsg("ERROR: Changing UDP-Port failed, Port not changed !!!\r");
              return;
            }
#endif /* OS_STACK */

          Close(ifpp->ISock);                   /* Alten Socket schliessen. */
          ifpp->ISock = tmp_fd ;                     /* Neuen Socket merken */
        }
        else
          {              /* Neuen TCP-Port liess sich nicht Initialisieren. */
            putmsg("ERROR: Changing UDP-Port failed, Port not changed !!!\r");
            return;
          }


        ifpp->tcpport = Htons((unsigned short)new_tcp_port);

        putmsg("TCP-Port successfully changed\r");
        return;

      case 'L':                                /* Sysop will LOGLevel aendern */
       if (!skipsp(&clicnt, &clipoi))
       {
         mbp = putals("IRC-Server:\r");
         putprintf(mbp, "My LogLevel: %d\r",ifpp->log); /* Loglevel anzeigen. */

         prompt(mbp);
         seteom(mbp);
         return;
       }

       newloglevel = nxtnum(&clicnt, &clipoi);   /* neuen Loglevel ermitteln. */

       if (  (newloglevel < 0)
           ||(newloglevel > 3))                     /* Pruefe neuen Loglevel. */
       {
         mbp = putals("IRC-Server:\r");
         putprintf(mbp, "Error: Log level worth from 0 to 3!\r");
         prompt(mbp);
         seteom(mbp);
         return;
       }

       ifpp->log = newloglevel;         /* Neuen Loglevel setzen und zeigen */
       mbp = putals("IRC-Server:\r");
       putprintf(mbp, "My Loglevel: %d\r", ifpp->log);

       prompt(mbp);
       seteom(mbp);
       return;
      break;


      default:                                      /* Ungueltiger Parameter. */
        putmsg("Invalid Parameter\r");
       return;
     }
  }

  mbp = putals("IRC-Server:\r\r*** Connect-Statistik ***\rDatum-----------IP--------------Call---\r");

  ifpp = &ifp[IRC_ID];

  /* Alle TCPIP-Aktivitaeten ausgeben */
  for (i = 0; i < ifpp->tcpstat; ++i)
    ShowTCP(tcpstat_tbl[ifpp->Interface][i], mbp, KISS_IRC);

  putprintf(mbp, "\r*** IRC-Server Einstellungen ***\r");

  putprintf(mbp, "My TCP-Port: %u\r", Ntohs(ifpp->tcpport));
  putprintf(mbp, "My LogLevel: %d\r",ifpp->log); /* Loglevel anzeigen. */
  prompt(mbp);
  seteom(mbp);
}

char *get_mode_flags2irc(int flags)
{
  static char mode[16];
  char *p = mode;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(get_mode_flags2irc)";
#endif /* DEBUG_MODUS */

  p = mode;
  if (flags & M_CHAN_I)
    *p++ = 's';

  if (flags & M_CHAN_M)
    *p++ = 'm';

  if (flags & M_CHAN_T)
    *p++ = 't';

  if (flags & M_CHAN_P)
    *p++ = 'i';

  if (flags & M_CHAN_S)
    *p++ = 'p';

  // always +n
  *p++ = 'n';
  //if (flags & M_CHAN_L)
  //*p++ = 'l';
  //*p++ = ' ';

  *p = 0;

  return mode;
}

BOOLEAN hasirc_ChOp(WORD chan)
{
  CONNECTION *p;
  CLIST *cl;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(hasirc_ChOp)";
#endif /* DEBUG_MODUS */

  for (p = connections; p; p = p->next) {
    if (p->type == CT_USER) {
      if (p->channel == chan && p->channelop)
        return(1);

      for (cl = p->chan_list; cl; cl = cl->next)
        if (cl->channel == chan && cl->channelop)
        return(1);
    }
  }
  return(0);
}

CHANNEL *insirc_channel(WORD chan)
{
  CHANNEL *ch, *ch1;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(insirc_channel)";
#endif /* DEBUG_MODUS */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - insirc_channel()\n");
#endif
  ch = (CHANNEL *) calloc(1, sizeof(CHANNEL));

  if (ch) {
    ch->chan = chan;

    if (!channels || channels->chan > chan) {
      ch->next = channels;
      channels = ch;
    } else {
      ch1 = channels;
      while (ch1->next) {
        if (ch1->next->chan > chan) {
          ch->next = ch1->next;
          ch1->next = ch;
          return(ch);
        }
        ch1 = ch1->next;
      }
      if (!ch1->next) {
        ch->next = ch1->next;
        ch1->next = ch;
      }
    }
  }
  return(ch);
}


int stringisnumber(char *p)
{
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(stringisnumber)";
#endif /* DEBUG_MODUS */

        if (!p)
                return 0;
        while(*p && isspace(*p & 0xff))
                p++;
        if (*p == '-')
                p++;
        if (!*p || *p < '0' || *p > '9')
                return 0;
        for (++p; *p && *p >= '0' && *p <= '9'; p++) ;
        if (!*p || isspace(*p & 0xff))
                return 1;
        return 0;
}

char *ts2irc(time_t gmt)
{
        static char buffer[80];
        static char monthnames[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
        struct tm *tm;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(ts2irc)";
#endif /* DEBUG_MODUS */

        tm = localtime(&gmt);
        if (tm) {
                if (gmt + 24 * 60 * 60 > currtime)
                        sprintf(buffer, "%2d:%02d", tm->tm_hour, tm->tm_min);
                else
                        sprintf(buffer, "%-3.3s %2d", monthnames + 3 * tm->tm_mon, tm->tm_mday);
        } else
                strcpy(buffer, "?????");
        return buffer;
}

struct channel *new_channel(int channel, char *fromuser)
{
        struct channel *ch;
        struct channel *chp;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(new_channel)";
#endif /* DEBUG_MODUS */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - new_channel()\n");
#endif


        ch = (CHANNEL *)calloc(1,sizeof(CHANNEL));
        chp = channels;
        if (chp != NULLCHANNEL) {
                ch->next = NULLCHANNEL;
                while ((chp != NULLCHANNEL) && (chp->chan < channel)) {
                        ch->next = chp;
                        chp = chp->next;
                }
                if (ch->next == NULLCHANNEL) {
                        ch->next = chp;
                        channels = ch;
                } else {
                        ch->next->next = ch;
                        ch->next = chp;
                }
        } else {
                ch->next = NULLCHANNEL;
                channels = ch;
        }
        ch->next = chp;
        ch->chan = channel;
        ch->tsetby[0] = '\0';
        ch->time = 0;
        //ch->name[0] = '\0';
        ch->flags = 0;
        ch->ctime = currtime;
        ch->expires = 0L;
        strncpy(ch->createby, fromuser ? fromuser : "", NAMESIZE);
        ch->createby[sizeof(ch->createby)-1] = 0;

        return ch;
}

/******************************************************************************/
/*                                                                            */
/* Eingehende Befehle bearbeiten.                                             */
/*                                                                            */
/******************************************************************************/
void ProcessIrcInput(char *cnvinbuf, CONNECTION *cp)
{
  CMDTABLE_IRC *IrcCmdp;
  char         *cmd;
  char          buffer[2048];
  int           cmdlen;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(ProcessIrcInput)";
#endif /* DEBUG_MODUS */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - ProcessIrcInput()\n");
#endif

  cnvinbuf[2048] = 0;

  cmd = getarg(cnvinbuf, 0);                              /* Befehl einlesen. */
  if (cmd[0] == FALSE)                                        /* kein befehl, */
    return;                                                /* brechen wir ab. */

  cmdlen = strlen(cmd);                            /* Befehlslange ermitteln. */
                                                 /* durchsuche Befehls-Liste. */


#ifdef MOD_L1IRC_C_LINEFEED
if(cmd[cmdlen-1] == '\n')
{
   cmd[cmdlen-1] = 0;
   cmdlen--;
}
#endif /* MOD_L1IRC_C_LINEFEED */

#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - input: %s\n", cmd);
#endif

  for (IrcCmdp = cmdtable_irc; IrcCmdp->name; IrcCmdp++)
  {
    if (!strncmp(IrcCmdp->name, cmd, cmdlen))          /* Befehl vergleichen. */
    {
      (*IrcCmdp->fnc) (cp);                             /* Befehl ausfuehren. */
      return;
    }
  }
                                                       /* Ungueltiger Befehl. */
  sprintf(buffer, ":%s 421 %s *** Unknown command '/%s'. Type /HELP for help.\n"
                , myhostname
                , (*cp->nickname ? cp->nickname : cp->name)
                , cmd);
  appenddirect(cp, buffer);
}

/******************************************************************************/
/*                                                                            */
/* Away (Ab/Anwesend) setzen.                                                 */
/*                                                                            */
/******************************************************************************/
void IrcAwayCommand(CONNECTION *cp)
{
  CONNECTION *p;
  char *AwayText;
  char buffer[2048];
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcAwayCommand)";
#endif /* DEBUG_MODUS */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcAwayCommand()\n");
#endif


  AwayText = getarg(NULL, GET_ALL);              /* akt. Away-Text ermitteln. */

  if (*AwayText == ':')                                     /* ':' im String. */
    AwayText++;                                              /* ':' loeschen. */

  if (AwayText == NULL)                                         /* Kein Text. */
    return;                                               /* Keine Aenderung. */

  for (p = connections; p; p = p->next)
  {
    if (p->type == CT_USER && !p->via && !Strcmp(p->name, cp->name))
        {
      setstring(&(p->away), AwayText, 256); /* Aktualisiere den aktuellen Away-Text. */
      p->atime = currtime;
      p->locked = 1;
    }
  }

  if (*AwayText != '\0')                                         /* Abmelden. */
    sprintf(buffer, ":%s 301 %s %s : %s\n"        /* IRC-Meldung vorbereiten, */
    , myhostname
    , cp->name
    , (*cp->nickname ? cp->nickname : cp->name)
    , AwayText);
  else                                                           /* Anmelden. */
    sprintf(buffer, ":%s 320 %s %s : is back\n"   /* IRC-Meldung vorbereiten. */
    , myhostname
    , cp->name
    , (*cp->nickname ? cp->nickname : cp->name));

  appendstring(cp, buffer);

#ifndef CONVNICK
  send_awaymsg(cp->name,    /* Neuen Away-Text an andere Host's weiterleiten. */
               cp->hosthostname,
               currtime,
               AwayText);
#else
  send_awaymsg(cp->name,    /* Neuen Away-Text an andere Host's weiterleiten. */
               cp->nickname,
               cp->host,
               currtime,
               AwayText);
#endif /* CONVNICK */
}

/******************************************************************************/
/*                                                                            */
/* Persoenlichen Text setzen.                                                 */
/*                                                                            */
/******************************************************************************/
void IrcPersonalCommand(CONNECTION *cp)
{
  char *pers;
  char  buffer[2048];
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcPersonalCommand)";
#endif /* DEBUG_MODUS */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcPersonalCommand()\n");
#endif

  pers = getarg(NULL, GET_ALL);                  /* akt. Pers-Text ermitteln. */

  if (*pers)
  {
    if (!strcmp(pers, "@"))               /* Pers-Text soll geloescht werden. */
      pers = "";                                       /* Pers-Text loeschen. */

    personalmanager(SET, cp, pers);               /* Datenbank aktualisieren. */

    if (pers != cp->pers)
    {
      setstring(&(cp->pers), pers, 256);           /* Neuen Pers-Text setzen. */
      cp->mtime = currtime;                          /* Aktuelle Zeit merken. */

      sprintf(buffer, ":%s 311 %s %s %s %s * :%s\n"
                    , myhostname
                    , cp->nickname
                    , cp->name
                    , (*cp->nickname ? cp->nickname : cp->name)
                    , cp->host
                    , (*pers && *pers != '@') ? pers : "");
      appendstring(cp, buffer);

#ifndef CONVNICK
      send_persmsg(cp->name, /* Neuen Pers-Text an andere HOST's weiterleiten.*/
                   myhostname,
                   cp->channel,
                   pers,
                   cp->time);
#else
      send_persmsg(cp->name, /* Neuen Pers-Text an andere HOST's weiterleiten.*/
                   cp->nickname,
                   myhostname,
                   cp->channel,
                   pers,
                   cp->time);
#endif /* CONVNICK */
    }
  }
}

void IrcNoticeCommand(CONNECTION *cp)
{
  char     buffer[2048];
  int      channel;
  char    *toname, *text;
  CHANNEL *ch;
  CLIST   *cl;
  char    *q;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcNoticeCommand)";
#endif /* DEBUG_MODUS */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcNoticeCommand()\n");
#endif

  *buffer = 0;

  toname = getarg(0, 0);
  if (*toname && (*toname == '#' || *toname == '~'))
    toname++;

  // notice commands should not be answered
  if (!*toname)
    return;

  text = getarg(NULL, GET_ALL);

  cp->locked = 1;

  if (*text == ':')
    text++;
  // dcc mess

  if ((q = strchr(text, '\001')))
  {
    text = q+1;
    if ((q = strchr(text, '\001')))
      *q = 0;

    if (!strncasecmp(text, "AWAY ", 5))
    {
      text = text+5;

      while (*text && isspace(*text & 0xff))
        text++;

      if (strcmp(cp->away, text))
        send_awaymsg(cp->name, cp->nickname, myhostname, currtime, text);

      return;
    }

    if (!strncasecmp(text, "ACTION ", 7))
    {
      text = text+7;

      while (*text && isspace(*text & 0xff))
        text++;

      if (strlen(text) > 384)
        text[384] = 0;
    }
    else
    {
      while (*text && isspace(*text & 0xff))
        text++;

      if (strlen(text) > 384)
        text[384] = 0;

      sprintf(buffer, "*** %s@%s ack :%s:", cp->name, cp->host, text);
    }
  }

  if (!*buffer)
    sprintf(buffer, "*** %s@%s notice: %s", cp->name, cp->host, text);

  if (isdigit(*toname & 0xff) && ((channel = atoi(toname)) > 0 || (channel == 0 && !strcmp(toname, "0"))))
  {
    // to channel. special care must be taken
    for (ch = channels; ch; ch = ch->next)
    {
      if (ch->chan == channel)
        break;
    }

    if (!ch)
      return;

    // only channels we are on (and have permission to talk)
    if (cp->operator == 2)
    {
      ;
    }
    else
      if  (cp->channel == ch->chan)
      {
        if ((ch->flags & M_CHAN_M) && !cp->channelop)
          return;
      }
      else
      {
        for (cl = cp->chan_list; cl; cl = cl->next)
          if (cl->channel == ch->chan)
            break;

          if (!cl)
            return;

          if ((ch->flags & M_CHAN_M) && !cl->channelop)
            return;
      }

      send_msg_to_channel("conversd", (short)channel, buffer);
  }
  else
  {
    send_msg_to_user("conversd", toname, buffer);
  }
}

void IrcChannelCommand(CONNECTION *cp)
{
  char           *chan;
  char            buffer[2048];
  int            newchannel;
  struct channel *ch;
  struct clist   *cl,
                 *cl2;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcChannelCommand)";
#endif /* DEBUG_MODUS */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcChannelCommand()\n");
#endif

  /* Channel holen. */
  chan = getarg(NULL, GET_NXTLC);

  if (*chan == '#')
    chan++;

  newchannel = atoi(chan);

  if (  !stringisnumber(chan)
          || newchannel < MINCHANNEL
          || newchannel > MAXCHANNEL)
  {
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcChannelCommand() - No such channel t1\n");
#endif
    sprintf(buffer, ":%s 403 %s %s :No such channel\n", cp->host,cp->nickname ? cp->nickname : cp->name, chan);
        appendstring(cp, buffer);
        appendprompt(cp, 0);
        return;
  }

#ifdef MOD_L1IRC_C_ALLOWCHANZERO
  if (newchannel == cp->channel && newchannel > 0)
#else /* MOD_L1IRC_C_ALLOWCHANZERO */
  if (newchannel == cp->channel)
#endif /* MOD_L1IRC_C_ALLOWCHANZERO */
  {
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcChannelCommand() - newchannel (%i) = cp->channel (%i)\n", newchannel, cp->channel);
#endif
    return;
  }

  for (ch = channels; ch; ch = ch->next)
  {
    if (ch->chan == newchannel)
    {
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcChannelCommand() - ch->chan (%i) == newchannel (%i)\n", ch->chan, newchannel);
#endif
      break;
    }
  }

  for (cl = cp->chan_list; cl; cl = cl->next)
  {
    if (cl->channel == newchannel)
    {
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcChannelCommand() - cl->channel (%i) == newchannel (%i) \n", cl->channel, newchannel);
#endif
      break;
    }
  }

  if (!cl && ch && (ch->flags & M_CHAN_P) && cp->invitation_channel != newchannel)
  {
    if (cp->operator == 2)
        {
      sprintf(buffer, ":%s 473 %s #%d :Nick/channel is temporarily unavailable. Try again if you really want to join.\n",cp->host,cp->nickname ? cp->nickname : cp->name, newchannel);

          cp->invitation_channel = newchannel;
        }
    else
        {
      sprintf(buffer, ":%s 473 %s #%d :Cannot join channel (+i)\n",cp->host,cp->nickname ? cp->nickname : cp->name, newchannel);
        }

    appendstring(cp, buffer);
    appendprompt(cp, 0);
    sprintf(buffer, "*** (%s) %s@%s tried to join your private channel.", ts2irc(currtime), cp->name, myhostname);
    send_msg_to_channel("conversd", (WORD)newchannel, buffer);
    return;
  }

  if (cl)
  {
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcChannelCommand() - cl\n");
#endif
    return;
  }

  if (!cl)
  {
    // channel delay mechanism (rfc2811)
    if (ch)
        {
/*        if (cp->operator != 2)

                sprintf(buffer, ":%s 437 %s #%d :Channel is temporarily unavailable\n", myhostname, cp->nickname, newchannel);
        appendstring(cp, buffer);
        appendprompt(cp, 0);
        return;
          }*/
        }

    cp->locked = 1;

#ifdef CONVNICK
  send_user_change_msg(cp->name,cp->nickname, cp->host, -1, (WORD)newchannel, cp->pers, cp->time);
#else
  send_user_change_msg(cp->name,cp->host, -1, (WORD)newchannel, cp->pers, cp->time);
#endif /* CONV_NICKNAME */

  if (!ch || ch->expires != 0L)
  {
    cp->locked = 0;
  }

  cl = (CLIST *) calloc(1, sizeof(CLIST));
  cl->next = NULLCLIST;
  cl->channelop = cp->channelop;
  cl->channel = cp->channel;
  cl->time = currtime;
  cp->mtime = currtime;
  cp->chan_list = cl;
#ifndef CONVNICK
  send_user_change_msg(cp->name, cp->host, -1, cp->channel, "", currtime);
#else
  send_user_change_msg(cp->name, cp->name, cp->host, -1, cp->channel, "", currtime);
#endif

  if (!ch || ch->expires != 0L)
  {
    cl->channelop = 1;
  }
  else
  {
    cl->channelop = 0;
  }


  cl->channel = newchannel;

  if (!cp->chan_list || cp->chan_list->channel > newchannel)
  {     /* only occurs with 1st entry */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcChannelCommand() - !cp->chan_list || cp->chan_list->channel > newchannel\n");
#endif
    cl->next = cp->chan_list;
    cp->chan_list = cl;
  }
  else
    for (cl2 = cp->chan_list; cl2; cl2 = cl2->next)
        {
      if (cl2->next)
          {     /* if there IS a next entry */
        if (cl2->next->channel > newchannel)
                {
                  cl->next = cl2->next;
                  cl2->next = cl;
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcChannelCommand() - IF - cl2->next->channel > newchannel\n");
#endif
                  break;
                 }
          }
          else
          {
            cl2->next = cl;
            cl->next = NULLCLIST;
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcChannelCommand() - ELSE - cl2->next->channel > newchannel\n");
#endif
#ifdef MOD_L1IRC_C_KICKONJOIN
/* DL1NE, kick aktuellen Channel, durch neuen Join */
  sprintf(buffer, ":%s!%s@%s PART #%d\n"
                  , (*cp->nickname ? cp->nickname : cp->name)
                  , cp->name
                  , myhostname
                  , cp->channel);
  appendstring(cp, buffer);
#endif /* MOD_L1IRC_C_KICKONJOIN */
            break;
          }
        }
  }

  cp->channel = newchannel;

  sprintf(buffer,":%s!%s@%s JOIN :#%d\n"
                    ,(*cp->nickname ? cp->nickname : cp->name)
                                , cp->name
                                , cp->host
                                , cp->channel);
  appendstring(cp, buffer);

  sprintf(buffer, ":%s 329 %s #%d %ld\n"
                    ,cp->host
                                ,(*cp->nickname ? cp->nickname : cp->name)
                                ,cp->channel
                                ,(ch ? ch->ctime : currtime));
  appendstring(cp, buffer);

  sprintf(buffer, ":%s MODE #%d +%s\n", myhostname, cp->channel, get_mode_flags2irc((ch ? ch->flags : 0)));
  appendstring(cp, buffer);


  if (ch && ch->expires == 0L)
  {
    if (ch->topic != NULL)
        {
      sprintf(buffer, ":%s 332 %s #%d :%s",cp->host,cp->nickname ? cp->nickname : cp->name, cp->channel, ch->topic);
      buffer[IRC_MAX_MSGSIZE] = 0;
      appendstring(cp, buffer);
      appendstring(cp, "\n");
      sprintf(buffer, ":%s 333 %s #%d %s %ld\n",cp->host,cp->nickname ? cp->nickname : cp->name, cp->channel, ch->tsetby, ch->time);
          appendstring(cp,buffer);
        }

        //cp->channelop = cl->channelop;
       cp->channelop = (hasirc_ChOp((WORD)newchannel)) ? 0 : 1;
  }
  else
  {
    cp->channelop = 1;

        if (ch)
        {
          strncpy(ch->createby, cp->name, NAMESIZE);
      ch->createby[sizeof(ch->createby)-1] = 0;
      ch->ctime = currtime;
          ch->expires = 0L;
        }
        else
        {
      new_channel(cp->channel, cp->name);
        }
  }

  sprintf(cp->ibuf, "NAMES #%d", cp->channel);
  getarg(cp->ibuf, 0);
  IrcNamesCommand(cp);
  appendprompt(cp, 0);
}

void IrcModeCommand(CONNECTION *cp)
{
  CONNECTION *p;
  CHANNEL    *ch;
  CLIST      *cl;
  char        buffer[2048];
  char       *arg,
             *c;
  int         remove   = 0;
  int         channel  = 0;
  WORD        oldflags = 0;
  int         op       = 0;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcModeCommand)";
#endif /* DEBUG_MODUS */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcModeCommand()\n");
#endif

  arg = getarg(0, 0);

  if (*arg == ':')
    arg++;

  if (*arg == '#')
    arg++;
  else
  {
    if (cp->IrcMode)
    {
      if (!strcmp(arg, "*"))
        return;

      if (cp->type == CT_USER)
      {
        char *arg2 = getarg(0, 0);

        if (!*arg)
        {
          sprintf(buffer, ":%s 461 %s MODE :Not enough parameters\n"
                        , cp->host
                        , (cp->nickname ? cp->nickname : cp->name));
          appendstring(cp, buffer);
          return;
        }

        if (  strcasecmp(arg, cp->nickname)
                        /*||strcasecmp(arg, cp->name)*/)
        {
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcChannelCommand() - No such channel t2\n");
#endif
            sprintf(buffer, ":%s 403 %s %s :No such channel\n"
                          , cp->host
                          , (cp->nickname ? cp->nickname : cp->name)
                          , arg);
            appendstring(cp, buffer);
          return;
        }

        *buffer = 0;

        if (!*arg2)
        {
          sprintf(buffer, ":%s 211 %s %s\n"
                        , myhostname
                        , cp->nickname
                        , (cp->verbose) ? "+sw" : "");
        }
        else
        {
          if (strstr(arg2, "+s") || strstr(arg2, "+w"))
            cp->verbose = 1;
          else
            if (strstr(arg2, "-s") || strstr(arg2, "-w"))
              cp->verbose = 0;
            else
            {
              sprintf(buffer, ":%s 501 %s :Unknown MODE flag: %s\n"
                            , cp->host
                            , (cp->nickname ? cp->nickname : cp->name)
                            , arg2);
            }

            if (!*buffer)
              sprintf(buffer, ":%s MODE %s %csw\n"
                            , myhostname
                            , cp->nickname
                            , (cp->verbose ? '+' : '-'));

            appendstring(cp, buffer);
        }
        return;
      }
    }
  }

  channel = atoi(arg);

  if (*arg && !strcmp(arg, "*"))
  {
    channel = cp->channel;
  }
  else
  {
    if (!*arg || !(isdigit(*arg & 0xff) && (channel > 0 || (!channel && !strcmp(arg, "0")))))
    {
      if (cp->type != CT_HOST)
      {
        if (*arg)
        {
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcChannelCommand() - No such channel t3\n");
#endif
          sprintf(buffer, ":%s 403 %s #%s :No such channel\n"
                        , cp->host
                        , (cp->nickname ? cp->nickname : cp->name)
                        , arg);
        }
        else
        {
          sprintf(buffer, ":%s 461 %s MODE :Not enough parameters\n"
                        , cp->host
                        , (cp->nickname ? cp->nickname : cp->name));
        }

        appendstring(cp, buffer);
        appendprompt(cp, 0);
      }

      return;
    }

    arg = getarg(0, 1);

    if (!*arg && cp->type == CT_USER)
    {
          for (ch = channels; ch; ch = ch->next)
          {
        if (ch->chan == channel)
                {
                  if (ch->expires != 0L)
                  {
            ch = 0;
            break;
                  }

                  sprintf(buffer, ":%s 324 %s #%d +%s\n",
                        cp->host,
                cp->nickname ? cp->nickname : cp->name, channel, get_mode_flags2irc(ch->flags));
          appendstring(cp, buffer);

          sprintf(buffer, ":%s 329 %s #%d %ld\n",
                        cp->host,
                cp->nickname ? cp->nickname : cp->name, channel, ch->ctime);
          appendstring(cp, buffer);
          appendprompt(cp, 0);
          break;
                }
          }

          if (!ch)
          {
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcChannelCommand() - No such channel t4, channel: %i\n", channel);
#endif /*
        sprintf(buffer, ":%s 403 %s #%d :No such channel\n",
              cp->host,
                          cp->nickname ? cp->nickname : cp->name, channel);
        appendstring(cp, buffer);
                appendprompt(cp, 0); */
          }
        }

  }


    for (cl = cp->chan_list; cl; cl = cl->next)
    {
      if (cl->channel == channel)
      {
        if (cl->channelop)
          op++;
        break;
      }
    }

    for (ch = channels; ch; ch = ch->next)
    {
      if (channel == ch->chan)
        break;
    }



      oldflags = ch->flags;

      while (*arg)
      {
        switch (*arg)
        {
        case '+':
          {
            remove = 0;
            arg++;
            break;
          }

        case '-':
          {
            remove = 1;
            arg++;
            break;
          }

        case 'S':
        case 's':
          {
            if (remove && ch)
            {
              if (!cp->IrcMode)
              {
                ch->flags -= (ch->flags & M_CHAN_S);
              }
              else
              {
                ch->flags -= (ch->flags & M_CHAN_I);
              }
            }
            else
            {
              if (!cp->IrcMode)
              {
                ch->flags |= M_CHAN_S;
              }
              else
              {
                ch->flags |= M_CHAN_I;
              }
            }

            arg++;
            break;
          }

        case 'P':
        case 'p':
          {
            if (remove && ch)
            {
              if (!cp->IrcMode)
              {
                ch->flags -= (ch->flags & M_CHAN_P);
              }
              else
              {
                ch->flags -= (ch->flags & M_CHAN_S);
              }
            }
            else
            {
              if (!cp->IrcMode)
              {
                ch->flags |= M_CHAN_P;
              }
              else
              {
                ch->flags |= M_CHAN_S;
              }
            }

            arg++;
            break;
          }

        case 'T':
        case 't':
          {
            if (remove && ch)
            {
              ch->flags -= (ch->flags & M_CHAN_T);
            }
            else
            {
              ch->flags |= M_CHAN_T;
            }

            arg++;
            break;
          }

        case 'I':
        case 'i':
          {
            if (remove && ch)
            {
              if (!cp->IrcMode)
              {
                ch->flags -= (ch->flags & M_CHAN_I);
              }
              else
              {
                ch->flags -= (ch->flags & M_CHAN_P);
              }
            }
            else
            {
              if (!cp->IrcMode)
              {
                ch->flags |= M_CHAN_I;
              }
              else
              {
                ch->flags |= M_CHAN_P;
              }
            }

            arg++;
            break;
          }

        case 'L':
        case 'l':
          {
            if (!cp->IrcMode)
            {
              if (remove && ch)
              {
                ch->flags -= (ch->flags & M_CHAN_L);
              }
              else
              {
                ch->flags |= M_CHAN_L;
              }
            }
            else
            {
              sprintf(buffer, ":%s 472 %s %c :is unknown mode char to me\n"
                            , cp->host
                            , (cp->nickname ? cp->nickname : cp->name)
                            , *arg);
              appendstring(cp, buffer);
            }

            arg++;
            break;
          }

        case 'M':
        case 'm':
          {
            if (remove && ch)
            {
              ch->flags -= (ch->flags & M_CHAN_M);
            }
            else
            {
              ch->flags |= M_CHAN_M;
            }

            arg++;
            break;
          }

        case 'O':
        case 'o':
          {
            arg++;
            while (*arg)
            {
              CONNECTION *p_found = 0;
              int         status_really_changed = 0;
              char       *fromname = cp->name;
              char       *fromnickname = (cp->type == CT_HOST ? cp->name : cp->nickname);
              int         user_found = 0;

              while (*arg == ' ')
                arg++;

              if (*arg == ':')
                arg++;

              c = arg;

              while (*c != ' ')
              {
                if (*c != '\0')
                {
                  c++;
                }
                else
                  break;
              }

              if (*c != '\0')
              {
                *c++ = '\0';
              }

              if (cp->type != CT_HOST && channel < 0 )
                continue;

              clear_locks();

              if (cp->type == CT_HOST)
                cp->locked = 1;

              for (p = connections; p; p = p->next)
              {
                if (p->type != CT_USER)
                  continue;

                if (!(!strcasecmp(p->name, arg) || (cp->type == CT_USER && !strcasecmp(p->nickname, arg))))
                  continue;

                user_found = 1;

                if (channel == -1)
                {
                  if (!p_found || !p->operator)
                    p_found = p;

                  if (!status_really_changed)
                  {
                    status_really_changed = (p->operator == 0);
                  }
                }
                else
                {
                  if (p->channel == channel)
                  {
                    if (!p_found || !p->channelop)
                      p_found = p;

                    if (!status_really_changed)
                      status_really_changed = (p->channelop == 0);

                    p->channelop = 1;
                  }

                  for (cl = p->chan_list; cl; cl = cl->next)
                  {
                    if (cl->channel == p->channel)
                    {
                      if (!p_found || !p->channelop)
                        p_found = p;

                      if (!status_really_changed)
                        status_really_changed = (cl->channelop == 0);

                      cl->channelop = 1;
                    }
                  }
                }
              }

              if (p_found)
              {
                send_opermsg(fromnickname, p_found->host, fromname, (WORD)channel, 0);
              }
              else
              {
                if (cp->type == CT_USER)
                {
                  if (cp->IrcMode)
                  {
                    if (user_found)
                      sprintf(buffer, ":%s 441 %s %s #%d :They aren't on that channel\n"
                                    , cp->host
                                    , cp->nickname
                                    , arg
                                    , channel);
                    else
                      sprintf(buffer, ":%s 401 %s %s :No such nick\n"
                                    , cp->host
                                    , cp->nickname
                                    , arg);

                    appendstring(cp, buffer);
                  }
                  else
                  {
                    if (user_found)
                      sprintf(buffer, "*** User not in channel: %s.\n"
                                    , arg);
                    else
                      sprintf(buffer, "*** No such user: %s.\n"
                                    , arg);

                    appendstring(cp, buffer);
                  }
                }
              }

              cp->locked = 1;
              arg = c;
            }
            break;

          }

        case 'N':
//        case 'B':
//        case 'b':
        case 'V':
        case 'v':
        case 'K':
        case 'k':
        case 'W':
        case 'w':
          {
            if (cp->IrcMode)
            {
              sprintf(buffer, ":%s 472 %s :Unknown MODE flag (%c).\n"
                            , cp->host
                            , (cp->nickname ? cp->nickname : cp->name)
                            , *arg);
              appendstring(cp, buffer);
            }

            arg++;
            break;
          }

        case 'n':
        default:
          {
            arg++;
            break;
          }
        }
      }

      if (ch && (ch->flags != oldflags))
      {
        clear_locks(); // send_opermsg may have locked it..
        cp->locked = 1;

        if (cp->type == CT_USER && !cp->IrcMode)
        {
          appendstring(cp, "*** Flags: ");
          appendstring(cp, get_mode_flags2irc(ch->flags));
          appendstring(cp, "\n");
        }

#ifndef L1IRC
        send_mode(ch);
#else
        send_mode(ch, cp, oldflags);
#endif /* L1IRC */
      }


  if (cp->type == CT_USER)
  {
    appendprompt(cp, 0);
  }
}

/* Private MSG verschicken. */
void IrcPrivmsgCommand(CONNECTION *cp)
{
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcPrivmsgCommand)";
#endif /* DEBUG_MODUS */

  msg_command(cp);
}

/* Channel schliessen. */
void IrcPartCommand(CONNECTION *cp)
{
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcPartCommand)";
#endif /* DEBUG_MODUS */

  leave_command(cp);
}

/* Help Kommando */
void IrcHelpCommand(CONNECTION *cp)
{
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcHelpCommand)";
#endif /* DEBUG_MODUS */

  help_command(cp);
}

/* Quit Kommando */
void IrcQuitCommand(CONNECTION *cp)
{
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcQuitCommand)";
#endif /* DEBUG_MODUS */

  bye_command(cp);
}

void IrcPingCommand(CONNECTION *cp)
{
  char *arg;
  char buffer[2048];
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcPingCommand)";
#endif /* DEBUG_MODUS */

  arg = getarg(0, 0);

  arg[cp->width] = 0;

  sprintf(buffer, ":%s PONG %s :%s\n", myhostname, myhostname, arg);
  appendstring(cp, buffer);
}

void IrcPongCommand(CONNECTION *cp)
{
  char   buffer[1024];
  char  *line;
  time_t response_time = 0L;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcPongCommand)";
#endif /* DEBUG_MODUS */

  line = getarg(0, 0);

  if (*line == ':')
    line++;

  if (isalpha(*line))
  {
    sprintf(buffer, "PING :%ld\n", currtime);
    appendstring(cp, buffer);
    return;
  }

  sscanf(line, "%ld", &response_time);

  if (response_time < 1L)
  {
    sprintf(buffer, ":%s 375 %s bad answer, sorry\n"
                  , myhostname
                  , (*cp->nickname ? cp->nickname : cp->name));
    appendstring(cp, buffer);
  }
  else
  {
    ts2();
    sprintf(buffer, ":%s 375 %s *** rtt %s <-> %s is %s\n"
                  , myhostname
                  , (*cp->nickname ? cp->nickname : cp->name)
                  , myhostname
                  , (*cp->name ? cp->name : "you")
                  , ts4(currtime - response_time));
    appendstring(cp, buffer);
  }
  appendprompt(cp, 0);
}

/* Alle User auf den aktuellen Channel bereitstellen. */
void IrcNamesCommand(CONNECTION *cp)
{
  CONNECTION *p;
  CHANNEL    *ch;
  CLIST      *cl2;
  char        buffer[BUFSIZ];
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcNamesCommand)";
#endif /* DEBUG_MODUS */

  /* IRC-Meldung vorbereiten. */
  sprintf(buffer,":%s 353 %s = #%d :"
                ,myhostname
                ,cp->nickname
                ,cp->channel);
  /* Und abschicken. */
  appendstring(cp,buffer);

  for (ch = channels; ch; ch = ch->next)
  {
    /* Aktueller Channel? */
    if (ch->chan != cp->channel)
      /* Nein, zum naechsten. */
      continue;

    for (p = connections; p; p = p->next)
    {
      /* Nur angemeldete User. */
      if (p->type == CT_USER)
      {
       /* if (ch->chan != cp->channel)
          break;*/
        /* User im gesuchten Channel. */
        if (p->channel != ch->chan)
        {
          /* Nein, dann suchen wir in der Channel-List. */
          for (cl2 = p->chan_list; cl2; cl2 = cl2->next)
          {
            /* User im gesuchten Channel. */
            if (cl2->channel == ch->chan)
              /* Eintrag gefunden. */
              break;
          }

          /* Kein Eintrag gefunden? */
          if (!cl2)
            /* Dann zum naechsten. */
            continue;
        }

        /* Username - IRC-Meldung vorbereiten. */
        sprintf(buffer, "%s%s "
                      ,(p->channelop ? "@" : "")
                      ,(*p->nickname ? p->nickname : p->name));
        /* Und abschicken. */
        appendstring(cp,buffer);
      }
    }
  }
  /* LISTENDE - IRC-Meldung vorbereiten. */
  sprintf(buffer, "\n:%s 366 %s #%d\n"
                ,myhostname
                ,cp->nickname
                ,cp->channel);
  /* Und abschicken. */
  appendstring(cp, buffer);
}

/* Auflistung aller User im angegebenen Kanal. */
void IrcWhoCommand(CONNECTION *cp)
{
  CONNECTION *p;
  CHANNEL    *ch;
  CLIST      *cl2;
  char        buffer[BUFSIZ];
  char       *chan;
  WORD        Channel;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcWhoCommand)";
#endif /* DEBUG_MODUS */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcWhoCommand()\n");
#endif

  /* Gesuchten Channel holen. */
  chan = getarg(NULL, GET_NXTLC);

  /* 1. Zeichen eine "#"? */
  if (*chan == '#')
    /* "#" entfernen. */
    chan++;

  Channel = atoi(chan);

  for (ch = channels; ch; ch = ch->next)
  {
    for (p = connections; p; p = p->next)
    {
      /* Nur angemeldete User. */
      if (p->type == CT_USER)
      {
        /* Unser Channel? */
        if (ch->chan != Channel)
          /* Zum naechsten. */
          break;

        /* Hat unser User den gesuchten Channel? */
        if (p->channel != ch->chan)
        {
          /* Nein, dann schauen wir noch in die Channel-Liste. */
          for (cl2 = p->chan_list; cl2; cl2 = cl2->next)
          {
            /* Hat unser User den gesuchten Channel? */
            if (cl2->channel == ch->chan)
              /* Channel gefunden. */
              break;
          }
          /* Eintrag gefunden. */
          if (!cl2)
            /* Nein, weitersuchen. */
            continue;
        }

        /* IRC-Meldung vorbereiten. */
        sprintf(buffer,":%s 352 %s %d %s%s %s %s %s %s%s%s :%d %s\n"
                      ,myhostname
                      ,cp->nickname
                      ,p->channel
                      ,""
                      ,p->name
                      ,p->host
                      ,p->host
                      ,p->nickname
                      ,(p->away ? "G" : "H")
                      ,""
                      ,(p->operator ? "@" : "")
                      ,0
                      ,(p->pers ? p->pers : ""));
        /* Und abschicken. */
        appendstring(cp, buffer);
      }
    }
  }

  /* LISTENDE - IRC-Meldung vorbereiten. */
  sprintf(buffer, ":%s 315 %s #%s\n"
                ,myhostname
                ,cp->nickname
                ,chan);
  /* Und abschicken. */
  appendstring(cp,buffer);
}

void IrcLinksCommand(CONNECTION *cp)
{
  DESTINATION *d;
  char         buffer[2048];
  char         *dest = 0;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcLinksCommand)";
#endif /* DEBUG_MODUS */

  if (cp->type == CT_USER)
    dest = getarg(0, 0);
  else
    dest = "";

  if (*dest == '\0')
  {
    for (d = destinations; d; d = d->next)
    {
      if (d->rtt)
      {
        if (d->name[0] == FALSE)
          continue;

        sprintf(buffer, ":%s 364 %s %s %s :%ld %s\n"
                      , cp->host
                      , (cp->nickname ? cp->nickname : cp->name)
                      , d->name
                      , (d->link->name ? d->link->name : myhostname)
                      , d->rtt
                      , d->rev);
        appendstring(cp, buffer);
      }
    }
  }

  sprintf(buffer, ":%s 364 %s %s %s :0 %s\n"
                , cp->host
                , (cp->nickname ? cp->nickname : cp->name)
                , cp->host
                , cp->host
                , "pp-3.14c");
  appendstring(cp, buffer);

  sprintf(buffer, ":%s 365 %s * :End of /LINKS list.\n"
                , cp->host
                , (cp->nickname ? cp->nickname : cp->name));
  appendstring(cp, buffer);
  appendprompt(cp, 0);
}

/* Auflisten aller aktuellen Channels, Kanal, Useranzahl und Topic. */
void IrcListCommand(CONNECTION *cp)
{
  CHANNEL    *ch;
  CONNECTION *p;
  CLIST      *cl;
  char        buffer[BUFSIZ];
  int         n;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcListCommand)";
#endif /* DEBUG_MODUS */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcListCommand()\n");
#endif

  for (ch = channels; ch; ch = ch->next)
  {
    /* Userzaehler auf 0 setzen. */
    n = 0;
    for (p = connections; p; p = p->next)
    {
      /* Nur angemeldete User. */
      if (p->type == CT_USER)
      {
        if (p->channel == ch->chan)
          /* Userzaehler um eins hoeher. */
          n++;
        else
          {
            for (cl = p->chan_list; cl; cl = cl->next)
              /* Hat unser User den gesuchten Channel? */
              if (cl->channel == ch->chan)
                /* Channel gefunden. */
                break;

            /* Eintrag gefunden? */
            if (cl)
              /* Userzaehler um eins hoeher. */
              n++;
          }
      }
    }

    /* IRC-Meldung vorbereiten. */
    sprintf(buffer, ":%s 322 %s #%d %d :%s\n"
                  ,cp->host
                  ,(*cp->nickname ? cp->nickname : cp->name)
                  ,ch->chan
                  ,n
                  ,(ch->topic ? ch->topic : ""));

    /* Und abschicken. */
    appendstring(cp, buffer);
  }

  /* LISTENDE - IRC-Meldung vorbereiten. */
  sprintf(buffer, ":%s 323 %s\n"
                ,cp->host
                ,(*cp->nickname ? cp->nickname : cp->name));

  /* Und abschicken. */
  appendstring(cp,buffer);
}

/* Informationen eines Users. */
void IrcWhoisCommand(CONNECTION *cp)
{
  char buffer[BUFSIZ];
  char *username;
  char *revision;
  CONNECTION  *p, *c;
  CHANNEL     *ch;
  DESTINATION *d;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcWhoisCommand)";
#endif /* DEBUG_MODUS */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcWhoisCommand()\n");
#endif

  /* Username holen. */
  username = getarg(0, 1);

  for (p = connections; p; p = p->next)
  {
    /* User angemeldet. */
    if (p->type == CT_USER)
    {
      /* Gesuchter Username? */
      if (  (strcmp(p->name, username) == FALSE)
          ||(strcmp(p->nickname, username) == FALSE))
      {
        /* IRC-Meldung vorbereiten. */
        sprintf(buffer, ":%s 311 %s %s %s%s %s * :%s%s\n"
                      ,myhostname
                      ,cp->nickname
                      ,p->nickname
                      ,""
                      ,p->name
                      ,p->host
                      ,(p->pers ? p->pers : "")
                      ," [NonAuth]");
        /* Und abschicken. */
        appendstring(cp,buffer);

        /* User als Sysop angemeldet. */
        if (p->operator)
        {
          /* IRC-Meldung vorbereiten. */
          sprintf(buffer, ":%s 313 %s %s :is an IRC operator\n"
                        ,myhostname
                        ,cp->nickname
                        ,p->nickname);
          /* Und abschicken. */
          appendstring(cp,buffer);
        }

        for (c = connections; c; c = c->next)
        {
          /* Nur angemeldete User. */
          if (c->type != CT_USER)
            /* Ein unangemeldeter oder HOST, zum naechsten. */
            continue;

          /* Vergleiche Username. */
          if (strcmp(p->name, c->name))
            /* Nicht unser gesuchter Username, zum naechten. */
            continue;

            /* Aktuellen Channel suchen. */
            for (ch = channels; ch; ch = ch->next)
              if (ch->chan == c->channel)
                /* gefunden. */
                break;

            /* IRC-Meldung vorbereiten. */
            sprintf(buffer, ":%s 319 %s %s :%s %d \n"
                          ,myhostname
                          ,cp->nickname
                          ,p->nickname
                          ,(c->channelop ? "@" : "")
                          ,c->channel);
            /* Und abschicken. */
            appendstring(cp, buffer);
          }

        /* User eigner Knoten. */
        if (!strcasecmp(p->host, myhostname))
          /* Markieren wir die Revision, */
          revision = strchr(REV, ':')+2;

        /* Pruefe alle Destinationen. */
        for (d = destinations; d; d = d->next)
          /* Vergleiche Hostname. */
          if (!strcasecmp(d->name, p->host))
           /* gefunden. */
           break;

        /* Revision, IRC-Meldung vorbereiten. */
        sprintf(buffer, ":%s 312 %s %s %s :%s\n"
                      ,myhostname
                      ,cp->nickname
                      ,p->nickname
                      ,p->host
                      ,(d ? d->rev : revision));
        /* Und abschicken. */
        appendstring(cp,buffer);

        /* Online, IRC-Meldung vorbereiten. */
        sprintf(buffer, ":%s 317 %s %s %ld %ld :seconds idle, signon time\n"
                      ,myhostname
                      ,cp->nickname
                      ,p->nickname
                      ,(currtime - p->mtime), p->time);
        /* Und abschicken. */
        appendstring(cp,buffer);
      }
    }
  }

  /* LISTENDE - IRC-Meldung vorbereiten. */
  sprintf(buffer, ":%s 318 %s %s\n"
                ,myhostname
                ,cp->nickname
                ,username);
  /* Und abschicken. */
  appendstring(cp,buffer);
}

/* Username aufloesen. */
void IrcUserCommand(CONNECTION *cp)
{
  char buffer[2048];
  char *user, *pers, *mode;

#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcUserCommand)";
#endif /* DEBUG_MODUS */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcUserCommand()\n");
#endif

  /* Ist es ein Host, ist der hier verkehrt. */
  if (cp->type == CT_HOST)
    /* Und weg damit. */
    return;

  // force charset
  cp->charset_in = cp->charset_out = ISO_STRIPED;

  /* Username holfen. */
  user = getarg(0, 0);

  mode = getarg(0, 0);

  /* Personal Text einlesen & speichern. */
  personalmanager(SET, cp, (pers = getarg(NULL, GET_ALL)));
  /* Personal Text setzen, max. zeichen 256!. */
  setstring(&(cp->pers), pers, 256);

  if (!*pers)
  {
    sprintf(buffer, ":%s 461 %s USER :Not enough parameters\n", myhostname, (*cp->nickname ? cp->nickname : "*"));
        appendstring(cp, buffer);
        return;
  }

  if (*cp->nickname)
  {
    strncpy(cp->name, cp->nickname, IRC_NAMESIZE);
  }
  else
  {
    // broken client: there are irc-clients which send 1. USER 2. NICK instead
    // of normal order 1. NICK 2. USER
        strncpy(cp->name, user, IRC_NAMESIZE);
  }

  cp->name[IRC_NAMESIZE] = 0;

        if (isdigit(*mode & 0xff)) {
                int i = atoi(mode);
                if (i == 0) {
                        // does not need an ack
                }
                if (i & 2) {
                        cp->verbose = 1;
                        sprintf(buffer, ":%s MODE %s +ws\n", myhostname, cp->name);
                        appendstring(cp, buffer);

                }
                if (i & 3) {
                        // +i not supporded be non-verbose. just don't ack
                }
                // rest: do'nt care
        }


  /* Verbose, defaultwert setzen. */
  cp->verbose = 0;

  /* User erstmal sperren .*/
  cp->locked = 1;
  /* Kanal wird spaeter gesetzt. */
  cp->channel = EOF;
  /* Als User markieren. */
  cp->type = CT_USER;
}

/******************************************************************************/
/*                                                                            */
/* Gibt Informationen ueber die Anzahl von Verbindungen, Global und Lokal.     */
/*                                                                            */
/******************************************************************************/
static void IrcLusersCommand(CONNECTION *cp)
{
  char         buffer[2048];
  DESTINATION *d;
  CHANNEL     *ch;
  CONNECTION  *p;
  int          n_users = 0;                                    /* Useranzahl. */
  int          n_dest,                                 /* Destination/Routen. */
               n_channels,                                    /* Kanalanzahl. */
               n_clients,                                      /* IRC-Client. */
               n_servers,                                      /* IRC-Server. */
               n_operators,                                /* Operatoranzahl. */
               n_typeunknown;                                   /* Unbekannt. */
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcLusersCommand)";
#endif /* DEBUG_MODUS */

  for (n_dest = 0, d = destinations; d; d = d->next)  /* Dest/Routen zaehlen. */
  {
    if (d->rtt && d->link)                            /* Dest/Route gefunden. */
      n_dest++;                                          /* um eins erhoehen. */
  }

  for (n_channels = 0, ch = channels; ch; ch = ch->next)  /* Kanaele zaehlen. */
  {
    if (!(ch->flags & M_CHAN_I))       /* keine unsichtbaren kanaele zaehlen. */
      n_channels++;                                      /* um eins erhoehen. */
  }

  for (n_clients = n_servers = n_typeunknown = n_operators = 0, p = connections; p; p = p->next)
  {
    switch (p->type)
    {
      case CT_HOST:
        n_servers++;
        break;

      case CT_USER:
        n_users++;

        if (!p->via)
          n_clients++;

        if (p->operator)
          n_operators++;

        break;

      case CT_UNKNOWN:
      default:
        n_typeunknown++;
    }
  }

  sprintf(buffer, ":%s 251 %s :Hier sind %d User auf %d Server\n", myhostname, cp->nickname, n_users, (n_dest) ? n_dest+1 : 1) ;
  appendstring(cp, buffer);

  if (n_operators)
  {
    sprintf(buffer, ":%s 252 %s %d Sysop%s online\n", myhostname, cp->nickname, n_operators, (n_operators != 1) ? "s" : "");
    appendstring(cp, buffer);
  }

  if (n_typeunknown)
  {
    sprintf(buffer, ":%s 253 %s %d :Ungueltige Verbindung%s\n", myhostname, cp->nickname, n_typeunknown, (n_typeunknown != 1) ? "en" : "");
    appendstring(cp, buffer);
  }

  sprintf(buffer, ":%s 254 %s %d :Kanael%s\n", myhostname, cp->nickname, n_channels, (n_channels != 1 ? "e" : ""));
  appendstring(cp, buffer);

  sprintf(buffer, ":%s 255 %s :Ich habe %d client%s auf %d Server\n", myhostname, cp->nickname, n_clients, (n_clients != 1) ? "s" : "", n_servers);
  appendstring(cp, buffer);
}

/******************************************************************************/
/*                                                                            */
/* Zusaetzlich Text an IRC-Client senden.                                     */
/*                                                                            */
/******************************************************************************/
static void IrcMotdCommand(CONNECTION *cp)
{
  FILE *fp;
  char  motdfile[MAXPATH + 1];
  char  buffer[80 + 1];
  char  buffer2[128];
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcMotdCommand)";
#endif /* DEBUG_MODUS */

  strcpy(motdfile, confpath);
  strcat(motdfile, IRC_TXT);

  if ((fp = xfopen(motdfile, "r")) == NULL)           /* Fehler beim oeffnen. */
    return;                                                     /* Abbrechen. */

  sprintf(buffer2, ":%s 372 %s :"
                , myhostname
                , cp->nickname);

  while (fgets(buffer, 80, fp))               /* Maximal 80 Zeichen einlesen. */
  {
    appendstring(cp, buffer2);                                /* Code-String. */
    appendstring(cp, buffer);                              /* Datei einlesen. */
  }

  sprintf(buffer, "\n:%s 376 %s :\n"
                , myhostname
                , cp->nickname);
  appendstring(cp, buffer);

  fclose(fp);                                            /* Datei schliessen. */
}

/******************************************************************************/
/*                                                                            */
/* IRC-Client anmelden.                                                       */
/*                                                                            */
/******************************************************************************/
static void IrcLogin(CONNECTION *cp)
{
  char buffer[2048];
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcLogin)";
#endif /* DEBUG_MODUS */

  cp->charset_in = cp->charset_out = ISO_STRIPED;

#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcLogin()\n");
#endif

  sprintf(buffer, "CAP\n:%s 001 %s :%s @ %s PingPong-Release %5.5s (TNN).\n"
                , myhostname
                , cp->nickname
                , convtype
                , myhostname
                , strchr(REV, ':')+2);
  appendstring(cp, buffer);

  sprintf(buffer, ":%s 004 %s %s :\n"
                , myhostname
                , cp->nickname
                , myhostname);

  appendstring(cp, buffer);

  IrcLusersCommand(cp);        /* Anzahl User, Server, Kanaele etc. ausgeben. */
  IrcMotdCommand(cp);               /* Zusaetzlichen Text ausgeben (irc.txt). */

  cp->width = 80;                                     /* Zeilengrenze setzen .*/

#ifdef MOD_L1IRC_C_INVITEONCONNECT
/* DL1NE, invite User zu #0 */
  sprintf(buffer, ":%s!%s@%s INVITE %s #0\n"
                  , (*cp->nickname ? cp->nickname : cp->name)
                  , cp->name
                  , myhostname
                  , (*cp->nickname ? cp->nickname : cp->name));
  appendstring(cp, buffer);
#endif /* MOD_L1IRC_C_INVITEONCONNECT */

}

/******************************************************************************/
/*                                                                            */
/* Nickname aufloesen.                                                        */
/*                                                                            */
/******************************************************************************/
static int validate_nickname(char *s)
{

#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - validate_nickname()\n");
#endif

        if (!s || !*s)
                return 0;
        for (; *s; s++) {
                int c = *s & 0xff;
                if (c >= 'A' && c <= 'Z')
                        continue;
                if (c >= 'a' && c <= 'z')
                        continue;
                if (isdigit(c))
                        continue;
                switch (c) {
                case '[':
                case ']':
                case '\\':
                case '`':
                case '_':
                case '^':
                case '{':
                case '|':
                case '}':
                case '-':
                        continue;
                default:
                        return 0;
                }
        }
        return 1;
}

static int check_nick_dupes(struct connection *cp, char *nick)
{

        struct connection *p;

        if (!strcasecmp(nick, cp->name))
                return 0;

        for (p = connections; p; p = p->next) {
                if (p->type != CT_USER || p == cp)
                        continue;
                if (!strcasecmp(p->name, cp->name))
                        continue;
                if (!strcasecmp(p->nickname, nick) || !strcasecmp(p->name, nick))
                        return 1;
        }
        return 0;
}

void IrcNickCommand(CONNECTION *cp)
{
  char *arg;
  char buffer[2048];
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcNickCommand)";
#endif /* DEBUG_MODUS */

  arg = getarg(0, 0);                                   /* Nickname einlesen. */

  if (cp->type == CT_HOST)                                          /* Host ? */
    return;                                                 /* Und weg damit. */

        // force charset
        cp->charset_in = cp->charset_out = ISO_STRIPED;

        if (*arg == ':')
                arg++;
        if (!*arg) {
                sprintf(buffer, ":%s 431 * :No nickname given\n", myhostname);
                appendstring(cp, buffer);
                return;
        }

        if (!validate_nickname(arg) || !strcasecmp(arg, "conversd"))
        {
      sprintf(buffer, ":%s 432 %s %s :Erroneous nickname\n", myhostname, (*cp->nickname ? cp->nickname : "*"), arg);
          appendstring(cp, buffer);
        }

        if (cp->type != CT_HOST && check_nick_dupes(cp, arg)) {
                sprintf(buffer, ":%s 433 %s %s :Nickname already in use\n", myhostname, (*cp->nickname ? cp->nickname : "*"), arg);
                appendstring(cp, buffer);
                return;
        }

  if (cp->away != NULL)                    /* Wenn ein Away-Text gesetzt ist, */
  {
    cp->away = NULL;                                        /* zuruecksetzen. */

#ifndef CONVNICK
  send_awaymsg(cp->name,    /* Neuen Away-Text an andere Host's weiterleiten. */
               cp->hosthostname,
               currtime,
               cp->away);
#else
  send_awaymsg(cp->name,    /* Neuen Away-Text an andere Host's weiterleiten. */
               cp->nickname,
               cp->host,
               currtime,
               cp->away);
#endif /* CONVNICK */
    return;
  }

  if (*arg == ':')                                  /* Gibt es ":" im string? */
    arg++;                                                   /* ":" loeschen. */

  if (!*arg)                                                  /* String leer? */
  {
    sprintf(buffer, ":%s 431 * :Kein Nickname angegeben!\n"
                  , myhostname);
    appendstring(cp, buffer);
    return;
  }

  if (*cp->nickname)                                     /* Schon eingeloggt. */
  {
    sprintf(buffer, ":%s!%s@%s NICK :%s\n"
                          , (*cp->nickname ? cp->nickname : cp->name)
                  , cp->name
                  , myhostname
                  , arg);
    appendstring(cp, buffer);                          /* Meldung abschicken. */

    strncpy(cp->OldNickname, cp->nickname, NAMESIZE);/* Alten Nick eintragen. */
    cp->nickname[NAMESIZE] = 0;

    strncpy(cp->nickname, arg, NAMESIZE);            /* Neuen Nick eintragen. */
    cp->nickname[NAMESIZE] = 0;
    return;
  }

  strncpy(cp->nickname, arg, NAMESIZE);                    /* Nick eintragen. */
  cp->nickname[NAMESIZE] = 0;

  if (!*cp->name)                              /* Noch kein Username bekannt? */
  {
    strncpy(cp->name, arg, NAMESIZE + 1);/* Nehmen wir den Nickn als Username.*/
    cp->name[NAMESIZE + 1] = 0;
  }

  sprintf(buffer, ":%s!%s@%s NICK :%s\r"
                    , (*cp->nickname ? cp->nickname : cp->name)
                , cp->name
                , myhostname
                , arg);
  appendstring(cp, buffer);                                /* Und abschicken. */

  strncpy(cp->host, myhostname, NAMESIZE);             /* Hostname eintragen. */
  cp->host[sizeof(cp->host)-1] = 0;

  cp->type = CT_USER;

  IrcLogin(cp);                                             /* IRC-Logintext. */
}

/******************************************************************************/
/*                                                                            */
/* Nickname an andere IRC-Client's weiterleiten.                              */
/*                                                                            */
/******************************************************************************/
void SendIrcNick(CONNECTION *p)
{
  CONNECTION *p2;
  CLIST      *cl,
             *cl2;
  char        buffer[2048];
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(SendIrcNick)";
#endif /* DEBUG_MODUS */

  for (p2 = connections; p2; p2 = p2->next)
  {
    if (p2->locked)                                  /* Sperre eingeschaltet. */
      continue;                                     /* zum naechsten Eintrag. */

    if (p2->type == CT_USER)                                    /* Type User. */
    {
      if (p2 == p)                          /* mich selber nicht weitersagen. */
        continue;                                   /* zum naechsten eintrag. */

      if (p2->channel != p->channel)                /* Kanal unterschiedlich. */
      {
        for (cl2 = p2->chan_list; cl2; cl2 = cl2->next)
        {
          if (cl2->channel == p->channel)
            break;
          else
          {
            for (cl = p->chan_list; cl; cl = cl->next)
            {
              if (cl->channel == cl2->channel)
                break;
            }
          }
        }
      }

      if (p2->IrcMode == FALSE)                            /* Kein IRC-Client */
        continue;                                   /* zum naechsten Eintrag. */
                                                          /* Keine Aenderung, */
      if (  (!strncasecmp(p->OldNickname, p->nickname, NAMESIZE))
          ||(!strncasecmp(p->name, p->nickname, NAMESIZE)))
        continue;                                   /* zum naechsten Eintrag. */

                                               /* Aktuellen Nick weitersagen. */
        sprintf(buffer, ":%s!%s@%s NICK %s\n"
                      , (*p->OldNickname ? p->name : p->OldNickname)
                      , p->name
                      , p->host
                      , (*p->nickname ? p->nickname : p->name));

          appenddirect(p2, buffer);
      break;
    }
  }
}

/******************************************************************************/
/*                                                                            */
/* Informationen vom User ausgeben.  .                                        */
/*                                                                            */
/******************************************************************************/
void IrcUserHostCommand(CONNECTION *cp)
{
  CONNECTION *p;
  char        buffer[2048];
  char        buffer2[2048];
  int         found = 0;
  char       *user;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcUserHostCommand)";
#endif /* DEBUG_MODUS */

  user = getarg(0, 0);                                      /* User einlesen. */

  if (!*user)                                         /* Kein User angegeben. */
  {                                               /* IRC-Meldung vorbereiten  */
    sprintf(buffer, ":%s 461 %s USERHOST :Not enough parameters\n"
                  , myhostname
                  , cp->nickname);
    appendstring(cp, buffer);                              /* und abschicken. */
    return;
  }

  sprintf(buffer2, ":%s 302 %s :"                 /* IRC-Meldung vorbereiten  */
                 , myhostname
                 , cp->nickname);

  while (*user)
  {
    p = 0;
    if (  (!strcasecmp(cp->nickname, user))   /* Uservergleich mit Nick/Name. */
        ||(!strcasecmp(cp->name,     user)))
      p = cp;                                            /* Eintrag gefunden. */
    else                                            /* Kein Eintrag gefunden. */
    {
      for (p = connections; p; p = p->next)        /* durchsuche convers-TBL. */
      {
        if (  (!strcasecmp(p->nickname, user))/* Uservergleich mit Nick/Name. */
            ||(!strcasecmp(p->name,     user)))
          break;                                         /* Eintrag gefunden. */
      }
    }

    if (!p)                                            /* Kein User gefunden. */
    {
      sprintf(buffer, ":%s 401 %s %s :No such nick\n"
                    , myhostname
                    , (*cp->nickname ? cp->nickname : cp->name)
                    , user);
      appendstring(cp,buffer);
    }
    else                                                    /* User gefunden. */
    {
      sprintf(buffer, "%s%s%s=%c%s%s@%s"
                    , (found) ? " " : ""
                    , (*p->nickname ? p->nickname : p->name)
                    , (p->operator ? "*" : "")
                    , (p->away ? '-' : '+')
                    , ""
                    , p->name
                    , p->host);

      if (strlen(buffer) + strlen(buffer2) > 510) /* Buf-laenge ueberschritten*/
        break;                                                  /* Abbrechen. */

      strcat(buffer2, buffer);                    /* Informationen anhaengen. */

      if (!found)                                /* Gibt noch keinen Eintrag. */
        found++;                                           /* min. 1 Eintrag. */
    }

    user = getarg(0, 0);                          /* Naechsten User einlesen. */
  }

  if (found)                                             /* Eintrag gefunden. */
  {
    appendstring(cp, buffer2);
    appendstring(cp, "\n");
  }
}

/******************************************************************************/
/*                                                                            */
/* Beendet die Verbindung eines Server's vom Netzwerk.                        */
/*                                                                            */
/******************************************************************************/
void IrcSquitCommand(CONNECTION *cp)
{
  char  buffer[2048];
  char *arg;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcSquitCommand)";
#endif /* DEBUG_MODUS */

  arg = getarg(0, 1);                                     /* Server einlesen. */

  if (!*arg)                                        /* kein Server angegeben. */
  {
    sprintf(buffer, ":%s 461 %s SQUIT :Not enough parameters\n"
                  , myhostname
                  , cp->nickname);
    appendstring(cp, buffer);
    return;
  }

  if (cp->operator != 2)                      /* Kein Sysop, keine Aenderung. */
  {
    sprintf(buffer, ":%s 481 %s :Permission Denied- You're not an IRC operator\n"
                  , myhostname
                  , cp->name);
    appendstring(cp, buffer);
    return;
  }

  sprintf(buffer, "/link reset %s", arg);        /* Reset-Befehl vorbereiten. */
  getarg(buffer, 0);

  links_command(cp);                                        /* Server killen. */
}

/******************************************************************************/
/*                                                                            */
/* Username an IRC-Server weiterleiten.                                       */
/*                                                                            */
/******************************************************************************/
void IrcLinkUser(char *user)
{
  char        *scall;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcLinkUser)";
#endif /* DEBUG_MODUS */

#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcLinkUser()\n");
#endif

  if (tcppoi->Interface != KISS_IRC)                   /* Kein IRC-Interface. */
  {
    user[0] = 0;                                      /* Rufzeichen loeschen. */
    tcppoi->disflg |= 0x80;                         /* Verbindung schliessen. */
    return;
  }

  if ((scall = strchr(user, ' ')) != FALSE)       /* Zum Rufzeichen springen. */
  {
    scall++;                                        /* Leerzeichen entfernen. */
    strncpy(user, scall, L2CALEN);                     /* Rufzeichen sichern. */
    user[6] = 0;                                        /* Sicher ist sicher. */

  }
}

/******************************************************************************/
/*                                                                            */
/* Nickname an IRC-Server weiterleiten.                                       */
/*                                                                            */
/******************************************************************************/
static void IrcLinkNick(char *header)
{
  READRX      *SRx;
  register int i;
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(IrcLinkNick)";
#endif /* DEBUG_MODUS */

#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - IrcLinkNick()\n");
#endif

  if (header[0] == FALSE)                             /* Kein Nick angegeben. */
    return;                                                     /* Abbrechen. */

  if ((SRx = (READRX *)allocb(ALLOC_L1TCPIP)) == NULL)    /* RX-Seg. besorgen */
    return;                                    /* Kein Segment frei, abbruch. */

  if ((SRx->Data = SetBuffer()) == NULL)                  /* Buffer besorgen. */
  {
    dealoc((MBHEAD *)ulink((LEHEAD *)SRx));          /* RX-Segment entsorgen. */
    return;                                      /* Buffer ist voll, abbruch. */
  }

  SRx->Sock      = tcppoi->sock;                            /* Socket setzen. */
  SRx->Interface = tcppoi->Interface;                    /* Interface setzen. */
  SRx->Mode      = tcppoi->mode;                        /* Stack-Mode setzen. */

  if (*header == CR || *header == LF)
          header++;

  for (i = 0; i < (signed)strlen(header); ++i)         /* Nickname eintragen. */
    putchr(header[i], SRx->Data);

  putchr(CR, SRx->Data);

  relink((LEHEAD *) SRx, (LEHEAD *)rxflRX.tail);       /* RX-Liste umhaengen. */
}

/* TOPIC/Thema etzen im angegebenen Kanal. */
void IrcTopicCommand(CONNECTION *cp)
{
  CHANNEL *ch;
  CONNECTION *p;
  register char *topic;
  char buffer[2048];
  time_t time;
  WORD channel;
  WORD topiclen;
  #ifdef DEBUG_MODUS
  lastfunc = "IrcTopicCommand";
#endif /* DEBUG_MODUS */


  channel = cp->channel;
  topic = getarg(NULL, GET_ALL);

  if (*topic == '#')
  {
          topic++;
          topiclen = strlen(topic);

          while (':')
          {
                if (topiclen == FALSE)
                  break;

            if (*topic == ':')
                {
                        topic++;
                        break;
                }

                topic++;
                topiclen--;
          }
  }

  for (p = connections; p; p = p->next)
    if (p->type == CT_USER && !strcmp(p->name, cp->name) && (p->channel == channel))
          break;

  time = currtime;

  for (ch = channels; ch; ch = ch->next)
  {
    if (ch->chan == channel)
          break;
  }

  if (ch)
  {
    if (ch->time > time)
          time = ch->time+1L;

    if (*topic)
        {
      if (p && (((ch->flags & M_CHAN_T) == 0) || cp->operator || p->channelop))
          {
        if (*topic == '@')
                {
          *topic = '\0';
#ifdef CONV_TOPIC
#ifdef SPEECH
          sprintf(buffer, speech_message(90), timestamp, channel, ch->name, ts(ch->time));
#else
          sprintf(buffer, "%sChannel topic on channel %d removed from %s (%s).\r"
                        , timestamp
                        , channel
                        , ch->name
                        , ts(ch->time));
#endif /* SPEECH */
#else
          sprintf(buffer, "%sChannel topic on channel %d removed.\r", timestamp, channel);
#endif /* CONV_TOPIC */
        } else {
#ifdef CONV_TOPIC
             /* Topic sichern. */
             setstring(&(ch->topic), topic, 512);
             /* Call sichern. */
             strncpy(ch->name, cp->name, NAMESIZE + 1);
             /* Nullzeichen setzen. */
             ch->name[sizeof(ch->name)-1] = 0;
             /* Zeit sichern. */
             ch->time = time;
#ifdef SPEECH
            sprintf(buffer, speech_message(91), timestamp, channel, ch->name, ts(ch->time));
#else
            sprintf(buffer, "%sChannel topic set on channel %d from %s (%s).\r"
                          , timestamp
                          , channel
                          , ch->name
                          , ts(ch->time));
#endif /* SPEECH */
#else
            sprintf(buffer, "%sChannel topic set on channel %d.\r", timestamp, channel);
#endif /* CONV_TOPIC */
#ifdef L1IRC
          if (cp->IrcMode == TRUE)
            sprintf(buffer, ":%s TOPIC #%d :%s\n", cp->nickname, channel, topic);
#endif /* L1IRC */
        }
#ifndef CONVNICK
        send_topic(cp->name, cp->host, time, channel, topic);
#else
        send_topic(cp->name, cp->nickname, cp->host, time, channel, topic);
#endif /* CONVNICK */
      } else {
#ifdef SPEECH
          sprintf(buffer, speech_message(68), timestamp);
#else
          sprintf(buffer, "%sYou are not an operator.\r", timestamp);
#endif
#ifdef L1IRC
        if (cp->IrcMode == TRUE)
          sprintf(buffer, ":%s 482 %s #%d :%s\n", myhostname, cp->name, channel, speech_message(68));
#endif /* L1IRC */
      }
    } else {
      if (ch->topic)
      {
#ifdef CONV_TOPIC
#ifdef SPEECH
        sprintf(buffer, speech_message(92), timestamp, channel, ch->name, ts(ch->time));
#else
        sprintf(buffer, "%sCurrent channel topic on channel %d from %s (%s) is\r               "
                      , timestamp
                      , channel
                      , ch->name
                      , ts(ch->time));
#endif /* SPEECH */
#else
        sprintf(buffer, "%sCurrent channel topic on channel %d is\r               "
                      , timestamp
                      , channel);
#endif /* CONV_TOPIC */
#ifndef L1IRC
        appenddirect(cp, buffer);
        appendstring(cp, ch->topic);
        strcpy(buffer, "\r");
#else
        if (cp->IrcMode == TRUE)
        {
          sprintf(buffer, ":%s 332 %s #%d :%s",cp->host,*cp->nickname ? cp->nickname : cp->name, cp->channel, ch->topic);
          buffer[2048] = 0;
          appendstring(cp, buffer);
          appendstring(cp, "\n");
          sprintf(buffer, ":%s 333 %s #%d %ld\n",cp->host,*cp->nickname ? cp->nickname : cp->name, channel, ch->time);
        }
        else
        {
          appenddirect(cp, buffer);
          appendstring(cp, ch->topic);
          strcpy(buffer, "\r");
        }
#endif /* L1IRC */
      } else {
#ifdef SPEECH
          sprintf(buffer, speech_message(93), timestamp, channel);
#else
          sprintf(buffer, "%sNo current channel topic on channel %d.\r", timestamp, channel);
#endif
#ifdef L1IRC
        if (cp->IrcMode == TRUE)
          sprintf(buffer, ":%s 331 %s #%d :There isn't a topic.\n", myhostname, cp->nickname, channel);
#endif /* L1IRC */
      }
    }
  } else {
#ifdef SPEECH
    sprintf(buffer, speech_message(94), timestamp, channel);
#else
    sprintf(buffer, "%sChannel channel %d non existent.\r", timestamp, channel);
#endif
#ifdef L1IRC
    if (cp->IrcMode == TRUE)
      sprintf(buffer, ":%s 442 %s #%d :%s\n", myhostname, cp->nickname, channel, speech_message(94));
#endif /* L1IRC */
  }
  appenddirect(cp, buffer);
  appendprompt(cp, 0);
}


BOOLEAN LoginIRC(MBHEAD *tbp)
{
  char ch;
  char scall[256];
  char tmpcall[256];
  WORD scall_len;
  WORD tmpcall_len;
  char *call;
  WORD  len = FALSE;
  char buffer[256 + 1];                 /* Buffer fuer erweiterte Logindaten. */
  char Upcall[10 + 1];
#ifdef DEBUG_MODUS
  lastfunc = "l1irc(LoginIRC)";
#endif /* DEBUG_MODUS */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1itrc.c - LoginIRC()\n");
#endif
  while (tbp->mbgc != tbp->mbpc)        /* Alle Zeichen aus den Buffer lesen. */
  {
    ch = getchr(tbp);                                       /* Zeichen holen. */

    if (  (ch == CR)                                /* Zeichen ein CR-Return, */
        ||(ch == LF)                                       /* oder LF-Return, */
        ||(ch == '#'))                                         /* oder Route. */
        {
      buffer[len] = '\0';                              /* Nullzeichen setzen. */

          if ((strstr(buffer, "CAP")) != FALSE)
          {
            len = 0;
            continue;
          }

          if ((call = strstr(buffer, "USER")) != FALSE)       /* Zum Rufzeichen springen. */
          {
        strcpy(ircloginbuf, call);
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - LoginIRC()\n");
#endif

                call = strchr(buffer, ' ');
        call++;                                           /* Leerzeichen entfernen. */

        strncpy(tmpcall, call, L2CALEN);                     /* Rufzeichen sichern. */
        tmpcall[6] = 0;                                    /* Sicher ist sicher. */

                tmpcall_len = strlen(tmpcall);
                len = 0;

#ifdef MOD_L1IRC_C_LINEFEED
/* DL1NE, Versuche Zeilenumbruch zu entfernen */
BOOLEAN ws = FALSE;
int i;
for (i = 0; i < tmpcall_len; i++)
{
   ws = FALSE;
   if(tmpcall[tmpcall_len - i] == '\n') ws = TRUE;
   if(tmpcall[tmpcall_len - i] == ' ') ws = TRUE;
   if(tmpcall[tmpcall_len - i] == '\r') ws = TRUE;
   if(tmpcall[tmpcall_len - i] == CR) ws = TRUE;
   if(tmpcall[tmpcall_len - i] == LF) ws = TRUE;
   if(tmpcall[tmpcall_len - i] == '\0') ws = TRUE;
   if (ws == FALSE) break;
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - LoginIRC() - Zeilenumbruch - Entferne Zeichen: %i\n", i);
#endif
   tmpcall[tmpcall_len - i] = 0;
}
#endif /* MOD_L1IRC_C_LINEFEED */
strncpy(scall, tmpcall, L2CALEN);
scall_len = strlen(scall);
scall[scall_len] = '\0';


        if (CheckCall(scall, scall_len))           /* Pruefe Loginrufzeichen.            */
                {
                 dealmb(tbp);                       /* Loginrufzeichen ist ungueltig,     */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: IRC Rufzeichen ungueltig! - %s -\n", scall);
#endif
          return(FALSE);                      /* Buffer entsorgen.                  */
                }

            /* ggf. Logbuch fuehren. */
        ifpp = SetInterface(tcppoi->Interface);
        call2str(Upcall, tcppoi->Upcall);
        T_LOGL2(TRUE, "(LoginTCP):%s\nEingeloggt als %s.\n"
                    , tcppoi->ip
                    , Upcall);

        tcppoi->login = TRUE;                 /* Markiere, User als eingeloggt.     */

        if (++nmbtcp > nmbtcp_max)            /* Maximalwert fuer die Statistik     */
          nmbtcp_max = nmbtcp;

        tcppoi->state = L2MCONNT;             /* Statusmeldung an den L7 Melden.    */
          }

          if (strstr(buffer, "NICK") != FALSE)
          {
            IrcLinkNick(buffer);
          }
        }

    buffer[len++] = ch;                                      /* Zeichen eintragen. */
#ifdef DEBUG_L1IRC_C
printf("DEBUG: l1irc.c - Buffer = %s\n", buffer);
#endif
  }

  dealmb(tbp);                          /* Buffer entsorgen.                  */
  return(TRUE);
}

#endif /* L1IRC */

/* End of src/l1irc.c */
