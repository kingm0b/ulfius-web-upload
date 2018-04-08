/**
 * 
 * Webservice IoT S3 PoC
 * 
 * Este serviço é responsável por escutar porta TCP definida
 * pela macro PORT e receber upload de arquivos via HTTP
 * armazenando os arquivos recebidos no path definido pela
 * macro STORAGE_FOLDER.
 *
 * Este fonte foi distribuído sob a licença MIT.
 * O código é derivado do programa sheep_counter, autor original:
 *
 * Copyright 2015-2017 Nicolas Mora <mail@babelouest.org>
 *
 */

#include <string.h>
#include <jansson.h>

#define U_DISABLE_CURL
#define U_DISABLE_WEBSOCKET
#include <ulfius.h>

#include "utils.h"

#define PORT 7437
#define PREFIX "/sheep"
#define FILE_PREFIX "/upload"
#define STATIC_FOLDER "static"
#define STORAGE_FOLDER "/var/cache/aws-storage/files"

int callback_static_file (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_upload_file (const struct _u_request * request, struct _u_response * response, void * user_data);
int file_upload_callback (const struct _u_request * request, 
                          const char * key, 
                          const char * filename, 
                          const char * content_type, 
                          const char * transfer_encoding, 
                          const char * data, 
                          uint64_t off, 
                          size_t size, 
                          void * user_data);

int main (int argc, char **argv) {

  struct _u_instance instance;
  
  y_init_logs("s3_bridge", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Iniciando Webservice...");
  
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
    return(1);
  }
  
  // Max post param size is 16 Kb, which means an uploaded file is no more than 16 Kb
  instance.max_post_param_size = 2048 * 1024;
  
  if (ulfius_set_upload_file_callback_function(&instance, &file_upload_callback, "PRA QUE ISSO SERVE?") != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_set_upload_file_callback_function");
  }
  
  // MIME types that will define the static files
  struct _u_map mime_types;
  u_map_init(&mime_types);
  u_map_put(&mime_types, ".html", "text/html");
  u_map_put(&mime_types, ".css", "text/css");
  u_map_put(&mime_types, ".js", "application/javascript");
  u_map_put(&mime_types, ".png", "image/png");
  u_map_put(&mime_types, ".jpeg", "image/jpeg");
  u_map_put(&mime_types, ".jpg", "image/jpeg");
  u_map_put(&mime_types, "*", "application/octet-stream");
  
  // Endpoint list declaration
  // The first 3 are webservices with a specific url
  // The last endpoint will be called for every GET call and will serve the static files
  ulfius_add_endpoint_by_val(&instance, "*", FILE_PREFIX, NULL, 1, &callback_upload_file, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", "*", NULL, 1, &callback_static_file, &mime_types);
  
  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK) {
    printf("Escutando na porta %d\n", instance.port);
    
    // Wait for the user to press <enter> on the console to quit the application
    getchar();
  } else {
    printf("Error starting framework\n");
  }

  // Clean the mime map
  u_map_clean(&mime_types);
  
  printf("Finalizado\n");
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
  
  y_close_logs();
  
  return 0;
}

int callback_static_file (const struct _u_request * request, struct _u_response * response, void * user_data) {
  void * buffer = NULL;
  long length;
  FILE * f;
  char  * file_path = msprintf("%s%s", STATIC_FOLDER, request->http_url);
  const char * content_type;
  
  if (access(file_path, F_OK) != -1) {
    f = fopen (file_path, "rb");
    if (f) {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = o_malloc(length*sizeof(void));
      if (buffer) {
        fread (buffer, 1, length, f);
      }
      fclose (f);
    }

    if (buffer) {
      content_type = u_map_get((struct _u_map *)user_data, get_filename_ext(request->http_url));
      response->binary_body = buffer;
      response->binary_body_length = length;
      u_map_put(response->map_header, "Content-Type", content_type);
      response->status = 200;
    } else {
      response->status = 404;
    }
  } else {
    response->status = 404;
  }
  o_free(file_path);
  return U_CALLBACK_CONTINUE;
}

int callback_upload_file (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * url_params = print_map(request->map_url), * headers = print_map(request->map_header), * cookies = print_map(request->map_cookie), 
        * post_params = print_map(request->map_post_body);

  ulfius_set_string_body_response(response, 200, "Recebido!");

  o_free(url_params);
  o_free(headers);
  o_free(cookies);
  o_free(post_params);

  return U_CALLBACK_CONTINUE;
}

int file_upload_callback (const struct _u_request * request, 
                          const char * key, 
                          const char * filename, 
                          const char * content_type, 
                          const char * transfer_encoding, 
                          const char * data, 
                          uint64_t off, 
                          size_t size, 
                          void * cls) {
  char fullfilename[2049];
  FILE * fp = NULL;
  size_t result;

  y_log_message(Y_LOG_LEVEL_DEBUG, "Got from file '%s' of the key '%s', offset %llu, size %zu, cls is '%s'", filename, key, off, size, cls);

  snprintf(fullfilename, 2049, "%s/%s", STORAGE_FOLDER, filename);
  y_log_message(Y_LOG_LEVEL_DEBUG, "fullfilename: %s", fullfilename);

  fp = fopen(fullfilename, "a+");

  if (fp == NULL) {
	  y_log_message(Y_LOG_LEVEL_DEBUG, "fopen(): fail");
	  return (1);
  }

  result = fwrite(data, size, 1, fp);

  if (result != size) {
	  y_log_message(Y_LOG_LEVEL_DEBUG, "fwrite(): fail dados gravados: %zu\n", result);
  }

  fclose(fp);
  return U_OK;
}
