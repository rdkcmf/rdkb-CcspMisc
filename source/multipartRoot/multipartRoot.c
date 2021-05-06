#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <msgpack.h>
#include <cJSON.h>
#include <stdint.h>
#include <getopt.h>
#include "base64.h"

#define WIFI_METADATA_MAP_SIZE                3
#define MULTIPART_DOC "/nvram/multipart.bin"
#define MAX_BUFSIZE 1024
#define OUTFILE "/tmp/testUtilityTemp.bin"
#define B64OUTFILE "/tmp/b64output.bin"

typedef struct multipart_subdoc
{
	char *name;
	char *version;
	char *data;
	size_t length;
}multipart_subdoc_t;

typedef struct{
    char *name;
    char *value;
    uint32_t value_size;
    uint16_t type;
}dataval_t;

typedef struct{
    size_t count;
    dataval_t *data_items;
} data1_t;

struct webcfg_token {
    const char *name;
    size_t length;
};

typedef struct wifi_appenddoc_struct{
    char * subdoc_name;
    uint32_t  version;
    uint16_t   transaction_id;
    size_t *count;
}wifi_appenddoc_t;
/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
static void packJsonString( cJSON *item, msgpack_packer *pk );
static void packJsonNumber( cJSON *item, msgpack_packer *pk );
static void packJsonArray( cJSON *item, msgpack_packer *pk, int isBlob );
static void packJsonObject( cJSON *item, msgpack_packer *pk, int isBlob);
static void packJsonBool(cJSON *item, msgpack_packer *pk, bool value);
static void packJsonNULL(cJSON *item, msgpack_packer *pk);
static void __msgpack_pack_string( msgpack_packer *pk, const void *string, size_t n );
static int convertJsonToBlob(char *data, char **encodedData, int isBlob);
static int convertJsonToMsgPack(char *data, char **encodedData, int isBlob);
static void decodeMsgpackData(char *encodedData, int encodedDataLen);
static int convertMsgpackToBlob(char *data, int size, char **encodedData);
static char *decodeBlobData(char *data);
static void __msgpack_pack_string_nvp( msgpack_packer *pk,
                                       const struct webcfg_token *token,
                                       const char *val );

static void __msgpack_pack_string_nvp1( msgpack_packer *pk,
                                       const struct webcfg_token *token,
                                       const char *val, int size );

int readFromFile(const char *filename, char **data, size_t *len);
int writeToFile(char *file_path, char *data, size_t size);

/*----------------------------------------------------------------------------*/
/*                             Internal Functions                             */
/*----------------------------------------------------------------------------*/
static void __msgpack_pack_string_nvp( msgpack_packer *pk,
                                       const struct webcfg_token *token,
                                       const char *val )
{
    if( ( NULL != token ) && ( NULL != val ) ) {
        __msgpack_pack_string( pk, token->name, token->length );
        __msgpack_pack_string( pk, val, strlen( val ) );
    }
}

static void __msgpack_pack_string_nvp1( msgpack_packer *pk,
                                       const struct webcfg_token *token,
                                       const char *val, int size )
{
    if( ( NULL != token ) && ( NULL != val ) ) {
        __msgpack_pack_string( pk, token->name, token->length );
        __msgpack_pack_string( pk, val, size );
    }
}

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

