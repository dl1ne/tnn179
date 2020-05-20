#ifdef L1IRC

#define KISS_IRC               25                     /* IRC KISS-Types       */
#define IRC_ID                  3                     /* Interface-ID IRC.    */

#define DEFAULT_IRC_PORT    13601                           /* Default Port.  */
#define IRC_TXT         "irc.txt"                     /* Fuer Login-Prompt.   */
#define IRCLOG          "irc.log"           /* Logdatei zum mit schreiben von */

#define ISO_STRIPED 1

#define MINCHANNEL	0

// cave: MAX_MSGSIZE must be >= IRC_MAX_MSGSIZE
#define IRC_MAX_MSGSIZE	510     /* message length excl. \r\n on transport layer */

extern char ircloginbuf[];

/* Channel schliessen. */
extern void leave_command(CONNECTION *);
/* Nachricht verschicken, */
extern void msg_command(CONNECTION *);

extern void links_command(CONNECTION *);

extern void help_command(CONNECTION *);


extern void SendIrcNick(CONNECTION *);
/* Username weiterleiten. */
extern void IrcLinkUser(char *);

extern void ProcessIrcInput(char *, CONNECTION *);

extern void ccpirc(void);           /* IRC-Server Einstellung aendern/setzen. */

extern BOOLEAN LoginIRC(MBHEAD *);

#endif /* L1IRC */
