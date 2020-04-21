 /**
  * Copyright 2019 Comcast Cable Communications Management, LLC
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
#include <getopt.h>
#include <string.h>
#include <msgpack.h>

const char *filename = NULL;

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
				msgpack_object_print(stdout, obj);
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
				msgpack_object_print(stdout, deserialized);
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
	}
	else
	{
		fprintf(stderr,"File not Found\n");
	}
}

int readBinFromFile(const char *filename, char **data, size_t *len)
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


int main( int argc, char *argv[] )
{
	const char *option_string = "m:b:h::";
        static const struct option options[] = {
		{ "help",  optional_argument, NULL, 'h' },
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
			default:
                		fprintf( stderr, "Usage:\n\nwebcfg_decoder [Type] [filename] \n\n" );
                		fprintf( stderr, "Type\n" );
                		fprintf( stderr, "    -b    if the input is base64 encoded file.\n\n" );
                		fprintf( stderr, "    -m    if the input is msgpack encoded file.\n\n" );
                		return -1;
		}
	}

	return 0;
}

