#include "DictStrings.hpp"

// Spanish translations for TaxonomyDict
const char* const DICT_STRINGS_ES[] = {
    // Window titles
    "Generador de Diccionario Taxonómico",

    // Menu items
    "&Archivo",
    "&Configuración",
    "Abrir diálogo de configuración",
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
    "Descargando... %d%%",
    "¡Descarga completada exitosamente! Use 'Cargar Archivo Existente' para procesarlo.",
    "Descarga fallida: %s",
    "Cargando archivo...",
    "Error al iniciar hilo de carga",
    "Error al cargar archivo",
    "Iniciando generación de diccionario...",
    "Generación de diccionario fallida",

    // Error messages
    "Por favor ingrese una URL para descargar",
    "La URL debe comenzar con http:// o https://",
    "Error al iniciar hilo de descarga (error: %d)",
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
    "Abriendo archivo: %s",
    "El archivo contiene %d archivos",
    "Archivo encontrado: %s",
    "Iniciando carga de datos taxonómicos...",
    "Buscando archivos de idiomas...",
    "Encontrados %d archivos de idiomas",
    "Procesando archivo de idioma: %s",
    "Idioma extraído: %s",
    "¡Archivo cargado exitosamente! Encontrados %d idiomas, %d entradas taxonómicas",
    "Error al abrir archivo",

    // Progress messages - Taxonomy loading
    "Intentando cargar datos taxonómicos...",
    "Error al leer 'taxa.csv', intentando 'taxonomy.csv'",
    "Error al leer 'taxonomy.csv', intentando 'taxa.txt'",
    "Error al leer 'taxa.txt'",
    "Archivo de taxonomía leído exitosamente, longitud de contenido: %d bytes",
    "Analizando datos CSV de taxonomía...",
    "Resultado del análisis: %s, entradas taxonómicas: %d",

    // Progress messages - Dictionary generation
    "Iniciando generación de diccionario...",
    "Generación de diccionario fallida - no se encontraron entradas",
    "Diccionario construido con %d entradas, creando directorio de salida...",
    "Directorio de salida listo, generando archivos...",
    "Generando archivo TSV...",
    "Generando archivos StarDict...",
    "¡Generación de diccionario completada exitosamente! Generadas %d entradas",
    "Cargando datos del idioma de origen: %s",
    "Error al cargar datos de nombres científicos",
    "Error al cargar datos del idioma de origen",
    "Cargadas %d entradas de origen, cargando idiomas de destino...",
    "Omitiendo Nombre Científico como idioma de destino (use 'Incluir Nombres Científicos' en su lugar)",
    "Cargando idioma de destino: %s",
    "Cargadas %d entradas para %s",
    "Construyendo mapeos de diccionario...",
    "Procesando entrada %d de %d...",
    "Construcción de diccionario completada, encontradas %d entradas",

    // Search UI
    "Buscar: %s",
};

// Compile-time validation
VALIDATE_TRANSLATION_TABLE(DictStringId, DICT_STRINGS_ES);
