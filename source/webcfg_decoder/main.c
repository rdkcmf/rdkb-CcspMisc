 /**
  * If not stated otherwise in this file or this component's Licenses.txt
  * file the following copyright and licenses apply:
  *
  * Copyright 2020 RDK Management
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <curl/curl.h>
#include <stdarg.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <msgpack.h>
#include "base64.h"
#include <rbus.h>

#define MAX_HEADER_LEN		   4096
#define MAX_BUF_SIZE	           256
#define CURL_TIMEOUT_SEC	   25L
#define SUBDOC_TAG_COUNT            4
#define WEBCFG_RBUS_METHOD         "Device.X_RDK_WebConfig.FetchCachedBlob"

struct token_data {
    size_t size;
    char* data;
};

const char *url = NULL;
const char *filename = NULL;
const char * docname = NULL;
const char * authfile = NULL;
const char *interface = NULL;
static bool docflag = false;
static int readBinFromFile(const char *filename, char **data, size_t *len);

int checkObject(const char * buf, size_t len)
{
    size_t offset = 0;
    msgpack_unpacked msg;
    msgpack_unpack_return mp_rv;

    msgpack_unpacked_init( &msg );

    /* The outermost wrapper MUST be a map. */
    mp_rv = msgpack_unpack_next( &msg, (const char*) buf, len, &offset );

    if((msg.data.type == MSGPACK_OBJECT_MAP) && (mp_rv == 2))
    {
         msgpack_unpacked_destroy(&msg);
         return 1;
    }
    else
    {
         msgpack_unpacked_destroy(&msg);
         return 0;
    }
}

void print_msgpack_object(FILE* fp, msgpack_object obj)
{
    switch(obj.type)
    {
        case MSGPACK_OBJECT_NIL:
            fprintf(fp, "nil");
            break;

        case MSGPACK_OBJECT_BOOLEAN:
            fprintf(fp, (obj.via.boolean ? "true" : "false"));
            break;

        case MSGPACK_OBJECT_POSITIVE_INTEGER:
            if (obj.via.u64 > ULONG_MAX)
                fprintf(fp, "over 4294967295");
            else
                fprintf(fp, "%lu", (unsigned long)obj.via.u64);
            break;

        case MSGPACK_OBJECT_NEGATIVE_INTEGER:
            if (obj.via.i64 > LONG_MAX)
                fprintf(fp, "over +2147483647");
            else if (obj.via.i64 < LONG_MIN)
                fprintf(fp, "under -2147483648");
            else
                fprintf(fp, "%ld", (signed long)obj.via.i64);
            break;

        case MSGPACK_OBJECT_FLOAT32:
        case MSGPACK_OBJECT_FLOAT64:
            fprintf(fp, "%f", obj.via.f64);
            break;

        case MSGPACK_OBJECT_STR:
            if(checkObject(obj.via.str.ptr,obj.via.str.size) == 1)
            {
                size_t offset = 0;
                msgpack_unpacked msg;

                msgpack_unpacked_init( &msg );

                /* The outermost wrapper MUST be a map. */
                msgpack_unpack_next( &msg, (const char*) obj.via.str.ptr, obj.via.str.size, &offset );

                msgpack_object* p = &msg.data;
                print_msgpack_object(fp, *p);
		msgpack_unpacked_destroy(&msg);
            }
            else
            {
		fprintf(fp, "\"");
                fwrite(obj.via.str.ptr, obj.via.str.size, 1, fp);
		fprintf(fp, "\"");
            }
            break;

        case MSGPACK_OBJECT_ARRAY:
            fprintf(fp, "[");
            if(obj.via.array.size != 0)
            {
                msgpack_object* p = obj.via.array.ptr;
                msgpack_object* const pend = obj.via.array.ptr + obj.via.array.size;
                print_msgpack_object(fp, *p);
                ++p;
                for(; p < pend; ++p)
                {
                    fprintf(fp, ", ");
                    print_msgpack_object(fp, *p);
                }
            }
            fprintf(fp, "]");
            break;

        case MSGPACK_OBJECT_MAP:
            fprintf(fp, "{");
            if(obj.via.map.size != 0)
            {
                msgpack_object_kv* p = obj.via.map.ptr;
                msgpack_object_kv* const pend = obj.via.map.ptr + obj.via.map.size;
                print_msgpack_object(fp, p->key);
                fprintf(fp, ":");
                print_msgpack_object(fp, p->val);
                ++p;
                for(; p < pend; ++p)
                {
                    fprintf(fp, ", ");
                    print_msgpack_object(fp, p->key);
                    fprintf(fp, ":");
                    print_msgpack_object(fp, p->val);
                }
            }
            fprintf(fp, "}");
            break;

        default:
            if (obj.via.u64 > ULONG_MAX)
                fprintf(fp, "#<UNKNOWN %i over 4294967295>", obj.type);
            else
                fprintf(fp, "#<UNKNOWN %i %lu>", obj.type, (unsigned long)obj.via.u64);
    }
}