static void packJsonNULL(cJSON *item, msgpack_packer *pk)
{
	if(item->string != NULL)
	{
		__msgpack_pack_string(pk, item->string, strlen(item->string));
	}
	msgpack_pack_nil(pk);
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
			case cJSON_NULL:
				packJsonNULL(arrItem, pk);
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
	if(strlen(blobData) > 0)
	{
		printf("%s\n",blobData);
		len = getEncodedBlob(blobData, &encodedBlob);
		printf("%s\n",encodedBlob);
	}
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
			case cJSON_NULL:
				packJsonNULL(child, pk);
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

/*=========================For two msgpack inputs=================================*/
uint16_t generateRandomId()
{
    FILE *fp;
	uint16_t random_key,sz;
	fp = fopen("/dev/urandom", "r");
	if (fp == NULL){
    		return 0;
    	}    	
	sz = fread(&random_key, sizeof(random_key), 1, fp);
	if (!sz)
	{	
		fclose(fp);
		printf("fread failed.\n");
		return 0;
	}
	printf("generateRandomId\n %d",random_key);
	fclose(fp);		
	return(random_key);
}

ssize_t wifi_pack_appenddoc(const wifi_appenddoc_t *appenddocData,void **data)
{
    size_t rv = -1;

    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init( &sbuf );
    msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );
    msgpack_zone mempool;
    msgpack_object deserialized;
    if( appenddocData != NULL )
    {
        struct webcfg_token APPENDDOC_MAP_SUBDOC_NAME;

        APPENDDOC_MAP_SUBDOC_NAME.name = "subdoc_name";
        APPENDDOC_MAP_SUBDOC_NAME.length = strlen( "subdoc_name" );
        __msgpack_pack_string_nvp( &pk, &APPENDDOC_MAP_SUBDOC_NAME, appenddocData->subdoc_name );

        struct webcfg_token APPENDDOC_MAP_VERSION;
             
        APPENDDOC_MAP_VERSION.name = "version";
        APPENDDOC_MAP_VERSION.length = strlen( "version" );
        __msgpack_pack_string( &pk, APPENDDOC_MAP_VERSION.name, APPENDDOC_MAP_VERSION.length );
        msgpack_pack_uint32(&pk,appenddocData->version);

        struct webcfg_token APPENDDOC_MAP_TRANSACTION_ID;
             
        APPENDDOC_MAP_TRANSACTION_ID.name = "transaction_id";
        APPENDDOC_MAP_TRANSACTION_ID.length = strlen( "transaction_id" );
        __msgpack_pack_string( &pk, APPENDDOC_MAP_TRANSACTION_ID.name, APPENDDOC_MAP_TRANSACTION_ID.length );
        msgpack_pack_uint16(&pk, appenddocData->transaction_id);
    }
    else 
    {    
        printf("Doc append data is NULL\n" );
        return rv;
    } 

    if( sbuf.data ) 
    {
        *data = ( char * ) malloc( sizeof( char ) * sbuf.size );

        if( NULL != *data ) 
        {
            memcpy( *data, sbuf.data, sbuf.size );
	    printf("sbuf.data of appenddoc is %s sbuf.size %zu\n", sbuf.data, sbuf.size);
            rv = sbuf.size;
        }
    }

    msgpack_zone_init(&mempool, 2048);

    msgpack_unpack(sbuf.data, sbuf.size, NULL, &mempool, &deserialized);
    msgpack_object_print(stdout, deserialized);

    msgpack_zone_destroy(&mempool);

    msgpack_sbuffer_destroy( &sbuf );
    return rv;   
}

/**
 * @brief alterMapData function to change MAP size of encoded msgpack object.
 *
 * @param[in] encodedBuffer msgpack object
 * @param[out] return 0 in success or less than 1 in failure case
 */

static int alterWifiMapData( char * buf )
{
    //Extract 1st byte from binary stream which holds type and map size
    unsigned char *byte = ( unsigned char * )( &( buf[0] ) ) ;
    int mapSize;
    printf("First byte in hex : %x\n", 0xff & *byte );
    //Calculate map size
    mapSize = ( 0xff & *byte ) % 0x10;

    if( mapSize == 15 )
    {
        printf("Msgpack Map (fixmap) is already at its MAX size i.e. 15\n" );
        return -1;
    }

    *byte = *byte + WIFI_METADATA_MAP_SIZE;
    mapSize = ( 0xff & *byte ) % 0x10;
    printf("New Map size : %d\n", mapSize );
    printf("First byte in hex : %x\n", 0xff & *byte );
    //Update 1st byte with new MAP size
    buf[0] = *byte;
    return 0;
}

char * base64wifiblobencoder(char * blob_data, size_t blob_size, int **encodedLen)
{
	char* b64buffer =  NULL;
	size_t encodeSize = -1;
   	printf("Data is %s\n", blob_data);
     	printf("-----------Start of Base64 Encode ------------\n");
        encodeSize = b64_get_encoded_buffer_size(blob_size);
        printf("encodeSize is %zu\n", encodeSize);
        b64buffer = malloc(encodeSize + 1);
        if(b64buffer != NULL)
        {
            memset( b64buffer, 0, sizeof( encodeSize )+1 );

            b64_encode((uint8_t *)blob_data, blob_size, (uint8_t *)b64buffer);
            b64buffer[encodeSize] = '\0' ;
        }
	**encodedLen = encodeSize;
	return b64buffer;
}

