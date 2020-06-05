#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <msgpack.h>
#include <cJSON.h>
#include <stdint.h>
#include <getopt.h>

#define MULTIPART_DOC "/nvram/multipart.bin"
#define MAX_BUFSIZE 512

typedef struct multipart_subdoc
{
	char *name;
	char *version;
	char *data;
	size_t length;
}multipart_subdoc_t;

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
static void packJsonString( cJSON *item, msgpack_packer *pk );
static void packJsonNumber( cJSON *item, msgpack_packer *pk );
static void packJsonArray( cJSON *item, msgpack_packer *pk, int isBlob );
static void packJsonObject( cJSON *item, msgpack_packer *pk, int isBlob);
static void packJsonBool(cJSON *item, msgpack_packer *pk, bool value);
static void __msgpack_pack_string( msgpack_packer *pk, const void *string, size_t n );
static int convertJsonToBlob(char *data, char **encodedData, int isBlob);
static int convertJsonToMsgPack(char *data, char **encodedData, int isBlob);
static void decodeMsgpackData(char *encodedData, int encodedDataLen);
static int convertMsgpackToBlob(char *data, int size, char **encodedData);
static char *decodeBlobData(char *data);

/*----------------------------------------------------------------------------*/
/*                             Internal Functions                             */
/*----------------------------------------------------------------------------*/

int processEncoding(char *filename, char *encoding, int isBlob)
{
	char outFile[128] = {'\0'};
	char *fileData = NULL;
    size_t len = 0;
	char *encodedData = NULL, *temp = NULL;
	int encodedLen = 0;
	
	if(readFromFile(filename, &fileData, &len))
	{
		if(strcmp(encoding,"B") == 0)
		{
			encodedLen = convertJsonToBlob(fileData, &encodedData, isBlob);
		}
		else if(strcmp(encoding, "M") == 0)
		{
			encodedLen = convertJsonToMsgPack(fileData, &encodedData, isBlob);
		}
		if(encodedLen > 0 && encodedData != NULL)
		{
			temp = strdup(filename);
			sprintf(outFile,"%s.bin",strtok(temp, "."));
			printf("Encoding is success. Hence writing encoded data to %s\n",outFile);
			if(writeToFile(outFile, encodedData, encodedLen) == 0)
			{
				fprintf(stderr,"%s File not Found\n", outFile);
				free(encodedData);
				free(temp);
				return 0;
			}
			free(encodedData);
			free(temp);
		}
		free(fileData);
	}
	else
	{
		fprintf(stderr,"%s File not Found\n",filename);
		return 0;
	}
	return 1;
}

static int getItemsCount(cJSON *object)
{
	int count = 0;
	while(object != NULL)
	{
		object = object->next;
		count++;
	}
	return count;
}
static void __msgpack_pack_string( msgpack_packer *pk, const void *string, size_t n )
{
	printf("%s:%s\n",__FUNCTION__,(char *)string);
    msgpack_pack_str( pk, n );
    msgpack_pack_str_body( pk, string, n );
}

static void packJsonString( cJSON *item, msgpack_packer *pk )
{
	if(item->string != NULL)
	{
		__msgpack_pack_string(pk, item->string, strlen(item->string));
	}
	__msgpack_pack_string(pk, item->valuestring, strlen(item->valuestring));
}

static void packJsonNumber( cJSON *item, msgpack_packer *pk )
{
	printf("%s:%d\n",__FUNCTION__,item->valueint);
	if(item->string != NULL)
	{
		__msgpack_pack_string(pk, item->string, strlen(item->string));
	}
	msgpack_pack_int(pk, item->valueint);
}

