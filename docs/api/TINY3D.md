# Tiny3D API

Librería de gráficos 3D para PS3. Proporciona acceso simplificado al RSX (GPU).

**Repositorio:** https://github.com/wargio/Tiny3D

---

## Inicialización

```c
#include <tiny3d.h>

// Inicializar con 1MB de memoria para texturas
tiny3d_Init(1024 * 1024);
```

---

## Funciones Principales

### Control de Frame

```c
// Limpiar pantalla (color ARGB, flags)
tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);

// Intercambiar buffers (mostrar frame)
tiny3d_Flip();
```

### Modos de Proyección

```c
// Modo 2D (coordenadas de pantalla)
tiny3d_Project2D();

// Modo 3D con perspectiva
tiny3d_Project3D();

// Proyección personalizada
tiny3d_SetProjectionMatrix(&matrix);
```

### Dibujo de Primitivas

```c
// Iniciar primitiva (QUADS, TRIANGLES, LINES, etc.)
tiny3d_SetPolygon(TINY3D_QUADS);

// Definir vértice con posición
tiny3d_VertexPos(x, y, z);

// Definir color del vértice (ARGB)
tiny3d_VertexColor(0xff00ff00);  // Verde

// Coordenadas de textura
tiny3d_VertexTexture(u, v);

// Finalizar primitiva
tiny3d_End();
```

### Blending y Alpha

```c
// Habilitar alpha test
tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);

// Configurar blending
tiny3d_BlendFunc(1, 
    TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
    TINY3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_DST_ALPHA_ZERO,
    TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
```

### Texturas

```c
// Cargar textura desde memoria
u32 *texture_pointer;
u32 texture_offset;

texture_pointer = tiny3d_AllocTexture(width * height * 4);
texture_offset = tiny3d_TextureOffset(texture_pointer);

// Configurar textura activa
tiny3d_SetTexture(0, texture_offset, width, height, 
                  width * 4, TINY3D_TEX_FORMAT_A8R8G8B8, TEXTWRAP_CLAMP);

// Habilitar/deshabilitar texturizado
tiny3d_SetTextureWrap(0, texture_offset, width, height, 
                      width * 4, TINY3D_TEX_FORMAT_A8R8G8B8, 
                      TEXTWRAP_CLAMP, TEXTWRAP_CLAMP, TEXTURE_LINEAR);
```

---

## Constantes Útiles

### Tipos de Primitivas
- `TINY3D_POINTS`
- `TINY3D_LINES`
- `TINY3D_LINE_STRIP`
- `TINY3D_TRIANGLES`
- `TINY3D_TRIANGLE_STRIP`
- `TINY3D_QUADS`

### Formatos de Textura
- `TINY3D_TEX_FORMAT_A8R8G8B8`
- `TINY3D_TEX_FORMAT_R5G6B5`
- `TINY3D_TEX_FORMAT_A1R5G5B5`
- `TINY3D_TEX_FORMAT_A4R4G4B4`

### Flags de Clear
- `TINY3D_CLEAR_ALL`
- `TINY3D_CLEAR_COLOR`
- `TINY3D_CLEAR_ZBUFFER`

---

## Ejemplo Completo

```c
#include <tiny3d.h>

int main() {
    tiny3d_Init(1024 * 1024);
    
    while (running) {
        tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
        tiny3d_Project2D();
        
        // Dibujar cuadrado rojo
        tiny3d_SetPolygon(TINY3D_QUADS);
        tiny3d_VertexPos(100, 100, 0);
        tiny3d_VertexColor(0xffff0000);
        tiny3d_VertexPos(200, 100, 0);
        tiny3d_VertexColor(0xffff0000);
        tiny3d_VertexPos(200, 200, 0);
        tiny3d_VertexColor(0xffff0000);
        tiny3d_VertexPos(100, 200, 0);
        tiny3d_VertexColor(0xffff0000);
        tiny3d_End();
        
        tiny3d_Flip();
    }
    
    return 0;
}
```

---

## Notas

- Las coordenadas en modo 2D son en píxeles (0,0 = esquina superior izquierda)
- Los colores usan formato ARGB (Alpha, Red, Green, Blue)
- Siempre llamar `tiny3d_Flip()` al final de cada frame
