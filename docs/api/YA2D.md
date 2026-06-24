# YA2D API

Yet Another 2D Library - Librería para gráficos 2D, manejo de texturas y sprites.

**Repositorio:** https://github.com/bucanero/ya2d_ps3

---

## Inicialización

```c
#include <ya2d/ya2d.h>

// Inicializar
ya2d_init();

// Al terminar
ya2d_deinit();
```

---

## Texturas

### Cargar Texturas

```c
// Desde archivo PNG
ya2d_Texture *tex = ya2d_loadPNGfromFile("/dev_hdd0/game/APPID/USRDIR/image.png");

// Desde archivo JPG
ya2d_Texture *tex = ya2d_loadJPGfromFile("/dev_hdd0/game/APPID/USRDIR/image.jpg");

// Desde buffer en memoria (para assets embebidos)
extern u8 image_png[];
extern u32 image_png_size;
ya2d_Texture *tex = ya2d_loadPNGfromBuffer(image_png, image_png_size);

// Liberar textura
ya2d_freeTexture(tex);
```

### Propiedades de Textura

```c
typedef struct {
    u32 width;          // Ancho en píxeles
    u32 height;         // Alto en píxeles
    u32 textureWidth;   // Ancho alineado (potencia de 2)
    u32 textureHeight;  // Alto alineado
    u32 *data;          // Puntero a datos
} ya2d_Texture;
```

---

## Dibujo de Texturas

### Básico

```c
// Dibujar textura en posición (x, y)
ya2d_drawTexture(texture, x, y);

// Dibujar con rotación (ángulo en grados)
ya2d_drawRotateTexture(texture, x, y, angle);

// Dibujar escalado
ya2d_drawScaleTexture(texture, x, y, scale_x, scale_y);
```

### Avanzado

```c
// Dibujar porción de textura (sprite sheet)
ya2d_drawTexturePartScale(texture, 
    x, y,                    // Posición destino
    src_x, src_y,            // Posición origen en textura
    src_w, src_h,            // Tamaño del recorte
    scale_x, scale_y);       // Escala

// Dibujar con color tint
ya2d_drawTextureTinted(texture, x, y, color);
```

---

## Controles de Entrada

```c
#include <ya2d/ya2d_controls.h>

// Leer estado del pad
ya2d_padRead();

// Verificar botones
if (ya2d_paddata[0].BTN_CROSS) {
    // Botón X presionado
}

if (ya2d_paddata[0].BTN_UP) {
    // D-Pad arriba
}

// Analógicos (0-255, centro = 128)
int lx = ya2d_paddata[0].ANA_L_H;  // Analógico izquierdo X
int ly = ya2d_paddata[0].ANA_L_V;  // Analógico izquierdo Y
```

### Botones Disponibles
- `BTN_CROSS`, `BTN_CIRCLE`, `BTN_SQUARE`, `BTN_TRIANGLE`
- `BTN_UP`, `BTN_DOWN`, `BTN_LEFT`, `BTN_RIGHT`
- `BTN_L1`, `BTN_L2`, `BTN_L3`
- `BTN_R1`, `BTN_R2`, `BTN_R3`
- `BTN_START`, `BTN_SELECT`

---

## Utilidades

### Pantalla

```c
// Obtener dimensiones de pantalla
int width = ya2d_screenWidth();
int height = ya2d_screenHeight();
```

### Frame Buffer

```c
// Iniciar frame
ya2d_screenClear();

// Finalizar frame
ya2d_screenFlip();
```

---

## Ejemplo Completo

```c
#include <ya2d/ya2d.h>

int main() {
    ya2d_init();
    
    // Cargar textura
    ya2d_Texture *logo = ya2d_loadPNGfromFile("/dev_hdd0/game/APPID/USRDIR/logo.png");
    
    while (running) {
        ya2d_padRead();
        
        if (ya2d_paddata[0].BTN_START) {
            running = 0;
        }
        
        ya2d_screenClear();
        
        // Dibujar logo centrado
        int x = (ya2d_screenWidth() - logo->width) / 2;
        int y = (ya2d_screenHeight() - logo->height) / 2;
        ya2d_drawTexture(logo, x, y);
        
        ya2d_screenFlip();
    }
    
    ya2d_freeTexture(logo);
    ya2d_deinit();
    
    return 0;
}
```

---

## Integración con Tiny3D

YA2D puede usarse junto con Tiny3D. Usa Tiny3D para el rendering de bajo nivel y YA2D para cargar/manejar texturas:

```c
tiny3d_Init(1024 * 1024);
ya2d_init();

ya2d_Texture *tex = ya2d_loadPNGfromFile("image.png");

// Usar la textura con Tiny3D...
tiny3d_SetTexture(0, offset, tex->width, tex->height, ...);
```

---

## Notas

- Las texturas se redimensionan internamente a potencias de 2
- Formatos soportados: PNG (con alpha), JPG
- Para sprites animados, usa sprite sheets y `ya2d_drawTexturePartScale()`
