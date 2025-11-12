#include "DictStrings.hpp"

// Spanish translations for TaxonomyDict
const char* const DICT_STRINGS_ES[] = {
    // Window titles
    "Generador de Diccionario Taxonómico",

    // Menu items
    "&Archivo",
    "&Salir\tCtrl+Q",
    "Salir de la aplicación",
    "A&yuda",
    "&Acerca de\tF1",
    "Acerca de esta aplicación",

    // Button labels
    "Descargar Archivo",
    "Cargar Archivo Existente...",
    "Generar Diccionario",
    "Examinar...",

    // Dialog titles
    "Acerca de",
    "Guardar archivo como...",
    "Elegir archivo",
    "Error",
    "URL Inválida",
    "Error de Hilo",

    // Section titles
    "Descarga de Archivo",
    "Selección de Idiomas",
    "Salida",

    // Control labels
    "URL de Taxonomía iNaturalist:",
    "Idioma de Origen:",
    "Idiomas de Destino:",
    "Ruta de Salida:",

    // Checkbox labels
    "Incluir Nombres Científicos",
    "Filtrar solo idiomas de destino",

    // Static values
    "Nombre Científico",
    "https://www.inaturalist.org/taxa/inaturalist-taxonomy.dwca.zip",
    "./output",

    // Status messages
    "Listo",
    "Descargando...",
    "Hilo de descarga iniciado...",
    "Descargando... %1$d%%",
    "¡Descarga completada exitosamente! Use 'Cargar Archivo Existente' para procesarlo.",
    "Descarga fallida: %1$s",
    "Cargando archivo...",
    "Error al iniciar hilo de carga",
    "Error al cargar archivo",
    "Iniciando generación de diccionario...",
    "Generación de diccionario fallida",

    // Error messages
    "Por favor ingrese una URL para descargar",
    "La URL debe comenzar con http:// o https://",
    "Error al iniciar hilo de descarga (error: %1$d)",
    "Error al iniciar hilo de carga",
    "Por favor seleccione un idioma de origen",
    "Por favor seleccione al menos un idioma de destino o incluya nombres científicos",
    "Error al iniciar hilo de generación de diccionario",
    "Error al cargar archivo",
    "Generación de diccionario fallida",
    "Error al inicializar CURL",

    // About dialog message (note: version will be prepended at runtime)
    "\n\nGenera diccionarios de traducción a partir de datos taxonómicos de iNaturalist.",

    // File dialog filters
    "Archivos ZIP (*.zip)|*.zip",

    // Progress messages - Archive operations
    "Abriendo archivo: %1$s",
    "El archivo contiene %1$d archivos",
    "Archivo encontrado: %1$s",
    "Iniciando carga de datos taxonómicos...",
    "Buscando archivos de idiomas...",
    "Encontrados %1$d archivos de idiomas",
    "Procesando archivo de idioma: %1$s",
    "Idioma extraído: %1$s",
    "¡Archivo cargado exitosamente! Encontrados %1$d idiomas, %2$d entradas taxonómicas",
    "Error al abrir archivo",

    // Progress messages - Taxonomy loading
    "Intentando cargar datos taxonómicos...",
    "Error al leer 'taxa.csv', intentando 'taxonomy.csv'",
    "Error al leer 'taxonomy.csv', intentando 'taxa.txt'",
    "Error al leer 'taxa.txt'",
    "Archivo de taxonomía leído exitosamente, longitud de contenido: %1$d bytes",
    "Analizando datos CSV de taxonomía...",
    "Resultado del análisis: %1$s, entradas taxonómicas: %2$d",

    // Progress messages - Dictionary generation
    "Iniciando generación de diccionario...",
    "Generación de diccionario fallida - no se encontraron entradas",
    "Diccionario construido con %1$d entradas, creando directorio de salida...",
    "Directorio de salida listo, generando archivos...",
    "Generando archivo TSV...",
    "Generando archivos StarDict...",
    "¡Generación de diccionario completada exitosamente! Generadas %1$d entradas",
    "Cargando datos del idioma de origen: %1$s",
    "Error al cargar datos de nombres científicos",
    "Error al cargar datos del idioma de origen",
    "Cargadas %1$d entradas de origen, cargando idiomas de destino...",
    "Omitiendo Nombre Científico como idioma de destino (use 'Incluir Nombres Científicos' en su lugar)",
    "Cargando idioma de destino: %1$s",
    "Cargadas %1$d entradas para %2$s",
    "Construyendo mapeos de diccionario...",
    "Procesando entrada %1$d de %2$d...",
    "Construcción de diccionario completada, encontradas %1$d entradas",

    // Search UI
    "Buscar: %1$s",
};

// Compile-time validation
VALIDATE_TRANSLATION_TABLE(DictStringId, DICT_STRINGS_ES);
