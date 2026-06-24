# MikMod API

Librería para reproducción de música en formatos tracker (MOD, S3M, XM, IT, etc.).

**Repositorio:** https://github.com/sezero/mikmod

---

## Inicialización

```c
#include <mikmod.h>

// Registrar drivers y loaders
MikMod_RegisterAllDrivers();
MikMod_RegisterAllLoaders();

// Inicializar (string vacío = driver por defecto)
if (MikMod_Init("")) {
    printf("Error: %s\n", MikMod_strerror(MikMod_errno));
    return -1;
}

// Al terminar
MikMod_Exit();
```

---

## Cargar y Reproducir Módulos

### Desde Archivo

```c
MODULE *module = Player_Load("/dev_hdd0/game/APPID/USRDIR/music.mod", 64, 0);

if (module) {
    Player_Start(module);
    
    while (Player_Active()) {
        MikMod_Update();
        // ... tu game loop
    }
    
    Player_Stop();
    Player_Free(module);
}
```

### Desde Memoria (Assets Embebidos)

```c
extern u8 music_s3m[];
extern u32 music_s3m_size;

// Crear reader desde memoria
MREADER *reader = /* crear reader personalizado */;
MODULE *module = Player_LoadGeneric(reader, 64, 0);
```

---

## Control de Reproducción

```c
// Iniciar reproducción
Player_Start(module);

// Pausar/Reanudar
Player_TogglePause();

// Detener
Player_Stop();

// Verificar si está activo
if (Player_Active()) {
    // Está reproduciendo
}

// Verificar si está pausado
if (Player_Paused()) {
    // Está pausado
}
```

---

## Control de Volumen

```c
// Volumen global (0-128)
Player_SetVolume(100);

// Obtener volumen actual
int vol = Player_GetVolume();
```

---

## Posición en el Módulo

```c
// Saltar a posición específica
Player_SetPosition(5);

// Obtener posición actual
int pos = Player_GetPosition();

// Obtener número total de posiciones
int total = module->numpos;
```

---

## Configuración de Audio

```c
// Antes de MikMod_Init():

// Frecuencia de muestreo (por defecto 44100)
md_mixfreq = 44100;

// Modo (DMODE_STEREO, DMODE_16BITS, etc.)
md_mode = DMODE_STEREO | DMODE_16BITS | DMODE_SOFT_MUSIC;

// Canales de audio
md_sndfxvolume = 128;  // Volumen de efectos
md_musicvolume = 128;  // Volumen de música
```

---

## Formatos Soportados

| Formato | Extensión | Descripción |
|---------|-----------|-------------|
| MOD | .mod | ProTracker |
| S3M | .s3m | Scream Tracker 3 |
| XM | .xm | FastTracker 2 |
| IT | .it | Impulse Tracker |
| 669 | .669 | Composer 669 |
| AMF | .amf | ASYLUM Music Format |
| STM | .stm | Scream Tracker 2 |

---

## Ejemplo Completo

```c
#include <mikmod.h>

static MODULE *music = NULL;

int init_audio(void)
{
    MikMod_RegisterAllDrivers();
    MikMod_RegisterAllLoaders();
    
    md_mode = DMODE_STEREO | DMODE_16BITS | DMODE_SOFT_MUSIC;
    
    if (MikMod_Init("")) {
        return -1;
    }
    
    return 0;
}

int load_music(const char *path)
{
    music = Player_Load(path, 64, 0);
    return music ? 0 : -1;
}

void play_music(void)
{
    if (music) {
        Player_Start(music);
    }
}

void update_audio(void)
{
    MikMod_Update();
}

void stop_music(void)
{
    Player_Stop();
}

void cleanup_audio(void)
{
    if (music) {
        Player_Free(music);
        music = NULL;
    }
    MikMod_Exit();
}

// En tu game loop:
int main()
{
    init_audio();
    load_music("/dev_hdd0/game/APPID/USRDIR/bgm.s3m");
    play_music();
    
    while (running) {
        update_audio();
        // ... resto del loop
    }
    
    cleanup_audio();
    return 0;
}
```

---

## Embeber Música en el Ejecutable

En el Makefile, los archivos `.mod` y `.s3m` en la carpeta `data/` se convierten automáticamente en objetos:

```c
// Acceder al archivo embebido
extern u8 bgmusic_s3m[];
extern u32 bgmusic_s3m_size;
```

---

## Notas para PS3

- El driver PSL1GHT usa el sistema de audio nativo
- Llamar `MikMod_Update()` regularmente en el game loop
- Los formatos IT y XM consumen más CPU que MOD/S3M
- Para mejor rendimiento, usar frecuencias más bajas (22050 Hz)
