#define MAX_BUF 1024
#define PORT 6012
#define FILES_PATH	"argazkiak"

#define ST_INIT	0
#define ST_AUTH	1
#define ST_MAIN	2
#define ST_PHOTO 3
#define ST_EXIT 4

#define COM_USER	0
#define COM_PASS	1
#define COM_POSITION	2
#define COM_RESET	3
#define COM_UP		4
#define COM_DOWN	5
#define COM_LEFT	6
#define COM_RIGHT	7
#define COM_PHOTO	8
#define COM_PHOTOP	9
#define COM_LOGOUT	10


char * KOMANDOAK[] = {"USER","PASS","POSITION","RESET","UP","DOWN","LEFT","RIGHT","PHOTO","PHOTOP", "LOGOUT",NULL};
char * erab_zer[] = {"kamera","kamera2",NULL};
char * pass_zer[] = {"123456","654321", NULL};
int egoera;

void saioa(int s);
int readline(int stream, char *buf, int tam);
int bilatu_string(char *string, char **string_zerr);
int bilatu_substring(char *string, char **string_zerr);
void ustegabekoa(int s);
int bidali_zerrenda(int s);
unsigned long toki_librea();
void sig_chld(int signal);
int ez_ezkutua(const struct dirent *entry);
void stringSortu(int x, int y, char* pos);
int ardatzaFrogatu(int y);