void msgPackDecoder()
{
	char *data = NULL;
    	size_t len = 0;

	size_t offset = 0;
        msgpack_unpacked msg;
        msgpack_unpack_return unpack_ret;

        msgpack_unpacked_init( &msg );

	if(readBinFromFile(filename, &data, &len))
	{
		unpack_ret = msgpack_unpack_next( &msg, (const char*) data, len, &offset );
		msgpack_object obj = msg.data;
		switch(unpack_ret)
		{
			case MSGPACK_UNPACK_SUCCESS:
				printf("\nmsgpack decoded data is:\n");
				print_msgpack_object(stdout, obj);
                    		printf("\n");
			break;
			case MSGPACK_UNPACK_EXTRA_BYTES:
				printf("MSGPACK_UNPACK_EXTRA_BYTES :%d\n",unpack_ret);
			break;
			case MSGPACK_UNPACK_CONTINUE:
				printf("MSGPACK_UNPACK_CONTINUE :%d\n",unpack_ret);
			break;
			case MSGPACK_UNPACK_PARSE_ERROR:
				printf("MSGPACK_UNPACK_PARSE_ERROR :%d\n",unpack_ret);
			break;
			case MSGPACK_UNPACK_NOMEM_ERROR:
				printf("MSGPACK_UNPACK_NOMEM_ERROR :%d\n",unpack_ret);
			break;
			default:
				printf("Message Pack decode failed with error: %d\n", unpack_ret);
		}

		msgpack_unpacked_destroy( &msg );
		free(data);
	}
	else
	{
		fprintf(stderr,"File not Found\n");
	}

}

void b64Decoder()
{
	char *data = NULL;
	char * decodeMsg = NULL;
	size_t decodeMsgSize = 0;
	size_t size = 0;
	size_t len = 0;

	msgpack_zone mempool;
	msgpack_object deserialized;
	msgpack_unpack_return unpack_ret;

	if(readBinFromFile(filename, &data, &len))
	{
		decodeMsgSize = b64_get_decoded_buffer_size(strlen(data));

		decodeMsg = (char *) malloc(sizeof(char) * decodeMsgSize);

		size = b64_decode( (const uint8_t *)data, strlen(data), (uint8_t *)decodeMsg );

		msgpack_zone_init(&mempool, 2048);
		unpack_ret = msgpack_unpack(decodeMsg, size, NULL, &mempool, &deserialized);
		switch(unpack_ret)
		{
			case MSGPACK_UNPACK_SUCCESS:
				printf("\nmsgpack decoded data is:\n");
				print_msgpack_object(stdout, deserialized);
                    		printf("\n");
			break;
			case MSGPACK_UNPACK_EXTRA_BYTES:
				printf("MSGPACK_UNPACK_EXTRA_BYTES :%d\n",unpack_ret);
			break;
			case MSGPACK_UNPACK_CONTINUE:
				printf("MSGPACK_UNPACK_CONTINUE :%d\n",unpack_ret);
			break;
			case MSGPACK_UNPACK_PARSE_ERROR:
				printf("MSGPACK_UNPACK_PARSE_ERROR :%d\n",unpack_ret);
			break;
			case MSGPACK_UNPACK_NOMEM_ERROR:
				printf("MSGPACK_UNPACK_NOMEM_ERROR :%d\n",unpack_ret);
			break;
			default:
				printf("Message Pack decode failed with error: %d\n", unpack_ret);
		}

		msgpack_zone_destroy(&mempool);
		free(decodeMsg);
		free(data);
	}
	else
	{
		fprintf(stderr,"File not Found\n");
	}
}