/**
 * @brief appendWifiEncodedData function to append two encoded buffer and change MAP size accordingly.
 * 
 * @note appendWifiEncodedData function allocates memory for buffer, caller needs to free the buffer(appendData)in
 * both success or failure case. use wrp_free_struct() for free
 *
 * @param[in] encodedBuffer msgpack object (first buffer)
 * @param[in] encodedSize is size of first buffer
 * @param[in] metadataPack msgpack object (second buffer)
 * @param[in] metadataSize is size of second buffer
 * @param[out] appendData final encoded buffer after append
 * @return  appended total buffer size or less than 1 in failure case
 */

size_t appendWifiEncodedData( void **appendData, void *encodedBuffer, size_t encodedSize, void *metadataPack, size_t metadataSize )
{
    //Allocate size for final buffer
    *appendData = ( void * )malloc( sizeof( char * ) * ( encodedSize + metadataSize ) );
	if(*appendData != NULL)
	{
		memcpy( *appendData, encodedBuffer, encodedSize );
		//Append 2nd encoded buf with 1st encoded buf
		memcpy( *appendData + ( encodedSize ), metadataPack, metadataSize );
		//Alter MAP
		int ret = alterWifiMapData( ( char * ) * appendData );
		printf("The value of ret in alterMapData  %d\n",ret);
		if( ret ) {
		    return -1;
		}
		return ( encodedSize + metadataSize );
	}
	else
	{
		printf("Memory allocation failed\n" );
	}
    return -1;
}

char * append_wifi_doc(char * subdoc_name, uint32_t version, uint16_t trans_id, char * blob_data, size_t blob_size, int *encodedLen)
{
    wifi_appenddoc_t *wifi_appenddata = NULL;
    size_t wifi_appenddocPackSize = -1;
    size_t wifi_embeddeddocPackSize = -1;
    void *wifi_appenddocdata = NULL;
    void *wifi_embeddeddocdata = NULL;
    char *wifi_finaldocdata = NULL;

    wifi_appenddata = (wifi_appenddoc_t *) malloc(sizeof(wifi_appenddoc_t ));
    if(wifi_appenddata != NULL)
    {   
        memset(wifi_appenddata, 0, sizeof(wifi_appenddoc_t));

        wifi_appenddata->subdoc_name = strdup(subdoc_name);
        wifi_appenddata->version = version;
        wifi_appenddata->transaction_id = trans_id;
	printf("subdoc_name: %s, version: %lu, transaction_id: %hu\n", subdoc_name, (unsigned long)version, trans_id);

    	wifi_appenddocPackSize = wifi_pack_appenddoc(wifi_appenddata, &wifi_appenddocdata);
	//msgpack_print(wifi_appenddocdata, wifi_appenddocPackSize);
    	printf("data packed is %s\n", (char*)wifi_appenddocdata);
 
    	free(wifi_appenddata->subdoc_name);
    	free(wifi_appenddata);

    	wifi_embeddeddocPackSize = appendWifiEncodedData(&wifi_embeddeddocdata, (void *)blob_data, blob_size, wifi_appenddocdata, wifi_appenddocPackSize);
    	printf("wifi_appenddocPackSize: %zu, blobSize: %zu, wifi_embeddeddocPackSize: %zu\n", wifi_appenddocPackSize, blob_size, wifi_embeddeddocPackSize);
    	printf("The wifi_embedded doc data is %s\n",(char*)wifi_embeddeddocdata);
	free(wifi_appenddocdata);
	//msgpack_print(wifi_embeddeddocdata, wifi_embeddeddocPackSize);
        printf("\n");
   	wifi_finaldocdata = base64wifiblobencoder((char *)wifi_embeddeddocdata, wifi_embeddeddocPackSize, &encodedLen);
    	printf("The wifi_encoded append doc is %s\n",wifi_finaldocdata);
	free(wifi_embeddeddocdata);
    }
    return wifi_finaldocdata;
}

