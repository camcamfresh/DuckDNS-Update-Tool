#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Return Values
#define OK 0
#define KO -1
#define CONFIG_FILE_MISSING 1
#define CONFIG_SUBDOMAINS_MISSING 2
#define CONFIG_TOKEN_MISSING 3
#define IP_ADDRESS_MISSING 4

#define BUFFER_SIZE 128
#define CONFIG_FILE_NAME "duckdns.config"
#define LOG_FILE_NAME "duckdns.log"


typedef struct Configuration
{
	char * adapter;
	char * subdomains;
	char * token;
} Configuration;

typedef struct IP
{
	char * ipv4;
	char * ipv6;
} IP;

int parseConfigurationFile(Configuration * pConfigFile);
int getPublicIP(IP * pIP);
int getAdapterIP(char * adapterName, IP * pIP);
int publishIP(Configuration * config, IP * pIP);
void logToFile(char * message);