static int readBinFromFile(const char *filename, char **data, size_t *len)
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
        /* CID: 143581 Argument cannot be negative*/
        if (ch_count < 0){
            fclose(fp);
            return 0;
        }
        fseek(fp, 0, SEEK_SET);
        *data = (char *) malloc(sizeof(char) * (ch_count + 1));
        /*CID: 143582 Ignoring number of bytes read*/
        if (1 != fread(*data, ch_count, 1, fp)){
            fclose(fp);
            return 0;
        }
        *len = (size_t)ch_count;
        fclose(fp);
        return 1;
}

/* @brief callback function for writing libcurl received data
 * @param[in] buffer curl delivered data which need to be saved.
 * @param[in] size size is always 1
 * @param[in] nmemb size of delivered data
 * @param[out] data curl response data saved.
*/
size_t cli_writer_callback_fn(void *buffer, size_t size, size_t nmemb, struct token_data *data)
{
    size_t index = data->size;
    size_t n = (size * nmemb);
    char* tmp;
    data->size += (size * nmemb);

    tmp = realloc(data->data, data->size + 1); // +1 for '\0'

    if(tmp) {
        data->data = tmp;
    } else {
        if(data->data) {
            free(data->data);
        }
        printf("Failed to allocate memory for data\n");
        return 0;
    }
    memcpy((data->data + index), buffer, n);
    data->data[data->size] = '\0';
    //printf("size * nmemb is %zu\n", size * nmemb);
    return size * nmemb;
}

int getauthtokenfromfile(const char * authfile, char **data)
{
	FILE *file;
	size_t size;
	int char_count = 0;
	file = fopen(authfile, "r");
	if (file == NULL)
	{
		printf("Failed to open file %s\n", authfile);
		return 0;
	}
	fseek(file, 0, SEEK_END);
	char_count = ftell(file);
	fseek(file, 0, SEEK_SET);
	//printf("The char_count is %d\n", char_count);
	*data = (char *) malloc(sizeof(char) * (char_count + 1));
	size = fread(*data, 1, char_count,file);
	if (!size)
	{
		fclose(file);
		printf("fread failed.\n");
		free(*data);
		return 0;
	}
	(*data)[char_count] ='\n';
	fclose(file);
	return 1;

}
/* @brief Function to create curl header options
 * @param[in] list temp curl header list
 * @param[in] device status value
 * @param[out] header_list output curl header list
*/
void cli_createCurlHeader( struct curl_slist *list, struct curl_slist **header_list, char * versionlist)
{
	char *version_header = NULL;
	char *auth_header = NULL;
	char *authdata = NULL;
	//printf("Start of createCurlheader\n");
	//Fetch auth JWT token from cloud.

	if(getauthtokenfromfile(authfile, &authdata) != 1)
	{
		printf("Unable to get auth code from file\n");
		if(authdata != NULL)
		{
			free(authdata);
			return;
		}
	}

	auth_header = (char *) malloc(sizeof(char)*MAX_HEADER_LEN);
	if(auth_header !=NULL)
	{
		snprintf(auth_header, MAX_HEADER_LEN, "Authorization:Bearer %s", authdata);
		list = curl_slist_append(list, auth_header);
		free(auth_header);
		free(authdata);
	}


	version_header = (char *) malloc(sizeof(char)*MAX_BUF_SIZE);
	if(version_header !=NULL)
	{
		snprintf(version_header, MAX_BUF_SIZE, "IF-NONE-MATCH:%s", versionlist);
		//printf("version_header formed %s\n", version_header);
		list = curl_slist_append(list, version_header);
		free(version_header);
	}
	list = curl_slist_append(list, "Accept: application/msgpack");

	*header_list = list;
}

