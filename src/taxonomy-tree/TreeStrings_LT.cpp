#include "TreeStrings.hpp"




// Lithuanian translations for TaxonomyTree
const char* const TREE_STRINGS_LT[] = {
    // Window title
    "Taksonomijos Medis - Taksonominio šeimos medžio peržiūros programa",

    // Menu bar labels
    "&Failas",
    "&Vaizdas",
    "&Pagalba",

    // File Menu items
    "&Atsisiųsti taksonomijos duomenis\tCtrl+D",
    "Atsisiųsti taksonomijos duomenis iš iNaturalist",
    "Į&kelti archyvą\tCtrl+O",
    "Įkelti taksonomijos duomenų archyvą",
    "Eksportuoti į &SVG\tCtrl+S",
    "Eksportuoti medį į SVG formatą",
    "Eksportuoti į &PNG\tCtrl+P",
    "Eksportuoti medį į PNG formatą",
    "&Nustatymai",
    "Atidaryti nustatymų dialogą",
    "Iš&eiti\tCtrl+Q",
    "Uždaryti programą",

    // View Menu items
    "&Vėduoklės išdėstymas\tCtrl+1",
    "Rodyti medį vėduoklės išdėstymu",
    "V&ertikalus išdėstymas\tCtrl+2",
    "Rodyti medį vertikaliai",
    "&Horizontalus išdėstymas\tCtrl+3",
    "Rodyti medį horizontaliai",
    "Rodyti &rėmelius\tCtrl+B",
    "Perjungti rėmelių rodymą",
    "Rodyti &visą medį\tCtrl+F",
    "Rodyti visą medį iki karalystės",
    "Numatytasis &mastelis (100%)\tCtrl+0",
    "Atstatyti mastelį į 100%",
    "&Priartinti\tCtrl+=",
    "Priartinti",
    "Nut&olinti\tCtrl+-",
    "Nutolinti",

    // Help Menu
    "&Apie\tF1",
    "Apie Taksonomijos Medį",

    // Dialog titles
    "Klaida",
    "Sėkmė",
    "Atšaukti",
    "Išsaugoti taksonomijos archyvą",
    "Atidaryti taksonomijos archyvą",
    "Eksportuoti į SVG",
    "Eksportuoti į PNG",
    "Apie Taksonomijos Medį",
    "Atsisiunčiama",
    "Įkeliama",
    "Indeksuojama",
    "Uždaroma",
    "Nustatymai",
    "PNG eksportavimas",

    // File dialog filters
    "ZIP failai (*.zip)|*.zip",
    "Visi failai (*.*)|*.*",
    "SVG failai (*.svg)|*.svg",
    "PNG failai (*.png)|*.png",

    // Default file names
    "inaturalist-taxonomy.zip",
    "taksonomijos_medis.svg",
    "taksonomijos_medis.png",

    // Button labels
    "Išvalyti sąrašą",
    "Naršyti...",
    "Gerai",
    "Atšaukti",

    // Control labels
    "Pagrindinė kalba:",
    "Antrinė kalba:",
    "Ieškoti taksonų:",
    "Paieška (dukart spustelėkite palyginimui):",
    "Palyginimas(dukart spustelėkite pašalinimui):",

    // Settings dialog labels
    "Taksonomijos failas",
    "Failo kelias:",
    "Įkelti taksonomijos duomenis paleidžiant",

    // Status messages
    "Paruošta",
    "Įkelta %llu taksonų",
    "Rasta %llu rezultatų",

    // Special values
    "(Nėra)",

    // Download messages
    "Atsisiunčiami taksonomijos duomenys...",
    "Nepavyko pradėti atsisiuntimo",
    "Atsisiuntimas atšauktas",
    "Atsisiuntimas sėkmingai baigtas!\n\nDabar galite įkelti archyvą naudodami Failas -> Įkelti archyvą.",
    "Atsisiuntimas nepavyko: %s",

    // Loading messages
    "Įkeliami taksonomijos duomenys...",
    "Failas neegzistuoja:\n%s",
    "Nepavyko įkelti taksonomijos duomenų",

    // Indexing messages
    "Kuriamas paieškos indeksas %s...",
    " ir %s",

    // Export messages
    "Nepasirinkta taksonų palyginimui",
    "Medis sėkmingai eksportuotas į SVG!",
    "Nepavyko eksportuoti medžio į SVG",
    "Medis sėkmingai eksportuotas į PNG su %ld DPI!",
    "Nepavyko eksportuoti medžio į PNG",
    "Neteisinga DPI reikšmė. Įveskite skaičių nuo 70 iki 600.",

    // DPI input
    "Įveskite DPI (70-600):",
    "150",

    // Closing message
    "Valoma ir uždaroma...\nPrašome palaukti.",

    // About dialog (note: version will be prepended at runtime to first string)
    "\n\n",
    "Taksonominio šeimos medžio peržiūros programa\n\n",
    "Palyginkite rūšis ir vizualizuokite jų taksonominius ryšius.",

    // Settins dialog
    "Kalba",
    "Naudoti sistemos kalbos nustatymus",
    "Aptikta kalba: %s",
    "Programos kalba:",

    // Progress messages for archive loading
    "Atidaromas archyvas: %s",
    "Nepavyko atidaryti archyvo",
    "Archyve yra %llu failų",
    "Įkeliami taksonomijos duomenys...",
    "Nepavyko įkelti taksonomijos duomenų",
    "Kuriama taksonomijos hierarchija...",
    "Ieškoma kalbų failų...",
    "Rasta %llu kalbų failų",
    "Archyvas įkeltas! %llu taksonų, %llu kalbos",
    "Skaitomas taksonomijos CSV failas...",
    "Nepavyko perskaityti taksonomijos failo",
    "Failas perskaitytas, dydis: %llu baitai",
    "Nepavyko apdoroti taksonomijos CSV",
    "Apdorota %llu taksonomijos įrašų",
};

// Compile-time validation
VALIDATE_TRANSLATION_TABLE(TreeStringId, TREE_STRINGS_LT);
