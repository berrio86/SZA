#define MAX_BUF 1024
#define SERVER "localhost"
#define PORT 6012

#define COM_USER	0
#define COM_PASS	1
#define COM_POSITION	2
#define COM_RESET	3
#define COM_UP		4
#define COM_DOWN	5
#define COM_LEFT	6
#define COM_RIGHT	7
#define COM_PHOTO	8
#define COM_FOTOP	9
#define COM_LOGOUT	10

#define OP_POS	1
#define OP_RST	2
#define OP_UP	3
#define OP_DOWN	4
#define OP_LEFT	5
#define OP_RIGHT	6
#define OP_PHOTO	7
#define OP_LOGOUT	8


char * KOMANDOAK[] = {"USER","PASS","POSITION","RESET","UP","DOWN","LEFT","RIGHT","PHOTO","FOTOP", "LOGOUT",NULL};
char * ER_MEZUAK[] =
{
	"Dena ondo. Errorerik ez.\n",
	"Erabiltzaile ezezaguna.\n",
	"Pasahitza okerra.\n",
	"Saioa ez da oraindik hasi.\n",
	"Arazoa argazkia ateratzerakoan.\n",
	"Espero ez den komandoa jaso da.\n",
	"Bestelako arazoa.\n"
};

int parse(char *status);
char *extract(const char *const string, const char *const left, const char *const right);
int menua();

