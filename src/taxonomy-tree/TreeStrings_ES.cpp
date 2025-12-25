#include "TreeStrings.hpp"

// Spanish translations for TaxonomyTree
const char* const TREE_STRINGS_ES[] = {
    // Window title
    "ÁrbolTaxonómico - Visor de Árbol Genealógico Taxonómico",

    // Menu bar labels
    "&Archivo",
    "&Vista",
    "A&yuda",

    // File Menu items
    "&Descargar Datos Taxonómicos\tCtrl+D",
    "Descargar datos taxonómicos de iNaturalist",
    "&Cargar Archivo\tCtrl+O",
    "Cargar archivo de datos taxonómicos",
    "Exportar a &SVG\tCtrl+S",
    "Exportar árbol a formato SVG",
    "Exportar a &PNG\tCtrl+P",
    "Exportar árbol a formato PNG",
    "Con&figuración",
    "Abrir diálogo de configuración",
    "&Salir\tCtrl+Q",
    "Salir de la aplicación",

    // View Menu items
    "Diseño de &Abanico\tCtrl+1",
    "Mostrar árbol en diseño de abanico",
    "Diseño &Vertical\tCtrl+2",
    "Mostrar árbol verticalmente",
    "Diseño &Horizontal\tCtrl+3",
    "Mostrar árbol horizontalmente",
    "Mostrar &Cajas\tCtrl+B",
    "Alternar cajas de nodos",
    "Mostrar Árbol &Completo\tCtrl+F",
    "Mostrar árbol completo hasta reino",
    "Zoom Pre&determinado (100%)\tCtrl+0",
    "Restablecer zoom al 100%",
    "A&cercar\tCtrl+=",
    "Acercar",
    "Ale&jar\tCtrl+-",
    "Alejar",

    // Help Menu
    "&Acerca de\tF1",
    "Acerca de ÁrbolTaxonómico",

    // Dialog titles
    "Error",
    "Éxito",
    "Cancelado",
    "Guardar archivo taxonómico",
    "Abrir archivo taxonómico",
    "Exportar a SVG",
    "Exportar a PNG",
    "Acerca de ÁrbolTaxonómico",
    "Descargando",
    "Cargando",
    "Indexando",
    "Cerrando",
    "Configuración",
    "Exportación PNG",

    // File dialog filters
    "Archivos ZIP (*.zip)|*.zip",
    "Todos los archivos (*.*)|*.*",
    "Archivos SVG (*.svg)|*.svg",
    "Archivos PNG (*.png)|*.png",

    // Default file names
    "inaturalist-taxonomy.zip",
    "arbol_taxonomico.svg",
    "arbol_taxonomico.png",

    // Button labels
    "Limpiar Lista",
    "Examinar...",
    "Aceptar",
    "Cancelar",

    // Control labels
    "Idioma Primario:",
    "Idioma Secundario:",
    "Buscar Taxones:",
    "Resultados de Búsqueda (doble clic para agregar):",
    "Lista de Comparación (doble clic para eliminar):",

    // Settings dialog labels
    "Archivo Taxonómico",
    "Ruta del Archivo:",
    "Cargar datos taxonómicos al inicio",

    // Status messages
    "Listo",
    "Cargados %llu taxones",
    "Encontrados %llu resultados",

    // Special values
    "(Ninguno)",

    // Download messages
    "Descargando datos taxonómicos...",
    "Error al iniciar descarga",
    "Descarga cancelada",
    "¡Descarga completada exitosamente!\n\nAhora puede cargar el archivo usando Archivo -> Cargar Archivo.",
    "Descarga fallida: %s",

    // Loading messages
    "Cargando datos taxonómicos...",
    "El archivo no existe:\n%s",
    "Error al cargar datos taxonómicos",

    // Indexing messages
    "Construyendo índice de búsqueda para %s...",
    " y %s",

    // Export messages
    "No hay taxones seleccionados para comparación",
    "¡Árbol exportado a SVG exitosamente!",
    "Error al exportar árbol a SVG",
    "¡Árbol exportado a PNG con %ld DPI exitosamente!",
    "Error al exportar árbol a PNG",
    "Valor DPI inválido. Por favor ingrese un número entre 70 y 600.",

    // DPI input
    "Ingrese DPI (70-600):",
    "150",

    // Closing message
    "Limpiando y cerrando...\nPor favor espere.",

    // About dialog (note: version will be prepended at runtime to first string)
    "\n\n",
    "Visor de Árbol Genealógico Taxonómico\n\n",
    "Compare especies y visualice sus relaciones taxonómicas.",

    // Settins dialog
    "Idioma",
    "Usar configuración de idioma del sistema",
    "Idioma Detectado: %s",
    "Idioma de la Aplicación:",

    // Progress messages for archive loading
    "Abriendo archivo: %s",
    "Error al abrir archivo",
    "El archivo contiene %llu archivos",
    "Cargando datos taxonómicos...",
    "Error al cargar datos taxonómicos",
    "Construyendo jerarquía taxonómica...",
    "Escaneando archivos de idioma...",
    "Se encontraron %llu archivos de idioma",
    "Archivo cargado! %llu taxones, %llu idiomas",
    "Leyendo archivo CSV de taxonomía...",
    "Error al leer archivo de taxonomía",
    "Archivo leído, tamaño: %llu bytes",
    "Error al analizar CSV de taxonomía",
    "Se analizaron %llu entradas taxonómicas",

    // Cache-related progress messages
    "Creando esquema de base de datos...",
    "Se encontraron %llu idiomas disponibles",
    "Insertando %llu entradas taxonómicas...",
    "Cargando idioma: %s",
    "Construyendo índice de búsqueda de nombres vernáculos...",
    "Construyendo índice de búsqueda de nombres científicos...",
    "Índice de búsqueda completo",
    "Caché construida completamente!",
    "Actualizando índice de búsqueda...",

    // Cache validation messages
    "Caché Desactualizada",
    "El archivo taxonómico ha sido actualizado. ¿Reconstruir caché?\n(Esto puede tardar 15-20 segundos)",
    "Archivo taxonómico guardado no encontrado:\n%s\n\nSeleccione un nuevo archivo mediante Archivo > Cargar Archivo.",
};

// Compile-time validation
VALIDATE_TRANSLATION_TABLE(TreeStringId, TREE_STRINGS_ES);