//&webConfigData, &res_Code, &transaction_uuid, ct, &dataSize, url, doclist, versionlist))
int cli_getHttpResponse(char **configData, long *code, char* contentType, size_t *dataSize, const char* url, char* doclist, char* versionlist)
{
	CURL *curl;
	CURLcode res;
	CURLcode time_res;
	struct curl_slist *list = NULL;
	struct curl_slist *headers_list = NULL;
	double total;
	long res_code = 0;
	char *ct = NULL;
	char *webConfigURL = NULL;
	int rv = 0;

	struct token_data data1;
	data1.size = 0;
	void * dataVal = NULL;
	char syncURL[256]={'\0'};

	curl = curl_easy_init();
	if(curl)
	{
		//this memory will be dynamically grown by write call back fn as required
		data1.data = (char *) malloc(sizeof(char) * 1);
		if(NULL == data1.data)
		{
			printf("Failed to allocate memory.\n");
			return 1;
		}
		data1.data[0] = '\0';
		cli_createCurlHeader(list, &headers_list, versionlist);

		if(url != NULL)
		{
			webConfigURL = strdup(url);
		}
		else
		{
			printf("Url is NULL\n");
			free(data1.data);
			curl_slist_free_all(headers_list);
			curl_easy_cleanup(curl);
			return 1;
		}

		if(strlen(webConfigURL) == 0)
		{
			printf("Failed to get configURL\n");
			free(data1.data);
			curl_slist_free_all(headers_list);
			curl_easy_cleanup(curl);
			return 1;
		}

		//printf("webConfigURL fetched is %s\n", webConfigURL);

		if(strlen(doclist) > 0)
		{
			//printf("doclist is %s\n", doclist);
			snprintf(syncURL, MAX_BUF_SIZE, "%s?group_id=%s", webConfigURL, doclist);
			free(webConfigURL);
			//printf("syncURL is %s\n", syncURL);
			webConfigURL =strdup( syncURL);
		}

		if(webConfigURL !=NULL)
		{
			//printf("Webconfig root ConfigURL is %s\n", webConfigURL);
			res = curl_easy_setopt(curl, CURLOPT_URL, webConfigURL );
		}
		else
		{
			printf("Failed to get webconfig configURL\n");
			free(data1.data);
			curl_slist_free_all(headers_list);
			curl_easy_cleanup(curl);
			return 1;
		}
		res = curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURL_TIMEOUT_SEC);

		if(interface !=NULL && strlen(interface) >0)
	        {
	                //printf("setting interface %s\n", interface);
			res = curl_easy_setopt(curl, CURLOPT_INTERFACE, interface);
	        }
		//printf("interface fetched is %s\n", interface);

		// set callback for writing received data
		dataVal = &data1;
		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cli_writer_callback_fn);
		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, dataVal);

		res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_list);

		//res = curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headr_callback);

		res = curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER);

		//res = curl_easy_setopt(curl, CURLOPT_CAINFO, CA_CERT_PATH);
		// disconnect if it is failed to validate server's cert
		res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		// Verify the certificate's name against host
		res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		// To use TLS version 1.2 or later
		res = curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
		// To follow HTTP 3xx redirections
		res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
		// Perform the request, res will get the return code
		res = curl_easy_perform(curl);

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res_code);
		//printf("webConfig curl response %d http_code %ld\n", res, res_code);
		*code = res_code;
		time_res = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total);
		if(time_res == 0)
		{
			//printf("curl response Time: %.1f seconds\n", total);
		}
		curl_slist_free_all(headers_list);
		free(webConfigURL);
		if(res != 0)
		{
			printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		else
		{
			if(res_code == 200)
                        {
				//printf("checking content type\n");
				curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
				//printf("ct is %s, content_res is %d\n", ct, content_res);

				if(ct !=NULL)
				{
					if(strncmp(ct, "multipart/mixed", 15) !=0)
					{
						printf("Content-Type is not multipart/mixed. Invalid\n");
					}
					else
					{
						//printf("Content-Type is multipart/mixed. Valid\n");
						strcpy(contentType, ct);

						*configData=data1.data;
						*dataSize = data1.size;
						//printf("Data size is %d\n",(int)data1.size);
						rv = 1;
					}
				}
			}
		}
		if(rv != 1)
		{
			free(data1.data);
		}
		curl_easy_cleanup(curl);
		return 0;
	}
	else
	{
		printf("curl init failure\n");
	}
	return 1;
}

