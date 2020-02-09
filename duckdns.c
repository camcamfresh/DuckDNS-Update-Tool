#include "duckdns.h"

int main(int argc, char ** argv)
{
	int funcResult = OK;
	Configuration config = {0};
	IP ip = {0};
	
	funcResult = parseConfigurationFile(&config);
	
	if(funcResult)
	{
		if(funcResult == CONFIG_FILE_MISSING)
			logToFile("ERROR: Missing configuration file.");
		else if(funcResult == CONFIG_SUBDOMAINS_MISSING)
			logToFile("ERROR: Missing subdomains in configuration file.");
		else if(funcResult == CONFIG_TOKEN_MISSING)
			logToFile("ERROR: Missing token in configuration file.");
		return KO;
	}
	
	if(config.adapter)
		funcResult = getAdapterIP(config.adapter, &ip);
	else
		funcResult = getPublicIP(&ip);
		
	if(funcResult)
	{
		logToFile("ERROR: IP Address could not be determined.");
		return KO;
	}

	publishIP(&config, &ip);
	
}

int parseConfigurationFile(Configuration * pConfigFile)
{
	FILE * pFile = fopen(CONFIG_FILE_NAME, "r");
	
	if(pFile == NULL)
		return CONFIG_FILE_MISSING;
	
	pConfigFile->adapter = NULL;
	
	int scanResult;
	char buffer[BUFFER_SIZE];
	
	while(fgets(buffer, BUFFER_SIZE, pFile) != NULL)
	{
		if(strstr(buffer, "adapter=") != NULL)
		{
			scanResult = sscanf(buffer, "adapter=%[^\n]", buffer);
			if(scanResult == 1)
			{
				pConfigFile->adapter = malloc((strlen(buffer) + 1) * sizeof(char));
				strcpy(pConfigFile->adapter, buffer);
			}
		}
		else if(strstr(buffer, "subdomains=") != NULL)
		{
			scanResult = sscanf(buffer, "subdomains=%[^\n]", buffer);
			if(scanResult == 1)
			{
				pConfigFile->subdomains = malloc((strlen(buffer) + 1) * sizeof(char));
				strcpy(pConfigFile->subdomains, buffer);
			}
		}
		else if(strstr(buffer, "token=") != NULL)
		{
			scanResult = sscanf(buffer, "token=%[^\n]", buffer);
			if(scanResult == 1)
			{
				pConfigFile->token = malloc((strlen(buffer) + 1) * sizeof(char));
				strcpy(pConfigFile->token, buffer);
			}
		}
	}

	if(pConfigFile->subdomains == NULL)
		return CONFIG_SUBDOMAINS_MISSING;
	if(pConfigFile->token == NULL)
		return CONFIG_TOKEN_MISSING;
	return OK;
}

int getPublicIP(IP * pIP)
{
	FILE * pCommandResult = popen("curl ipv4bot.whatismyipaddress.com", "r");
	
	if(pCommandResult)
	{
		char buffer[BUFFER_SIZE];
		int scanResult;
		
		while(fgets(buffer, BUFFER_SIZE, pCommandResult) != NULL)
		{
			scanResult = sscanf(buffer, "%[^\n]", buffer);
			if(scanResult == 1)
			{
				pIP->ipv4 = malloc((strlen(buffer) + 1) * sizeof(char));
				strcpy(pIP->ipv4, buffer);
			}
		}
		
		pclose(pCommandResult);
		return OK;
	}

	return IP_ADDRESS_MISSING;
}

int getAdapterIP(char * adapterName, IP * pIP)
{
	FILE * pCommandResult = popen("ipconfig", "r");
	
	if(pCommandResult)
	{
		char buffer[BUFFER_SIZE];
		int adapterLines;
		
		while(fgets(buffer, BUFFER_SIZE, pCommandResult) != NULL)
		{
			if(buffer[0] >= '!' && buffer[0] <= '~')
			{
				if(strstr(buffer, adapterName) != NULL)
					adapterLines = 1;
				else
					adapterLines = 0;
			}
			else if(adapterLines)
			{
				if(strstr(buffer, "IPv4 Address") != NULL)
				{
					char * lastSpace = strrchr(buffer, ' ');
					lastSpace++;
					
					pIP->ipv4 = malloc((strlen(buffer)) * sizeof(char));
					sscanf(lastSpace, "%[^\n]", pIP->ipv4);
				}
			}
		}
		
		pclose(pCommandResult);
	}
	
	return pIP->ipv4 ? OK : IP_ADDRESS_MISSING;
}

int publishIP(Configuration * config, IP * pIP)
{
	char * sysCommand = malloc(BUFFER_SIZE * sizeof(char));
	strcpy(sysCommand, "curl \"https://www.duckdns.org/update?domains=");
	strcat(sysCommand, config->subdomains);
	strcat(sysCommand, "&token=");
	strcat(sysCommand, config->token);
	strcat(sysCommand, "&ip=");
	strcat(sysCommand, pIP->ipv4);
	strcat(sysCommand, "\"");
	
	FILE * pCommandResult = popen(sysCommand, "r");
	
	if(fgetc(pCommandResult) == 'O')
		strcpy(sysCommand, "Success: IP Updated - ");
	else
		strcpy(sysCommand, "ERROR: IP NOT UPDATED - ");
	strcat(sysCommand, pIP->ipv4);
	logToFile(sysCommand);
	
	pclose(pCommandResult);
	free(sysCommand);
}

void logToFile(char * message)
{
	char * sysCommand = malloc( sizeof("echo ") - 1
			+ strlen(message) + sizeof(" >> ") - 1
			+ sizeof(LOG_FILE_NAME));
	
	strcpy(sysCommand, "echo ");
	strcat(sysCommand, message);
	strcat(sysCommand, " >> ");
	strcat(sysCommand, LOG_FILE_NAME);
		
	system(sysCommand);
	free(sysCommand);
}