static void packJsonBool(cJSON *item, msgpack_packer *pk, bool value)
{
	__msgpack_pack_string(pk, item->string, strlen(item->string));
	if(value)
	{
		msgpack_pack_true(pk);
	}
	else
	{
		msgpack_pack_false(pk);
	}
}
static void packJsonArray(cJSON *item, msgpack_packer *pk, int isBlob)
{
	int arraySize = cJSON_GetArraySize(item);
	printf("%s:%s\n",__FUNCTION__, item->string);
	if(item->string != NULL)
	{
		//printf("packing %s\n",item->string);
		__msgpack_pack_string(pk, item->string, strlen(item->string));
	}
	msgpack_pack_array( pk, arraySize );
	int i=0;
	for(i=0; i<arraySize; i++)
	{
		cJSON *arrItem = cJSON_GetArrayItem(item, i);
		switch((arrItem->type) & 0XFF)
		{
			case cJSON_True:
				packJsonBool(arrItem, pk, true);
				break;
			case cJSON_False:
				packJsonBool(arrItem, pk, false);
				break;
			case cJSON_String:
				packJsonString(arrItem, pk);
				break;
			case cJSON_Number:
				packJsonNumber(arrItem, pk);
				break;
			case cJSON_Array:
				packJsonArray(arrItem, pk, isBlob);
				break;
			case cJSON_Object:
				packJsonObject(arrItem, pk, isBlob);
				break;
		}
	}
}

int getEncodedBlob(char *data, char **encodedData)
{
	cJSON *jsonData=NULL;
	int encodedDataLen = 0;
	printf("------- %s -------\n",__FUNCTION__);
	jsonData=cJSON_Parse(data);
	if(jsonData != NULL)
	{
		msgpack_sbuffer sbuf1;
		msgpack_packer pk1;
		msgpack_sbuffer_init( &sbuf1 );
		msgpack_packer_init( &pk1, &sbuf1, msgpack_sbuffer_write );
		packJsonObject(jsonData, &pk1, 1);
		if( sbuf1.data )
		{
		    *encodedData = ( char * ) malloc( sizeof( char ) * sbuf1.size );
		    if( NULL != *encodedData )
			{
		        memcpy( *encodedData, sbuf1.data, sbuf1.size );
			}
			encodedDataLen = sbuf1.size;
		}
		msgpack_sbuffer_destroy(&sbuf1);
		cJSON_Delete(jsonData);
	}
	else
	{
		printf("Failed to parse JSON\n");
	}
	printf("------- %s -------\n",__FUNCTION__);
	return encodedDataLen;
}
static void packBlobData(cJSON *item, msgpack_packer *pk )
{
	char *blobData = NULL, *encodedBlob = NULL;
	int len = 0;
	printf("------ %s ------\n",__FUNCTION__);
	blobData = strdup(item->valuestring);
	printf("%s\n",blobData);
	len = getEncodedBlob(blobData, &encodedBlob);
	printf("%s\n",encodedBlob);
	__msgpack_pack_string(pk, item->string, strlen(item->string));
	__msgpack_pack_string(pk, encodedBlob, len);
	free(encodedBlob);
	free(blobData);
	printf("------ %s ------\n",__FUNCTION__);
}

static void packJsonObject( cJSON *item, msgpack_packer *pk, int isBlob )
{
	printf("%s\n",__FUNCTION__);
	if(item->string != NULL)
	{
		__msgpack_pack_string(pk, item->string, strlen(item->string));
	}
	cJSON *child = item->child;
	msgpack_pack_map( pk, getItemsCount(child));
	while(child != NULL)
	{
		switch((child->type) & 0XFF)
		{
			case cJSON_True:
				packJsonBool(child, pk, true);
				break;
			case cJSON_False:
				packJsonBool(child, pk, false);
				break;
			case cJSON_String:
				if(child->string != NULL && (strcmp(child->string, "value") == 0) && isBlob == 1)
				{
					packBlobData(child, pk);
				}
				else
				{
					packJsonString(child, pk);
				}
				break;
			case cJSON_Number:
				packJsonNumber(child, pk);
				break;
			case cJSON_Array:
				packJsonArray(child, pk, isBlob);
				break;
			case cJSON_Object:
				packJsonObject(child, pk, isBlob);
				break;
		}
		child = child->next;
	}
}