void cli_line_parser(char *ptr, int no_of_bytes, char **name_space, uint32_t *etag, char **data, size_t *data_size)
{
	char *content_type = NULL;

	/*for storing respective values */
	if(0 == strncmp(ptr,"Content-type: ",strlen("Content-type")))
	{
		content_type = strndup(ptr+(strlen("Content-type: ")),no_of_bytes-((strlen("Content-type: "))));
		if(strncmp(content_type, "application/msgpack",strlen("application/msgpack")) !=0)
		{
			//printf("Content-type not msgpack: %s", content_type);
		}
		free(content_type);
	}
	else if(0 == strncasecmp(ptr,"Namespace",strlen("Namespace")))
	{
	        *name_space = strndup(ptr+(strlen("Namespace: ")),no_of_bytes-((strlen("Namespace: "))));
	}
	else if(0 == strncasecmp(ptr,"Etag",strlen("Etag")))
	{
	        char * temp = strndup(ptr+(strlen("Etag: ")),no_of_bytes-((strlen("Etag: "))));
		if(temp)
		{
			*etag = strtoul(temp,0,0);
			free(temp);
		}
	}
	else if(strstr(ptr,"parameters"))
	{
		*data = malloc(sizeof(char) * no_of_bytes );
		*data = memcpy(*data, ptr, no_of_bytes );
		//store doc size of each sub doc
		*data_size = no_of_bytes;
	}

}
void cli_subdoc_parser(char *ptr, int no_of_bytes)
{
	char *name_space = NULL;
	char *data = NULL;
	uint32_t  etag = 0;
	size_t data_size = 0;

	char* str_body = NULL;
	str_body = malloc(sizeof(char) * no_of_bytes + 1);
	str_body = memcpy(str_body, ptr, no_of_bytes + 1);

	char *ptr_lb=str_body;
	char *ptr_lb1=str_body;
	int index1=0, index2 =0;
	int count = 0;

	while((ptr_lb - str_body) < no_of_bytes)
	{
		if(count < SUBDOC_TAG_COUNT)
		{
			ptr_lb1 =  memchr(ptr_lb+1, '\n', no_of_bytes - (ptr_lb - str_body));
			if(0 != memcmp(ptr_lb1-1, "\r",1 ))
			{
				ptr_lb1 = memchr(ptr_lb1+1, '\n', no_of_bytes - (ptr_lb - str_body));
			}
			index2 = ptr_lb1-str_body;
			index1 = ptr_lb-str_body;
			cli_line_parser(str_body+index1+1,index2 - index1 - 2, &name_space, &etag, &data, &data_size);
			ptr_lb++;
			ptr_lb = memchr(ptr_lb, '\n', no_of_bytes - (ptr_lb - str_body));
			count++;
		}
		else             //For data bin segregation
		{
			index2 = no_of_bytes+1;
			index1 = ptr_lb-str_body;
			cli_line_parser(str_body+index1+1,index2 - index1 - 2, &name_space, &etag, &data, &data_size);
			break;
		}
	}

	if(etag != 0 && (name_space != NULL && strcmp(name_space, docname)==0) && data != NULL && data_size != 0 )
	{
	    docflag = true;
	    printf("The docname is %s\n", docname);
	    printf("The Etag version is %lu\n",(long)etag);
            size_t offset = 0;
	    msgpack_unpacked msg;

            msgpack_unpacked_init( &msg );

            /* The outermost wrapper MUST be a map. */
            msgpack_unpack_next( &msg, (const char*) data, data_size, &offset );
	    msgpack_object obj = msg.data;
            print_msgpack_object(stdout, obj);
	    //printf("\n\n\nMSGPACK_OBJECT_MAP is %d  msg.data.type %d unpack ret %d\n\n", MSGPACK_OBJECT_MAP, msg.data.type, mp_rv);
            msgpack_unpacked_destroy(&msg);

	}

	if(name_space != NULL)
	{
		free(name_space);
	}

	if(data != NULL)
	{
		free(data);
	}

        if(str_body != NULL)
	{
		free(str_body);
	}
}