ssize_t webcfg_pack_doc(const data1_t *packData, void **data)
{
	size_t rv = -1;
	msgpack_sbuffer sbuf;
	msgpack_packer pk;
	msgpack_sbuffer_init( &sbuf );
	msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );
	int i =0;

	if( packData != NULL && packData->count != 0 )
	{
		int count = packData->count;
		msgpack_pack_map( &pk, 1);
		__msgpack_pack_string( &pk, "PublicHotspotData", strlen("PublicHotspotData") );
		msgpack_pack_array( &pk, count );

		for( i = 0; i < count; i++ ) //1 element
		{
			msgpack_pack_map( &pk, 2); //name, value, type
			struct webcfg_token MAP_NAME;

			MAP_NAME.name = "Name";
			MAP_NAME.length = strlen( "Name" );
			printf("The count is %d\n", count);
			__msgpack_pack_string_nvp( &pk, &MAP_NAME, packData->data_items[i].name );

			struct webcfg_token MAP_VALUE;

			MAP_VALUE.name = "Value";
			MAP_VALUE.length = strlen( "Value" );
			__msgpack_pack_string_nvp1( &pk, &MAP_VALUE, packData->data_items[i].value, (int)packData->data_items[i].value_size );
		}
	}
	else
	{
		printf("parameters is NULL\n" );
		return rv;
	}

	if( sbuf.data )
	{
		*data = ( char * ) malloc( sizeof( char ) * sbuf.size );

		if( NULL != *data )
		{
			memcpy( *data, sbuf.data, sbuf.size );
			//printf("sbuf.data is %s sbuf.size %ld\n", sbuf.data, sbuf.size);
			rv = sbuf.size;
		}
	}

	msgpack_sbuffer_destroy( &sbuf );
	return rv;
}

ssize_t webcfg_pack_rootdoc(const char * blob, void **data, size_t size)
{
	size_t rv = -1;
	msgpack_sbuffer sbuf;
	msgpack_packer pk;
	msgpack_sbuffer_init( &sbuf );
	msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );
	int i =0;

	if( blob != NULL )
	{
		int count = 1;
		msgpack_pack_map( &pk, 1);
		__msgpack_pack_string( &pk, "parameters", strlen("parameters") );
		msgpack_pack_array( &pk, count );

		for( i = 0; i < count; i++ ) //1 element
		{
			msgpack_pack_map( &pk, 3); //name, value, type
			struct webcfg_token MAP_NAME;

			MAP_NAME.name = "name";
			MAP_NAME.length = strlen( "name" );
			printf("The count is %d\n", count);
			__msgpack_pack_string_nvp( &pk, &MAP_NAME, "Device.GRE.X_RDK_PublicHotspotData" );

			struct webcfg_token MAP_VALUE;

			MAP_VALUE.name = "value";
			MAP_VALUE.length = strlen( "value" );
			__msgpack_pack_string_nvp1( &pk, &MAP_VALUE, blob, (int)size);

			struct webcfg_token MAP_DATATYPE;

			MAP_DATATYPE.name = "dataType";
			MAP_DATATYPE.length = strlen( "dataType" );
			__msgpack_pack_string( &pk, MAP_DATATYPE.name, MAP_DATATYPE.length);
			msgpack_pack_uint64(&pk,12);
		}
	}
	else
	{
		printf("parameters is NULL\n" );
		return rv;
	}

	if( sbuf.data )
	{
		*data = ( char * ) malloc( sizeof( char ) * sbuf.size );

		if( NULL != *data )
		{
			memcpy( *data, sbuf.data, sbuf.size );
			//printf("sbuf.data is %s sbuf.size %ld\n", sbuf.data, sbuf.size);
			rv = sbuf.size;
		}
	}

	msgpack_sbuffer_destroy( &sbuf );
	return rv;
}

