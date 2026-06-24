# libcurl API

Librería para transferencias HTTP, HTTPS, FTP y más protocolos.

**Documentación oficial:** https://curl.se/libcurl/c/

---

## Inicialización

```c
#include <curl/curl.h>

// Inicializar libcurl (una vez al inicio)
curl_global_init(CURL_GLOBAL_DEFAULT);

// Al terminar la aplicación
curl_global_cleanup();
```

---

## Peticiones HTTP Básicas

### GET Request

```c
CURL *curl = curl_easy_init();

if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://example.com/api/data");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        printf("Error: %s\n", curl_easy_strerror(res));
    }
    
    curl_easy_cleanup(curl);
}
```

### Capturar Respuesta

```c
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    char *buffer = (char *)userp;
    strncat(buffer, contents, realsize);
    return realsize;
}

void fetch_data(void)
{
    CURL *curl = curl_easy_init();
    char response[4096] = {0};
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://api.example.com/data");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            printf("Response: %s\n", response);
        }
        
        curl_easy_cleanup(curl);
    }
}
```

---

## Descargar Archivos

```c
static size_t write_file_callback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}

int download_file(const char *url, const char *output_path)
{
    CURL *curl = curl_easy_init();
    FILE *fp = fopen(output_path, "wb");
    
    if (!curl || !fp) return -1;
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    CURLcode res = curl_easy_perform(curl);
    
    fclose(fp);
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK) ? 0 : -1;
}
```

---

## POST Request

```c
void post_data(void)
{
    CURL *curl = curl_easy_init();
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://api.example.com/submit");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=value&foo=bar");
        
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}
```

### POST con JSON

```c
void post_json(void)
{
    CURL *curl = curl_easy_init();
    struct curl_slist *headers = NULL;
    
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://api.example.com/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"key\":\"value\"}");
        
        curl_easy_perform(curl);
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}
```

---

## HTTPS con PolarSSL

libcurl en PS3 usa PolarSSL para conexiones seguras:

```c
curl_easy_setopt(curl, CURLOPT_URL, "https://secure.example.com");
curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);  // Deshabilitar verificación (desarrollo)
curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
```

---

## Progreso de Descarga

```c
static int progress_callback(void *clientp, double dltotal, double dlnow, 
                            double ultotal, double ulnow)
{
    if (dltotal > 0) {
        int percent = (int)((dlnow / dltotal) * 100);
        printf("\rDescargando: %d%%", percent);
    }
    return 0;
}

void download_with_progress(const char *url)
{
    CURL *curl = curl_easy_init();
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
    
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}
```

---

## Opciones Comunes

| Opción | Descripción |
|--------|-------------|
| `CURLOPT_URL` | URL destino |
| `CURLOPT_TIMEOUT` | Timeout en segundos |
| `CURLOPT_FOLLOWLOCATION` | Seguir redirecciones |
| `CURLOPT_USERAGENT` | User-Agent string |
| `CURLOPT_WRITEFUNCTION` | Callback para recibir datos |
| `CURLOPT_WRITEDATA` | Puntero para el callback |
| `CURLOPT_POSTFIELDS` | Datos para POST |
| `CURLOPT_HTTPHEADER` | Headers personalizados |

---

## Códigos de Error Comunes

| Código | Significado |
|--------|-------------|
| `CURLE_OK` | Éxito |
| `CURLE_COULDNT_CONNECT` | No se pudo conectar |
| `CURLE_OPERATION_TIMEDOUT` | Timeout |
| `CURLE_SSL_CONNECT_ERROR` | Error SSL |
| `CURLE_WRITE_ERROR` | Error de escritura |

---

## Notas para PS3

- Requiere inicializar el módulo de red antes de usar
- Las conexiones HTTPS usan PolarSSL
- La verificación de certificados puede fallar; usar `SSL_VERIFYPEER = 0` para desarrollo
- Soporta HTTP, HTTPS, FTP, FTPS