int cli_parseMultipartDocument(void *config_data, char *ct , size_t data_size)
{
	char *boundary = NULL;
	char *str=NULL;
	char *line_boundary = NULL;
	char *last_line_boundary = NULL;
	char *str_body = NULL;
	int boundary_len =0;
	int count =0;

	//printf("ct is %s\n", ct );
	// fetch boundary
	str = strtok(ct,";");
	str = strtok(NULL, ";");
	boundary= strtok(str,"=");
	boundary= strtok(NULL,"=");
	//printf( "boundary %s\n", boundary );

	if(boundary !=NULL)
	{
		boundary_len= strlen(boundary);
		line_boundary  = (char *)malloc(sizeof(char) * (boundary_len +5));
		snprintf(line_boundary,boundary_len+5,"--%s\r\n",boundary);
		//printf( "line_boundary %s, len %zu\n", line_boundary, strlen(line_boundary) );

		last_line_boundary  = (char *)malloc(sizeof(char) * (boundary_len + 5));
		snprintf(last_line_boundary,boundary_len+5,"--%s--",boundary);
		//printf( "last_line_boundary %s, len %zu\n", last_line_boundary, strlen(last_line_boundary) );
		// Use --boundary to split
		str_body = malloc(sizeof(char) * data_size + 1);
		str_body = memcpy(str_body, config_data, data_size + 1);
		free(config_data);
		int part_count = 0;
		char *ptr=str_body;
		char *ptr1=str_body;
		char *ptr_counter = str_body;
		int index_1=0, index_2 =0;

		/* For Subdocs count */
		while((ptr_counter - str_body) < (int)data_size )
		{
			ptr_counter = memchr(ptr_counter, '-', data_size - (ptr_counter - str_body));
			if(0 == memcmp(ptr_counter, last_line_boundary, strlen(last_line_boundary)))
			{
				part_count++;
				break;
			}
			else if(0 == memcmp(ptr_counter, line_boundary, strlen(line_boundary)))
			{
				part_count++;
			}
			ptr_counter++;
		}
		//printf("Size of the docs is :%d\n", (part_count-1));

		while((ptr - str_body) < (int)data_size)
		{
			ptr = memchr(ptr, '-', data_size - (ptr - str_body));
			if(0 == memcmp(ptr, last_line_boundary, strlen(last_line_boundary)))
			{
				//printf("last line boundary \n");
				break;
			}
			else if(0 == memcmp(ptr, line_boundary, strlen(line_boundary)))
			{
				ptr = ptr+(strlen(line_boundary))-1;
				ptr1 = ptr+1;
				part_count = 1;
				while(0 != part_count % 2)
				{
					ptr1 = memchr(ptr1, '-', data_size - (ptr1 - str_body));
					if(0 == memcmp(ptr1, last_line_boundary, strlen(last_line_boundary)))
					{
						index_2 = ptr1-str_body;
						index_1 = ptr-str_body;
						cli_subdoc_parser(str_body+index_1,index_2 - index_1 - 2);
						break;
					}
					else if(0 == memcmp(ptr1, line_boundary, strlen(line_boundary)))
					{
						index_2 = ptr1-str_body;
						index_1 = ptr-str_body;
						cli_subdoc_parser(str_body+index_1,index_2 - index_1 - 2);
						part_count++;
						count++;
					}
					ptr1 = memchr(ptr1, '\n', data_size - (ptr1 - str_body));
					ptr1++;
				}
			}
			ptr = memchr(ptr, '\n', data_size - (ptr - str_body));
			ptr++;
		}
		free(str_body);
		free(line_boundary);
		free(last_line_boundary);

		if(docflag)
		{

			printf("\n\nprocessMsgpackSubdoc success\n");
			return 0;
		}
		else
		{
			printf("\n\nThe subdoc is not present or entered subdoc name is not valid\n");
		}
		return 1;
	}
    else
    {
		printf("Multipart Boundary is NULL\n");
		return 1;
    }


}