static int convertJsonToMsgPack(char *data, char **encodedData, int isBlob)
{
	cJSON *jsonData=NULL;
	int encodedDataLen = 0;
	jsonData=cJSON_Parse(data);
	if(jsonData != NULL)
	{
		msgpack_sbuffer sbuf;
		msgpack_packer pk;
		msgpack_sbuffer_init( &sbuf );
		msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );
		packJsonObject(jsonData, &pk, isBlob);
		if( sbuf.data )
		{
		    *encodedData = ( char * ) malloc( sizeof( char ) * sbuf.size );
		    if( NULL != *encodedData )
			{
		        memcpy( *encodedData, sbuf.data, sbuf.size );
			}
			encodedDataLen = sbuf.size;
		}
		msgpack_sbuffer_destroy(&sbuf);
		cJSON_Delete(jsonData);
	}
	else
	{
		printf("Failed to parse JSON\n");
	}
	return encodedDataLen;
}

static int convertJsonToBlob(char *data, char **encodedData, int isBlob)
{
	char *msgpackData = NULL, *blobData = NULL, *decodedBlob = NULL;
	int msgpackDataLen = 0, blobDataLen = 0;
	printf("********* Converting json to msgpack *******\n");
	msgpackDataLen = convertJsonToMsgPack(data, &msgpackData, isBlob);
	if(msgpackDataLen > 0)
	{
		printf("Converting Json data to msgpack is success\n");
		printf("Converted msgpack data is \n%s\n",msgpackData);
		decodeMsgpackData(msgpackData, msgpackDataLen);
	}
	else
	{
		printf("Failed to encode json data to msgpack\n");
		return 0;
	}
	printf("********* Converting msgpack to blob *******\n");
	blobDataLen = convertMsgpackToBlob(msgpackData, msgpackDataLen, &blobData);
	if(blobDataLen>0)
	{
		printf("Json is converted to blob\n");
		printf("blob data is \n%s\n",blobData);
		*encodedData = blobData;
		decodedBlob = decodeBlobData(blobData);
		printf("Decoded blob data is \n%s\n",decodedBlob);
	}
	else
	{
		printf("Failed to encode msgpack data to blob\n");
		free(msgpackData);
		return 0;
	}
	if(strcmp(msgpackData, decodedBlob) == 0)
	{
		printf("Encoded msgpack data and decoded blob data are equal\n");
	}
	free(msgpackData);
	return blobDataLen;
}

static void decodeMsgpackData(char *encodedData, int encodedDataLen)
{
	/* deserialize the buffer into msgpack_object instance. */
	/* deserialized object is valid during the msgpack_zone instance alive. */
	msgpack_zone mempool;
	msgpack_zone_init(&mempool, 2048);

	msgpack_object deserialized;
	msgpack_unpack(encodedData, encodedDataLen, NULL, &mempool, &deserialized);
	printf("Decoded msgpack data is \n");
	/* print the deserialized object. */
	msgpack_object_print(stdout, deserialized);
	puts("");
	msgpack_zone_destroy(&mempool);	
}

static int convertMsgpackToBlob(char *data, int size, char **encodedData)
{
	char* b64buffer =  NULL;
	int b64bufferSize = b64_get_encoded_buffer_size( size );
	b64buffer = malloc(b64bufferSize + 1);
    if(b64buffer != NULL)
    {
        memset( b64buffer, 0, sizeof( b64bufferSize )+1 );

        b64_encode((uint8_t *)data, size, (uint8_t *)b64buffer);
        b64buffer[b64bufferSize] = '\0' ;
		*encodedData = b64buffer;
	}
	return b64bufferSize;
}

static char *decodeBlobData(char *data)
{
	int size = 0;
	char *decodedData = NULL;
    size = b64_get_decoded_buffer_size(strlen(data));
    decodedData = (char *) malloc(sizeof(char) * size);
    if(decodedData)
    {
		memset( decodedData, 0, sizeof(char) *  size );
		size = b64_decode( (const uint8_t *)data, strlen(data), (uint8_t *)decodedData );
		decodeMsgpackData(decodedData, size);
	}
	return decodedData;
}