int processPacking(char *filename1, char *filename2, uint32_t version, char * subdocname)
{
	char *fileData1 = NULL;
	char *fileData2 = NULL;
    	size_t len1, len2 = 0;
	void *packedData, *packedRootData = NULL;
	//data1_t *packRootData = NULL;
	data1_t *packBlobData = NULL;
	size_t rootPackSize, blobPackSize = -1;
	int encodedLen = 0;
	char * encodedData = NULL;
	
	if(readFromFile(filename1, &fileData1, &len1) && readFromFile(filename2, &fileData2, &len2))
	{
		packBlobData = ( data1_t * ) malloc( sizeof( data1_t ) );
		if(packBlobData != NULL && fileData1 != NULL && fileData2 != NULL)
		{
			printf("went here\n");
			memset(packBlobData, 0, sizeof(data1_t));

			packBlobData->count = 2;
			packBlobData->data_items = (dataval_t *) malloc( sizeof(dataval_t) * packBlobData->count );
			memset( packBlobData->data_items, 0, sizeof(dataval_t) * packBlobData->count );

			packBlobData->data_items[0].name = strdup("Device.GRE.X_RDK_TunnelData");
			packBlobData->data_items[0].value = malloc(sizeof(char) * len1+1);
			memset(packBlobData->data_items[0].value, 0, sizeof(char) * len1+1);
			packBlobData->data_items[0].value = memcpy(packBlobData->data_items[0].value, fileData1, len1+1);
			packBlobData->data_items[0].value[len1] = '\0';
			packBlobData->data_items[0].value_size = len1;
			printf("went here2\n");
			packBlobData->data_items[0].type = 12;

			packBlobData->data_items[1].name = strdup("Device.WiFi.X_RDK_VapData");
			packBlobData->data_items[1].value = malloc(sizeof(char) * len2+1);
			memset(packBlobData->data_items[1].value, 0, sizeof(char) * len2+1);
			packBlobData->data_items[1].value = memcpy(packBlobData->data_items[1].value, fileData2, len2+1);
			packBlobData->data_items[1].value[len2] = '\0';
			packBlobData->data_items[1].value_size = len2;
			packBlobData->data_items[1].type = 12;
		}
		else
		{
			printf("Failed in memory allocation\n");
			free(fileData1);
			free(fileData2);
			return 0;
		}

		printf("Before here\n");
		blobPackSize = webcfg_pack_doc( packBlobData, &packedData);
		printf("blobPackSize is %zu\n", blobPackSize);
		
		if(blobPackSize > 0)
		{
			encodedData = append_wifi_doc(subdocname, version, generateRandomId(), (char *)packedData, blobPackSize, &encodedLen);
			if(writeToFile(B64OUTFILE, (char *)encodedData, (size_t)encodedLen) == 0)
			{
				fprintf(stderr,"%s File not Found\n", B64OUTFILE);
				free(encodedData);
				return 0;
			}
			free(encodedData);
			rootPackSize = webcfg_pack_rootdoc( (const char *)packedData, &packedRootData, blobPackSize);
			printf("rootPackSize is %zu\n", rootPackSize);
		}
		else
		{
			printf("Failed in memory allocation\n");
			free(fileData1);
			free(fileData2);
			free(packedData);
			return 0;
		}

		if(rootPackSize > 0 )
		{
			printf("Packing is success. Hence writing Packed data to %s\n", OUTFILE);
			if(writeToFile(OUTFILE, (char *)packedRootData, rootPackSize) == 0)
			{
				fprintf(stderr,"%s File not Found\n", OUTFILE);
				free(packedRootData);
				return 0;
			}
			free(packedRootData);
		}
		free(fileData1);
		free(fileData2);
		free(packedData);
	}
	else
	{
		fprintf(stderr,"File not Found\n");
		return 0;
	}
	return 1;
}

/*=========================For two msgpack inputs=================================*/