void cloudConfig(char **args)
{
	char *webConfigData = NULL;
	long res_code;
	size_t dataSize=0;
	char ct[256] = {0};
	//printf("Proceeding to client_util method\n");

	url = strdup(args[2]);
	docname = strdup(args[3]);
	authfile = strdup(args[4]);
	interface = strdup(args[5]);

	//printf("The url is %s\nThe doclist is %s\nThe versionlist is %s\n", url, doclist, versionlist);

	if(cli_getHttpResponse(&webConfigData, &res_code, ct, &dataSize, url, "root", "0"))
	{
		//Do nothing
	}
	//printf("The res_code is %ld\n", res_code);

/*HANDLE THE RESPONSE*/

	int first_digit=0;
	if(res_code == 304)
	{
		printf("webConfig is in sync with cloud. res_code:%ld\n", res_code);
	}
	else if(res_code == 200)
	{
		printf("webConfig is not in sync with cloud. res_code:%ld\n\n\n", res_code);

		if(webConfigData !=NULL && (strlen(webConfigData)>0))
		{
			cli_parseMultipartDocument(webConfigData, ct, dataSize);
		}
		else
		{
			printf("webConfigData is empty\n");
		}
	}
	else if(res_code == 204)
	{
		printf("No configuration available for this device. res_code:%ld\n", res_code);
	}
	else if(res_code == 403)
	{
		printf("Token is expired/invalid, use new valid token. res_code:%ld\n", res_code);
	}
	else if(res_code == 429)
	{
		printf("No action required from client. res_code:%ld\n", res_code);
	}
	first_digit = (int)(res_code / pow(10, (int)log10(res_code)));
	if((res_code !=403) && (first_digit == 4)) //4xx
	{
		printf("Action not supported. res_code:%ld\n", res_code);
	}
	else //5xx & all other errors
	{
		printf("Error code returned. res_code:%ld\n", res_code);
	}

	return;
}