int readFromFile(const char *filename, char **data, size_t *len)
{
       FILE *fp;
       int ch_count = 0;
       fp = fopen(filename, "r+");
       if (fp == NULL)
       {
               return 0;
       }
       fseek(fp, 0, SEEK_END);
       ch_count = ftell(fp);
      fseek(fp, 0, SEEK_SET);
       *data = (char *) malloc(sizeof(char) * (ch_count + 1));
       fread(*data, 1, ch_count,fp);
       *len = (size_t)ch_count;
       fclose(fp);
       return 1;
}

int parseSubDocArgument(char **args, int count, multipart_subdoc_t **docs)
{
	int i = 0, j = 0, isBlob = 0;
	char *fileName = NULL;
	char *fileData = NULL;
	char outFile[128];
    size_t len = 0;
	char *tempStr= NULL, *docData = NULL, *blobStr = NULL;
	for (i = 2; i<count; i++)
	{
		isBlob = 0;
		docData = strdup(args[i]);
		tempStr = docData;
		(*docs)[j].version = strdup(strsep(&tempStr,","));
		(*docs)[j].name = strdup(strsep(&tempStr,","));
		fileName = strdup(strsep(&tempStr,","));
		if(tempStr != NULL)
		{
			blobStr = strsep(&tempStr,",");
			if(strcmp(blobStr, "blob") == 0)
			{
				isBlob = 1;
			}
		}
		free(docData);
		if(processEncoding(fileName, "M", isBlob))
		{
			sprintf(outFile,"%s.bin",strtok(fileName,"."));
			if(readFromFile(outFile, &fileData, &len))
			{
				(*docs)[j].data = (char  *)malloc(sizeof(char)*len);
				memcpy((*docs)[j].data, fileData, len);
				(*docs)[j].length = len;
				free(fileData);
			}
			else
			{
				printf("Failed to open %s\n",fileName);
				(*docs)[j].data = NULL;
				(*docs)[j].length = 0;
			}
		}
		else
		{
				return 0;
			}
		free(fileName);
		j++;
	}
	return 1;
}

int append_str (char * dest, char * src)
{
	int len;
	len = strlen (src);
	if (len > 0)
	    strncpy (dest, src, len);
	return len;
}

void generateBoundary(char *s ) 
{
	const int len = 50;
	int i = 0;
    static const char charset[] =
        "0123456789"
		"+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
	srand((unsigned int)clock());
    for (i = 0; i < len; ++i) {
        s[i] = charset[rand() % (sizeof(charset) - 1)];
    }

    s[len] = 0;
}

int add_header(char *name, char *value, char *buffer)
{
	int bufLength = strlen(name)+strlen(value)+2;
	sprintf(buffer, "%s%s\r\n",name,value);
	return bufLength;
}

int getSubDocBuffer(multipart_subdoc_t subdoc, char **buffer)
{
	int bufLength = 0, length = 0;
	char  * hdrbuf = NULL;
	char  * pHdr = NULL;
	hdrbuf = (char *) malloc (sizeof(char)*(MAX_BUFSIZE+subdoc.length));
	memset (hdrbuf, 0, sizeof(char)*(MAX_BUFSIZE+subdoc.length));
	pHdr = hdrbuf;
	length = add_header("Content-type: ","application/msgpack", pHdr);
	pHdr += length;
	length = add_header("Etag: ",subdoc.version, pHdr);
	pHdr += length;
	length = add_header("Namespace: ",subdoc.name, pHdr);
	pHdr += length;
	length = append_str(pHdr, "\n");
	pHdr += length;
	length = append_str(pHdr, subdoc.data);
	pHdr += length;
	length = append_str(pHdr, "\r\n\n");
	pHdr += length;
	*buffer = hdrbuf;
	bufLength = strlen(*buffer);
	return bufLength;
}
int writeToFile(char *file_path, char *data, size_t size)
{
	FILE *fp;
	fp = fopen(file_path , "w+");
	if (fp == NULL)
	{
		printf("Failed to open file %s\n", file_path );
		return 0;
	}
	if(data !=NULL)
	{
		fwrite(data, size, 1, fp);
		fclose(fp);
		return 1;
	}
	else
	{
		printf("writeToFile failed, Data is NULL\n");
		fclose(fp);
		return 0;
	}
}

