#include "common.h"

void fail(int severity, char *msg)
{
	char sev[4+1]; sev[0] = '\0';
	switch(severity)
	{
		case 0:
			sprintf(sev, "ERR"); break;
		case 1:
			sprintf(sev, "WARN"); break;
		case 2:
			sprintf(sev, "DBG"); break;
	}
	printf("[%s] %s\n", sev, msg);
}