int rbusFetch(char **args)
{
	rbusHandle_t handle;
	rbusObject_t inParams;
	rbusObject_t outParams;
	rbusValue_t value;

	int rc = RBUS_ERROR_SUCCESS;

	char * docname = NULL;
	docname = strdup(args[2]);

	rc = rbus_open(&handle, "webcfg_decoder");
	if(rc != RBUS_ERROR_SUCCESS)
	{
		printf("rbus_open failed: %d\n", rc);
		goto exit1;
	}

	rbusObject_Init(&inParams, NULL);

	rbusValue_Init(&value);
	rbusValue_SetString(value, docname);
	rbusObject_SetValue(inParams, "docname", value);
	rbusValue_Release(value);

	rc = rbusMethod_Invoke(handle, WEBCFG_RBUS_METHOD, inParams, &outParams);

	rbusObject_Release(inParams);

	if(rc == RBUS_ERROR_SUCCESS)
	{
		const uint8_t * data = NULL;
		uint32_t etag = 0;
		int len = 0;
		size_t offset = 0;
		msgpack_unpacked msg;

		msgpack_unpacked_init( &msg );

		rbusValue_t etagValue = rbusObject_GetValue(outParams, "etag");
		if(etagValue != NULL)
		{
			etag = rbusValue_GetUInt32(etagValue);
		}

		rbusValue_t blobValue = rbusObject_GetValue(outParams, "data");
		if(blobValue != NULL)
		{
			data = rbusValue_GetBytes(blobValue, &len);
		}

		printf("\n");
		printf("The docname is %s\n", docname);
		printf("Etag version is %lu\n", (long) etag);
		printf("\n");
		msgpack_unpack_next( &msg, (const char*) data, len, &offset );
		msgpack_object obj = msg.data;
		print_msgpack_object(stdout, obj);
		printf("\n");
		msgpack_unpacked_destroy(&msg);
	}
	else
	{
		uint8_t error_code = 0;
		char * error_string = NULL;
		int len = 0;
		
		rbusValue_t errCodeVal = rbusObject_GetValue(outParams, "error_code");
		if(errCodeVal != NULL)
		{
			error_code = rbusValue_GetUInt8(errCodeVal);
		}

		rbusValue_t errStrValue = rbusObject_GetValue(outParams, "error_string");
		if(errStrValue != NULL)
		{
			error_string = (char *)rbusValue_GetString(errStrValue, &len);
		}

		printf("rbusMethod_Invoke failed for %s with errno: %d and err: '%s'\n\r", WEBCFG_RBUS_METHOD, (int)error_code, error_string);
	}

	rbusObject_Release(outParams);
	rbus_close(handle);

	free(docname);

	exit1:

	return rc;
}

int main( int argc, char *argv[] )
{
	const char *option_string = "m:b:c:f:h::";
        static const struct option options[] = {
		{ "help",  optional_argument, NULL, 'h' },
		{ "cloudconfig", required_argument, NULL, 'c' },
		{ "mpfetch", required_argument, NULL, 'f' },
		{ "base64",  required_argument, NULL, 'b' },
		{ "msgpack", required_argument, NULL, 'm' },
		{ NULL, 0, NULL, 0 }
    	};
	int i = 0;
	int ch = 0;

	while( -1 != (ch = getopt_long(argc, argv, option_string, options, &i)) )
	{
		switch( ch )
		{
			case 'b':
				filename = optarg;
				if( NULL == filename )
				{
        				fprintf( stderr, "Filename is missing.\n" );
        				return -2;
   				}
				b64Decoder();
				break;
			case 'm':
				filename = optarg;
				if( NULL == filename )
				{
        				fprintf( stderr, "Filename is missing.\n" );
        				return -2;
   				}
				msgPackDecoder();
				break;
			case 'c':
				if(argc == 6)
				{
					cloudConfig(argv);
				}
				else
				{
					fprintf( stderr, "Invalid inputs, please check webcfg_decoder -h for usage\n");
					return -2;
				}
				break;
			case 'f':
				if(argc == 3)
				{
					rbusFetch(argv);
				}
				else
				{
					fprintf( stderr, "Invalid inputs, please check webcfg_decoder -h for usage\n");
					return -2;
				}
				break;
			default:
                		fprintf( stderr, "Usage:\n\nwebcfg_decoder [Type] [filename] \n\n" );
                		fprintf( stderr, "Type\n" );
                		fprintf( stderr, "    -b    if the input is base64 encoded file.\n\n" );
                		fprintf( stderr, "    -m    if the input is msgpack encoded file.\n\n" );
				fprintf( stderr, "    -c    if the input is to be fetched from cloud config\n\n" );
				fprintf( stderr, "    -f    if the input is to be fetched from webconfig cache\n\n" );
				fprintf( stderr, "For Type -c use the following format\n\n" );
				fprintf( stderr, "webcfg_decoder -c \"<webconfig_cloud_URL>\" \"<subdocname>\" \"<AuthTokenFileName>\" \"<NetworkInterfaceName>\"\n\n" );
				fprintf( stderr, "For Type -f use the following format\n\n" );
				fprintf( stderr, "webcfg_decoder -f \"<subdocname>\" \n\n" );
                		return -1;
		}
	}

	return 0;
}