int parseSubDocArgument(char **args, int count, multipart_subdoc_t **docs)
{
	int i = 0, j = 0, isBlob = 0;
	char *fileName1 = NULL;
	char *fileName2 = NULL;
	char *fileData = NULL;
	char outFile[128];
    	size_t len = 0;
	uint32_t version = 0;
	int multilevelBlob = 0;
	char *tempStr= NULL, *docData = NULL, *blobStr = NULL;
	for (i = 2; i<count; i++)
	{
		isBlob = 0;
		docData = strdup(args[i]);
		tempStr = docData;
		(*docs)[j].version = strdup(strsep(&tempStr,","));
		version = strtoul((*docs)[j].version, 0 ,0);
		(*docs)[j].name = strdup(strsep(&tempStr,","));
		fileName1 = strdup(strsep(&tempStr,","));
		if(tempStr != NULL)
		{
			blobStr = strsep(&tempStr,",");
			if(strcmp(blobStr, "blob") == 0)
			{
				isBlob = 1;
			}
			else
			{
				fileName2 = strdup(blobStr);
				multilevelBlob = 1;
			}
		}
		free(docData);
		if(!multilevelBlob)
		{
			if(processEncoding(fileName1, "M", isBlob))
			{
				sprintf(outFile,"%s.bin",strtok(fileName1,"."));
				if(readFromFile(outFile, &fileData, &len))
				{
					(*docs)[j].data = (char  *)malloc(sizeof(char)*len);
					memcpy((*docs)[j].data, fileData, len);
					(*docs)[j].length = len;
					free(fileData);
				}
				else
				{
					printf("Failed to open %s\n",fileName1);
					(*docs)[j].data = NULL;
					(*docs)[j].length = 0;
				}
			}
			else
			{
		                        /* CID: 155140 Resource leak*/
		                        free(fileName1);
					return 0;
			}
		}
		else
		{
			if(processPacking(fileName1, fileName2, version, (*docs)[j].name))
			{
				if(readFromFile(OUTFILE, &fileData, &len))
				{
					(*docs)[j].data = (char  *)malloc(sizeof(char)*len);
					memcpy((*docs)[j].data, fileData, len);
					(*docs)[j].length = len;
					free(fileData);
				}
				else
				{
					printf("Failed to open %s\n",OUTFILE);
					(*docs)[j].data = NULL;
					(*docs)[j].length = 0;
				}
			}
			else
			{
	                        free(fileName1);
				free(fileName2);
				return 0;
			}
			
		}
		if(fileName1 != NULL)
		{
			free(fileName1);
		}
		if(fileName2 != NULL)
		{
			free(fileName2);
		}
		j++;
	}
	return 1;
}

int append_str (char * dest, char * src, int destLength)
{
	memcpy (dest, src, destLength);
	return destLength;
}

void generateBoundary(char *s, int len ) 
{
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
    /* CID: 155144 Out-of-bounds access*/
    s[len - 1] = '\0';
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
	bufLength += length;
	pHdr += length;
	length = add_header("Etag: ",subdoc.version, pHdr);
	bufLength += length;
	pHdr += length;
	length = add_header("Namespace: ",subdoc.name, pHdr);
	bufLength += length;
	pHdr += length;
	length = append_str(pHdr, "\r\n", 2);
	bufLength += length;
	pHdr += length;
	length = append_str(pHdr, subdoc.data, subdoc.length);
	bufLength += length;
	pHdr += length;
	length = append_str(pHdr, "\r\n", 2);
	bufLength += length;
	pHdr += length;
	*buffer = hdrbuf;
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
	char boundary[50];
	char *temp = NULL;
	char  * tempBuf = NULL;
	int subDocsDataSize = 0, subdocLen = 0, bufLen = 0, len = 0, j = 0;
	generateBoundary(boundary, sizeof(boundary));
	printf("boundary: %s\n",boundary);
	subDocsDataSize = getSubDocsDataSize(subdocs, subDocCount);
	printf("Subdocs data size: %d\n",subDocsDataSize);
	printf("Allocated memory to buffer is %d\n",MAX_BUFSIZE+subDocsDataSize);
	tempBuf = (char *)malloc(sizeof(char)*((MAX_BUFSIZE*2)+subDocsDataSize));
	memset (tempBuf, 0, sizeof(char)*(MAX_BUFSIZE+subDocsDataSize));
	temp = tempBuf;
	len = append_str(temp, "HTTP 200 OK\r\n", strlen("HTTP 200 OK")+2);
	bufLen += len;
	temp += len;
	len = add_header("Content-type: multipart/mixed; boundary=",boundary,temp);
	bufLen += len;
	temp += len;
	len = add_header("Etag: ",rootVersion,temp);
	bufLen += len;
	temp += len;
	len = append_str(temp, "\n", 1);
	bufLen += len;
	temp += len;
	for (j = 0; j<subDocCount; j++)
	{
		len = add_header("--",boundary,temp);
		bufLen += len;
		temp += len;
		char *subDocBuffer = NULL;
		subdocLen = getSubDocBuffer(subdocs[j], &subDocBuffer);
		printf("subdocLen: %d\n", subdocLen);
		memcpy(temp, subDocBuffer, subdocLen);
		bufLen += subdocLen;
		temp += subdocLen;
		free(subDocBuffer);
	}
	len = append_str(temp, "--", 2);
	bufLen += len;
	temp += len;
	len = append_str(temp, boundary, strlen(boundary));
	bufLen += len;
	temp += len;
	len = append_str(temp, "--\r\n", 4);
	bufLen += len;
	temp += len;
	*buffer = tempBuf;
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
