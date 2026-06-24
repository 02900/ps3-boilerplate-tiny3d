# Mini18n API

Librería minimalista para internacionalización (i18n) y localización.

**Repositorio:** https://github.com/bucanero/mini18n

---

## Conceptos Básicos

Mini18n permite traducir textos en tu aplicación usando archivos `.po` (formato gettext).

```c
#include <mini18n.h>

// La macro _() traduce un string
printf("%s\n", _("Hello World"));
```

---

## Inicialización

```c
// Cargar archivo de idioma
mini18n_set_locale("/dev_hdd0/game/APPID/USRDIR/LANG/es.po");

// O desde buffer en memoria
mini18n_set_locale_buffer(po_data, po_size);
```

---

## Traducir Textos

### Macro Principal

```c
// Uso básico
const char *text = _("Original text");

// En printf
printf(_("Welcome, %s!\n"), username);

// En asignaciones
char message[256];
snprintf(message, sizeof(message), _("Score: %d"), score);
```

### Función Alternativa

```c
// Equivalente a _()
const char *translated = mini18n_get("Original text");
```

---

## Formato de Archivos .po

Los archivos `.po` contienen pares de texto original y traducido:

```po
# Comentario
msgid "Hello World"
msgstr "Hola Mundo"

msgid "Exit"
msgstr "Salir"

msgid "Settings"
msgstr "Configuración"

# Con formato
msgid "Score: %d"
msgstr "Puntuación: %d"

# Texto multilínea
msgid "Welcome to the game!\nPress START to begin."
msgstr "¡Bienvenido al juego!\nPresiona START para comenzar."
```

---

## Estructura de Carpetas Recomendada

```
USRDIR/
└── LANG/
    ├── en.po    # Inglés
    ├── es.po    # Español
    ├── fr.po    # Francés
    ├── de.po    # Alemán
    └── pt.po    # Portugués
```

---

## Detectar Idioma del Sistema

```c
#include <sysutil/sysutil.h>

const char* get_system_language(void)
{
    u32 lang;
    sysUtilGetSystemParamInt(SYSUTIL_SYSTEMPARAM_ID_LANG, &lang);
    
    switch (lang) {
        case SYSUTIL_LANG_JAPANESE:    return "ja";
        case SYSUTIL_LANG_ENGLISH_US:  return "en";
        case SYSUTIL_LANG_FRENCH:      return "fr";
        case SYSUTIL_LANG_SPANISH:     return "es";
        case SYSUTIL_LANG_GERMAN:      return "de";
        case SYSUTIL_LANG_ITALIAN:     return "it";
        case SYSUTIL_LANG_PORTUGUESE_PT:
        case SYSUTIL_LANG_PORTUGUESE_BR: return "pt";
        default: return "en";
    }
}

void init_language(void)
{
    char path[256];
    const char *lang = get_system_language();
    
    snprintf(path, sizeof(path), 
             "/dev_hdd0/game/APPID/USRDIR/LANG/%s.po", lang);
    
    mini18n_set_locale(path);
}
```

---

## Ejemplo Completo

### main.c

```c
#include <stdio.h>
#include <mini18n.h>

int main()
{
    // Cargar español
    mini18n_set_locale("/dev_hdd0/game/APPID/USRDIR/LANG/es.po");
    
    // Estos textos se traducirán automáticamente
    printf("%s\n", _("Hello World"));
    printf("%s\n", _("Press X to continue"));
    printf(_("Score: %d\n"), 1000);
    
    return 0;
}
```

### LANG/es.po

```po
msgid "Hello World"
msgstr "Hola Mundo"

msgid "Press X to continue"
msgstr "Presiona X para continuar"

msgid "Score: %d"
msgstr "Puntuación: %d"
```

### Salida

```
Hola Mundo
Presiona X para continuar
Puntuación: 1000
```

---

## Crear Archivos .po

### Manualmente

Crea un archivo de texto con extensión `.po`:

```po
msgid "texto original"
msgstr "texto traducido"
```

### Con Herramientas

Puedes usar herramientas como:
- **Poedit** - Editor gráfico de archivos .po
- **xgettext** - Extrae strings del código fuente

---

## Fallback

Si un texto no tiene traducción, se devuelve el texto original:

```c
mini18n_set_locale("es.po");  // Solo tiene "Hello"

printf("%s\n", _("Hello"));   // Imprime traducción
printf("%s\n", _("Goodbye")); // Imprime "Goodbye" (sin traducir)
```

---

## Notas

- Los archivos `.po` deben estar en UTF-8
- La macro `_()` es un alias para `mini18n_get()`
- Para textos con formato, mantener los especificadores (`%s`, `%d`, etc.)
- El orden de los especificadores puede cambiar en diferentes idiomas