int getSubDocsDataSize(multipart_subdoc_t *subdocs, int count)
{
	int total = 0;
	int i=0;
	for(i=0; i<count; i++)
	{
		total += subdocs[i].length;
	}
	return total;
}

int generateMultipartBuffer(char *rootVersion, int subDocCount, multipart_subdoc_t *subdocs, char **buffer)
{
	char boundary[50] = {'\0'};
	char *temp = NULL;
	char  * tempBuf = NULL;
	int subDocsDataSize = 0, subdocLen = 0, bufLen = 0, len = 0, j = 0;
	generateBoundary(boundary);
	printf("boundary: %s\n",boundary);
	subDocsDataSize = getSubDocsDataSize(subdocs, subDocCount);
	printf("Subdocs data size: %d\n",subDocsDataSize);
	printf("Allocated memory to buffer is %d\n",MAX_BUFSIZE+subDocsDataSize);
	tempBuf = (char *)malloc(sizeof(char)*((MAX_BUFSIZE*2)+subDocsDataSize));
	memset (tempBuf, 0, sizeof(char)*(MAX_BUFSIZE+subDocsDataSize));
	temp = tempBuf;
	len = append_str(temp, "HTTP 200 OK\r\n");
	temp += len;
	len = add_header("Content-type: multipart/mixed; boundary=",boundary,temp);
	temp += len;
	len = add_header("Etag: ",rootVersion,temp);
	temp += len;
	len = append_str(temp, "\n");
	temp += len;
	for (j = 0; j<subDocCount; j++)
	{
		len = add_header("--",boundary,temp);
		temp += len;
		char *subDocBuffer = NULL;
		subdocLen = getSubDocBuffer(subdocs[j], &subDocBuffer);
		printf("subdocLen: %d\n", subdocLen);
		strncpy(temp, subDocBuffer, subdocLen);
		temp += subdocLen;
		free(subDocBuffer);
	}
	len = append_str(temp, "--");
	temp += len;
	sprintf(tempBuf,"%s%s--\r\n",tempBuf,boundary);
	*buffer = tempBuf;
	bufLen = (int)(strlen(*buffer));
	printf("bufLen: %d\n",bufLen);
	return bufLen;
}

int main(int argc, char *argv[])
{
	int subDocCount = 0;
	char *rootVersion = NULL;
	multipart_subdoc_t *subdocs = NULL;
	char  * buffer = NULL;
	int bufLen = 0;
	if(argc < 3)
	{
		printf("Usage: ./multipartDoc <root-version> <subDocVersion1,subDocName1,subDocFilePath1,blob> ... <subDocVersionN,subDocNameN,subDocFilePathN,blob>\n");
		exit(1);
	}
	rootVersion = strdup(argv[1]);
	printf("rootVersion: %s\n",rootVersion);
	subDocCount = argc - 2;
	printf("subDocCount: %d\n",subDocCount);
	subdocs = (multipart_subdoc_t  *)malloc(sizeof(multipart_subdoc_t )*subDocCount);
	memset(subdocs, 0, sizeof(multipart_subdoc_t )*subDocCount);
	if(!parseSubDocArgument(argv, argc, &subdocs))
	{
		free(rootVersion);
		free(subdocs);
		return 0;
	}
	bufLen = generateMultipartBuffer(rootVersion, subDocCount, subdocs, &buffer);
	if(bufLen > 0)
	{
		printf("Multipart buffer length is %d\n",bufLen);
		if(writeToFile(MULTIPART_DOC, buffer, bufLen) == 0)
		{
			fprintf(stderr,"%s File not Found\n",MULTIPART_DOC);
		}
	}
	else
	{
		fprintf(stderr, "Failed to generate multipart buffer\n");
	}
	if(subdocs != NULL)
	{
		int i=0;
		for(i=0; i< subDocCount; i++)
		{
			free(subdocs[i].name);
			free(subdocs[i].version);
			free(subdocs[i].data);
		}
		free(subdocs);
	}
	free(buffer);
	free(rootVersion);
	return 0;
}
